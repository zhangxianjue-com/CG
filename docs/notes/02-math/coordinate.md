---
title: "坐标系"
---

# 坐标系

<Diagram src="/diagrams/02-math/coordinate.svg" alt="坐标系" source="/notes/02-math/coordinate.excalidraw" />

## 图文要点

2D/3D 直角坐标系 · 左手系 vs 右手系 · 极坐标 · 球坐标
### ① 2D 直角坐标系 · ② 3D 坐标系：左手系 vs 右手系

右手系（z 朝屏幕外） · 左手系（z 朝屏幕里）
P (x, y)<br>z（入屏）<br>z（出屏） 区分方法：右手系满足 x×y = z；图形学默认用右手系
### ③ 极坐标（2D） · ④ 球坐标（3D）

P (r, θ, φ)
y = r·sinθ · P (r, θ)
x = r·sinθ·cosφ<br>y = r·sinθ·sinφ<br>z = r·cosθ

| y（入屏） | 单位球（r=1）：<br>P = (sinθ·cosφ, sinθ·sinφ, cosθ) |
| --- | --- |
| x = r·cosθ | θ：与 +z 轴夹角；φ：xy 平面方位角 |

极坐标 ↔ 直角坐标：x = r·cosθ，y = r·sinθ；r = √(x²+y²)，θ = atan2(y, x)      球坐标：x = r·sinθ·cosφ，y = r·sinθ·sinφ，z = r·cosθ<br>踩坑：① 图形学默认右手系（z 出屏），左右手系只差 z 方向 ② 球坐标 θ 有两种定义（与 +z 夹角 / 与 xy 平面夹角），用前先确认 ③ atan2(y, x) 参数顺序易写反
---

> 📝 本页由脚本从 [`notes/02-math/coordinate.excalidraw`](https://github.com/zhangxianjue-com/CG/blob/main/notes/02-math/coordinate.excalidraw) 自动生成。
> 原图：[02-math/coordinate.excalidraw](/notes/02-math/coordinate.excalidraw)（可用 [Excalidraw](https://excalidraw.com) 或 VSCode 扩展打开编辑）
