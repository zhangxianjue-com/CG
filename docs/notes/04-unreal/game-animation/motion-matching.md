---
title: "Motion Matching 运行匹配"
---

# Overview 总览

```md
运动匹配/
├── 动画数据库/
│ ├── 姿势特征
│ └── 轨迹特征
├── 查询与搜索/
│ ├── 查询向量
│ ├── 代价函数
│ └── KD-tree 加速
├── 惯性混合/
└── UE5 集成/
├── Pose Search Schema
├── Motion Matching 节点
├── Chooser 系统
└── 游戏动画示例
```

# FAQ 常见问题

1. 什么是 Motion Matching？

   ::: details 查看答案
   数据驱动的角色动画方案：运行时从海量动画数据库搜索「与当前姿势、未来期望轨迹最相似」的片段来播放，动画过渡规则从「人工编写」变成「数据搜索」。起源 Ubisoft 2016 GDC 演讲，代表作《荣耀战魂》（For Honor）。
   :::

2. 运动匹配和动画状态机有什么区别？

   ::: details 查看答案
   状态机手工连线 + 条件过渡，加动作要改状态图；运动匹配由特征搜索自动过渡，加动画进数据库即可，数据库越大越自然。代价：需要动捕数据 + 特征调参。
   :::

3. 搜索性能扛得住吗？

   ::: details 查看答案
   数据库每帧一个特征向量，用 KD-tree 等空间划分加速最近邻搜索，实时没问题。
   :::

4. UE5 里怎么上手？

   ::: details 查看答案
   以官方《游戏动画示例》(Game Animation Sample) 项目为例，按下面步骤一步一步来（UE 5.4+，截图来自 Epic 官方文档）：

   1. **启用 Motion Matching 插件**

      Edit → Plugins 里搜索 Motion Matching，勾选启用后重启编辑器。

      ![启用 Motion Matching 插件](https://d1iv7db44yhgxn.cloudfront.net/documentation/images/1bb23912-4193-4005-9dd4-e305391be619/image_100.png)

   2. **下载《游戏动画示例》项目**

      在 Fab 商店免费下载 Game Animation Sample——官方示例自带配置好的运动匹配系统，边看边学最省事。

      ![从 Fab 下载游戏动画示例项目](https://d1iv7db44yhgxn.cloudfront.net/documentation/images/7d4dac20-17b2-41ae-af37-7a502f8df42b/projectmarketpalce.png)

   3. **创建 Pose Search Schema**

      内容浏览器右键 → Animation → Pose Search Schema，并选择角色骨架。它是运动匹配的「配置中心」：定义用哪些特征、怎么采样。

      ![创建 Pose Search Schema](https://d1iv7db44yhgxn.cloudfront.net/documentation/images/21c4c2ec-1c28-49db-a1f5-45c64fe98489/image_500.png)

   4. **配置特征：骨骼通道与轨迹通道**

      在 Schema 编辑器里配置骨骼通道与轨迹通道的采样点、权重，新手先用默认值即可。

      ![Pose Search Schema 编辑器](https://d1iv7db44yhgxn.cloudfront.net/documentation/images/4b2a3d1a-8c02-46dc-98fa-807b36c566b0/image_600.png)

   5. **导入动画，创建 Pose Search Database**

      内容浏览器 (+) Add → Animation → Motion Matching → Pose Search Database，把动捕动画放进去——构建时每帧都会预计算特征向量。

      ![创建 Pose Search Database](https://d1iv7db44yhgxn.cloudfront.net/documentation/images/2c715ad6-9f13-4bce-bded-dcebf47c3a2e/image_800.png)

   6. **AnimGraph 接入 Motion Matching 节点**

      角色 AnimBP 里右键 → Pose Search → Motion Matching，配合 Pose History 节点替代传统状态机，连上 Schema 与 Database，跑起来看效果。

      ![AnimGraph 里的 Motion Matching 节点](https://d1iv7db44yhgxn.cloudfront.net/documentation/images/ba398142-5036-4e3c-85d2-ec69330f44f7/image_1600.png)

   :::
