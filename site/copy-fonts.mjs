// Excalidraw 在运行时以相对页面 URL fetch("./fonts/...") 加载字体，
// 构建后把官方字体资源拷到 dist/fonts/（否则文字渲染会退回系统字体）。
import { cpSync } from "node:fs";

cpSync("node_modules/@excalidraw/excalidraw/dist/prod/fonts", "dist/fonts", {
  recursive: true,
});
console.log("fonts copied to dist/fonts");
