<template>
  <div class="this" v-bind:class="{ expanded: expanded }">
    <img class="icon" :src="require('Imgs/user32.png')">
    <div class="name">{{ UserName }}</div>
    <div class="btn-group">
      <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="User edit" v-on:click="Edit"><img :src="require('Imgs/user_edit.png')"></button>
      <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="User logout" v-on:click="Logout"><img :src="require('Imgs/user_go.png')"></button>
    </div>
    <user-logout ref="logout"></user-logout>
    <user-edit v-if="ShowUser" ref="userEdit"></user-edit>
    <offline ref="offline"></offline>
  </div>
</template>

<script>
  import UserLogout from './User/LogoutDialog.vue';
  import UserEdit from './User/Dialog.vue';
  import Offline from './User/Offline.vue';
  export default {
    props: { expanded: Boolean },
    data() {
      return { UserName: '', ShowUser: false };
    },
    methods: {
      Logout: function() {
        window.location.href = 'Logout';
      },
      Edit: function() {
        $.ajax({ url: "getUser", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }),
          success: (result) => {
            this.$refs.userEdit.LoadUser(result);
          }
        });
      },
//      PingServer: function() {
//        $.ajax({ url: 'Ping', timeout: 5000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }),
//          success: function() {
//            this.$refs.logout.hide();
//            this.$refs.offline.hide();
//          }.bind(this),
//          complete: function() {
//            this.Timer = setTimeout(this.PingServer, 2000);
//          }.bind(this),
//          error: function(request) {
//            if(request.status == 401) {
//              this.$refs.logout.show();
//              this.$refs.offline.hide();
//            } else {
//              this.$refs.offline.show();
//              this.$refs.logout.hide();
//            }
//          }.bind(this)
//        });
//      }
    },
    created: function() {
      if(localStorage.getItem('UserName') === undefined) {
        window.location.href = 'Logout';
      } else {
        this.UserName = localStorage.getItem('UserName');
      }
      this.Timer = null;
    },
    mounted: function() {
//      this.PingServer();
    },
    components: { UserLogout, UserEdit, Offline }
  }
</script>

<style scoped>
  .this {
    background-color: #474544;
    color: #FAFAFA;
    width: 304px;
    height: 32px;
    display: flex;
    margin-left: 0;
    transition: margin-left 0.2s ease-in-out;
    border-bottom: 1px solid #333;
  }
  .this.expanded {
    margin-left: -48px;
    transition: margin-left 0.2s ease-in-out;
  }
  .icon {
    height: 24px;
    width: 24px;
    margin: 4px 12px;
  }
  .name {
    width: 192px;
    padding-left: 5px;
    line-height: 24px;
    padding-top: 4px;
  }
  .btn-group {
    margin-top: 4px;
  }
</style>