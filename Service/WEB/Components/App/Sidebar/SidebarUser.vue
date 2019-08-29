<template>
  <div>
    <div class="sidenav-header clearfix">
      <div>{{ UserName }}</div>
      <div class="btn-group pull-right">
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="User edit" v-on:click="Edit"><img :src="require('Imgs/user_edit.png')"></button>
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="User logout" v-on:click="Logout"><img :src="require('Imgs/user_go.png')"></button>
      </div>
    </div>
    <user-logout ref="logout"></user-logout>
    <user-edit ref="userEdit"></user-edit>
    <offline ref="offline"></offline>
  </div>
</template>

<script>
  import UserLogout from './User/LogoutDialog.vue';
  import UserEdit from './User/User.vue';
  import Offline from './User/Offline.vue';
  export default {
    data() {
      return { UserName: '', ShowEdit: false };
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
  
<style>
  .sidenav-header > div:first-child {
    width: 180px;
  }
  .sidenav-header {
    background-color: #474544;
  }
</style>