---
title: "面向对象编码"
---

# 面向对象编码

渲染引擎天然适合面向对象：向量、材质、相机、渲染器都是"数据 + 行为"的对象。核心原则：**用封装管住状态，用 RAII 管住资源，用多态扩展行为**。

## class 与封装

```cpp
class Vec3 {
public:                                  // 对外接口
    Vec3(double x, double y, double z) : x_(x), y_(y), z_(z) {}
    double length() const { return std::sqrt(x_*x_ + y_*y_ + z_*z_); }

private:                                 // 内部状态不对外暴露
    double x_, y_, z_;
};
```

- `public` 放接口，`private` 藏实现——外部改不了 `x_` 才能保证不变量（如"法线永远是单位向量"）
- 成员函数末尾的 `const` 承诺不修改对象：能加就加

## 构造 / 析构与 RAII

```cpp
class Image {
public:
    Image(int w, int h) : w_(w), h_(h), pixels_(w * h) {}   // 构造时分配
    // 析构自动释放：不需要手动 delete，离开作用域必然执行
private:
    int w_, h_;
    std::vector<unsigned char> pixels_;
};
```

**RAII（资源获取即初始化）**：资源（内存、文件、锁）的申请和释放绑在对象的构造/析构上。用 `std::vector`、`std::string`、`std::unique_ptr` 代替裸 `new`/`delete`，泄漏和双重释放基本绝迹。

## 头文件 / 源文件分离

```
vec3.h    ← 声明：类长什么样（include 这个）
vec3.cpp  ← 定义：实现放这里（只编译一次）
```

- 头文件用 `#pragma once` 防重复包含
- 声明放 `.h`、实现放 `.cpp`，编译更快，依赖更清晰

## 继承与多态

```cpp
class Renderer {
public:
    virtual ~Renderer() = default;                    // 基类析构必须 virtual
    virtual void render(const Scene& scene) = 0;      // 纯虚 = 接口
};

class Rasterizer : public Renderer {                  // 光栅化实现
public:
    void render(const Scene& scene) override;         // override 防手滑
};

class PathTracer : public Renderer {                  // 路径追踪实现
public:
    void render(const Scene& scene) override;
};

std::unique_ptr<Renderer> r = std::make_unique<PathTracer>();
r->render(scene);                                     // 运行期分派到 PathTracer
```

图形学的经典用法：同一份场景，光栅化和光线追踪是两个 `Renderer` 子类（对应路线图阶段 4 的"双后端对照渲染"）。

## 值语义 vs 引用语义

```cpp
Vec3 a = b;        // 值语义：完整拷贝（数学对象就该这样）
auto& ref = b;     // 引用语义：别名，改它就改原对象
```

向量、矩阵、颜色用值语义（小而自包含）；纹理、网格等大资源用指针/引用共享，配合 `shared_ptr` 管理生命周期。

## 踩坑记录

- **基类析构不加 virtual**：`Renderer* p = new Rasterizer; delete p;` 只调基类析构 → 子类资源泄漏
- **对象切片**：`Renderer r = rasterizer;`（值拷贝到基类对象）会丢掉子类部分，多态失效
- **名字隐藏**：子类定义 `foo(int)` 会把基类所有 `foo` 重载藏起来，需要 `using Base::foo;`
- **初始化顺序**：成员按声明顺序初始化，不是按构造函数初始化列表顺序——列表顺序和声明顺序保持一致
- **虚函数里别调虚函数**（构造/析构中）：此时子类还没构造完，调的是基类版本
