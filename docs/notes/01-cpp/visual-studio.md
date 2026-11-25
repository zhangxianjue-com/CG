---
title: "Visual Studio 编辑器"
---

> Visual Studio 是 Windows 上的主力 C++ 开发环境 IDE，除了有点重，没啥缺点。

# Overview 总览

```md
visual-studio/
├── 安装/
│ ├── 下载: https://visualstudio.microsoft.com/zh-hans/downloads/
│ └── IDE（Community 免费）
└── END
```

# FAQ 常见问题

1. VS 在哪里下载？

   ::: details 查看答案
   官方下载页：https://visualstudio.microsoft.com/zh-hans/downloads/，个人/学习用免费的 **Community** 版即可。
   1. 打开下载页 → 在「下载 Visual Studio」下选 Community
   2. 运行安装器 vs_Community.exe
   3. 勾选工作负载「使用 C++ 的桌面开发」→ 安装

   :::

2. 安装时要勾选哪些组件？

   ::: details 查看答案
   核心只勾一个工作负载：**「使用 C++ 的桌面开发」**。

   右侧「安装详细信息」默认包含（不够再说）：
   1. MSVC v143 编译器工具集
   2. Windows SDK
   3. 适用于 Windows 的 C++ CMake 工具

   安装位置默认 C 盘即可；磁盘紧张时「安装位置」可改到其他盘，但只能省下主 IDE + 工作负载的体积——Installer 本体、.NET SDK、部分 Windows SDK/运行时组件仍固定装在 C 盘。
   :::
