import { defineConfig } from 'vitepress'

export default defineConfig({
  base: '/CG/',
  lang: 'zh-CN',
  title: 'CG · 图形学学习笔记',
  description: '从零学习计算机图形学：路线图、笔记与渲染引擎',
  themeConfig: {
    nav: [
      { text: '首页', link: '/' },
      { text: '路线图', link: '/notes/roadmap' },
      { text: '工具', link: '/tools/publisher' },
      { text: 'GitHub', link: 'https://github.com/zhangxianjue-com/CG' },
    ],
    sidebar: [
      {
        text: '学习路线',
        items: [{ text: 'CG · 图形学学习路线图', link: '/notes/roadmap' }],
      },
      {
        text: '基础知识',
        items: [
          { text: '渲染器原理', link: '/notes/03-renderer/renderer' },
          {
            text: '数学基础',
            items: [
              { text: '坐标系', link: '/notes/02-math/coordinate' },
              { text: '向量与矩阵', link: '/notes/02-math/vector-matrix' },
            ],
          },
          {
            text: 'CPP 编程语言',
            items: [
              { text: 'Visual Studio 编辑器', link: '/notes/01-cpp/visual-studio' },
              { text: '数据类型', link: '/notes/01-cpp/data-types' },
              { text: '面向对象编码', link: '/notes/01-cpp/oop' },
              { text: 'GLM 数学库', link: '/notes/01-cpp/glm' },
            ],
          },
        ],
      },
      {
        text: 'Unreal Engine 游戏引擎',
        items: [
          {
            text: 'Game Animation 游戏动画',
            items: [
              { text: 'Motion Matching 运行匹配', link: '/notes/04-unreal/game-animation/motion-matching' },
            ],
          },
        ],
      },
      {
        text: '工具',
        items: [{ text: 'yushu-project-management', link: '/tools/publisher' }],
      },
    ],
    outline: { label: '本页目录', level: [2, 3] },
    docFooter: { prev: '上一篇', next: '下一篇' },
    lastUpdated: { text: '最近更新' },
    search: { provider: 'local' },
  },
})
