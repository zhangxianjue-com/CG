---
title: yushu-project-management
---

# Overview 总览

```md
yushu-project-management/
├── 源码/
│   ├── main.cpp
│   ├── engine.cpp
│   ├── cli.cpp
│   └── tui.cpp
├── 使用/
│   ├── TUI 终端界面
│   └── --cli 命令行
├── 构建流程/
│   └── npm run docs:build
├── 发布流程/
│   ├── .nojekyll
│   ├── git add -f
│   └── git push origin main
└── 验证/
    └── WinHTTP 200
```

# FAQ 常见问题

1. 一键构建并发布做了哪几步？

   构建（npm run docs:build）→ 发布（git add -f 提交推送）→ 验证（HTTP GET 返回 200 即部署生效）。

2. 为什么不能把 VitePress outDir 直接设为 docs/？

   VitePress 构建强制 emptyOutDir，会连源文件一起清空；所以先构建到 .vitepress/dist，再由 scripts/publish-site.mjs 复制到 docs/ 根目录。

3. 为什么必须保证 docs/.nojekyll？

   GitHub Pages 会用 Jekyll 处理站点，点开头目录（.vitepress）被排除导致 404；发布器会自愈创建该文件。

4. 发布时为什么不拉取远端？

   直接以本地构建内容提交推送（用户明确要求）；远端有新提交时 push 会被拒，需手动 git pull --rebase。

5. 推送被拒绝怎么办？

   远端有新提交、本地历史落后：手动 git pull --rebase 处理后再发布。

6. 推送失败（权限）怎么办？

   SSH 密钥无该仓库写权限，检查 ~/.ssh。

7. 未找到 Node.js 怎么办？

   安装 Node.js 并加入 PATH。

8. exe 无法覆盖怎么办？

   旧版发布器正在运行，先关闭再复制。

9. 站点 404 / .vitepress 资源丢失怎么办？

   docs/.nojekyll 缺失，发布器会自愈创建。

10. 怎么重新编译发布器？

    改 tools/publisher-cpp/ 源码后：cmake --preset vs2022 → cmake --build build/vs2022 --config Release → 把 exe 复制到 tools/。
