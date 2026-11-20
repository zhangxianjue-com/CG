# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概览

CG 图形学学习项目：VitePress 笔记站点（`docs/`）+ C++ demo（`demos/`）+ 发布工具（`tools/`）。文档全中文，代码注释也以中文为主。

## 部署架构（最重要，跨多文件才能看懂）

GitHub Pages **直接部署 main 分支的 `/docs` 文件夹**，站点构建产物是**提交进仓库**的：

1. `npm run docs:build`（在 `docs/` 下）= `vitepress build`（输出到 `.vitepress/dist/`）+ `scripts/publish-site.mjs`（清理上次产物后把 dist 复制到 `docs/` 根目录）
   - 不能把 VitePress outDir 直接设为 `docs/`：构建强制 emptyOutDir 会连源文件一起删，所以两段式
2. 发布器把被 `.gitignore` 忽略的产物 `git add -f` 后提交并推送 main
3. `docs/.nojekyll` 必须存在（Pages 会用 Jekyll 排除点开头目录导致 404，发布器会自愈创建）
4. **发布时不 pull 远端**（用户明确要求：直接以本地构建内容提交推送）；远端有别人提交时 push 会被拒，需手动 `git pull --rebase`

⚠️ `docs/` 里源文件和构建产物**共存**：`*.md`、`public/`、`.vitepress/`、`scripts/` 是源文件；`index.html`、`404.html`、`assets/`、`diagrams/`、`hashmap.json`、`notes/**/*.html` 是产物，**不要手改产物**。

## 常用命令

```bash
# 文档站点（在 docs/ 下）
npm run docs:dev        # 本地开发服务器
npm run docs:build      # 构建 + 产物铺到 docs/ 根（会改动 docs/ 下产物文件）
npx vitepress build     # 仅验证构建，产物留在 .vitepress/dist（不改 docs/ 根）

# 发布器（C++ 版，tools/publisher-cpp/）
cmake --preset vs2022
cmake --build build/vs2022 --config Release
cp build/vs2022/Release/yushu-project-management.exe tools/   # ⚠️ 运行中的 exe 被锁定，先关闭
"tools/yushu-project-management.exe" --cli all               # 或 build / publish / open，--repo 指定仓库路径

# demos（每个 demo 是独立 CMake 工程，各自目录下执行）
cd demos/phase1-GLM
cmake --preset vs2022
cmake --build build/vs2022 --config Release
build/vs2022/Release/glm_demo.exe
```

本机 CMake 不在 PATH：`C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`（VS 2022 Build Tools 自带）。

## 结构要点

- **docs/**：笔记页面直接以 Markdown 维护在 `docs/notes/`（曾用 Excalidraw 生成，**已彻底移除，不要再引入**）；侧边栏是 `docs/.vitepress/config.mts` 里的静态配置（没有自动清单）；配图 SVG 放 `docs/public/diagrams/`，页面里用普通 md 图片语法引用
- **tools/publisher-cpp/**：C++20 + FTXUI（FetchContent 拉 v6.1.9）重写的「yushu-project-management」。`src/` 四模块：`main.cpp` 入口分发（`--cli` 参数 → 命令行模式，否则 TUI）、`engine.cpp`（构建/发布/验证流程：`CreateProcessW`+管道执行命令、WinHTTP 验证站点）、`cli.cpp`、`tui.cpp`（全屏终端 UI + 工作线程）。`tools/yushu-project-management.exe` 是**被 git 跟踪的编译产物**，源码改动后要重新编译复制
- **demos/**：每阶段一个独立目录（`phase0-cpp` C++ 基础、`phase1-GLM` 数学库），各自带 CMakeLists.txt + CMakePresets.json；GLM 1.0.3 通过 FetchContent 引入（纯头文件）
- **重构进行中**：根 `CMakeLists.txt`/`CMakePresets.json`、`engine/`、`third_party/` 已被用户删除，demos 暂时是独立工程；README 的「构建」章节因此过期，以各 demo 目录内实际配置为准

## 约定

- 发布器以 `user.name=CGPublish` 身份提交（`deploy: CG 笔记站点 <时间>`），不要用个人身份发 deploy 提交
- Claude Code 的 git 提交信息结尾加 `Co-Authored-By: Claude Code <noreply@anthropic.com>`
- 用户是中文使用者，回复和文档用中文
- md 笔记按统一模板写，页面由四部分组成（示例：`docs/notes/04-unreal/game-animation/motion-matching.md`）：
  1. frontmatter 写 `title`（页面不写 H1 标题）
  2. `> 一句话概述`：一句话概括整页内容
  3. `# Overview 知识体系`：用 ```md 代码块画目录树（目录名带 `/`，├── │ └── 多层嵌套），配图放树之后
  4. `# FAQ 常见问题`：有序列表列问题（问题以？结尾独占一行），答案缩进写在问题下方、用 `::: details 查看答案` 折叠块包裹；答案里可配图、可嵌套有序列表分步
- `test*.text` 是用户个人临时文件（已 gitignore），`test1.md` 等未跟踪文件不要动
