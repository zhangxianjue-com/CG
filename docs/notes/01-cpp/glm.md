---
title: "GLM 数学库"
---

# Overview 总览

```md
glm/
├── 安装/
│   └── CMake FetchContent（1.0.3，纯头文件）
├── 常用头文件/
│   ├── glm/glm.hpp
│   └── glm/gtc/matrix_transform.hpp
├── 核心类型与运算/
│   ├── vec2 / vec3 / vec4
│   ├── mat3 / mat4
│   ├── length / normalize
│   └── dot / cross
├── 变换/
│   ├── translate
│   ├── rotate
│   └── scale
├── 相机与投影/
│   ├── lookAt
│   └── perspective
└── 配套 demo/
    └── demos/phase1-GLM
```

# FAQ 常见问题

1. GLM 需要编译或链接吗？

   不需要：纯头文件库，include 即用。本仓库用 CMake FetchContent 拉取 1.0.3。

2. GLM 是行主序还是列主序？

   列主序：mat[i] 取到的是第 i 列；与 DirectX 的行主序混用时注意转置。

3. 为什么旋转 90° 结果不对？

   角度参数要用弧度：glm::radians(度数)，直接传 90 会转出 5156°。

4. 为什么 glm::vec3 v; 是垃圾值？

   默认构造不初始化（性能考虑）；要零向量写 glm::vec3 v(0.0f);

5. glm::mat4() 和 glm::mat4(1.0f) 有什么区别？

   mat4() 是全零矩阵；mat4(1.0f) 是单位矩阵。变换前忘了乘单位矩阵，顶点会被清零。

6. 向量参与平移要 w 为几？

   w=1 表示点（参与平移）：mat4 * vec4(v, 1.0f)；方向向量用 w=0（不参与平移）。

7. asin/acos 为什么出 NaN？

   边缘输入超出 [-1,1]，先 clamp 到 [-1,1]。

8. CPU 端用 float 版还是 double 版？

   带 d 前缀的 double 版：glm::dvec3、glm::dmat4（配合「CPU 用 double」约定）。

9. 复合变换 M = T*R*S 的应用顺序是什么？

   最右边的 S 最先应用：先缩放 → 再旋转 → 最后平移；列向量约定矩阵左乘向量。
