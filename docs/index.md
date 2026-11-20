---
title: CG · 图形学学习笔记
---

# Overview 总览

```md
CG/
├── docs/
│   ├── notes/
│   │   ├── 01-cpp/
│   │   ├── 02-math/
│   │   ├── 03-renderer/
│   │   └── 04-unreal/
│   ├── public/diagrams/
│   ├── scripts/
│   └── .vitepress/
├── demos/
│   └── phase1-GLM/
└── tools/
    └── publisher-cpp/
```

# FAQ 常见问题

1. 这个项目是做什么的？

   从零重新学习计算机图形学：一边记笔记，一边写 demo，最终沉淀出一个小型渲染引擎。

2. 学习路线分几个阶段？

   5 个：0 C++ 与数学基础 → 1 CPU 光线追踪（RT in One Weekend 三部曲）→ 2 软件光栅化（tinyrenderer）→ 3 GPU 实时渲染（learnopengl）→ 4 引擎整合与进阶。

3. 笔记写在哪里？

   全部 Markdown 直接维护在 docs/notes/；构建产物提交进仓库，GitHub Pages 直接部署 main 分支的 /docs 文件夹。

4. 配图 SVG 放在哪里？

   docs/public/diagrams/，页面里用普通 md 图片语法引用。

5. 怎么发布到线上？

   桌面发布器 tools/yushu-project-management.exe「一键构建并发布」，或 npm run docs:build 手动铺产物；推送后约 1 分钟自动生效。

6. 渲染引擎最后要做成什么样？

   双后端对照渲染：同一份场景，光栅化与光线追踪两种实现互相对照验证。
