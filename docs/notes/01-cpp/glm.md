---
title: "GLM 数学库"
---

# GLM 数学库

[GLM](https://github.com/g-truc/glm)（OpenGL Mathematics）是 C++ 的数学库：**纯头文件**、类型与函数风格和 GLSL 着色器一致，学一套写法 CPU/GPU 通用。渲染引擎的向量、矩阵、变换都靠它。

## 安装

本仓库用 FetchContent 在 CMake 配置时自动下载（版本 1.0.3）：

```cmake
include(FetchContent)
FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.3
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glm)

add_executable(glm_demo main.cpp)
target_include_directories(glm_demo PRIVATE ${glm_SOURCE_DIR})
```

不需要编译、不需要链接库——include 即用。

## 常用头文件

```cpp
#include <glm/glm.hpp>                    // 核心：vec2/3/4、mat3/4、dot/cross…
#include <glm/gtc/matrix_transform.hpp>   // 变换：translate/rotate/scale/lookAt/perspective
```

## 核心类型与运算

```cpp
glm::vec3 v{3.0f, 4.0f, 0.0f};
float len = glm::length(v);               // 5
glm::vec3 n = glm::normalize(v);          // 单位向量
float d   = glm::dot(a, b);               // 点乘：标量，判夹角/投影
glm::vec3 c = glm::cross(a, b);           // 叉乘：垂直向量，仅 3D
glm::vec3 sum = a + b * 2.0f;             // 运算符重载，按分量
```

带 `d` 前缀是 double 版：`glm::dvec3`、`glm::dmat4`（配合[数据类型](/notes/01-cpp/data-types)的"CPU 用 double"约定）。

## 变换

```cpp
glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 2));      // 平移
glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),         // 绕 z 轴转 90°
                          glm::vec3(0, 0, 1));
glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f));             // 缩放 2 倍

glm::mat4 M = T * R * S;                   // 复合：先缩放 → 再旋转 → 最后平移
glm::vec4 p2 = M * glm::vec4(p, 1.0f);     // 列向量约定：矩阵左乘向量
```

**顺序读法**：`M = T*R*S` 作用于顶点 `v' = M·v`，最右边的 S 最先应用。这和[向量与矩阵](/notes/02-math/vector-matrix)笔记里"推荐列向量"的约定一致。

## 相机与投影（MVP）

```cpp
glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 5),     // 相机位置
                             glm::vec3(0, 0, 0),     // 看向目标
                             glm::vec3(0, 1, 0));    // up 向量
glm::mat4 proj = glm::perspective(glm::radians(45.0f),  // 垂直 FOV
                                  16.0f / 9.0f,         // 宽高比
                                  0.1f, 100.0f);        // 近/远平面

glm::mat4 mvp = proj * view * M;              // 世界 → 裁剪空间
glm::vec4 clip = mvp * glm::vec4(p, 1.0f);
glm::vec3 ndc = glm::vec3(clip) / clip.w;     // 透视除法 → NDC ∈ [-1,1]
```

## 配套 demo

`demos/phase1-GLM/main.cpp` 用断言验证了以上所有结论（长度/叉乘/复合变换/逆矩阵/lookAt/NDC）：

```bash
cd demos/phase1-GLM
cmake --preset vs2022
cmake --build build/vs2022 --config Release
build/vs2022/Release/glm_demo.exe
```

全部通过会输出 `全部通过 ✓`。

## 踩坑记录

- **GLM 是列主序**：`mat[i]` 取到的是第 i 列；内存布局与 DirectX 的行主序相反，混用 API 时注意转置
- **角度用弧度**：`rotate/perspective` 都要 `glm::radians(度数)`，直接传 90 会转出 5156°
- **默认构造不初始化**：`glm::vec3 v;` 是垃圾值（性能考虑）。要零向量用 `glm::vec3 v(0.0f);`
- **矩阵单位化**：`glm::mat4(1.0f)` 是单位矩阵，`glm::mat4()` 是全零——变换前忘了乘单位矩阵，顶点会被清零
- **向量齐次坐标**：`mat4 * vec4(v, 1.0f)` 的 w=1 表示点（参与平移）；方向向量用 w=0（不参与平移）
- **反三角函数范围**：`asin/acos` 在边缘输入会出 NaN，先 clamp 到 [-1,1]
