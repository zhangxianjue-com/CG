#include "engine.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>

#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <thread>

#pragma comment(lib, "winhttp.lib")

namespace cgpublish {

namespace fs = std::filesystem;

// ---------- 编码 ----------

std::string ToUtf8(const std::wstring& w) {
  if (w.empty()) return {};
  int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0,
                              nullptr, nullptr);
  std::string s(n, '\0');
  WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), s.data(), n, nullptr,
                      nullptr);
  return s;
}

std::wstring ToWide(const std::string& s) {
  if (s.empty()) return {};
  int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
  std::wstring w(n, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
  return w;
}

// ---------- 路径 ----------

std::string ExeDir() {
  wchar_t buf[MAX_PATH] = {};
  GetModuleFileNameW(nullptr, buf, MAX_PATH);
  std::wstring p(buf);
  size_t i = p.find_last_of(L"\\/");
  return i == std::wstring::npos ? std::string(".") : ToUtf8(p.substr(0, i));
}

// C++20 里 path::u8string() 返回 std::u8string（char8_t），这里统一转成 UTF-8 的 std::string
std::string PathToUtf8(const fs::path& p) { return ToUtf8(p.wstring()); }

std::string FindRepoRoot(const std::string& start) {
  fs::path d = start.empty() ? fs::current_path() : fs::u8path(start);
  if (!d.is_absolute()) d = fs::absolute(d);
  while (!d.empty()) {
    if (fs::exists(d / ".git")) return PathToUtf8(d);
    fs::path parent = d.parent_path();
    if (parent == d) break;
    d = parent;
  }
  return {};
}

std::string DocsDir(const std::string& repoRoot) {
  return PathToUtf8(fs::u8path(repoRoot) / "docs");
}

std::string SiteIndex(const std::string& repoRoot) {
  return PathToUtf8(fs::u8path(DocsDir(repoRoot)) / "index.html");
}

// ---------- 命令执行 ----------

namespace {

void TrimCr(std::string& s) {
  if (!s.empty() && s.back() == '\r') s.pop_back();
}

// 读取管道并按行回调；stderr 行标记为错误
void PumpPipe(HANDLE h, bool isErr, const LogFn& log) {
  char buf[4096];
  std::string pending;
  DWORD read = 0;
  while (ReadFile(h, buf, sizeof(buf), &read, nullptr) && read > 0) {
    pending.append(buf, read);
    size_t pos = 0;
    while (true) {
      size_t nl = pending.find('\n', pos);
      if (nl == std::string::npos) break;
      std::string line = pending.substr(pos, nl - pos);
      TrimCr(line);
      if (log) log(line, isErr);
      pos = nl + 1;
    }
    if (pos > 0) pending.erase(0, pos);
  }
  if (!pending.empty()) {
    TrimCr(pending);
    if (log) log(pending, isErr);
  }
}

// 通用执行：cmdLine 为完整命令行（已含引号转义），stdout/stderr 分管道逐行回调。
bool ExecImpl(const std::wstring& cmdLine, const std::string& workDir,
              const LogFn& log, std::string* captureOut) {
  SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
  HANDLE outR = nullptr, outW = nullptr, errR = nullptr, errW = nullptr;
  if (!CreatePipe(&outR, &outW, &sa, 0)) return false;
  if (!captureOut && !CreatePipe(&errR, &errW, &sa, 0)) {
    CloseHandle(outR);
    CloseHandle(outW);
    return false;
  }
  SetHandleInformation(outR, HANDLE_FLAG_INHERIT, 0);
  if (errR) SetHandleInformation(errR, HANDLE_FLAG_INHERIT, 0);

  // stdin 指向 NUL，避免子进程读键盘
  HANDLE nulIn = CreateFileW(L"NUL", GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                             OPEN_EXISTING, 0, nullptr);

  STARTUPINFOW si{};
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESTDHANDLES;
  si.hStdOutput = outW;
  si.hStdError = captureOut ? outW : errW;  // capture 模式合并到 stdout
  si.hStdInput = nulIn ? nulIn : GetStdHandle(STD_INPUT_HANDLE);

  PROCESS_INFORMATION pi{};
  std::wstring workDirW = ToWide(workDir);
  std::wstring mutableCmd = cmdLine;  // CreateProcessW 可能修改命令行缓冲区
  BOOL ok = CreateProcessW(nullptr, mutableCmd.data(), nullptr, nullptr, TRUE,
                           CREATE_NO_WINDOW, nullptr, workDirW.c_str(), &si, &pi);
  if (nulIn) CloseHandle(nulIn);
  CloseHandle(outW);
  if (errW) CloseHandle(errW);
  if (!ok) {
    CloseHandle(outR);
    if (errR) CloseHandle(errR);
    return false;
  }

  std::string captured;
  std::thread errThread;
  std::thread outThread([&] {
    if (captureOut) {
      char buf[4096];
      DWORD read = 0;
      while (ReadFile(outR, buf, sizeof(buf), &read, nullptr) && read > 0)
        captured.append(buf, read);
    } else {
      PumpPipe(outR, false, log);
    }
  });
  if (errR) errThread = std::thread([&] { PumpPipe(errR, true, log); });

  WaitForSingleObject(pi.hProcess, INFINITE);
  outThread.join();
  if (errThread.joinable()) errThread.join();

  DWORD exitCode = 0;
  GetExitCodeProcess(pi.hProcess, &exitCode);
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);
  CloseHandle(outR);
  if (errR) CloseHandle(errR);

  if (captureOut) *captureOut = std::move(captured);
  return exitCode == 0;
}

// cmd.exe 包装执行（chcp 65001 保证子进程输出 UTF-8）
bool RunCmdImpl(const std::string& workDir, const std::string& cmdline,
                const LogFn& log, std::string* captureOut) {
  std::wstring cmd = L"cmd.exe /d /s /c \"chcp 65001 >nul && " +
                     ToWide(cmdline) + L"\"";
  return ExecImpl(cmd, workDir, log, captureOut);
}

}  // namespace

