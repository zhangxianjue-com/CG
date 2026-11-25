#!/usr/bin/env node
/**
 * publish-site.mjs — 准备 .vitepress/dist 作为 gh-pages 分支的推送内容。
 * 部署方式：GitHub Pages 部署 gh-pages 分支（根目录），main 分支只保留源文件。
 * 发布器（tools/publisher-cpp）会把 dist 产物 force-add 进一个临时提交，
 * 推送到 gh-pages 分支后撤销该提交（git reset），main 不受影响。
 *
 * 不能把 VitePress 的 outDir 直接设成 docs/：VitePress 构建强制 emptyOutDir，
 * 会清空整个 docs/（连源文件一起删掉）。所以始终构建到 .vitepress/dist，
 * 本脚本只负责保证 dist/.nojekyll 存在（Pages 部署 gh-pages 根目录时，
 * 没有它 Jekyll 会排除点开头目录导致 404）。
 */
import { existsSync, writeFileSync } from 'node:fs'
import { dirname, join, resolve } from 'node:path'
import { fileURLToPath } from 'node:url'

const DOCS = resolve(dirname(fileURLToPath(import.meta.url)), '..')
const DIST = join(DOCS, '.vitepress', 'dist')

if (!existsSync(join(DIST, 'index.html'))) {
  console.error('[publish-site] ✗ 未找到构建产物，请先运行 vitepress build')
  process.exit(1)
}

// 保证 .nojekyll 存在：gh-pages 部署时跳过 Jekyll，否则 .vitepress 等点开头目录无法访问
const nojekyll = join(DIST, '.nojekyll')
if (!existsSync(nojekyll)) writeFileSync(nojekyll, '')

console.log('[publish-site] ✓ dist 已就绪（含 .nojekyll），可由发布器推送到 gh-pages')
