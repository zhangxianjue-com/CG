#!/usr/bin/env node
/**
 * render-notes.mjs — 把 notes/ 里的 .excalidraw 笔记转成 VitePress 内容：
 *   1. 用 @excalidraw/utils 在 Node 中离线渲染 SVG 图  → docs/public/diagrams/<相对路径>.svg
 *   2. 提取图内文字元素，生成 markdown 页面             → docs/notes/<相对路径>.md
 *   3. 复制原始 .excalidraw 文件供下载/编辑            → docs/public/notes/<相对路径>.excalidraw
 *   4. 生成侧边栏清单                                  → docs/.vitepress/notes-manifest.json
 *
 * 特殊处理：notes/01-roadmap/roadmap.excalidraw 生成路线图看板页
 * （阶段标题 + 卡片清单，卡片 link 指向对应笔记页面）。
 */
import { readFileSync, writeFileSync, mkdirSync, copyFileSync, rmSync, readdirSync, statSync, existsSync } from 'node:fs'
import { dirname, join, resolve, relative, sep, posix } from 'node:path'
import { fileURLToPath } from 'node:url'
import { JSDOM } from 'jsdom'
import { exportScene } from '@excalidraw/utils'

const DOCS = resolve(dirname(fileURLToPath(import.meta.url)), '..')
const ROOT = resolve(DOCS, '..')
const NOTES = join(ROOT, 'notes')
const PUBLIC_NOTES = join(DOCS, 'public', 'notes')
const PUBLIC_DIAGRAMS = join(DOCS, 'public', 'diagrams')
const PAGES = join(DOCS, 'notes')
const MANIFEST = join(DOCS, '.vitepress', 'notes-manifest.json')

// 领域目录 → 侧边栏分组名
const GROUP_TITLES = {
  '01-roadmap': '学习路线',
  '02-math': '数学基础',
  '03-renderer': '渲染器原理',
}

// 代码字体（Excalidraw fontFamily: 3 = Cascadia）
const CODE_FONT = 3

const log = (...a) => console.log('[render-notes]', ...a)

// ---------- 工具 ----------

function walk(dir, out = []) {
  for (const ent of readdirSync(dir, { withFileTypes: true })) {
    const p = join(dir, ent.name)
    if (ent.isDirectory()) walk(p, out)
    else if (ent.isFile() && ent.name.endsWith('.excalidraw')) out.push(p)
  }
  return out
}

const toPosix = (p) => p.split(sep).join('/')

function escapeHtml(s) {
  return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
}

/** 取笔记标题：字体最大的自由文本（并列取 y 最小者） */
function findTitle(elements, fallback) {
  let best = null
  for (const e of elements) {
    if (e.type !== 'text' || e.containerId || !e.text || !String(e.text).trim()) continue
    const fs = e.fontSize ?? 20
    if (!best || fs > best.fontSize || (fs === best.fontSize && e.y < best.y)) best = e
  }
  return best ? String(best.text).trim() : fallback
}

// ---------- SVG 渲染 ----------

async function renderSvg(scene) {
  const dom = new JSDOM('<!doctype html><html><body></body></html>')
  const svgEl = await exportScene({
    type: 'svg',
    data: {
      elements: scene.elements ?? [],
      appState: scene.appState ?? {},
      files: scene.files ?? null,
    },
    config: {
      canvasBackgroundColor: '#ffffff',
      theme: 'light',
      skipInliningFonts: true, // 不联网内嵌字体，中文依赖系统字体回退
      padding: 12,
      environment: { document: dom.window.document },
    },
  })
  let svg = svgEl.outerHTML
  if (!svg.includes('xmlns')) {
    svg = svg.replace(/^<svg/, '<svg xmlns="http://www.w3.org/2000/svg"')
  }
  return svg
}

// ---------- 文字 → markdown ----------

function isHeadingText(t) {
  // 含数学符号的公式/注释行（如 |v| = √(3²+4²)、v = (3, 4, 0)）不当标题
  const mathy = /[=√×÷|]/.test(t.text)
  if (t.fs >= 20 && !mathy) return 2 // ##
  if (t.fs >= 17 && !(mathy && t.text.length > 8)) return 3 // ###
  return 0
}

