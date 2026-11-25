# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概览

CG 图形学学习项目：VitePress 笔记站点（`docs/`）+ C++ demo（`demos/`）+ 发布工具（`tools/`）。文档全中文，代码注释也以中文为主。

## 部署架构（最重要，跨多文件才能看懂）

GitHub Pages **部署 gh-pages 分支根目录**，站点构建产物**不进 main**：

1. `npm run docs:build`（在 `docs/` 下）= `vitepress build`（输出到 `.vitepress/dist/`）+ `scripts/publish-site.mjs`（确保 `dist/.nojekyll` 存在）
   - 不能把 VitePress outDir 直接设为 `docs/`：构建强制 emptyOutDir 会连源文件一起删，所以构建到 `.vitepress/dist`（gitignore 的中间产物）
2. 发布器（C++）把 dist 推成 gh-pages 分支**根目录**：独立临时 index（`GIT_INDEX_FILE`）`git add -f docs/.vitepress/dist` → `git write-tree --prefix` 把 dist 子树提为根目录树 → `git commit-tree` 生成孤儿提交（`CGPublish` 身份，gh-pages 只保留最新一次快照）→ `git push <sha>:gh-pages --force`。全程不碰 main 和用户自己的 index/HEAD；流程在 %TEMP% 的 bat 脚本里执行，结果经结果文件回传（cmd 退出码经 call/&& 传播不可靠，勿改回依赖 exit code）
   - ⚠️ gh-pages 分支上必须是**站点文件本身**（根目录有 index.html），不能把整个仓库推上去——根下没有 index.html 时 Pages 构建失败，线上会一直保留上一次旧部署
3. `dist/.nojekyll` 必须存在（Pages 会用 Jekyll 排除点开头目录导致 404，publish-site.mjs 会自愈创建）
4. **发布时不 pull 远端**（用户明确要求：直接以本地构建内容推送）；main 的提交推送由用户自行管理，远端有别人提交时 push 会被拒，需手动 `git pull --rebase`

⚠️ `docs/` 里只有源文件：`*.md`、`public/`、`.vitepress/`、`scripts/`；`.vitepress/dist/` 是构建中间产物（gitignore），**不要手改产物**。Pages 的 Source 需在 GitHub Settings→Pages 设为 gh-pages 分支（根目录）——2026-11-25 已确认：线上旧内容 = 该设置还指向旧来源（main/docs），gh-pages 推得再勤也不会生效；发布器现在会在发布后对比线上 hashmap.json 报警这个问题。

## 常用命令

```bash
# 文档站点（在 docs/ 下）
npm run docs:dev        # 本地开发服务器
npm run docs:build      # 构建到 .vitepress/dist + 确保 dist/.nojekyll
npx vitepress build     # 仅验证构建，产物留在 .vitepress/dist

# 发布器（C++ 版，tools/publisher-cpp/）
cmake --preset vs2026
cmake --build build/vs2026 --config Release
cp build/vs2026/Release/yushu-project-management.exe tools/   # ⚠️ 运行中的 exe 被锁定，先关闭
"tools/yushu-project-management.exe" --cli all               # 或 build / publish / verify / open，--repo 指定仓库路径

# demos（每个 demo 是独立 CMake 工程，各自目录下执行）
cd demos/phase1-GLM
cmake --preset vs2026
cmake --build build/vs2026 --config Release
build/vs2026/Release/glm_demo.exe
```

本机 CMake：winget 装的 Kitware CMake 4.4.4 在 PATH；VS 2026（18.12）装在 `D:\Apps\VS\App`，未装 VS 自带 CMake 组件（旧 C 盘 VS 2022 Build Tools 已卸载）。

## 结构要点

- **docs/**：笔记页面直接以 Markdown 维护在 `docs/notes/`（曾用 Excalidraw 生成，**已彻底移除，不要再引入**）；侧边栏是 `docs/.vitepress/config.mts` 里的静态配置（没有自动清单）；配图 SVG 放 `docs/public/diagrams/`，页面里用普通 md 图片语法引用
- **tools/publisher-cpp/**：C++20 + FTXUI（FetchContent 拉 v6.1.9）重写的「yushu-project-management」。`src/` 四模块：`main.cpp` 入口分发（`--cli` 参数 → 命令行模式，否则 TUI）、`engine.cpp`（构建/发布/验证流程：`CreateProcessW`+管道执行命令、WinHTTP 验证站点）、`cli.cpp`、`tui.cpp`（全屏终端 UI + 工作线程）。`tools/yushu-project-management.exe` 是**被 git 跟踪的编译产物**，源码改动后要重新编译复制
- **demos/**：2026-11-25 已清空；内容由用户手动编写重建，**不要用 AI 生成 demo 内容**。原结构供参考：每阶段一个独立目录（如 `phase1-GLM`），各自带 CMakeLists.txt + CMakePresets.json，GLM 通过 FetchContent 引入（纯头文件）
- **重构进行中**：根 `CMakeLists.txt`/`CMakePresets.json`、`engine/`、`third_party/` 已被用户删除，demos 暂时是独立工程；README 的「构建」章节因此过期，以各 demo 目录内实际配置为准

## 约定

- 发布器以 `user.name=CGPublish` 身份提交（`deploy: CG 笔记站点 <时间>`），不要用个人身份发 deploy 提交
- Claude Code 的 git 提交信息结尾加 `Co-Authored-By: Claude Code <noreply@anthropic.com>`
- 用户是中文使用者，回复和文档用中文
- md 笔记按统一模板写，页面由四部分组成（示例：`docs/notes/04-unreal/game-animation/motion-matching.md`）：
  1. frontmatter 写 `title`（页面不写 H1 标题）
  2. `> 一句话概述`：一句话概括整页内容
  3. `# Markmap 知识体系`：用 ```md 代码块画目录树（目录名带 `/`，├── │ └── 多层嵌套），配图放树之后
  4. `# FAQ 常见问题`：有序列表列问题（问题以？结尾独占一行），答案缩进写在问题下方、用 `::: details 查看答案` 折叠块包裹；答案里可配图、可嵌套有序列表分步
- 笔记配图用手绘 **SVG 示意图**（参考 `docs/notes/01-cpp/visual-studio.md` 的 FAQ 配图），放 `docs/public/diagrams/<章节>/`，页面用普通 md 语法 `/diagrams/<章节>/xxx.svg` 引用：
  1. 画「示意界面」而非伪造截图：只画关键元素（勾选框、标签页、行号、按钮、卡片 + 箭头流程），不追求像素级还原
  2. 统一视觉：白底；圆角矩形面板——普通 `#f8f9fa` 底 + `#dee2e6` 边，强调面板 `#e7f5ff` 底 + `#1971c2` 边；标题 20px 加粗 `#343a40`；正文 14px `#495057`；注释 13px `#868e96`；中文 `Segoe UI, Microsoft YaHei, Helvetica, sans-serif`，代码 `Consolas, monospace`；快捷键用圆角胶囊徽章（浅蓝/浅黄底）
  3. 宽度约 720，多步流程用箭头串联的卡片布局
  4. 绘制前尽量搜索网上真实界面图（官方文档/截图）作参照，元素名称、布局与真实 UI 一致，不要凭空乱画