bool RunCmd(const std::string& workDir, const std::string& cmdline,
            const LogFn& log) {
  return RunCmdImpl(workDir, cmdline, log, nullptr);
}

std::string CaptureCmd(const std::string& workDir, const std::string& cmdline) {
  std::string out;
  RunCmdImpl(workDir, cmdline, nullptr, &out);
  // 去掉首尾空白
  size_t a = out.find_first_not_of(" \t\r\n");
  if (a == std::string::npos) return {};
  size_t b = out.find_last_not_of(" \t\r\n");
  return out.substr(a, b - a + 1);
}

// ---------- Git / 站点地址 ----------

std::string GetRemoteUrl(const std::string& repoRoot) {
  return CaptureCmd(repoRoot, "git remote get-url origin");
}

std::optional<std::pair<std::string, std::string>> ParseOwnerRepo(
    const std::string& remoteUrl) {
  std::string u = remoteUrl;
  // trim
  size_t a = u.find_first_not_of(" \t\r\n");
  if (a == std::string::npos) return std::nullopt;
  u = u.substr(a);
  size_t b = u.find_last_not_of(" \t\r\n");
  u = u.substr(0, b + 1);

  if (u.rfind("git@", 0) == 0) {
    u = u.substr(u.find(':') + 1);
  } else if (u.rfind("ssh://", 0) == 0) {
    u = u.substr(u.find('@') + 1);
    while (!u.empty() && u.front() == '/') u.erase(u.begin());
    size_t i = u.find('/');
    if (i == std::string::npos) return std::nullopt;
    u = u.substr(i + 1);
  } else {
    size_t i = u.find("://");
    if (i == std::string::npos) return std::nullopt;
    u = u.substr(i + 3);
    i = u.find('/');
    if (i == std::string::npos) return std::nullopt;
    u = u.substr(i + 1);
  }
  if (u.size() > 4 && u.compare(u.size() - 4, 4, ".git") == 0)
    u = u.substr(0, u.size() - 4);

  // 拆成恰好两段 owner/repo
  size_t slash = u.find('/');
  if (slash == std::string::npos) return std::nullopt;
  std::string owner = u.substr(0, slash);
  std::string repo = u.substr(slash + 1);
  if (owner.empty() || repo.empty() || repo.find('/') != std::string::npos)
    return std::nullopt;
  if (owner.find(':') != std::string::npos ||
      owner.find('\\') != std::string::npos)
    return std::nullopt;
  return std::make_pair(owner, repo);
}

std::string SiteUrl(const std::string& owner, const std::string& repo) {
  if (repo == owner + ".github.io")
    return "https://" + owner + ".github.io/";
  return "https://" + owner + ".github.io/" + repo + "/";
}

// ---------- 构建 ----------

bool BuildAsync(const std::string& repoRoot, const LogFn& log) {
  std::string docs = DocsDir(repoRoot);
  if (!fs::is_directory(fs::u8path(docs))) {
    if (log) log("找不到 docs 目录：" + docs, true);
    return false;
  }
  if (log) log("→ 检查 Node.js", false);
  if (!RunCmd(docs, "node --version", log)) {
    if (log) log("未找到 Node.js，请先安装（https://nodejs.org）并加入 PATH", true);
    return false;
  }
  if (!fs::exists(fs::u8path(docs) / "node_modules")) {
    if (log) log("→ 首次构建：安装依赖 npm install", false);
    if (!RunCmd(docs, "npm install", log)) {
      if (log) log("依赖安装失败，请检查网络后重试", true);
      return false;
    }
  }
  if (log) log("→ 构建 VitePress 站点（npm run docs:build，产物铺到 docs/ 根）", false);
  if (!RunCmd(docs, "npm run docs:build", log)) {
    if (log) log("站点构建失败", true);
    return false;
  }
  if (log) log("✓ 构建完成：" + docs, false);
  return true;
}

