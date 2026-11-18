---
title: "渲染器原理"
---

# 渲染器原理

<Diagram src="/diagrams/03-renderer/renderer.svg" alt="渲染器原理" source="/notes/03-renderer/renderer.excalidraw" />

## 图文要点

参考 GAMES101 导论：图形学（游戏工业界）≈ 渲染
### ① 什么是渲染

在游戏工业界，图形学 ≈ 渲染（Rendering）<br>渲染：计算屏幕上每个像素的颜色 (R, G, B)<br>1 个像素 = (34, 177, 76)<br>3 个分量：R 红 · G 绿 · B 蓝，各 8 bit → 24 bit 真彩色
### ② 最基础的渲染器架构

```txt
Color screen_pixels[W][H];                        // 屏幕缓冲：每像素一个颜色
for (int y = 0; y < H; y++)                       // 遍历每一行
    for (int x = 0; x < W; x++)                   // 遍历每一列
        screen_pixels[x][y] = RenderPixel(x, y);  // 核心：算这个像素的颜色
present(screen_pixels);                           // 把缓冲显示到屏幕
```

RenderPixel(x, y) 是渲染器的核心 —— 后续所有阶段，本质都在回答「一个像素的颜色怎么算」
### ③ 渲染器长什么样

场景 几何 · 材质 光照 · 相机 · 渲染器 逐像素计算颜色 · 输出 屏幕像素阵列 (W×H)
两条主流路线：光栅化（逐三角形投影 → 填像素） · 光线追踪（逐像素发射光线求交）
### ④ 踩坑记录

先做离线渲染：像素写进 PPM 文件再打开看，比直接上窗口 / GPU 简单一个数量级<br>调试时把像素涂成纯色（纯红 / 纯绿），一眼看出坐标与循环边界是否正确
---

> 📝 本页由脚本从 Excalidraw 笔记原稿自动生成。
> 原图：[03-renderer/renderer.excalidraw](/notes/03-renderer/renderer.excalidraw)（可用 [Excalidraw](https://excalidraw.com) 或 VSCode 扩展打开编辑）
