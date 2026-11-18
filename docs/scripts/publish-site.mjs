#!/usr/bin/env node
/**
 * publish-site.mjs — 把 .vitepress/dist 的构建产物铺到 docs/ 根目录。
 * 部署方式：GitHub Pages 直接部署 main 分支的 /docs 文件夹，
 * 因此站点产物（index.html、assets、diagrams、notes/*.html…）需要落在 docs/ 根，
 * 与 VitePress 源文件（*.md、public/、.vitepress/、scripts/）共存。
 *
 * 不能把 VitePress 的 outDir 直接设成 docs/：VitePress 构建强制 emptyOutDir，
 * 会清空整个 docs/（连源文件一起删掉）。所以先构建到 .vitepress/dist，
 * 再由本脚本清理上次产物后整目录复制过去。
 */
import { cpSync, existsSync, readdirSync, rmSync, writeFileSync } from 'node:fs'
import { dirname, join, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'

const DOCS = resolve(dirname(fileURLToPath(import.meta.url)), '..')
const DIST = join(DOCS, '.vitepress', 'dist')

// 上次构建的生成物（notes/ 目录只清 *.html，*.md 是源文件不能动）
for (const p of ['index.html', '404.html', 'hashmap.json']) {
  rmSync(join(DOCS, p), { force: true })
}
for (const d of ['assets', 'diagrams']) {
  rmSync(join(DOCS, d), { recursive: true, force: true })
}
const notesDir = join(DOCS, 'notes')
if (existsSync(notesDir)) {
  for (const ent of readdirSync(notesDir)) {
    if (ent.endsWith('.html')) {
      rmSync(join(notesDir, ent), { force: true })
    }
  }
}

if (!existsSync(join(DIST, 'index.html'))) {
  console.error('[publish-site] ✗ 未找到构建产物，请先运行 vitepress build')
  process.exit(1)
}
cpSync(DIST, DOCS, { recursive: true })

// 保证 .nojekyll 存在：Pages 部署 /docs 时跳过 Jekyll，否则 .vitepress 等点开头目录无法访问
const nojekyll = join(DOCS, '.nojekyll')
if (!existsSync(nojekyll)) writeFileSync(nojekyll, '')

console.log('[publish-site] ✓ 构建产物已铺到 docs/ 根目录')
