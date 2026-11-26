import { defineConfig } from "vitepress";

export default defineConfig({
  base: "/CG/",
  lang: "zh-CN",
  title: "CG · 图形学学习笔记",
  description: "从零学习计算机图形学：路线图、笔记与渲染引擎",
  themeConfig: {
    nav: [
      { text: "首页", link: "/" },
      { text: "路线图", link: "/notes/roadmap" },
      { text: "工具", link: "/tools/publisher" },
      { text: "GitHub", link: "https://github.com/zhangxianjue-com/CG" },
    ],
    sidebar: [
      {
        text: "学习路线",
        items: [{ text: "CG · 图形学学习路线图", link: "/notes/roadmap" }],
      },
      {
        text: "基础知识",
        items: [
          { text: "渲染器原理", link: "/notes/03-renderer/renderer" },
          {
            text: "数学基础",
            items: [
              { text: "坐标系", link: "/notes/02-math/coordinate" },
              { text: "向量与矩阵", link: "/notes/02-math/vector-matrix" },
            ],
          },
          {
            text: "CPP 编程语言",
            items: [
              {
                text: "代码编辑器",
                items: [
                  { text: "Visual Studio 编辑器", link: "/notes/01-cpp/editor/visual-studio" },
                  { text: "VS Code 编辑器", link: "/notes/01-cpp/editor/vs-code" },
                ],
              },
              {
                text: "编码",
                items: [
                  { text: "编译器", link: "/notes/01-cpp/coding/compiler" },
                  { text: "数据类型", link: "/notes/01-cpp/coding/data-types" },
                  { text: "语句", link: "/notes/01-cpp/coding/statement" },
                  { text: "面向对象编程", link: "/notes/01-cpp/coding/oop" },
                  { text: "模块化编程", link: "/notes/01-cpp/coding/module" },
                ],
              },

              {
                text: "项目构建",
                items: [
                  { text: "CMake 构建工具", link: "/notes/01-cpp/build/cmake" },
                  { text: "vcpkg 依赖库管理工具", link: "/notes/01-cpp/build/vcpkg" },
                ],
              },
              {
                text: "第三方库（协同合作）",
                items: [{ text: "GLM 数学库", link: "/notes/01-cpp/library/glm" }],
              },
            ],
          },
        ],
      },
      {
        text: "工具",
        items: [{ text: "yushu-project-management", link: "/tools/publisher" }],
      },
    ],
    outline: { label: "本页目录", level: [2, 3] },
    docFooter: { prev: "上一篇", next: "下一篇" },
    lastUpdated: { text: "最近更新" },
    search: { provider: "local" },
  },
});
