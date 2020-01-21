<template>
  <div class="this" :class="{ expanded: expanded }">
    <img class="icon" :src="require('Imgs/user32.png')">
    <div class="btn-group">
      <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="User edit" @click="Edit"><img :src="require('Imgs/user_edit.png')"></button>
      <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="User logout" @click="window.location.href='Logout'"><img :src="require('Imgs/user_go.png')"></button>
    </div>
    <div class="name">{{ (user==null) ? '-' : user.Name }}</div>
    <user-logout ref="logout"></user-logout>
    <user-edit v-if="ShowDialog" :user="user"></user-edit>
    <offline ref="offline"></offline>
  </div>
</template>

<script>
  import UserLogout from './User/LogoutDialog.vue';
  import UserEdit from './User/Dialog.vue';
  import Offline from './User/Offline.vue';
  export default {
    props: { user: null, expanded: Boolean },
    data() {
      return { ShowDialog: false };
    },
    created: function() {
      if(localStorage.getItem("UserName") == null) {
        window.location.href='Logout';
        return;
      }
      this.$http.post('getUser', { User: localStorage.getItem("UserName") }).then(
        (result) => { this.SetUser(result.data); },
        (fail) => { window.location.href='Logout'; }
      );
    },
    methods: {
      SetUser: function(user) {
        if(user != null) {
//          analyzer.GetSeriesRange = this.GetSeriesRange.bind(this);
//          analyzer.GetPricesRange = this.GetPricesRange.bind(this);
//          analyzer.GetSeriesLast = this.GetSeriesLast.bind(this);
//          analyzer.GetSeriesNow = this.GetSeriesNow.bind(this);
//          analyzer.FormatAggregation = this.FormatAggregation.bind(this);
//          analyzer.FormatTime = this.FormatTime.bind(this);
//          analyzer.FormatTimeMS = this.FormatTimeMS.bind(this);
//          analyzer.FormatDateTime = this.FormatDateTime.bind(this);
        }
        this.$emit('update:user', user);
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
//    created: function() {
//      if(localStorage.getItem('UserName') == null) {
//        window.location.href = 'Logout';
//      } else {
//
//        this.UserName = localStorage.getItem('UserName');
//      }
//      this.Timer = null;
//    },
//    mounted: function() {
//      this.PingServer();
//    },
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
    padding-left: 2px;
    line-height: 24px;
    padding-top: 4px;
  }
  .btn-group {
    margin-top: 4px;
    margin-left: 2px;
  }
</style>