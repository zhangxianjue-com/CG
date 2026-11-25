---
title: yushu-project-management
---

> 自研的 C++ 发布器（FTXUI TUI + CLI）：一键构建 VitePress 站点并把 dist 产物推送到 gh-pages 分支，main 分支只保留源文件。

# Markmap 知识体系

```md
yushu-project-management/
├── 源码/
│   ├── main.cpp（入口分发 --cli / TUI）
│   ├── engine.cpp（构建/发布/验证）
│   ├── cli.cpp
│   └── tui.cpp（FTXUI 终端界面）
├── 使用/
│   ├── TUI 终端界面
│   └── --cli [build|publish|all|verify|open]
├── 构建流程/
│   ├── npm run docs:build
│   └── 产物在 .vitepress/dist（含 .nojekyll）
├── 发布流程/
│   ├── 独立临时 index 暂存 dist（git add -f）
│   ├── write-tree --prefix 把 dist 子树提为根目录树
│   ├── commit-tree 生成孤儿提交（CGPublish 身份）
│   └── push <提交>:gh-pages --force（不碰 main/用户 index）
└── 验证/
    ├── WinHTTP 200
    └── 对比线上 hashmap.json 判断内容是否最新
```

# FAQ 常见问题

1. 一键构建并发布做了哪几步？

   ::: details 查看答案
   构建（npm run docs:build，产物在 .vitepress/dist）→ 发布（用独立临时 index 把 dist 写成根目录树 → 孤儿提交 → 强推 gh-pages，站点文件位于分支根目录）→ 验证（HTTP GET 返回 200，且对比线上 hashmap.json 与本地产物的页面哈希清单，不一致会警告"线上内容未更新"并提示排查 Pages Source 设置）。
   :::

2. 为什么不能把 VitePress outDir 直接设为 docs/？

   ::: details 查看答案
   VitePress 构建强制 emptyOutDir，会连源文件一起清空；所以先构建到 .vitepress/dist（gitignore 的中间产物），再由发布器推送到 gh-pages 分支，main 不存放任何产物。
   :::

3. 为什么必须保证 dist/.nojekyll？

   ::: details 查看答案
   GitHub Pages 默认用 Jekyll 处理站点，点开头目录（.vitepress）会被排除导致 404；publish-site.mjs 构建后自愈创建 dist/.nojekyll。
   :::

4. 发布时为什么不拉取远端？

   ::: details 查看答案
   直接以本地构建内容提交推送（用户明确要求）。发布器只推 gh-pages 分支（--force 覆盖上一次快照），全程不碰 main、不产生临时提交；main 的推送由你自己管理，远端有别人提交时 main 的 push 会被拒，需手动 git pull --rebase。
   :::

5. main 推送被拒绝怎么办？

   ::: details 查看答案
   远端有新提交、本地历史落后：手动 git pull --rebase 处理后再推。
   :::

6. 推送失败（权限）怎么办？

   ::: details 查看答案
   SSH 密钥无该仓库写权限，检查 ~/.ssh。
   :::

7. 未找到 Node.js 怎么办？

   ::: details 查看答案
   安装 Node.js 并加入 PATH。
   :::

8. exe 无法覆盖怎么办？

   ::: details 查看答案
   旧版发布器正在运行，先关闭再复制。
   :::

9. 站点 404 / .vitepress 资源丢失怎么办？

   ::: details 查看答案
   gh-pages 分支根目录缺 .nojekyll，重新构建（publish-site.mjs 会自愈创建）再发布。注意 Pages 部署的是 gh-pages 分支**根目录**——站点文件（index.html 等）必须在分支根，不能嵌套在 docs/.vitepress/dist/ 里。
   :::

10. 线上站点一直显示旧内容、和本地对不上怎么办？

    ::: details 查看答案
    多半是 GitHub Pages 的 Source 设置还指向旧来源。去仓库 Settings → Pages → Build and deployment → Source 选 Deploy from a branch → 分支 gh-pages、文件夹 / (root) → Save。设置正确后，发布器每次推送都会在约 1 分钟内自动部署新内容。仓库是私有的情况下 Settings 只能自己登录查看。
    :::

11. 怎么重新编译发布器？

    ::: details 查看答案
    改 tools/publisher-cpp/ 源码后：cmake --preset vs2026 → cmake --build build/vs2026 --config Release → 把 exe 复制到 tools/。
    :::
