<template>
  <q-layout id="app" view="hHh lpR fFf" class="bg-grey-2">
    <q-header v-if="(User.User != null) && (UserLoaded == true)">
      <q-toolbar class="bg-grey-9 q-px-xs">
        <q-btn dense flat :icon="mdiMenu" class="lt-md q-mr-xs" @click="Expanded=!Expanded" />
        <q-toolbar-title class="q-pa-none">
          <img src="MainLogo.png" style="margin-top: 12px; max-width: 428px" />
        </q-toolbar-title>
        <topbar-indicators v-if="!TwoRows"/>
      </q-toolbar>
      <div v-if="TwoRows" class="bg-grey-3">
        <topbar-indicators class="justify-center" />
        <q-separator />
      </div>
    </q-header>
    <q-drawer v-if="(User.User != null) && (UserLoaded == true)" show-if-above v-model="Expanded" :mini="MiniState" :mini-width="56" mini-to-overlay persistent @mouseover="MiniState=false" @mouseleave="HideBar" :width="300"  dark class="bg-grey-9">
      <q-scroll-area class="fit">
        <q-list>
          <q-separator />
          <sidebar-analyzer :status="TransferStatus" />
          <q-separator />
          <sidebar-user />
          <q-separator />
          <sidebar-menu />
        </q-list>
      </q-scroll-area>
    </q-drawer>
    <q-page-container>
      <q-page class="row no-wrap">
        <div class="col q-ma-none">
          <div class="column full-height">
            <q-scroll-area class="col q-pa-none fit">
              <router-view class="q-pa-sm" v-if="(User.User != null) && (UserLoaded == true)" />
              <login v-if="(User.User == null) && (UserLoaded == true)" />
            </q-scroll-area>
          </div>
        </div>
      </q-page>
    </q-page-container>
    <q-resize-observer @resize="onResize" />
  </q-layout>
</template>

<script>
  import { mdiMenu } from '@quasar/extras/mdi-v6';
  import Login from './Login.vue';
  import TopbarIndicators from './TopbarIndicators.vue';
  import SidebarMenu from './SidebarMenu.vue';
  import SidebarUser from './SidebarUser.vue';
  import SidebarAnalyzer from './SidebarAnalyzer.vue';
  export default {
    data: function() {
      return { TwoRows: true, UserLoaded: false, MiniState: false, Expanded: false, DesktopMode: false, NoNetwork: false, TransferStatus: this.StatusTransfer(['Events', 'Alarms']) };
    },
    created: function() {
      this.mdiMenu = mdiMenu;
      this.$q.addressbarColor.set('#a2e3fa');
      this.User.GetCurrent().then(
        () => this.UserLoaded = true,
        () => this.UserLoaded = true
      );
    },
    watch: {
      'User.User': function(val) {
        if(val == null) return;
        this.Analyzer.GetCurrent().then(
          () => {},
          () => this.$q.notify({ message: 'Previous analyzer not found, choose new one', timeout: 3000, color: 'red', position: 'bottom-right' })
        );
      }
    },
//    mounted: function() {
      //this.$q.iconSet.set(IconSet);

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
//    },
    methods: {
      HideBar: function() {
        setTimeout(() => { this.MiniState = true }, 500);
      },
      onResize: function(val) {
        this.MiniState = true;
        this.TwoRows = (val.width < 768);
        if(val.width > 1023) {
          this.Expanded = true;
          this.DesktopMode = true;
        } else {
          this.Expanded = false;
          this.DesktopMode = false;
        }
      }
    },
    components: { Login, TopbarIndicators, SidebarUser, SidebarAnalyzer, SidebarMenu }
  }
</script>

<style scoped>
::v-deep .fullscreen.q-drawer__backdrop:not(.no-pointer-events) {
  background-color: rgba( 0, 47.48, 52.52, 0.9) !important;
}
</style>