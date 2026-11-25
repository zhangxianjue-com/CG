---
title: yushu-project-management
---

> 自研的 C++ 发布器（FTXUI TUI + CLI）：一键构建 VitePress 站点并把 dist 产物推送到 gh-pages 分支，main 分支只保留源文件。

# Overview 总览

```md
yushu-project-management/
├── 源码/
│   ├── main.cpp（入口分发 --cli / TUI）
│   ├── engine.cpp（构建/发布/验证）
│   ├── cli.cpp
│   └── tui.cpp（FTXUI 终端界面）
├── 使用/
│   ├── TUI 终端界面
│   └── --cli [build|publish|all|open]
├── 构建流程/
│   ├── npm run docs:build
│   └── 产物在 .vitepress/dist（含 .nojekyll）
├── 发布流程/
│   ├── git add -f docs/.vitepress/dist
│   ├── 临时提交（CGPublish 身份）
│   ├── git push origin HEAD:gh-pages --force
│   └── git reset --mixed HEAD~1（撤销临时提交）
└── 验证/
    └── WinHTTP 200
```

# FAQ 常见问题

1. 一键构建并发布做了哪几步？

   ::: details 查看答案
   构建（npm run docs:build，产物在 .vitepress/dist）→ 发布（force-add dist → 临时提交 → 推送 gh-pages --force → 撤销本地临时提交）→ 验证（HTTP GET 返回 200 即部署生效）。
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
   直接以本地构建内容提交推送（用户明确要求）。发布器只推 gh-pages 分支（--force 覆盖），不碰 main；main 的推送由你自己管理，远端有别人提交时 main 的 push 会被拒，需手动 git pull --rebase。
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
   gh-pages 分支根目录缺 .nojekyll，重新构建（publish-site.mjs 会自愈创建）再发布。
   :::

10. 怎么重新编译发布器？

    ::: details 查看答案
    改 tools/publisher-cpp/ 源码后：cmake --preset vs2022 → cmake --build build/vs2022 --config Release → 把 exe 复制到 tools/。
    :::