/** 坐标轴标签等噪音：单个拉丁/希腊字符（含上标变音符），或 "x = 3" 之类的轴刻度 */
function isAxisNoise(t) {
  if (t.length <= 1) return true
  if (/^[A-Za-z0-9θφ]\p{M}*$/u.test(t)) return true
  if (/^[A-Za-zθφ]\s*=/.test(t) && t.length <= 6) return true
  return false
}

/** 估算文本宽度（excalidraw 文件的 width 字段不可信）：CJK 1em，其余 0.55em，多行取最长行 */
function estWidth(text, fs) {
  let max = 0
  for (const line of text.split('\n')) {
    let w = 0
    for (const ch of line) {
      w += /[⺀-鿿豈-﫿＀-￯]/.test(ch) ? fs : fs * 0.55
    }
    max = Math.max(max, w)
  }
  return max
}

/** 按 y 聚类成行，再按 x 间距拆成列簇 */
function cluster(elements, title) {
  const texts = elements
    .filter(
      (e) =>
        e.type === 'text' &&
        e.text &&
        String(e.text).trim() &&
        !e.containerId &&
        !isAxisNoise(String(e.text).trim()),
    )
    .map((e) => {
      const text = String(e.text).trim()
      const fs = e.fontSize ?? 20
      return {
        x: e.x ?? 0,
        y: e.y ?? 0,
        w: estWidth(text, fs),
        fs,
        font: e.fontFamily ?? 2,
        text,
      }
    })
    .sort((a, b) => a.y - b.y || a.x - b.x)

  const rows = []
  for (const t of texts) {
    if (t.text === title) continue // 标题由页面 h1 承担
    const last = rows[rows.length - 1]
    if (last && Math.abs(t.y - last.anchorY) <= 26) last.items.push(t)
    else rows.push({ anchorY: t.y, items: [t] })
  }
  const X_GAP = 140
  return rows.map((row) => {
    const sorted = [...row.items].sort((a, b) => a.x - b.x)
    const cols = []
    for (const t of sorted) {
      const last = cols[cols.length - 1]
      if (last && t.x - last.right <= X_GAP) {
        last.items.push(t)
        last.right = Math.max(last.right, t.x + t.w)
      } else {
        cols.push({ items: [t], right: t.x + t.w })
      }
    }
    return cols
  })
}

/** 单元格：先转义 HTML，再转义表格管道符，换行改 <br> */
function escapeCell(s) {
  return escapeHtml(s).replace(/\|/g, '\\|').replace(/\n/g, '<br>')
}

/** 生成非路线图笔记页正文（标题 + 图文要点） */
function extractMarkdown(elements, title) {
  const rows = cluster(elements, title)
  const out = []
  let tableCols = 0
  let tableRows = [] // 单元格行（第一行为表头）
  let para = [] // 连续单列段落行

  const flushPara = () => {
    if (para.length) {
      out.push(para.join('<br>'))
      para = []
    }
  }
  const flushTable = () => {
    if (!tableCols) return
    if (tableRows.length === 1) {
      // 只有表头没有内容的表格 → 退化为一行段落
      out.push(tableRows[0].join(' · ').replace(/\\\|/g, '|').replace(/<br>/g, ' '))
    } else {
      out.push(`| ${tableRows[0].join(' | ')} |`)
      out.push(`|${' --- |'.repeat(tableCols)}`)
      for (const r of tableRows.slice(1)) out.push(`| ${r.join(' | ')} |`)
      out.push('')
    }
    tableCols = 0
    tableRows = []
  }

  for (const cols of rows) {
    // 代码块（Cascadia 字体行）
    if (cols.length === 1 && cols[0].items[0].font === CODE_FONT) {
      flushPara()
      flushTable()
      out.push('```txt')
      out.push(cols[0].items.map((t) => t.text).join('\n'))
      out.push('```')
      out.push('')
      continue
    }

    if (cols.length === 1) {
      const c = cols[0]
      const joined = c.items.map((t) => t.text).join(' ')
      const heading = isHeadingText(c.items[0])
      if (heading) {
        flushPara()
        flushTable()
        out.push(`${'#'.repeat(heading)} ${escapeHtml(joined)}`)
        out.push('')
      } else if (joined.includes('\n') && c.items.length === 1) {
        flushPara()
        flushTable()
        out.push(escapeHtml(joined).replace(/\n/g, '<br>'))
        out.push('')
      } else {
        flushTable()
        para.push(escapeHtml(joined).replace(/\n/g, ' '))
      }
      continue
    }

    // 多列行 → 表格；其中含大字号标题时合并成一行标题
    const hasHeading = cols.some((c) => isHeadingText(c.items[0]))
    if (hasHeading) {
      flushPara()
      flushTable()
      const texts = cols.map((c) => c.items.map((t) => t.text).join(' '))
      out.push(`### ${escapeHtml(texts.join(' · '))}`)
      out.push('')
      continue
    }
    flushPara()
    const cells = cols.map((c) => escapeCell(c.items.map((t) => t.text).join(' ')))
    if (tableCols === cols.length) {
      tableRows.push(cells)
    } else {
      flushTable()
      tableCols = cols.length
      tableRows = [cells]
    }
  }
  flushPara()
  flushTable()
  return out.join('\n').replace(/\n{3,}/g, '\n\n').trim()
}

