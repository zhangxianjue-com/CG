---
title: CG 笔记发布器
---

# CG 笔记发布器

Windows 桌面小程序（C# WinForms / .NET 8，单文件发布）：一键把 `docs/` 里的 Markdown 笔记构建成 VitePress 站点并发布到 GitHub Pages。

- 已编译程序：`tools/CG笔记发布器.exe`
- 源码：`tools/publisher/`（`Program.cs` 单文件 + `CGPublish.csproj`）
- 配套脚本：`docs/scripts/publish-site.mjs`（构建产物铺到 `docs/` 根目录）

## 使用（GUI）

1. 双击 `tools/CG笔记发布器.exe`
2. 「仓库目录」自动检测（从 exe 所在目录向上找 `.git`），也可点「浏览…」手动选择
3. 按需点击：

| 按钮 | 作用 |
| --- | --- |
| 🚀 一键构建并发布 | 构建 → 发布 → 验证站点可访问，一条龙 |
| 构建站点 | 仅 `npm run docs:build` |
| 发布到 GitHub Pages | 仅 git 暂存、提交、推送 |
| 打开站点 | 用浏览器打开 Pages 地址 |
| 清空日志 | 清空日志框 |

底部状态栏显示 ✓/✗ 结果，日志框实时滚动输出命令结果。

## 使用（CLI）

```
CG笔记发布器.exe --cli all                  # 构建 + 发布 + 验证（默认命令）
CG笔记发布器.exe --cli build                # 仅构建
CG笔记发布器.exe --cli publish              # 仅发布
CG笔记发布器.exe --cli open                 # 打开站点
CG笔记发布器.exe --cli all --repo E:\Github\CG   # 手动指定仓库路径
```

- 不带 `--repo` 时从 exe 所在目录向上自动找 `.git` 仓库根
- 退出码：`0` 成功 / `1` 流程失败 / `2` 参数或环境错误

## 重新编译发布器

改了 `tools/publisher/` 源码后：

```bash
dotnet publish tools/publisher -c Release
cp tools/publisher/bin/Release/net8.0-windows/win-x64/publish/CG笔记发布器.exe tools/
```

- 要求 .NET 8 SDK
- `CGPublish.csproj` 关键配置：`net8.0-windows` + WinExe、`PublishSingleFile`（单文件 exe）、`SelfContained=false`（框架依赖，**运行机器需要 .NET 8 桌面运行时**）
- ⚠️ 若旧版发布器正在运行，exe 被占用无法覆盖——先关闭再复制

## 实现原理

全部代码在 `tools/publisher/Program.cs`（约 470 行），三个层次：

```
Program.Main ──┬── --cli 参数 ──> Cli.MainAsync（命令行模式）
               └── 无参数 ──────> MainForm（WinForms 界面）
```

- **PublishEngine**：构建/发布/验证流程引擎，GUI 与 CLI 共用；通过 `Log` 回调把输出接到日志框或控制台
  - `FindRepoRoot`：从起始目录向上找 `.git`
  - `RunAsync` / `CaptureAsync`：用 `cmd.exe /d /s /c` 包装执行命令，UTF-8 解码输出
  - `ParseOwnerRepo`：兼容 `git@`、`ssh://`、`https://` 三种远端格式解析 owner/repo
  - `SiteUrl`：区分项目页（`owner.github.io/repo/`）与用户页（`owner.github.io/`）
- **Cli**：命令分发。WinExe 无控制台句柄，手动把 stdout/stderr 替换为 UTF-8 流（否则中文乱码）
- **MainForm**：仓库路径选择 + 4 个按钮 + 深色日志框；`_busy` 锁防重入，`BeginInvoke` 跨线程回显日志

### 构建流程（BuildAsync）

```
检查 docs/ 存在 → node --version → 无 node_modules 则 npm install → npm run docs:build
```

### 发布流程（PublishAsync）

```
检查 docs/index.html 存在
→ 保证 docs/.nojekyll（跳过 Jekyll，否则 .vitepress 等点开头目录 404）
→ git add docs + git add -f 强制加入被 .gitignore 忽略的产物
→ 无变更则跳过提交
→ 以 CGPublish 身份提交（deploy: CG 笔记站点 <时间>）
→ git push origin main
```

> 发布不拉取远端：直接使用本地构建产物提交并推送。若远端有新提交导致推送被拒绝，需先手动 `git pull --rebase` 再重试。

### 验证（VerifyAsync）

HTTP GET 站点地址，返回 200 即部署生效；否则提示 Pages 部署中（约 1 分钟）。

## 构建与部署链路

`npm run docs:build` 由两步组成：

1. `vitepress build` —— 输出到 `docs/.vitepress/dist/`
2. `docs/scripts/publish-site.mjs` —— 清理上次产物后，把 `dist` 整目录复制到 `docs/` 根目录

**为什么不能把 VitePress 的 outDir 直接设成 `docs/`**：VitePress 构建强制 emptyOutDir，会连源文件一起清空。所以先构建到中间目录，再复制过去。

发布器随后把被 `.gitignore` 忽略的产物（`docs/index.html`、`404.html`、`assets/`、`diagrams/`、`hashmap.json`、`notes/*.html`）force-add 提交并推送。**GitHub Pages 直接部署 main 分支的 `/docs` 文件夹**，推送后约 1 分钟自动生效。

## 常见问题

| 现象 | 原因与处理 |
| --- | --- |
| 推送被拒绝（远端有新提交） | 本地历史落后远端：手动 `git pull --rebase` 处理后再发布 |
| 推送失败 | SSH 密钥无该仓库写权限，检查 `~/.ssh` |
| 未找到 Node.js | 安装 Node.js 并加入 PATH |
| 依赖安装失败 | npm install 网络问题，检查网络后重试 |
| exe 无法覆盖 | 旧版发布器正在运行，先关闭 |
| 站点 404 / .vitepress 资源丢失 | `docs/.nojekyll` 缺失，发布器会自愈创建 |
