<script setup>
import { withBase } from 'vitepress'

defineProps({
  src: { type: String, required: true },
  alt: { type: String, default: '' },
  source: { type: String, default: '' },
})
</script>

<template>
  <figure class="diagram-card">
    <a :href="withBase(src)" target="_blank" class="diagram-img-link" title="点击查看原图">
      <img :src="withBase(src)" :alt="alt" loading="lazy" />
    </a>
    <figcaption>
      <span class="diagram-tip">💡 Excalidraw 笔记原图</span>
      <a v-if="source" :href="withBase(source)" :download="source.split('/').pop()">
        ⬇ 下载 .excalidraw 源文件
      </a>
    </figcaption>
  </figure>
</template>
