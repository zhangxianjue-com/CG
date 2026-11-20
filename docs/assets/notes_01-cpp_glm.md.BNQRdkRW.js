import{_ as a,o as i,c as n,a2 as l}from"./chunks/framework.DtFQgA2F.js";const d=JSON.parse('{"title":"GLM 数学库","description":"","frontmatter":{"title":"GLM 数学库"},"headers":[],"relativePath":"notes/01-cpp/glm.md","filePath":"notes/01-cpp/glm.md","lastUpdated":1795163934000}'),p={name:"notes/01-cpp/glm.md"};function e(t,s,h,E,k,r){return i(),n("div",null,[...s[0]||(s[0]=[l(`<h1 id="overview-总览" tabindex="-1">Overview 总览 <a class="header-anchor" href="#overview-总览" aria-label="Permalink to &quot;Overview 总览&quot;">​</a></h1><div class="language-md vp-adaptive-theme"><button title="Copy Code" class="copy"></button><span class="lang">md</span><pre class="shiki shiki-themes github-light github-dark vp-code" tabindex="0"><code><span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">glm/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 安装/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── CMake FetchContent（1.0.3，纯头文件）</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 常用头文件/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── glm/glm.hpp</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── glm/gtc/matrix_transform.hpp</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 核心类型与运算/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── vec2 / vec3 / vec4</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── mat3 / mat4</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── length / normalize</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── dot / cross</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 变换/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── translate</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── rotate</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── scale</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 相机与投影/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── lookAt</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── perspective</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">└── 配套 demo/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    └── demos/phase1-GLM</span></span></code></pre></div><h1 id="faq-常见问题" tabindex="-1">FAQ 常见问题 <a class="header-anchor" href="#faq-常见问题" aria-label="Permalink to &quot;FAQ 常见问题&quot;">​</a></h1><ol><li><p>GLM 需要编译或链接吗？</p><p>不需要：纯头文件库，include 即用。本仓库用 CMake FetchContent 拉取 1.0.3。</p></li><li><p>GLM 是行主序还是列主序？</p><p>列主序：mat[i] 取到的是第 i 列；与 DirectX 的行主序混用时注意转置。</p></li><li><p>为什么旋转 90° 结果不对？</p><p>角度参数要用弧度：glm::radians(度数)，直接传 90 会转出 5156°。</p></li><li><p>为什么 glm::vec3 v; 是垃圾值？</p><p>默认构造不初始化（性能考虑）；要零向量写 glm::vec3 v(0.0f);</p></li><li><p>glm::mat4() 和 glm::mat4(1.0f) 有什么区别？</p><p>mat4() 是全零矩阵；mat4(1.0f) 是单位矩阵。变换前忘了乘单位矩阵，顶点会被清零。</p></li><li><p>向量参与平移要 w 为几？</p><p>w=1 表示点（参与平移）：mat4 * vec4(v, 1.0f)；方向向量用 w=0（不参与平移）。</p></li><li><p>asin/acos 为什么出 NaN？</p><p>边缘输入超出 [-1,1]，先 clamp 到 [-1,1]。</p></li><li><p>CPU 端用 float 版还是 double 版？</p><p>带 d 前缀的 double 版：glm::dvec3、glm::dmat4（配合「CPU 用 double」约定）。</p></li><li><p>复合变换 M = T<em>R</em>S 的应用顺序是什么？</p><p>最右边的 S 最先应用：先缩放 → 再旋转 → 最后平移；列向量约定矩阵左乘向量。</p></li></ol>`,4)])])}const o=a(p,[["render",e]]);export{d as __pageData,o as default};