// ---------- 路线图看板 ----------

function generateRoadmap(scene, title) {
  const elements = scene.elements ?? []
  // 阶段标题：fs>=20 的自由文本（排除页面标题与图例）
  const stages = elements
    .filter((e) => e.type === 'text' && !e.containerId && (e.fontSize ?? 20) >= 20 && String(e.text).trim() !== title)
    .sort((a, b) => a.y - b.y)
  // 卡片：带绑定文本的矩形（link 可为空）
  const cards = []
  for (const e of elements) {
    if (e.type !== 'rectangle') continue
    const label = elements.find((t) => t.type === 'text' && t.containerId === e.id)
    if (!label) continue
    cards.push({ y: label.y, x: label.x, text: String(label.text).trim(), link: e.link ?? null })
  }
  cards.sort((a, b) => a.y - b.y || a.x - b.x)

  const out = [`# ${title}`, '']
  out.push(`<Diagram src="/diagrams/01-roadmap/roadmap.svg" alt="${escapeHtml(title)}" source="/notes/01-roadmap/roadmap.excalidraw" />`)
  out.push('')

  let si = 0
  const stageOf = (y) => {
    while (si < stages.length - 1 && y >= stages[si + 1].y) si++
    return si < stages.length ? si : -1
  }
  let lastStage = -1
  for (const card of cards) {
    const s = stageOf(card.y)
    if (s !== lastStage && s >= 0) {
      out.push(`## ${String(stages[s].text).trim()}`)
      out.push('')
      lastStage = s
    }
    const done = /^✓/.test(card.text)
    const name = card.text.replace(/^✓\s*/, '')
    const page = card.link
      ? linkToPage(card.link, 'notes/01-roadmap/roadmap.excalidraw')
      : null
    out.push(`- [${done ? 'x' : ' '}]${page ? ` [${name}](${page})` : ` ${name}`}`)
  }
  return out.join('\n').trim()
}

/** 卡片 link（相对 .excalidraw 所在目录）→ 生成页面地址 */
function linkToPage(link, fromRel) {
  const fromDir = dirname(fromRel)
  const target = posix.normalize(posix.join(fromDir, link)).replace(/\.excalidraw$/, '')
  return `/${target}` // 形如 /notes/02-math/coordinate
}

// ---------- 主流程 ----------

