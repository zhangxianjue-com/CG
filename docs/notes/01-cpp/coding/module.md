---
title: "模块化编程"
---

> C++20 模块用 `import`/`export` 取代 `#include` 的文本粘贴：编译快、宏不泄漏、不依赖顺序——与头文件可混用，但生态仍在迁移中。

# Markmap 知识体系

```md
module/
├── 概念/
│ ├── 与头文件的区别（文本粘贴 vs 编译期接口）
│ └── 与第三方库的关系（来源 vs 形式）
├── 语法/
│ ├── export module（模块接口声明）
│ ├── export（导出函数/类）
│ └── import（导入）
├── 工程配置/
│ ├── VS 2026 原生支持（.ixx 接口文件）
│ └── CMake 3.28+ CXX_MODULES（实验性）
└── END
```

# FAQ 常见问题

1. 模块和头文件有什么区别？

   ::: details 查看答案
   头文件是**文本粘贴**：`#include` 把头文件内容复制进每个 .cpp 再各自编译——宏会泄漏、依赖 include 顺序、每份都要重编一遍。

   模块是**编译期接口**：模块编译一次生成接口描述（BMI），导入方直接用——编译更快、宏不泄漏、不依赖顺序、接口与实现隔离。

   一句话：头文件是「抄写」，模块是「引用编译好的接口」。
   :::

2. 模块语法怎么写？

   ::: details 查看答案

   ```cpp
   // math.ixx —— 模块接口单元
   export module math;                 // 声明模块
   export int add(int a, int b) {      // export 导出接口
     return a + b;
   }

   // main.cpp
   import math;                        // 导入自己的模块
   import <iostream>;                  // 标准库也能 import
   int main() {
     std::cout << add(1, 2);           // 3
   }
   ```

   要点：`export module 名字;` 声明模块；`export` 标记对外可见的接口；`import` 导入。未 export 的内容模块外部不可见。
   :::

3. 怎么在工程里启用？

   ::: details 查看答案
   1. **VS 2022 17.5+ / VS 2026**：原生支持，接口文件用 `.ixx` 扩展名（编译器自动识别），其余按普通源文件编译
   2. **CMake**：工程声明 C++20（`cxx_std_20`）；3.28+ 提供实验性的 CXX_MODULES 源文件声明
   3. 命令行 MSVC：`cl /std:c++latest /c math.ixx` 先编译接口，再用 `/reference` 传给依赖它的单元（VS 工程自动处理）

   本项目 demos 暂未使用模块，仍以头文件方式组织。
   :::

4. 模块和 #include 能混用吗？

   ::: details 查看答案
   能。同一个工程里 `import` 与 `#include` 可以共存：依赖头文件库照旧 include，自己的新代码用模块；也可以在头文件库外面包一层模块接口（`module;` 全局模块片段 + export），给老库提供模块化入口。
   :::

5. 为什么现在第三方库还都是头文件？

   ::: details 查看答案
   生态迁移慢：主流库（boost 等）的模块化还在进行中；构建系统、工具链、教程资料都在追赶。所以今天实际工程仍是「自己的新代码用模块、外部依赖 #include」的混合状态，两种形式会长期共存。
   :::
