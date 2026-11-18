---
title: CG · 图形学学习笔记
---

# CG · 图形学学习笔记

从零重新学习计算机图形学：一边记笔记，一边写 demo，最终沉淀出一个小型渲染引擎。

> 📌 本站直接在 `docs/` 里维护：页面是 Markdown，配图是 SVG。

## 快速入口

- 🗺️ [学习路线图](/notes/roadmap) — 四阶段路线图与进度看板
- ➗ [数学基础](/notes/02-math/vector-matrix) — 向量与矩阵、坐标系
- 🖥️ [渲染器原理](/notes/03-renderer/renderer) — 渲染是什么、渲染器架构

## 学习路线

| 阶段 | 内容 | 参考资料 |
|---|---|---|
| 0 | C++ 与数学基础 | 自写数学库 |
| 1 | CPU 光线追踪 | [Ray Tracing in One Weekend](https://raytracing.github.io/) 三部曲 |
| 2 | 软件光栅化 | [tinyrenderer](https://github.com/ssloy/tinyrenderer) |
| 3 | GPU 实时渲染 | [learnopengl.com](https://learnopengl.com/) |
| 4 | 引擎整合与进阶 | — |

## 笔记工作流

1. 在 `docs/notes/` 里直接维护 Markdown 页面，配图 SVG 放 `docs/public/diagrams/`
2. 打开桌面端发布器，点「一键构建并发布」——构建产物自动铺到 `docs/` 并推送到 GitHub Pages

## 关于发布器

`tools/CG笔记发布器.exe` 是一个本地桌面小程序：一键完成「VitePress 构建 + 产物铺到 docs/ 并推送」，详见[工具文档](/tools/publisher)。GitHub Pages 直接部署 main 分支的 `docs/` 文件夹，推送后约 1 分钟自动生效。
