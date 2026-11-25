#include "engine.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>

#include <chrono>
#include <cstdio>
#include <cstring>
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

// 盘符大写化：小写盘符 cwd（如 e:\...）下 VitePress 构建会报 imports TypeError，
// 统一成 E:\ 再交给 npm/cmake，规避这个坑。
std::string NormalizeDrive(std::string p) {
  if (p.size() >= 2 && p[1] == ':' && p[0] >= 'a' && p[0] <= 'z')
    p[0] = (char)(p[0] - 'a' + 'A');
  return p;
}

std::string FindRepoRoot(const std::string& start) {
  fs::path d = start.empty() ? fs::current_path() : fs::u8path(start);
  if (!d.is_absolute()) d = fs::absolute(d);
  while (!d.empty()) {
    if (fs::exists(d / ".git")) return NormalizeDrive(PathToUtf8(d));
    fs::path parent = d.parent_path();
    if (parent == d) break;
    d = parent;
  }
  return {};
}

std::string DocsDir(const std::string& repoRoot) {
  return PathToUtf8(fs::u8path(repoRoot) / "docs");
}

std::string DistDir(const std::string& repoRoot) {
  return PathToUtf8(fs::u8path(DocsDir(repoRoot)) / ".vitepress" / "dist");
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
  return (int)exitCode;
}

// cmd.exe 包装执行（chcp 65001 保证子进程输出 UTF-8），返回子进程退出码
int RunCmdImpl(const std::string& workDir, const std::string& cmdline,
               const LogFn& log, std::string* captureOut) {
  std::wstring cmd = L"cmd.exe /d /s /c \"chcp 65001 >nul && " +
                     ToWide(cmdline) + L"\"";
  return ExecImpl(cmd, workDir, log, captureOut);
}

}  // namespace

bool RunCmd(const std::string& workDir, const std::string& cmdline,
            const LogFn& log) {
  return RunCmdImpl(workDir, cmdline, log, nullptr) == 0;
}

int RunCmdCode(const std::string& workDir, const std::string& cmdline,
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
  std::string root = NormalizeDrive(repoRoot);
  std::string docs = DocsDir(root);
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
  if (log) log("→ 构建 VitePress 站点（npm run docs:build，产物在 .vitepress/dist）", false);
  if (!RunCmd(docs, "npm run docs:build", log)) {
    if (log) log("站点构建失败", true);
    return false;
  }
  if (log) log("✓ 构建完成：" + DistDir(root), false);
  return true;
}

// ---------- 发布 ----------

// 发布脚本（写盘到 %TEMP% 后执行；纯 ASCII，中文提交信息经 %~1 参数传入）。
// 关键点：Pages 部署 gh-pages 分支的【根目录】，所以 gh-pages 上必须只有站点文件本身，
// 不能把整个仓库（dist 嵌套在 docs/.vitepress/dist/ 里）推上去——根下没有 index.html，
// Pages 构建会失败并一直保留上一次旧部署。
// 做法：用独立临时 index（GIT_INDEX_FILE）暂存 dist，write-tree --prefix 把
// docs/.vitepress/dist 子树写成根目录树，commit-tree 生成孤儿提交（CGPublish 身份），
// 最后 push <sha>:refs/heads/gh-pages --force。全程不碰用户的 index/HEAD，main 无临时提交。
// 结果写进 %TEMP%\cg-ghp-result.txt（不依赖 exit code：cmd 经 call/&& 复合命令调用
// 时 exit /b 的退出码传播不可靠）。内容：OK 推送成功 / SKIP 内容无变化 / FAIL:步骤 失败。
constexpr const char* kPublishBat = R"bat(@echo off
setlocal
cd /d "%~2"
set "RESULT=%TEMP%\cg-ghp-result.txt"
del /q "%RESULT%" 2>nul
set "GIT_AUTHOR_NAME=CGPublish"
set "GIT_AUTHOR_EMAIL=publish@cg.local"
set "GIT_COMMITTER_NAME=CGPublish"
set "GIT_COMMITTER_EMAIL=publish@cg.local"
set "GIT_INDEX_FILE=%TEMP%\cg-ghp-index"
del /q "%GIT_INDEX_FILE%" 2>nul
git read-tree --empty || (echo FAIL:read-tree>"%RESULT%" & exit /b 1)
rem core.autocrlf=false：dist 文件是 LF 行尾，仓库 autocrlf=true 时 git add 会逐文件刷
rem "LF will be replaced by CRLF" 警告（几十条刷屏）。存储内容本就不做转换，关掉无影响。
git -c core.autocrlf=false add -f docs/.vitepress/dist || (echo FAIL:add>"%RESULT%" & exit /b 1)
for /f "delims=" %%T in ('git write-tree --prefix^=docs/.vitepress/dist/') do set "TREE=%%T"
if not defined TREE (echo FAIL:write-tree>"%RESULT%" & exit /b 1)
set "REMOTE_TREE="
git fetch origin gh-pages --quiet 2>nul
if not errorlevel 1 for /f "delims=" %%R in ('git log -1 --format^=%%T FETCH_HEAD 2^>nul') do set "REMOTE_TREE=%%R"
if "%TREE%"=="%REMOTE_TREE%" (echo SKIP>"%RESULT%" & exit /b 3)
for /f "delims=" %%C in ('git commit-tree %TREE% -m "%~1" -m "Co-Authored-By: Claude Code ^<noreply@anthropic.com^>"') do set "COMMIT=%%C"
if not defined COMMIT (echo FAIL:commit-tree>"%RESULT%" & exit /b 1)
git push origin %COMMIT%:refs/heads/gh-pages --force || (echo FAIL:push>"%RESULT%" & exit /b 1)
del /q "%GIT_INDEX_FILE%" 2>nul
echo OK>"%RESULT%"
exit /b 0
)bat";

