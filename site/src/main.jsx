import React, { useCallback, useEffect, useState } from "react";
import { createRoot } from "react-dom/client";
import { Excalidraw } from "@excalidraw/excalidraw";
import "@excalidraw/excalidraw/index.css";

// 从 site/dist/ 出发：
//   ../notes.json  -> site/notes.json（笔记清单）
//   ../../notes/…  -> 仓库根的 notes/（GitHub Pages 以仓库根为站点根）
const MANIFEST_URL = "../notes.json";
const REPO_ROOT = "../../";

/** 把笔记里的相对链接（如 ../01-roadmap/roadmap.excalidraw）解析成仓库内路径 */
function resolveLink(fromNotePath, link) {
  try {
    const resolved = new URL(link, "https://x/" + fromNotePath).pathname; // /notes/01-roadmap/roadmap.excalidraw
    return resolved.replace(/^\//, "");
  } catch {
    return null;
  }
}

/** 场景内元素 link 重写为 #/notes/<路径>，让 roadmap 卡片点击跳到对应笔记 */
function rewriteLinks(scene, notePath) {
  const elements = (scene.elements || []).map((el) => {
    if (typeof el.link === "string" && el.link.endsWith(".excalidraw")) {
      const target = resolveLink(notePath, el.link);
      if (target) return { ...el, link: "#/notes/" + target };
    }
    return el;
  });
  return { ...scene, elements };
}

function App() {
  const [notes, setNotes] = useState([]);
  const [current, setCurrent] = useState(null);
  const [scene, setScene] = useState(null);
  const [error, setError] = useState(null);
  const [loading, setLoading] = useState(false);
  const [isMobile, setIsMobile] = useState(
    () => window.matchMedia("(max-width: 767px)").matches,
  );
  const [menuOpen, setMenuOpen] = useState(false);

  // 响应式：窄屏（手机）启用抽屉侧栏
  useEffect(() => {
    const mq = window.matchMedia("(max-width: 767px)");
    const on = (e) => setIsMobile(e.matches);
    mq.addEventListener("change", on);
    return () => mq.removeEventListener("change", on);
  }, []);

  useEffect(() => {
    fetch(MANIFEST_URL)
      .then((r) => {
        if (!r.ok) throw new Error("清单加载失败: " + r.status);
        return r.json();
      })
      .then(setNotes)
      .catch((e) => setError(String(e)));
  }, []);

  const loadNote = useCallback(async (note) => {
    setCurrent(note);
    setError(null);
    setLoading(true);
    try {
      const r = await fetch(REPO_ROOT + note.path);
      if (!r.ok) throw new Error("加载失败: " + r.status);
      const raw = await r.json();
      setScene(rewriteLinks(raw, note.path));
    } catch (e) {
      setScene(null);
      setError(String(e));
    } finally {
      setLoading(false);
    }
  }, []);

  const noteFromHash = useCallback(
    (hash) => {
      const m = /^#\/notes\/(.+)$/.exec(hash);
      if (!m) return null;
      return notes.find((n) => n.path === m[1]) || null;
    },
    [notes],
  );

  // 清单就绪后按 hash 或默认第一项加载
  useEffect(() => {
    if (!notes.length) return;
    const target = noteFromHash(location.hash) || notes[0];
    loadNote(target);
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [notes]);

  useEffect(() => {
    const onHash = () => {
      const target = noteFromHash(location.hash);
      if (target && target !== current) loadNote(target);
    };
    window.addEventListener("hashchange", onHash);
    return () => window.removeEventListener("hashchange", onHash);
  }, [notes, current, noteFromHash, loadNote]);

  const open = (note) => {
    location.hash = "#/notes/" + note.path;
    loadNote(note);
    setMenuOpen(false);
  };

  const noteList = (
    <ul style={{ listStyle: "none", margin: 0, padding: "0 8px 16px" }}>
      {notes.map((n) => (
        <li key={n.path}>
          <button
            onClick={() => open(n)}
            style={{
              display: "block",
              width: "100%",
              textAlign: "left",
              padding: isMobile ? "12px 10px" : "8px 10px",
              marginBottom: 4,
              border: "none",
              borderRadius: 6,
              cursor: "pointer",
              fontSize: isMobile ? 14 : 13,
              background: current && current.path === n.path ? "#d0ebff" : "transparent",
              color: "#343a40",
            }}
          >
            {n.title}
          </button>
        </li>
      ))}
    </ul>
  );

  const sidebarHeader = (
    <>
      <div style={{ padding: "14px 16px 6px", fontSize: 14, fontWeight: 700, color: "#343a40" }}>
        CG 图形学笔记
      </div>
      <div style={{ padding: "0 16px 10px", fontSize: 12, color: "#868e96" }}>
        点击切换笔记，图内链接可跳转
      </div>
    </>
  );

  const canvas = (
    <div style={{ flex: 1, position: "relative", minHeight: 0 }}>
      {error && (
        <div style={{ padding: 24, color: "#c92a2a", fontSize: 14 }}>出错了：{error}</div>
      )}
      {!error && scene && (
        <Excalidraw
          key={current ? current.path : "scene"}
          initialData={{
            elements: scene.elements,
            appState: { ...(scene.appState || {}), scrollToContent: true },
          }}
          viewModeEnabled={true}
        />
      )}
    </div>
  );

  // ===== 手机布局：顶部栏 + 抽屉侧栏 =====
  if (isMobile) {
    return (
      <div
        style={{
          display: "flex",
          flexDirection: "column",
          height: "100%",
          fontFamily: "system-ui, sans-serif",
        }}
      >
        <header
          style={{
            display: "flex",
            alignItems: "center",
            gap: 10,
            padding: "8px 12px",
            borderBottom: "1px solid #e0e0e0",
            background: "#fafafa",
            flexShrink: 0,
          }}
        >
          <button
            onClick={() => setMenuOpen((v) => !v)}
            aria-label="打开笔记列表"
            style={{
              fontSize: 18,
              lineHeight: 1,
              padding: "7px 11px",
              border: "1px solid #dee2e6",
              borderRadius: 6,
              background: "#fff",
              cursor: "pointer",
            }}
          >
            ☰
          </button>
          <div style={{ flex: 1, minWidth: 0 }}>
            <strong
              style={{
                display: "block",
                fontSize: 14,
                color: "#212529",
                overflow: "hidden",
                textOverflow: "ellipsis",
                whiteSpace: "nowrap",
              }}
            >
              {current ? current.title : "加载中…"}
            </strong>
            {current && (
              <a
                href={REPO_ROOT + current.path}
                style={{ fontSize: 12, color: "#1971c2", textDecoration: "none" }}
                title="下载/查看原始 .excalidraw 文件"
              >
                原始文件 ↗
              </a>
            )}
          </div>
          {loading && <span style={{ fontSize: 12, color: "#868e96" }}>加载中…</span>}
        </header>

        {menuOpen && (
          <div
            style={{
              position: "fixed",
              inset: 0,
              zIndex: 10,
              background: "rgba(0,0,0,0.35)",
            }}
            onClick={() => setMenuOpen(false)}
          >
            <aside
              onClick={(e) => e.stopPropagation()}
              style={{
                position: "absolute",
                top: 0,
                left: 0,
                bottom: 0,
                width: 270,
                maxWidth: "85vw",
                background: "#fafafa",
                borderRight: "1px solid #e0e0e0",
                overflowY: "auto",
                boxShadow: "2px 0 12px rgba(0,0,0,0.18)",
              }}
            >
              {sidebarHeader}
              {noteList}
            </aside>
          </div>
        )}

        {canvas}
      </div>
    );
  }

  // ===== 桌面布局：左侧固定侧栏 =====
  return (
    <div style={{ display: "flex", height: "100%", fontFamily: "system-ui, sans-serif" }}>
      <aside
        style={{
          width: 240,
          minWidth: 240,
          borderRight: "1px solid #e0e0e0",
          overflowY: "auto",
          background: "#fafafa",
        }}
      >
        {sidebarHeader}
        {noteList}
      </aside>
      <main style={{ flex: 1, display: "flex", flexDirection: "column", minWidth: 0 }}>
        <div
          style={{
            display: "flex",
            alignItems: "center",
            gap: 12,
            padding: "8px 16px",
            borderBottom: "1px solid #e0e0e0",
            fontSize: 13,
            color: "#495057",
          }}
        >
          <strong style={{ fontSize: 14, color: "#212529" }}>
            {current ? current.title : "加载中…"}
          </strong>
          {current && (
            <a
              href={REPO_ROOT + current.path}
              style={{ color: "#1971c2", textDecoration: "none" }}
              title="下载/查看原始 .excalidraw 文件"
            >
              原始文件 ↗
            </a>
          )}
          {loading && <span style={{ color: "#868e96" }}>加载中…</span>}
        </div>
        {canvas}
      </main>
    </div>
  );
}

createRoot(document.getElementById("root")).render(<App />);
