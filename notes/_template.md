# 笔记模板（Excalidraw 版）

> 约定：一个领域一个目录 `notes/NN-area/`（全局编号），每个主题一张 `<topic>.excalidraw`，**不生成 .md**。
> 笔记与路线图互链：roadmap 中对应主题卡片的 `link` 直接指向 `../NN-area/<topic>.excalidraw`。

## 每个主题的 .excalidraw 应包含（按从上到下布局）

1. **标题**（text，字号 24，深色 #343a40）：`<主题名>`
2. **图解主体**：核心原理图（坐标/管线/结构示意，配色用调色板：红 #e03131 / 绿 #2f9e44 / 蓝 #1971c2 / 紫 #9c36b5 / 橙 #e8590c / 青 #0c8599，填充用对应浅色）
3. **公式要点**（text，字号 16，深色）：关键数学推导，2-3 行以内
4. **踩坑记录**（text，字号 16，灰色 #868e96）：实际 bug 与原因（笔记里最值钱的部分）

## 流程

1. 学完主题 → 在所属领域目录 `notes/NN-area/` 下绘制 `<topic>.excalidraw`（新领域则新建目录；可用画布服务器 http://127.0.0.1:3000，绘制后 export 到目录）
2. 在 [roadmap.excalidraw](../01-roadmap/roadmap.excalidraw) 对应卡片上加 `link: "../NN-area/<topic>.excalidraw"`（新主题若无卡片则先加卡片）
3. 在 [INDEX.md](../INDEX.md) 打勾并更新链接
4. 用 VSCode 的 Excalidraw 扩展（pomdtr.excalidraw-editor）打开编辑与预览
