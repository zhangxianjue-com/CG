import{_ as a,o as i,c as n,a2 as p}from"./chunks/framework.DtFQgA2F.js";const l="/CG/assets/roadmap.CTlS2bnY.svg",g=JSON.parse('{"title":"CG · 图形学学习路线图","description":"","frontmatter":{"title":"CG · 图形学学习路线图"},"headers":[],"relativePath":"notes/roadmap.md","filePath":"notes/roadmap.md","lastUpdated":1794984217000}'),E={name:"notes/roadmap.md"};function h(e,s,k,t,r,d){return i(),n("div",null,[...s[0]||(s[0]=[p(`<h1 id="overview-总览" tabindex="-1">Overview 总览 <a class="header-anchor" href="#overview-总览" aria-label="Permalink to &quot;Overview 总览&quot;">​</a></h1><div class="language-md vp-adaptive-theme"><button title="Copy Code" class="copy"></button><span class="lang">md</span><pre class="shiki shiki-themes github-light github-dark vp-code" tabindex="0"><code><span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">路线图/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 阶段 0 · 基础/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── ✔ 渲染器原理</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── ✔ 坐标系</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── ✔ 向量与矩阵</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 矩阵与变换</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 四元数</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── C++ 复习</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 阶段 1 · CPU 光线追踪/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 输出第一张图（PPM）</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 光线与球求交</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 表面法线与多物体</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 抗锯齿</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 漫反射材质</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 金属材质与反射</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 玻璃与折射</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 相机与视野</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 景深</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 运动模糊</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── BVH 加速结构</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 纹理与柏林噪声</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 四边形·实例化·灯光</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 体积（烟雾）</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 蒙特卡洛积分与 PDF</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 分层采样与抖动</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 重要性采样</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 路径追踪</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 直接光照采样（NEE）</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── 多重重要性采样（MIS）</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 阶段 2 · 软件光栅化/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 画线（Bresenham）</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 三角形光栅化与重心坐标</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── z-buffer</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 透视投影</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 相机变换</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 着色器模型</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 阴影贴图</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 环境光遮蔽</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 镜面反射</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── 法线贴图</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">├── 阶段 3 · GPU 实时渲染/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 你好，窗口与三角形</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 着色器</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 纹理</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 变换与坐标系统</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 相机</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 基础光照与材质</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 模型加载</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 深度·模板·混合·面剔除</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 帧缓冲与后处理</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 阴影映射</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 法线贴图与视差贴图</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── HDR 与泛光</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── 延迟着色</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── SSAO</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── PBR 理论</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   ├── PBR 光照</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">│   └── IBL 漫反射 + 镜面</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">└── 阶段 4 · 引擎整合与进阶/</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    ├── 场景/材质统一描述层</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    ├── 双后端对照渲染</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    ├── 多线程与 job 系统</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    ├── 资产加载 obj/gltf</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    ├── 引擎模块整理与测试</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    ├── GPU 光线追踪</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    ├── 体积渲染</span></span>
<span class="line"><span style="--shiki-light:#24292E;--shiki-dark:#E1E4E8;">    └── 降噪</span></span></code></pre></div><p><img src="`+l+'" alt="CG · 图形学学习路线图"></p><h1 id="faq-常见问题" tabindex="-1">FAQ 常见问题 <a class="header-anchor" href="#faq-常见问题" aria-label="Permalink to &quot;FAQ 常见问题&quot;">​</a></h1><ol><li><p>为什么从 CPU 光线追踪开始，而不是直接学 GPU？</p><p>离线渲染把像素写进文件，没有窗口/GPU 负担，先把数学和算法吃透。</p></li><li><p>为什么光线追踪和光栅化都要学？</p><p>两条主流路线互补，阶段 4 做双后端对照渲染，同一场景两种实现互相验证。</p></li><li><p>每阶段的参考资料是什么？</p><p>阶段 0 自写数学库；阶段 1 Ray Tracing in One Weekend 三部曲；阶段 2 tinyrenderer；阶段 3 learnopengl.com。</p></li><li><p>进度怎么记录？</p><p>直接勾选本页 Overview 树里的条目（✔），构建发布后即上线。</p></li></ol>',5)])])}const y=a(E,[["render",h]]);export{g as __pageData,y as default};
