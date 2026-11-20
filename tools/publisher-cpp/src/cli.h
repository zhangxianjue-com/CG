#pragma once

namespace cgpublish {

// 命令行模式：--cli [build|publish|all|open] [--repo 路径]
// 返回退出码：0 成功 / 1 流程失败 / 2 参数或环境错误
int CliMain(int argc, wchar_t** argv);

}  // namespace cgpublish
