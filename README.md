# CG — 图形学学习与渲染引擎

从零重新学习计算机图形学：一边记笔记，一边写 demo，最终沉淀出一个小型渲染引擎。

## 目标

- **离线渲染器**：CPU 路径追踪（阶段 1 起步）
- **实时渲染引擎**：光栅化管线，先软件实现再上 GPU（阶段 2-3）
- **双后端统一**：同一场景既能光栅化渲染也能路径追踪（阶段 4）

## 路线图

| 阶段 | 内容 | 参考资料 |
|---|---|---|
| 0 | C++ 与数学基础 | 自写数学库 |
| 1 | CPU 光线追踪 | [Ray Tracing in One Weekend](https://raytracing.github.io/) 三部曲 |
| 2 | 软件光栅化 | [tinyrenderer](https://github.com/ssloy/tinyrenderer) |
| 3 | GPU 实时渲染 | [learnopengl.com](https://learnopengl.com/) |
| 4 | 引擎整合与进阶 | — |

详细进度看板见 [notes/INDEX.md](notes/INDEX.md)。

## 目录结构

```
CG/
├── notes/                   # 笔记系统（一个领域一个目录，笔记即 Excalidraw 图）
│   ├── INDEX.md             # 进度看板
│   ├── _template.md         # 笔记模板（Excalidraw 版约定）
│   ├── 01-roadmap/          # 路线图（Excalidraw，与各笔记双向互链）
│   └── 02-math/             # 数学笔记：vec3.excalidraw · coordinate.excalidraw
├── engine/                  # 渲染引擎库（静态库 cg），从 demo 共享代码中生长
│   ├── include/cg/          # 公共头文件（math/ core/ integrators/ raster/ io/）
│   └── src/
├── demos/                   # demo 系统（一个 demo 一个可执行文件）
│   ├── phase0-basics/
│   ├── phase1-raytracing/
│   ├── phase2-rasterizer/
│   └── phase3-opengl/
└── third_party/             # 第三方依赖（通过 FetchContent / vcpkg 引入）
```

## 工作流规则

1. **一个主题 = 一张 `<topic>.excalidraw`（笔记即图解，不生成 .md，按领域归入 `notes/NN-area/`） + 一个 demo**
2. **代码下沉规则**：demo 里出现两次以上且稳定的代码 → 下沉到 `engine/`，demo 改为引用
3. **笔记与 [路线图](notes/01-roadmap/roadmap.excalidraw) 互链**：路线图主题卡片的 `link` 直接指向该主题的 `.excalidraw`
4. 阶段 0-2 的 demo 是命令行程序，输出 PPM/PNG 图片，不依赖窗口

## 构建

**依赖**：CMake ≥ 3.24、C++20 编译器（推荐 Visual Studio 2022，含"使用 C++ 的桌面开发"工作负载）。

```bash
cmake --preset vs2022
cmake --build build/vs2022
build/vs2022/bin/Debug/01_math.exe
```

用其他工具链（MinGW/Ninja）时，在 [CMakePresets.json](CMakePresets.json) 里加一个对应的 configure preset 即可。
