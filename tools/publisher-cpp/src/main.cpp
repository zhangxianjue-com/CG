#include <string>

#include "cli.h"
#include "tui.h"

// 入口：带 --cli 走命令行模式，否则启动终端界面（FTXUI）
int wmain(int argc, wchar_t** argv) {
  for (int i = 1; i < argc; i++) {
    if (std::wstring(argv[i]) == L"--cli") return cgpublish::CliMain(argc, argv);
  }
  return cgpublish::TuiMain();
}