// 把 bat 内容转成 CRLF 行尾（LF-only 的 bat 在 cmd 下有解析隐患）
std::string ToCrlf(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 16);
  for (char ch : s) {
    if (ch == '\n') out += '\r';
    out += ch;
  }
  return out;
}

bool PublishAsync(const std::string& repoRoot, const LogFn& log) {
  std::string root = NormalizeDrive(repoRoot);
  std::string dist = DistDir(root);
  if (!fs::exists(fs::u8path(dist) / "index.html")) {
    if (log) log("构建产物不存在（.vitepress/dist/index.html），请先构建站点", true);
    return false;
  }
  // 保证 dist/.nojekyll 存在：gh-pages 部署时跳过 Jekyll，否则点开头目录被排除
  std::string nojekyll = PathToUtf8(fs::u8path(dist) / ".nojekyll");
  if (!fs::exists(fs::u8path(nojekyll))) {
    {
      FILE* f = nullptr;
      if (fopen_s(&f, nojekyll.c_str(), "wb") == 0 && f) fclose(f);
    }
    if (log) log("→ 已创建 dist/.nojekyll（跳过 Jekyll，确保 .vitepress 目录可访问）", false);
  }

  // 提交信息时间戳
  auto now = std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
  localtime_s(&tm, &t);
  char stamp[32];
  std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M", &tm);
  std::string msg = std::string("deploy: CG 笔记站点 ") + stamp;

  // 写发布脚本到 %TEMP%（CRLF 行尾）
  wchar_t tmpDirW[MAX_PATH];
  GetTempPathW(MAX_PATH, tmpDirW);
  fs::path bat = fs::path(tmpDirW) / "cg-ghp-publish.bat";
  {
    FILE* f = nullptr;
    if (fopen_s(&f, PathToUtf8(bat).c_str(), "wb") != 0 || !f) {
      if (log) log("无法写入临时发布脚本：" + PathToUtf8(bat), true);
      return false;
    }
    std::string batCrlf = ToCrlf(kPublishBat);
    fwrite(batCrlf.data(), 1, batCrlf.size(), f);
    fclose(f);
  }

  if (log) log("→ 生成站点快照并推送到 gh-pages（站点文件位于分支根目录）", false);
  fs::path result = fs::path(tmpDirW) / "cg-ghp-result.txt";
  std::error_code ec;
  fs::remove(result, ec);
  RunCmdCode(
      root,
      "call \"" + PathToUtf8(bat) + "\" \"" + msg + "\" \"" + root + "\"", log);
  fs::remove(bat, ec);

  // 读结果文件判定（不依赖 exit code：cmd 经 call/&& 复合命令调用时
  // exit /b 的退出码传播不可靠）
  std::string status;
  {
    FILE* f = nullptr;
    if (fopen_s(&f, PathToUtf8(result).c_str(), "rb") == 0 && f) {
      char buf[256];
      size_t n = fread(buf, 1, sizeof(buf) - 1, f);
      fclose(f);
      buf[n] = '\0';
      status = buf;
    }
  }
  fs::remove(result, ec);
  size_t a = status.find_first_not_of(" \t\r\n");
  size_t b = status.find_last_not_of(" \t\r\n");
  status = (a == std::string::npos) ? std::string()
                                    : status.substr(a, b - a + 1);
  if (status == "OK") {
    if (log) log("✓ 已推送到 gh-pages 分支，GitHub Pages 将自动重新部署", false);
    return true;
  }
  if (status == "SKIP") {
    if (log) log("站点内容无变化，跳过推送", false);
    return true;
  }
  if (status.rfind("FAIL:", 0) == 0) {
    if (log) log("推送失败（" + status.substr(5) + "）", true);
    return false;
  }
  if (log) log("推送失败：发布脚本未正常执行（远端不可达或 SSH 密钥无写权限）", true);
  return false;
}