// ---------- 发布 ----------

bool PublishAsync(const std::string& repoRoot, const LogFn& log) {
  if (!fs::exists(fs::u8path(SiteIndex(repoRoot)))) {
    if (log) log("构建产物不存在（docs/index.html），请先构建站点", true);
    return false;
  }
  // 保证 docs/.nojekyll 存在：否则 GitHub Pages 用 Jekyll 构建并排除点开头目录
  std::string nojekyll = PathToUtf8(fs::u8path(DocsDir(repoRoot)) / ".nojekyll");
  if (!fs::exists(fs::u8path(nojekyll))) {
    {
      FILE* f = nullptr;
      if (fopen_s(&f, nojekyll.c_str(), "wb") == 0 && f) fclose(f);
    }
    if (log) log("→ 已创建 docs/.nojekyll（跳过 Jekyll，确保 .vitepress 目录可访问）", false);
  }
  if (log) log("→ 暂存站点产物（git add docs + 强制加入被忽略的生成文件）", false);
  if (!RunCmd(repoRoot,
              "git add docs && git add -f docs/index.html docs/404.html "
              "docs/assets docs/diagrams docs/hashmap.json docs/notes",
              log))
    return false;

  std::string hasChanges =
      CaptureCmd(repoRoot, "git diff --cached --quiet && echo YES || echo NO");
  if (hasChanges == "YES") {
    if (log) log("站点内容无变化，跳过提交", false);
    return true;
  }

  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
  localtime_s(&tm, &t);
  char stamp[32];
  std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M", &tm);
  std::string msg = std::string("deploy: CG 笔记站点 ") + stamp;

  if (log) log("→ 提交站点产物（身份：CGPublish）", false);
  if (!RunCmd(repoRoot,
              "git -c user.name=CGPublish -c user.email=publish@cg.local "
              "commit -m \"" + msg +
                  "\" -m \"Co-Authored-By: Claude Code <noreply@anthropic.com>\"",
              log)) {
    if (log) log("提交失败", true);
    return false;
  }
  if (log) log("→ 推送 main 分支（触发 GitHub Pages 重新部署）", false);
  if (!RunCmd(repoRoot, "git push origin main", log)) {
    if (log) log("推送失败：远端可能有新提交（本地历史落后，需先手动 git pull --rebase 处理），"
                 "或 SSH 密钥（~/.ssh）无该仓库写权限",
                 true);
    return false;
  }
  if (log) log("✓ 已推送，GitHub Pages 将自动重新部署 docs/ 文件夹", false);
  return true;
}

// ---------- 验证 ----------

std::string VerifyAsync(const std::string& owner, const std::string& repo) {
  std::string url = SiteUrl(owner, repo);
  std::string host = owner + ".github.io";
  std::string path = (repo == owner + ".github.io") ? "/" : "/" + repo + "/";

  HINTERNET hSession =
      WinHttpOpen(L"yushu-project-management/2.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                  WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession) return "访问失败（WinHttpOpen）：" + url;

  HINTERNET hConn = WinHttpConnect(hSession, ToWide(host).c_str(),
                                   INTERNET_DEFAULT_HTTPS_PORT, 0);
  if (!hConn) {
    WinHttpCloseHandle(hSession);
    return "访问失败（WinHttpConnect）：" + url;
  }
  HINTERNET hReq =
      WinHttpOpenRequest(hConn, L"GET", ToWide(path).c_str(), nullptr,
                         WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                         WINHTTP_FLAG_SECURE);
  std::string result;
  if (!hReq) {
    result = "访问失败（WinHttpOpenRequest）：" + url;
  } else {
    BOOL ok = WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                 WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    if (ok) ok = WinHttpReceiveResponse(hReq, nullptr);
    DWORD status = 0;
    DWORD size = sizeof(status);
    if (ok)
      WinHttpQueryHeaders(hReq,
                          WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                          WINHTTP_HEADER_NAME_BY_INDEX, &status, &size,
                          WINHTTP_NO_HEADER_INDEX);
    if (status == 200)
      result = "✓ 站点可访问：" + url;
    else if (status != 0)
      result = "站点暂时返回 HTTP " + std::to_string(status) +
               "（Pages 部署中，约 1 分钟内生效）：" + url;
    else
      result = "访问失败（网络错误）：" + url;
    WinHttpCloseHandle(hReq);
  }
  WinHttpCloseHandle(hConn);
  WinHttpCloseHandle(hSession);
  return result;
}

}  // namespace cgpublish
