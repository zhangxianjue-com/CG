#include "cli.h"

#include "engine.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#include <iostream>
#include <string>
#include <vector>

namespace cgpublish {

int CliMain(int argc, wchar_t** argv) {
  // 控制台输出按 UTF-8 解释（中文不乱码）
  SetConsoleOutputCP(CP_UTF8);

  std::vector<std::string> args;
  for (int i = 1; i < argc; i++) args.push_back(ToUtf8(argv[i]));

  std::string cmd;  // 第一个非 -- 参数；缺省 all
  std::string repo;
  for (size_t i = 0; i < args.size(); i++) {
    if (args[i] == "--repo" && i + 1 < args.size()) {
      repo = args[++i];
      continue;
    }
    if (cmd.empty() && !args[i].empty() && args[i][0] != '-') cmd = args[i];
  }
  if (cmd.empty()) cmd = "all";

  auto log = [](const std::string& line, bool isErr) {
    if (isErr)
      std::cerr << "[ERR] " << line << "\n";
    else
      std::cout << line << "\n";
  };

  std::string repoRoot = !repo.empty() ? repo : FindRepoRoot(ExeDir());
  if (repoRoot.empty()) {
    std::cout << "找不到仓库根目录（.git），请用 --repo 指定\n";
    return 2;
  }
  std::cout << "仓库：" << repoRoot << "\n";

  if (cmd == "build") return BuildAsync(repoRoot, log) ? 0 : 1;

  if (cmd == "publish") return PublishAsync(repoRoot, log) ? 0 : 1;

  if (cmd == "all") {
    if (!BuildAsync(repoRoot, log)) return 1;
    if (!PublishAsync(repoRoot, log)) return 1;
    auto remote = GetRemoteUrl(repoRoot);
    auto p = ParseOwnerRepo(remote);
    if (p) std::cout << VerifyAsync(p->first, p->second) << "\n";
    return 0;
  }

  if (cmd == "open") {
    auto remote = GetRemoteUrl(repoRoot);
    auto p = ParseOwnerRepo(remote);
    if (!p) {
      std::cout << "无法从远端地址解析 owner/repo\n";
      return 2;
    }
    std::wstring url = ToWide(SiteUrl(p->first, p->second));
    ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    return 0;
  }

  std::cout << "用法：yushu-project-management.exe --cli [build|publish|all|open] [--repo 路径]\n";
  return 2;
}

}  // namespace cgpublish
