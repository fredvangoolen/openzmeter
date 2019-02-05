<template>
  <div id="app">
    <div id="DisableDiv" class="overlay"></div>
    <navbar-title></navbar-title>    
    <navbar-indicators ref="indicator_bar" interval="5000"></navbar-indicators>
    <sidebar></sidebar>
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
import NavbarTitle from './NavbarTitle.vue';
import NavbarIndicators from './NavbarIndicators.vue';
import Sidebar from './Sidebar.vue';
export default {
  name: 'app',
  data () {
    return {
      msg: 'Welcome to Your Vue.js App'
    }
  },
  components: {
    NavbarTitle,
    NavbarIndicators,
    Sidebar
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
