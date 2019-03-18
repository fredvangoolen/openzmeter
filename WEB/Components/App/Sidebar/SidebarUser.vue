<template>
  <div>
    <div class="sidenav-header clearfix">
      <div>{{ UserName }}</div>
      <div class="btn-group pull-right">
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="User edit" id="btnUserEdit"><img :src="require('Imgs/user_edit.png')"></button>
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="User logout" @click="Logout"><img :src="require('Imgs/user_go.png')"></button>
      </div>
    </div>
    <user-logout v-if="showLogout" @close="showLogout = false"></user-logout>
    <user-edit ref="UserEdit"></user-edit>
  </div>
</template>

<script>
  import UserLogout from './User/LogoutDialog.vue';
  import UserEdit from './User/User.vue';
  export default {
    data() {
      return { UserName: '', showLogout: false };
    },
    methods: {
      Logout() {
        window.location.href = 'Logout';
      }
    },
    created: function() {
      if(localStorage.getItem('UserName') === undefined) {
        window.location.href = 'Logout';
      } else {
        this.UserName = localStorage.getItem('UserName');
      }
      $(document).ajaxError(function( event, jqxhr, settings, exception ) {
        if(jqxhr.status === 401) {
         this.showLogout = true;
        }
      }.bind(this));
    },
    components: { UserLogout, UserEdit }
  }
</script>
  
<style>
  .sidenav-header > div:first-child {
    width: 180px;
  }
</style>