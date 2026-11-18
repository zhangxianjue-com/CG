---
title: "CG · 图形学学习路线图"
---

# CG · 图形学学习路线图

<Diagram src="/diagrams/01-roadmap/roadmap.svg" alt="CG · 图形学学习路线图" source="/notes/01-roadmap/roadmap.excalidraw" />

## 阶段 0 · 基础

- [x] [渲染器原理](/notes/03-renderer/renderer)
- [x] [坐标系](/notes/02-math/coordinate)
- [x] [向量与矩阵](/notes/02-math/vector-matrix)
- [ ] 矩阵与变换
- [ ] 四元数
- [ ] C++ 复习
## 阶段 1 · CPU 光线追踪（RT in One Weekend 三部曲）

- [ ] 输出第一张图（PPM）
- [ ] 光线与球求交
- [ ] 表面法线与多物体
- [ ] 抗锯齿
- [ ] 漫反射材质
- [ ] 金属材质与反射
- [ ] 玻璃与折射
- [ ] 相机与视野
- [ ] 景深
- [ ] 运动模糊
- [ ] BVH 加速结构
- [ ] 纹理与柏林噪声
- [ ] 四边形·实例化·灯光
- [ ] 体积（烟雾）
- [ ] 蒙特卡洛积分与 PDF
- [ ] 分层采样与抖动
- [ ] 重要性采样
- [ ] 路径追踪
- [ ] 直接光照采样（NEE）
- [ ] 多重重要性采样（MIS）
## 阶段 2 · 软件光栅化（tinyrenderer）

- [ ] 画线（Bresenham）
- [ ] 三角形光栅化与重心坐标
- [ ] z-buffer
- [ ] 透视投影
- [ ] 相机变换
- [ ] 着色器模型
- [ ] 阴影贴图
- [ ] 环境光遮蔽
- [ ] 镜面反射
- [ ] 法线贴图
## 阶段 3 · GPU 实时渲染（learnopengl）

- [ ] 你好，窗口与三角形
- [ ] 着色器
- [ ] 纹理
- [ ] 变换与坐标系统
- [ ] 相机
- [ ] 基础光照与材质
- [ ] 模型加载
- [ ] 深度·模板·混合·面剔除
- [ ] 帧缓冲与后处理
- [ ] 阴影映射
- [ ] 法线贴图与视差贴图
- [ ] HDR 与泛光
- [ ] 延迟着色
- [ ] SSAO
- [ ] PBR 理论
- [ ] PBR 光照
- [ ] IBL 漫反射 + 镜面
## 阶段 4 · 引擎整合与进阶

- [ ] 场景/材质统一描述层
- [ ] 双后端对照渲染
- [ ] 多线程与 job 系统
- [ ] 资产加载 obj/gltf
- [ ] 引擎模块整理与测试
- [ ] GPU 光线追踪
- [ ] 体积渲染
- [ ] 降噪
---

> 📝 本页由脚本从 Excalidraw 笔记原稿自动生成。
> 原图：[01-roadmap/roadmap.excalidraw](/notes/01-roadmap/roadmap.excalidraw)（可用 [Excalidraw](https://excalidraw.com) 或 VSCode 扩展打开编辑）
