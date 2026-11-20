# CG — 图形学学习与渲染引擎

从零重新学习计算机图形学：一边记笔记，一边写 demo，最终沉淀出一个小型渲染引擎。

## 目标

- **离线渲染器**：CPU 路径追踪（阶段 1 起步）
- **实时渲染引擎**：光栅化管线，先软件实现再上 GPU（阶段 2-3）
- **双后端统一**：同一场景既能光栅化渲染也能路径追踪（阶段 4）

## 在线预览

笔记站点由 [VitePress](https://vitepress.dev) 构建并部署到 GitHub Pages（公开仓库）：

➡️ **https://zhangxianjue-com.github.io/CG/**

笔记页面直接以 Markdown 维护在 `docs/notes/`，配图为 SVG。

## 路线图

| 阶段 | 内容 | 参考资料 |
|---|---|---|
| 0 | C++ 与数学基础 | 自写数学库 |
| 1 | CPU 光线追踪 | [Ray Tracing in One Weekend](https://raytracing.github.io/) 三部曲 |
| 2 | 软件光栅化 | [tinyrenderer](https://github.com/ssloy/tinyrenderer) |
| 3 | GPU 实时渲染 | [learnopengl.com](https://learnopengl.com/) |
| 4 | 引擎整合与进阶 | — |

详细进度看板见 [站点路线图页](https://zhangxianjue-com.github.io/CG/notes/roadmap.html)。

## 发布笔记站点

站点的构建与发布由终端小工具一键完成：**`tools/yushu-project-management.exe`**（源码在 `tools/publisher-cpp/`，C++20 + FTXUI，VS2022 编译）。

流程：

1. 维护 `docs/` 里的站点内容：页面在 `docs/notes/`（Markdown），配图 SVG 在 `docs/public/diagrams/`
2. 双击 `tools/yushu-project-management.exe` → 点「一键构建并发布」
   - VitePress 构建，`docs/scripts/publish-site.mjs` 把产物铺到 `docs/` 根目录
   - 提交 docs/ 并推送（GitHub Pages 直接部署 main 分支的 `docs/` 文件夹，约 1 分钟生效）

也可以命令行使用：`yushu-project-management.exe --cli all`（或 `build` / `publish` / `open` 单步）。

## 目录结构

```
CG/
├── docs/                    # VitePress 笔记站点（GitHub Pages 直接部署此目录）
│   ├── index.md             # 站点首页（构建后生成 index.html、assets/ 等产物，勿手改产物）
│   ├── notes/               # 笔记页面（.md 源 + 构建出的 .html）
│   ├── public/              # SVG 配图
│   ├── .vitepress/          # 站点配置 / 主题
│   └── scripts/             # publish-site.mjs（产物铺到 docs/）
├── tools/                   # yushu-project-management.exe（源码 publisher-cpp/）
├── prompt/                  # Claude Code 提示词
├── engine/                  # 渲染引擎库（静态库 cg），从 demo 共享代码中生长
│   ├── include/cg/          # 公共头文件（math/ core/ integrators/ raster/ io/）
│   └── src/
├── demos/                   # demo 系统
│   ├── phase0-cpp/          # C++ 基础源码
│   └── phase1-GLM/          # 3D 数学源码
└── third_party/             # 第三方依赖（通过 FetchContent / vcpkg 引入）
```

## 工作流规则

1. **站点页面直接在 `docs/notes/` 里维护**（Markdown 源），配图 SVG 放 `docs/public/diagrams/`，通过发布器一键构建发布
2. **代码下沉规则**：demo 里出现两次以上且稳定的代码 → 下沉到 `engine/`，demo 改为引用
3. 阶段 0-2 的 demo 是命令行程序，输出 PPM/PNG 图片，不依赖窗口

## 构建

**依赖**：CMake ≥ 3.24、C++20 编译器（推荐 Visual Studio 2022，含"使用 C++ 的桌面开发"工作负载）。

```bash
cmake --preset vs2022
cmake --build build/vs2022
build/vs2022/bin/Debug/01_math.exe
```

用其他工具链（MinGW/Ninja）时，在 [CMakePresets.json](CMakePresets.json) 里加一个对应的 configure preset 即可。