// ---------- 验证 ----------

// WinHTTP GET：返回状态码与响应体（最多 maxBody 字节）
bool HttpGet(const std::wstring& host, const std::wstring& path, DWORD& status,
             std::string& body, size_t maxBody) {
  HINTERNET hSession =
      WinHttpOpen(L"yushu-project-management/2.1", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                  WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (!hSession) return false;
  HINTERNET hConn = WinHttpConnect(hSession, host.c_str(),
                                   INTERNET_DEFAULT_HTTPS_PORT, 0);
  if (!hConn) {
    WinHttpCloseHandle(hSession);
    return false;
  }
  HINTERNET hReq =
      WinHttpOpenRequest(hConn, L"GET", path.c_str(), nullptr,
                         WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                         WINHTTP_FLAG_SECURE);
  if (!hReq) {
    WinHttpCloseHandle(hConn);
    WinHttpCloseHandle(hSession);
    return false;
  }
  BOOL ok = WinHttpSendRequest(hReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                               WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
  if (ok) ok = WinHttpReceiveResponse(hReq, nullptr);
  status = 0;
  if (ok) {
    DWORD size = sizeof(status);
    WinHttpQueryHeaders(hReq, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &status, &size,
                        WINHTTP_NO_HEADER_INDEX);
  }
  body.clear();
  if (ok && status == 200) {
    char buf[4096];
    DWORD read = 0;
    while (body.size() < maxBody &&
           WinHttpReadData(hReq, buf, sizeof(buf), &read) && read > 0)
      body.append(buf, read);
  }
  WinHttpCloseHandle(hReq);
  WinHttpCloseHandle(hConn);
  WinHttpCloseHandle(hSession);
  return ok && status != 0;
}

// 去首尾空白
std::string TrimWs(std::string s) {
  size_t a = s.find_first_not_of(" \t\r\n");
  size_t b = s.find_last_not_of(" \t\r\n");
  return (a == std::string::npos) ? std::string() : s.substr(a, b - a + 1);
}

std::string VerifyAsync(const std::string& owner, const std::string& repo,
                        const std::string& repoRoot) {
  std::string url = SiteUrl(owner, repo);
  std::string host = owner + ".github.io";
  std::string base = (repo == owner + ".github.io") ? "/" : "/" + repo + "/";
  std::string root = NormalizeDrive(repoRoot);

  DWORD status = 0;
  std::string body;
  bool reachable = HttpGet(ToWide(host), ToWide(base), status, body, 4096);
  if (!reachable || status == 0) return "访问失败（网络错误）：" + url;
  if (status != 200)
    return "站点暂时返回 HTTP " + std::to_string(status) +
           "（Pages 部署中，约 1 分钟内生效）：" + url;

  // 内容新鲜度：对比线上 hashmap.json（VitePress 全页哈希表，任一笔记改动
  // 都会改变哈希）与本地已发布产物。刚推送后 Pages 要 1~2 分钟才完成部署，
  // 短暂不一致是正常的；持续不一致基本只有一种情况——Pages 的 Source 还指向
  // 旧分支（main/docs），根本没在读 gh-pages 分支。
  std::string localHashmap;
  {
    std::string p = PathToUtf8(fs::u8path(DistDir(root)) / "hashmap.json");
    FILE* f = nullptr;
    if (fopen_s(&f, p.c_str(), "rb") == 0 && f) {
      char buf[8192];
      size_t n = fread(buf, 1, sizeof(buf) - 1, f);
      fclose(f);
      buf[n] = '\0';
      localHashmap = buf;
    }
  }
  DWORD hStatus = 0;
  std::string liveHashmap;
  HttpGet(ToWide(host), ToWide(base + "hashmap.json"), hStatus, liveHashmap, 8192);
  if (!localHashmap.empty() &&
      TrimWs(localHashmap) == TrimWs(liveHashmap))
    return "✓ 站点可访问且内容为最新发布：" + url;
  if (!localHashmap.empty())
    return "⚠ 站点可访问，但线上内容还没更新：刚发布的话等 1~2 分钟 Pages 部署完成即可；" \
           "若持续如此，多半是 Pages 的 Source 还指向旧分支（main/docs），请到 " \
           "Settings→Pages 把 Source 改为 gh-pages（根目录）：https://github.com/" +
           owner + "/" + repo + "/settings/pages";
  return "✓ 站点可访问：" + url + "（本地无构建产物，未比对内容版本）";
}

}  // namespace cgpublish
