---
title: "Visual Studio 编辑器"
---

# Overview 总览

```md
visual-studio/
├── 安装/
│   ├── IDE（Community）
│   └── Build Tools（命令行）
├── 项目形态/
│   ├── .sln 解决方案
│   └── CMake 项目
├── 常用快捷键/
│   ├── F5 / Ctrl+F5
│   ├── F9 / F10 / F11
│   ├── Ctrl+K Ctrl+C / Ctrl+K Ctrl+D
│   └── F12
├── 关键设置/
│   ├── UTF-8（/utf-8）
│   ├── /W4
│   └── x64
└── 调试技巧/
    ├── 断点 + 监视
    ├── 条件断点
    ├── 即时窗口
    ├── 内存窗口
    └── 像素涂纯色
```

# FAQ 常见问题

1. IDE 和 Build Tools 怎么选？

   日常开发用 Community IDE（官网下载，勾选「使用 C++ 的桌面开发」）；仅命令行/CI 用 winget install Microsoft.VisualStudio.2022.BuildTools。

2. .sln 和 CMake 项目有什么区别？

   .sln 是 VS 原生工程文件；CMake 项目用 CMakeLists.txt 描述构建，VS 直接打开目录。本项目 demos/phase1-GLM、tools/publisher-cpp 都是 CMake 项目。

3. Debug 和 Release 行为为什么不一样？

   Debug 变量初始化为特定模式、STL 越界检查会触发断言；性能测试只在 Release 做。

4. 中文字符串乱码甚至编译报错怎么办？

   源码统一 UTF-8，CMake 里对 MSVC 加 /utf-8。

5. 中文路径有问题吗？

   中文路径 + 旧工具链可能出问题：工程路径保持纯 ASCII（本项目 E:\Github\CG 即如此）。

6. 改了 CMakeLists 后 VS 里没生效？

   CMake 缓存过期：重新配置（或删 build 目录重来）。

7. 循环几百万次怎么调试？

   用条件断点按条件中断（如 x == 255）；或把像素涂成纯色（纯红/纯绿），一眼看出坐标与循环边界是否正确。
