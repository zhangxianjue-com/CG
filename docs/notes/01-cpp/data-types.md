---
title: "数据类型"
---

# Markmap 知识体系

```md
数据类型/
├── 基本类型/
│   ├── bool
│   ├── char
│   ├── short
│   ├── int
│   ├── long long
│   ├── float
│   └── double
├── 常量/
│   ├── const
│   └── constexpr
├── 引用与指针/
│   ├── 引用
│   └── 指针
├── auto 类型推导
├── 容器/
│   ├── std::vector
│   └── std::string
├── 结构体 struct
└── enum class
```

# FAQ 常见问题

1. CPU 端用 float 还是 double？为什么？

   CPU 用 double（约 16 位有效数字），上传 GPU 时再转 float。float 只有约 7 位，光线追踪数百万次乘加的累积误差会以噪点、命中偏移的形式出现；GPU 显存和带宽有限所以仍用 float。

2. 字面量后缀 3.14f、42u、42L 分别是什么？

   3.14f 是 float、42u 是 unsigned、42L 是 long；无后缀的 3.14 是 double。

3. const 和 constexpr 有什么区别？

   const 是运行期常量（值不可改）；constexpr 是编译期常量，可用于数组大小。屏幕尺寸、采样数、材质参数尽量用 constexpr，编译期就能检查。

4. 引用和指针有什么区别？什么时候用哪个？

   引用是别名：不可重绑定、必须初始化；指针存地址：可重绑定、可为空。传参不想拷贝用 const&（只读）或 &（要修改）；指针留给「可能没有值」和数组/动态内存的场景。

5. auto 推导有什么陷阱？

   auto 会剥掉引用和 const：auto p = getVec(); 是一次拷贝，大结构体要用 auto& 或 const auto&。

6. 为什么动态数据优先用 std::vector？

   自动管理内存、动态增长，reserve 可预留容量避免反复分配；C 风格数组只在尺寸编译期确定且固定时用。

7. 为什么 1 / 2 等于 0？

   整数除法。想得 0.5 写 1.0 / 2 或 double(1) / 2。

8. 为什么 for (size_t i = n - 1; i >= 0; --i) 会死循环？

   size_t 无符号，i >= 0 永远为真。

9. 浮点数怎么比较相等？

   别用 ==，用 std::fabs(a - b) < 1e-9（见 phase1-GLM demo 的 near() 写法）。

10. 隐式转换会丢精度吗？

    int → double 无损；double → int 截断（(int)3.7 == 3）；float → double 无益（精度已丢）。
