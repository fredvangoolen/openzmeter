<template>
  <div class="navbar-nav" @mouseenter="$emit('expandmenu')" :class="{ 'expanded': expanded }">
    <div class="menu-wrapper">
      <sidebar-analyzer :analyzer="analyzer" @update:analyzer="$emit('update:analyzer', $event)" :expanded="expanded"></sidebar-analyzer>
      <sidebar-user :user="user" @update:user="$emit('update:user', $event)" :expanded="expanded"></sidebar-user>
      <sidebar-menu :analyzer="analyzer" :expanded="expanded"></sidebar-menu>
    </div>
  </div>
</template>

<script>
import SidebarUser from './SidebarUser.vue';
import SidebarAnalyzer from './SidebarAnalyzer.vue';
import SidebarMenu from './SidebarMenu.vue';
export default {
  props: { analyzer: Object, user: Object, expanded: Boolean },
  data() {
    return { };
  },
  components: { SidebarUser, SidebarAnalyzer, SidebarMenu }
}
</script>

<style scoped>
  .navbar-nav.expanded {
    width: 256px;
    transition: width 0.2s  ease-in-out;
  }
  .navbar-nav.show {
    transform: translate(0, 0);
  }
  .menu-wrapper {
    min-height: calc(100vh - 48px);
  }
  .navbar-nav {
    overflow: hidden;
    bottom: 0px;
    background: #212121;
    display: block;
    font-size: 16px;
    font-weight: 400;
    min-height: 100%;
    left: 0;
    position: fixed;
    transition: transform .2s;
    top: 0px;
    width: 0px;
    transition: width 0.2s ease-in-out;
    z-index: 1000;
    margin: 0px;
  }
  @media(min-width:1280px) {
    .navbar-nav {
      min-height: calc(100vh - 56px);
      transform: none;
      top: 56px;
      width: 48px;
    }
    .menu-wrapper {
      min-height: calc(100vh - 48px - 56px);
    }
  }
</style>