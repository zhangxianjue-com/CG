#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace cgpublish {

// 日志回调：(行文本, 是否错误)。可能从工作线程调用。
using LogFn = std::function<void(const std::string& line, bool isError)>;

// 路径与编码
std::string ToUtf8(const std::wstring& w);
std::wstring ToWide(const std::string& s);
std::string ExeDir();
std::string FindRepoRoot(const std::string& start);
std::string DocsDir(const std::string& repoRoot);
std::string DistDir(const std::string& repoRoot);

// 命令执行（cmd.exe 包裹，UTF-8 解码输出，逐行回调）
bool RunCmd(const std::string& workDir, const std::string& cmdline, const LogFn& log);
int RunCmdCode(const std::string& workDir, const std::string& cmdline,
               const LogFn& log);
std::string CaptureCmd(const std::string& workDir, const std::string& cmdline);

// Git / 站点地址
std::string GetRemoteUrl(const std::string& repoRoot);
std::optional<std::pair<std::string, std::string>> ParseOwnerRepo(
    const std::string& remoteUrl);
std::string SiteUrl(const std::string& owner, const std::string& repo);

// 构建 / 发布 / 验证（对应原 C# PublishEngine）
bool BuildAsync(const std::string& repoRoot, const LogFn& log);
bool PublishAsync(const std::string& repoRoot, const LogFn& log);
std::string VerifyAsync(const std::string& owner, const std::string& repo);

}  // namespace cgpublish
