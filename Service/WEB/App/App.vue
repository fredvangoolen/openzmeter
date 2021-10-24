<template>
  <q-layout id="app" view="hHh Lpr lff">
    <q-header v-if="(User.User != null) && (UserLoaded == true)">
      <q-toolbar class="bg-grey-9 q-px-xs">
        <q-btn dense flat icon="menu" class="lt-md q-mr-xs" @click="$refs.Sidebar.toggle()" />
        <q-toolbar-title class="q-pa-none">
          <img src="MainLogo.png" style="margin-top: 12px; max-width: 428px" />
        </q-toolbar-title>
        <!--topbar-indicators /-->
      </q-toolbar>
    </q-header>
    <q-drawer v-if="(User.User != null) && (UserLoaded == true)" ref="Sidebar" v-model="Expanded" persistent mini-to-overlay :mini="MiniState" @mouseover="MiniState=false" @mouseout="MiniState=true" :width="300" :mini-width="38" dark class="bg-grey-9">
      <q-list style="width: 300px" class="justify-left">
        <q-separator />
        <sidebar-analyzer :status="TransferStatus" />
        <q-separator />
        <sidebar-user />
        <q-separator />
        <sidebar-menu />
      </q-list>
    </q-drawer>
    <q-page-container>
      <router-view class="q-pa-sm" v-if="(User.User != null) && (UserLoaded == true)" />
      <login v-if="(User.User == null) && (UserLoaded == true)" />
      <!--offline v-if="NoNetwork"></offline-->
    </q-page-container>
    <q-resize-observer @resize="onResize" />
  </q-layout>
</template>

<script>
  import Login from './Login.vue';
  import TopbarIndicators from './TopbarIndicators.vue';
//  let Offline = defineAsyncComponent(() => import('./Offline.vue'));
  import SidebarMenu from './SidebarMenu.vue';
  import SidebarUser from './SidebarUser.vue';
  import SidebarAnalyzer from './SidebarAnalyzer.vue';
  import IconSet from 'quasar/icon-set/svg-material-icons';
  export default {
    data: function() {
      return { UserLoaded: false ,MiniState: false, Expanded: false, DesktopMode: false, NoNetwork: false, TransferStatus: this.StatusTransfer(['Events', 'Alarms']) };
    },
    mounted: function() {
      this.$q.iconSet.set(IconSet);
      this.User.getCurrent().then(
        () => this.UserLoaded = true,
        () => this.UserLoaded = true
      );
      this.Analyzer.getCurrent().then(
        () => {},
        () => {}
      );
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
    methods: {
//      HideBar: function() {
//        if(this.DesktopMode == true) {
//          this.MiniState = false;
//        } else {
//          this.Expanded = true;
//        }
//        console.log("bliu")
//      },
      onResize: function(val) {
        this.MiniState = true;
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