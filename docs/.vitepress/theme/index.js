import DefaultTheme from 'vitepress/theme'
import Diagram from './Diagram.vue'
import './custom.css'

export default {
  extends: DefaultTheme,
  enhanceApp({ app }) {
    app.component('Diagram', Diagram)
  },
}
