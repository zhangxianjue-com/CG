import { readFileSync } from 'node:fs'
import { defineConfig } from 'vitepress'

// 侧边栏清单由 scripts/render-notes.mjs 生成（先于 vitepress 构建运行）
const manifest = JSON.parse(
  readFileSync(new URL('./notes-manifest.json', import.meta.url), 'utf-8'),
)

export default defineConfig({
  base: '/CG/',
  // public/ 里的 .excalidraw 原文件不是 markdown 页面，跳过死链检查
  ignoreDeadLinks: [/\.excalidraw$/],
  lang: 'zh-CN',
  title: 'CG · 图形学学习笔记',
  description: '从零学习计算机图形学：路线图、笔记与渲染引擎',
  themeConfig: {
    nav: [
      { text: '首页', link: '/' },
      { text: '路线图', link: '/notes/roadmap' },
      { text: 'GitHub', link: 'https://github.com/zhangxianjue-com/CG' },
    ],
    sidebar: [
      { text: '开始', items: [{ text: '学习进度看板', link: '/' }] },
      ...manifest.sidebar,
    ],
    outline: { label: '本页目录', level: [2, 3] },
    docFooter: { prev: '上一篇', next: '下一篇' },
    lastUpdated: { text: '最近更新' },
    search: { provider: 'local' },
  },
})