async function main() {
  // notes/ 已从仓库移除：目录不存在时跳过生成，保留已有页面与侧边栏清单；
  // 以后把 notes/ 放回仓库根目录即可恢复「笔记 → 站点」的自动转换。
  const files = existsSync(NOTES) ? walk(NOTES) : []
  log(`发现 ${files.length} 个 .excalidraw 文件`)
  if (files.length === 0) {
    log('notes/ 目录不存在，跳过页面生成（保留已有页面与清单）')
    return
  }

  rmSync(PAGES, { recursive: true, force: true })
  rmSync(PUBLIC_NOTES, { recursive: true, force: true })
  rmSync(PUBLIC_DIAGRAMS, { recursive: true, force: true })
  mkdirSync(PAGES, { recursive: true })
  mkdirSync(PUBLIC_NOTES, { recursive: true })
  mkdirSync(PUBLIC_DIAGRAMS, { recursive: true })

  const groups = new Map() // dir -> [{text, link}]
  let failures = 0

  for (const file of files) {
    const rel = toPosix(relative(NOTES, file)) // 如 02-math/coordinate.excalidraw
    const relNoExt = rel.replace(/\.excalidraw$/, '')
    const [dir, name] = [dirname(rel), relNoExt.replace(/\.[^.]*$/, '')]
    const pageRel = rel === '01-roadmap/roadmap.excalidraw' ? 'roadmap' : relNoExt
    try {
      const scene = JSON.parse(readFileSync(file, 'utf-8'))
      const title = findTitle(scene.elements ?? [], name)

      // 1) SVG
      const svg = await renderSvg(scene)
      const svgPath = join(PUBLIC_DIAGRAMS, `${relNoExt}.svg`)
      mkdirSync(dirname(svgPath), { recursive: true })
      writeFileSync(svgPath, svg, 'utf-8')
      log(`SVG  ${rel} → public/diagrams/${relNoExt}.svg (${(svg.length / 1024).toFixed(0)} KB)`)

      // 2) 原文件
      const rawPath = join(PUBLIC_NOTES, rel)
      mkdirSync(dirname(rawPath), { recursive: true })
      copyFileSync(file, rawPath)

      // 3) markdown 页面
      const isRoadmap = rel === '01-roadmap/roadmap.excalidraw'
      const srcNote = `\n---\n\n> 📝 本页由脚本从 [\`notes/${rel}\`](https://github.com/zhangxianjue-com/CG/blob/main/notes/${rel}) 自动生成。\n> 原图：[${rel}](/notes/${rel})（可用 [Excalidraw](https://excalidraw.com) 或 VSCode 扩展打开编辑）`
      let md
      if (isRoadmap) {
        // 路线图页自身就是完整页面（标题 + 图 + 看板清单）
        md = `---\ntitle: ${JSON.stringify(title)}\n---\n\n${generateRoadmap(scene, title)}${srcNote}\n`
      } else {
        const body = extractMarkdown(scene.elements ?? [], title)
        md = `---\ntitle: ${JSON.stringify(title)}\n---\n\n# ${title}\n\n<Diagram src="/diagrams/${relNoExt}.svg" alt="${escapeHtml(title)}" source="/notes/${rel}" />\n\n## 图文要点\n\n${body}${srcNote}\n`
      }
      const mdPath = join(PAGES, `${pageRel}.md`)
      mkdirSync(dirname(mdPath), { recursive: true })
      writeFileSync(mdPath, md, 'utf-8')
      log(`MD   ${rel} → notes/${pageRel}.md`)

      // 4) 侧边栏清单
      if (!groups.has(dir)) groups.set(dir, [])
      groups.get(dir).push({ text: title, link: `/notes/${pageRel}` })
    } catch (err) {
      failures++
      console.error(`[render-notes] ✗ 处理 ${rel} 失败：`, err)
    }
  }

  const sidebar = [...groups.entries()]
    .sort(([a], [b]) => a.localeCompare(b))
    .map(([dir, items]) => ({
      text: GROUP_TITLES[dir] ?? dir,
      items: items.sort((a, b) => a.link.localeCompare(b.link)),
    }))
  writeFileSync(MANIFEST, JSON.stringify({ sidebar }, null, 2), 'utf-8')
  log(`清单已写入 .vitepress/notes-manifest.json（${sidebar.length} 个分组）`)
  if (failures) {
    console.error(`[render-notes] 有 ${failures} 个文件处理失败`)
    process.exitCode = 1
  }
}

main()
