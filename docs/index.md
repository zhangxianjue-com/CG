---
title: CG · 图形学学习笔记
---

# CG · 图形学学习笔记

从零重新学习计算机图形学：一边记笔记，一边写 demo，最终沉淀出一个小型渲染引擎。

> 📌 笔记原稿是 [Excalidraw](https://excalidraw.com) 图（存于 `notes/`，一图一主题）；本站由脚本**自动转换**而来：图渲染成 SVG，图内文字提取成可搜索的页面。

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

1. 学完一个主题 → 在 `notes/NN-area/` 下画一张 `<topic>.excalidraw`（约定见 [笔记模板](https://github.com/zhangxianjue-com/CG/blob/main/notes/_template.md)）
2. 在路线图卡片上加 `link` 指向该文件
3. 打开桌面端发布器，点「一键构建并发布」——脚本自动生成页面并推送到 GitHub Pages

## 关于发布器

`tools/CG笔记发布器.exe` 是一个本地桌面小程序：一键完成「渲染 SVG + 生成页面 + VitePress 构建 + 推送 gh-pages 分支」。首次发布前需在仓库 Settings → Pages 里把 Source 选为 `gh-pages` 分支（只需一次），或把 GitHub Token 填入发布器自动开启。
