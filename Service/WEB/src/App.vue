<template>
  <div id="app">
  <header>
    <div id="app">
    <NavbarIndicators ref="indicator_bar" interval="5000"></NavbarIndicators>
  </div>
  </header>
        <aside class="sidebar">
            <router-link active-class="is-active" class="link" :to="{ name: 'Dashboard' }"> Dashboard  </router-link>
            <router-link active-class="is-active" class="link" :to="{ name: 'Dashboard' }"> Dashboard  </router-link>
      </aside>
  <div class="content" id="wrapper">
    <router-view></router-view>
  </div>
</div>
</template>

<script>
import NavbarIndicators from './NavbarIndicators.vue';
export default {
  name: 'app',
  data () {
    return {
      msg: 'Welcome to Your Vue.js App'
    }
  },
  components: {
    'NavbarIndicators': NavbarIndicators
  }
}
</script>

<style>
#wrapper {
    position: fixed;
    z-index: 0;
    padding: 10px;
    overflow-x: hidden;
    overflow-y: auto;
    left: 0px;
    top: 116px;
    bottom: 0px;
    right: 0px;
}
@media (min-width: 768px) {
#wrapper {
    top: 56px;
}
}

@media (min-width: 1024px) {
#wrapper {
    left: 256px;
}
}
</style>
