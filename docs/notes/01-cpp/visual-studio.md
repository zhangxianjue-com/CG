---
title: "Visual Studio 编辑器"
---

# Visual Studio 编辑器

C++ 开发的主力环境。本机已安装 **VS 2022 Build Tools**（命令行编译），完整 IDE 用 Community 版（个人免费）。

## 安装

- **IDE**：官网下载 Visual Studio 2022 Community，勾选「使用 C++ 的桌面开发」工作负载
- **仅编译**（命令行/CI）：`winget install Microsoft.VisualStudio.2022.BuildTools`，加装 C++ 工作负载后自带 `cl.exe` + MSBuild + CMake

## 项目形态

| 形态 | 说明 | 本项目 |
| --- | --- | --- |
| .sln 解决方案 | VS 原生工程文件 | — |
| CMake 项目 | 用 `CMakeLists.txt` 描述构建，VS 直接打开目录 | `demos/phase1-GLM`、`tools/publisher-cpp` |

命令行构建（不依赖 IDE）：

```bash
cmake --preset vs2022
cmake --build build/vs2022 --config Release
```

## 常用快捷键

| 快捷键 | 作用 |
| --- | --- |
| F5 | 启动调试 |
| Ctrl+F5 | 运行不调试 |
| Ctrl+B / F7 | 生成 |
| F9 | 断点 |
| F10 / F11 | 单步跳过 / 单步进入 |
| Shift+F11 | 跳出当前函数 |
| Ctrl+K Ctrl+C | 注释选中行 |
| Ctrl+K Ctrl+D | 格式化文档 |
| F12 | 跳转到定义 |

## 关键设置

- **编码**：源码统一 UTF-8，CMake 里对 MSVC 加 `/utf-8`，否则中文字符串乱码甚至编译报错
- **警告等级**：至少 `/W4`，配合 `-WX` 把警告当错误
- **架构**：确认选 x64（x86 下 32 位指针和整型会带来隐蔽 bug）

## 调试技巧（图形学常用）

1. **断点 + 监视**：把 `vec3` 等变量加进监视窗口，展开看分量
2. **条件断点**：循环几百万次时按条件中断（如 `x == 255`）
3. **即时窗口**：断点处随手执行表达式验证猜想
4. **内存窗口**：看缓冲区的原始字节，排查像素/坐标错位
5. **把像素涂纯色**：输出纯红/纯绿图片，一眼看出坐标与循环边界是否正确（见[渲染器原理](/notes/03-renderer/renderer)踩坑）

## 踩坑记录

- Debug 与 Release 行为差异：Debug 变量初始化为特定模式、STL 越界检查会触发断言；性能测试只在 Release 做
- 中文路径 + 旧工具链可能出问题：工程路径保持纯 ASCII（本项目 `E:\Github\CG` 即如此）
- 修改 CMakeLists 后 VS 里的 CMake 缓存可能过期：重新配置（或删 build 目录重来）
