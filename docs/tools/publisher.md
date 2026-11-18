---
title: CG 项目管理器
---

# CG 项目管理器

终端界面小工具（C++20 + [FTXUI](https://github.com/ArthurSonzogni/FTXUI)）：一键把 `docs/` 里的 Markdown 笔记构建成 VitePress 站点并发布到 GitHub Pages。

- 已编译程序：`tools/CG 项目管理器.exe`
- 源码：`tools/publisher-cpp/`（`src/` 下 engine / cli / tui / main 四个模块）
- 配套脚本：`docs/scripts/publish-site.mjs`（构建产物铺到 `docs/` 根目录）

## 使用（终端界面）

1. 双击 `tools/CG 项目管理器.exe`，打开全屏终端界面
2. 「仓库目录」自动检测（从 exe 所在目录向上找 `.git`），也可点「浏览…」手动选择
3. 按需点击：

| 按钮 | 作用 |
| --- | --- |
| 一键构建并发布 | 构建 → 发布 → 验证站点可访问，一条龙 |
| 构建站点 | 仅 `npm run docs:build` |
| 发布到 GitHub Pages | 仅 git 暂存、提交、推送 |
| 打开站点 | 用浏览器打开 Pages 地址 |
| 清空日志 | 清空日志区 |
| 退出 | 关闭（也可直接按 Esc） |

底部状态栏显示 ✓/✗ 结果；日志区滚动输出命令结果（错误行红色），保留最近 25 行。构建/发布跑在工作线程，界面不卡顿。

## 使用（CLI）

```
CG 项目管理器.exe --cli all                  # 构建 + 发布 + 验证（默认命令）
CG 项目管理器.exe --cli build                # 仅构建
CG 项目管理器.exe --cli publish              # 仅发布
CG 项目管理器.exe --cli open                 # 打开站点
CG 项目管理器.exe --cli all --repo E:\Github\CG   # 手动指定仓库路径
```

- 不带 `--repo` 时从 exe 所在目录向上自动找 `.git` 仓库根
- 退出码：`0` 成功 / `1` 流程失败 / `2` 参数或环境错误

## 重新编译发布器

改了 `tools/publisher-cpp/` 源码后（要求 VS 2022 Build Tools + CMake ≥ 3.24）：

```bash
cmake --preset vs2022
cmake --build build/vs2022 --config Release
cp build/vs2022/Release/CG 项目管理器.exe tools/
```

- FTXUI 由 FetchContent 在 configure 时自动下载，无需手动安装
- 依赖仅 Windows SDK（winhttp / shell32 / ole32），**运行机器不需要任何运行时**，单 exe 即用
- ⚠️ 若旧版发布器正在运行，exe 被占用无法覆盖——先关闭再复制

## 实现原理

代码在 `tools/publisher-cpp/src/`，四个模块：

```
main.cpp ──┬── --cli 参数 ──> cli.cpp（命令行模式）
           └── 无参数 ──────> tui.cpp（FTXUI 终端界面）
                    共用 engine.cpp（构建/发布/验证流程引擎）
```

- **engine**：流程引擎，界面与 CLI 共用；通过 `LogFn` 回调把输出接到日志区或控制台
  - `FindRepoRoot`：从起始目录向上找 `.git`
  - `RunCmd` / `CaptureCmd`：`CreateProcessW` + 管道逐行读取，命令前加 `chcp 65001` 保证 UTF-8 输出
  - `ParseOwnerRepo`：兼容 `git@`、`ssh://`、`https://` 三种远端格式解析 owner/repo
  - `SiteUrl`：区分项目页（`owner.github.io/repo/`）与用户页（`owner.github.io/`）
  - `VerifyAsync`：WinHTTP 请求站点，200 即部署生效
- **cli**：命令分发与退出码（0/1/2），`SetConsoleOutputCP(CP_UTF8)` 防中文乱码
- **tui**：FTXUI 组件树（输入框 + 按钮行 + 日志 Renderer），任务跑在 `std::thread`，日志入队后 `PostEvent(Event::Custom)` 触发重绘；共享状态用 `shared_ptr + mutex` 跨线程传递

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
