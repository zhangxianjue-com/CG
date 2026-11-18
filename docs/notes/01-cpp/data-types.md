---
title: "数据类型"
---

# 数据类型

C++ 的数值类型是图形学代码的地基——选错类型，累积误差会以噪点、抖动、穿模的形式还回来。

## 基本类型

| 类型 | 大小 | 用途 |
| --- | --- | --- |
| bool | 1 | 逻辑值 |
| char | 1 | 字符/字节 |
| short | 2 | 几乎不用 |
| int | 4 | 整数、循环、索引 |
| long long | 8 | 大整数 |
| float | 4 | GPU 上的标准浮点 |
| double | 8 | CPU 端计算的默认浮点 |

字面量后缀：`3.14f`（float）、`3.14`（double）、`42u`（unsigned）、`42L`（long）。

## float 还是 double

**规则：CPU 端计算用 double，上传 GPU 时再转 float。**

```cpp
double x = 1.0;             // CPU 计算：double
float  gx = (float)x;       // 进 GPU 缓冲：float
```

理由：float 只有约 7 位有效数字，光线追踪里数百万次乘加累积误差会出现噪点、命中偏移；double 约 16 位，代价只是 CPU 慢一点。GPU 显存和带宽有限，所以仍用 float。

## const 与 constexpr

```cpp
const double pi = 3.141592653589793;   // 运行期常量：值不可改
constexpr int W = 800;                  // 编译期常量：可用于数组大小
```

图形学里的屏幕尺寸、采样数、材质参数尽量 `constexpr`，编译期就能检查。

## 引用与指针

```cpp
double v = 5.0;
double& ref = v;      // 引用：别名，不可重绑定，必须初始化
double* ptr = &v;     // 指针：存地址，可重绑定，可为空

void scale(double& out, const double& in) { out = in * 2.0; }
```

- 传参不想拷贝用 `const&`（只读）或 `&`（要修改）
- 指针用于"可能没有值"的场景，用完判空
- 现代 C++ 优先引用，指针留给数组/动态内存

## auto 与类型推导

```cpp
auto v = glm::dvec3(1.0, 2.0, 3.0);   // 自动推导类型
for (auto& p : points) p *= 2.0;      // 要修改必须加 &，否则只改拷贝
```

陷阱：`auto` 会剥掉引用和 const——`auto p = getVec();` 是一次拷贝，大结构体用 `auto&` 或 `const auto&`。

## 容器：优先 std::vector / std::string

```cpp
#include <vector>
std::vector<double> xs(800 * 600);     // 预分配像素缓冲
xs.push_back(1.0);                      // 动态增长
xs.reserve(1024);                       // 预留容量避免反复分配

std::string s = "frame_" + std::to_string(i) + ".ppm";
```

C 风格数组（`double arr[100]`）只在尺寸编译期确定且固定时用；动态数据一律 vector。

## 结构体：自定义类型

```cpp
struct Vec3 {
    double x = 0, y = 0, z = 0;         // 默认成员初始化
    double length() const { return std::sqrt(x*x + y*y + z*z); }
};

Vec3 v{1.0, 2.0, 3.0};                  // 聚合初始化
double len = v.length();
```

## enum class：强类型枚举

```cpp
enum class Axis { X, Y, Z };            // 不会与 int 隐式混用
Axis a = Axis::Z;
```

## 踩坑记录

- **整数除法**：`1 / 2 == 0`。想得 0.5 写 `1.0 / 2` 或 `double(1) / 2`
- **unsigned 陷阱**：`for (size_t i = n - 1; i >= 0; --i)` 死循环——`size_t` 无符号，`i >= 0` 永远为真
- **浮点比较**：`a == b` 常失败，用 `std::fabs(a - b) < 1e-9`（见 phase1-GLM demo 的 `near()` 写法）
- **隐式转换**：int → double 无损，double → int 截断（`(int)3.7 == 3`），float → double 无益（精度已丢）
- **初始化**：局部基础类型不初始化是垃圾值；成员变量给默认值兜底
