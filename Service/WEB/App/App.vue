<template>
  <q-layout id="app" view="hHh Lpr lff">
    <q-header v-if="Users.User != null">
      <q-toolbar class="bg-grey-9 q-px-xs">
        <q-btn dense flat icon="menu" class="lt-md q-mr-xs" @click="$refs.Sidebar.toggle()" />
        <q-toolbar-title class="q-pa-none">
          <img src="MainLogo.png" style="margin-top: 12px; max-width: 428px" />
        </q-toolbar-title>
        <topbar-indicators />
      </q-toolbar>
    </q-header>
    <q-drawer v-if="Users.User != null" ref="Sidebar" v-model="Expanded" mini-to-overlay :mini="MiniState" @mouseover="MiniState=false" @mouseout="MiniState=true" :width="300" :mini-width="48" dark class="bg-grey-9">
      <q-list class="justify-left">
        <q-separator />
        <sidebar-analyzer :expanded="IsExpanded" :status="TransferStatus"></sidebar-analyzer>
        <q-separator />
        <sidebar-user :expanded="IsExpanded" />
        <q-separator />
        <sidebar-menu :expanded="IsExpanded" />
      </q-list>
    </q-drawer>
    <q-page-container>
      <router-view class="q-pa-sm"></router-view>
      <offline v-if="NoNetwork"></offline>
    </q-page-container>
    <q-resize-observer @resize="onResize" />
  </q-layout>
</template>

<script>
  import { defineAsyncComponent } from 'vue';
  let TopbarIndicators = defineAsyncComponent(() => import('./TopbarIndicators.vue'));
  let Offline = defineAsyncComponent(() => import('./Offline.vue'));
  let SidebarMenu = defineAsyncComponent(() => import('./SidebarMenu.vue'));
  let SidebarUser = defineAsyncComponent(() => import('./SidebarUser.vue'));
  let SidebarAnalyzer = defineAsyncComponent(() => import('./SidebarAnalyzer.vue'));
  let IconSet = defineAsyncComponent(() => import('quasar/icon-set/svg-material-icons'));
  export default {
    data: function() {
      return { MiniState: false, Expanded: false, DesktopMode: false, NoNetwork: false, TransferStatus: this.StatusTransfer(['Events', 'Alarms']) };
    },
    created: function() {
      this.$q.iconSet.set(IconSet);
//      window.EventBus.$on('UserUnathorized', () => {
//        localStorage.setItem("UserName", undefined);
//        this.$router.replace('Login');
//        this.User = null;
//      });
//      window.EventBus.$on('NoNetwork', (b) => { this.NoNetwork = b; });
//      if(window.LocalConnection == true) {
//        this.$router.replace('Config');
//      } else {
//        this.Post('getUser', { User: localStorage.getItem("UserName") }).then(
//          (result) => { this.User = result; },
//          () => {
//            localStorage.setItem("UserName", undefined);
//            this.$router.replace('Login');
//            this.User = null;
//          }
//        );
//      }
    },
    computed: {
      IsExpanded: function() {
        return (this.DesktopMode && !this.MiniState) || (!this.DesktopMode && this.Expanded);
      }
    },
            watch: {

//      Analyzer: function() {
//        this.TransferStatus.Update(this.Analyzer);
//      }
    },
    methods: {
      onResize: function(val) {
        if(val.width > 1023) {
          this.MiniState = true;
          this.Expanded = true;
          this.DesktopMode = true;
        } else {
          this.MiniState = false;
          this.Expanded = false;
          this.DesktopMode = false;
        }
      }
    },
    components: { TopbarIndicators, SidebarUser, SidebarAnalyzer, SidebarMenu, Offline }
  }
</script>

<style scoped>
  ::v-deep(.q-drawer__content) {
    overflow: hidden;
  }
</style>