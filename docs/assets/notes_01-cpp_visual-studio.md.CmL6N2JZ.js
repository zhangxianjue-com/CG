import{_ as a,o as i,c as n,a2 as l}from"./chunks/framework.DtFQgA2F.js";const o=JSON.parse('{"title":"Visual Studio 编辑器","description":"","frontmatter":{"title":"Visual Studio 编辑器"},"headers":[],"relativePath":"notes/01-cpp/visual-studio.md","filePath":"notes/01-cpp/visual-studio.md","lastUpdated":1795163934000}'),p={name:"notes/01-cpp/visual-studio.md"};function e(t,s,E,h,k,d){return i(),n("div",null,[...s[0]||(s[0]=[l(`<h1 id="overview-总览" tabindex="-1">Overview 总览 <a class="header-anchor" href="#overview-总览" aria-label="Permalink to &quot;Overview 总览&quot;">​</a></h1><div class="language-md vp-adaptive-theme"><button title="Copy Code" class="copy"></button><span class="lang">md</span><pre class="shiki shiki-themes github-light github-dark vp-code" tabindex="0"><code><span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">visual-studio/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 安装/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── IDE（Community）</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── Build Tools（命令行）</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 项目形态/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── .sln 解决方案</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── CMake 项目</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 常用快捷键/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── F5 / Ctrl+F5</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── F9 / F10 / F11</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── Ctrl+K Ctrl+C / Ctrl+K Ctrl+D</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── F12</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 关键设置/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── UTF-8（/utf-8）</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── /W4</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── x64</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">└── 调试技巧/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    ├── 断点 + 监视</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    ├── 条件断点</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    ├── 即时窗口</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    ├── 内存窗口</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    └── 像素涂纯色</span></span></code></pre></div><h1 id="faq-常见问题" tabindex="-1">FAQ 常见问题 <a class="header-anchor" href="#faq-常见问题" aria-label="Permalink to &quot;FAQ 常见问题&quot;">​</a></h1><ol><li><p>IDE 和 Build Tools 怎么选？</p><p>日常开发用 Community IDE（官网下载，勾选「使用 C++ 的桌面开发」）；仅命令行/CI 用 winget install Microsoft.VisualStudio.2022.BuildTools。</p></li><li><p>.sln 和 CMake 项目有什么区别？</p><p>.sln 是 VS 原生工程文件；CMake 项目用 CMakeLists.txt 描述构建，VS 直接打开目录。本项目 demos/phase1-GLM、tools/publisher-cpp 都是 CMake 项目。</p></li><li><p>Debug 和 Release 行为为什么不一样？</p><p>Debug 变量初始化为特定模式、STL 越界检查会触发断言；性能测试只在 Release 做。</p></li><li><p>中文字符串乱码甚至编译报错怎么办？</p><p>源码统一 UTF-8，CMake 里对 MSVC 加 /utf-8。</p></li><li><p>中文路径有问题吗？</p><p>中文路径 + 旧工具链可能出问题：工程路径保持纯 ASCII（本项目 E:\\Github\\CG 即如此）。</p></li><li><p>改了 CMakeLists 后 VS 里没生效？</p><p>CMake 缓存过期：重新配置（或删 build 目录重来）。</p></li><li><p>循环几百万次怎么调试？</p><p>用条件断点按条件中断（如 x == 255）；或把像素涂成纯色（纯红/纯绿），一眼看出坐标与循环边界是否正确。</p></li></ol>`,4)])])}const c=a(p,[["render",e]]);export{o as __pageData,c as default};
