<template>
  <div id="app" v-if="User!=null">
    <div class="sidenav-overlay" :class="{ 'overlay-enabled': Expanded }" @mouseenter="CloseBar"></div>
    <topbar :analyzer="Analyzer" @expandmenu="ShowBar" :expanded="Expanded" :status="TransferStatus.Data"></topbar>
    <sidebar :analyzer.sync="Analyzer" :user.sync="User" @expandmenu="ShowBar" :expanded="Expanded" :status="TransferStatus.Data"></sidebar>
    <router-view :analyzer="Analyzer" :user="User" id="contents"></router-view>
    <portal-target name="dialogs" :multiple="true"></portal-target>
    <offline v-if="NoNetwork"></offline>
  </div>
  <router-view v-else :user.sync="User"></router-view>
</template>

<script>
  import Topbar from './Topbar.vue';
  import Sidebar from './Sidebar.vue';
  import Offline from './Offline.vue';
  export default {
    data: function() {
      return { Analyzer: null, User: null, Expanded: false, CloseTimer: null, NoNetwork: false, TransferStatus: this.StatusTransfer(['Events', 'Alarms']) };
    },
    created: function() {
      window.EventBus.$on('UserUnathorized', () => {
        localStorage.setItem("UserName", undefined);
        this.$router.replace('Login');
        this.User = null;
      });
      window.EventBus.$on('NoNetwork', (b) => { this.NoNetwork = b; });
      if(window.LocalConnection == true) {
        this.$router.replace('Config');
      } else {
        this.Post('getUser', { User: localStorage.getItem("UserName") }).then(
          (result) => { this.User = result; },
          () => {
            localStorage.setItem("UserName", undefined);
            this.$router.replace('Login');
            this.User = null;
          }
        );
      }
    },
    watch: {
      Analyzer: function() {
        this.TransferStatus.Update(this.Analyzer);
      }
    },
    methods: {
      ShowBar: function() {
        if(this.CloseTimer) clearTimeout(this.CloseTimer);
        this.Expanded = true;
      },
      CloseBar: function() {
        this.CloseTimer = setTimeout(() => { this.Expanded = false; }, 250);
      }
    },
    components: { Topbar, Sidebar, Offline }
  }
</script>

<style>
  #contents {
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
  @media (min-width: 840px) {
    #contents {
      top: 56px;
    }
  }
  .sidenav-overlay {
    background: #000;
    opacity: 0.0;
    display: block;
    height: 0px;
    left: 0px;
    position: fixed;
    top: 0px;
    width: 100%;
    z-index: 95;
    transition: height 0ms 200ms, opacity 200ms 0ms;
  }
  .overlay-enabled {
    height: 100%;
    opacity: 0.5;
    transition: height 0ms 0ms, opacity 200ms 0ms;
  }
  @media (min-width: 1280px) {
    #contents {
      left: 48px;
    }
    .sidenav-overlay {
      top: 56px;
    }
  }
</style>