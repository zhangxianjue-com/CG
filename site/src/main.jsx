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
  };

  return (
    <div style={{ display: "flex", height: "100vh", fontFamily: "system-ui, sans-serif" }}>
      <aside
        style={{
          width: 240,
          minWidth: 240,
          borderRight: "1px solid #e0e0e0",
          overflowY: "auto",
          background: "#fafafa",
        }}
      >
        <div style={{ padding: "14px 16px 6px", fontSize: 14, fontWeight: 700, color: "#343a40" }}>
          CG 图形学笔记
        </div>
        <div style={{ padding: "0 16px 10px", fontSize: 12, color: "#868e96" }}>
          点击左侧切换笔记，图内链接可跳转
        </div>
        <ul style={{ listStyle: "none", margin: 0, padding: "0 8px 16px" }}>
          {notes.map((n) => (
            <li key={n.path}>
              <button
                onClick={() => open(n)}
                style={{
                  display: "block",
                  width: "100%",
                  textAlign: "left",
                  padding: "8px 10px",
                  marginBottom: 4,
                  border: "none",
                  borderRadius: 6,
                  cursor: "pointer",
                  fontSize: 13,
                  background: current && current.path === n.path ? "#d0ebff" : "transparent",
                  color: "#343a40",
                }}
              >
                {n.title}
              </button>
            </li>
          ))}
        </ul>
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
      </main>
    </div>
  );
}

createRoot(document.getElementById("root")).render(<App />);
