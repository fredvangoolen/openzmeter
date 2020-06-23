<template>
  <div class="this" :class="{ expanded: expanded }">
    <img class="icon" :src="require('Imgs/user32.png')">
    <div class="name">{{ (user==null) ? '-' : user.Name }}</div>
    <div class="btn-group">
      <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="User edit" @click="ShowDialog=true"><img :src="require('Imgs/user_edit.png')"></button>
      <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="User logout" @click="Logout"><img :src="require('Imgs/user_go.png')"></button>
    </div>
    <user-logout ref="logout"></user-logout>
    <user-edit v-if="ShowDialog" :user="user" @close="ShowDialog=false"></user-edit>
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
        this.Logout();
      } else {
        this.GetUser();
      }
    },
    methods: {
      GetUser: function() {
        this.ShowDialog = false;
        this.$http.post('getUser', { User: localStorage.getItem("UserName") }).then(
          (result) => { this.$emit('update:user', result.data); },
          (fail) => { this.Logout(); }
        );
      },
      Logout: function() {
        this.$http.post('logOut', {}).then(
          (result) => { location.reload(); },
          (fail) => { location.reload(); }
        );
      }
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
    width: 197px;
    padding-left: 2px;
    line-height: 24px;
    padding-top: 4px;
    text-overflow: ellipsis;
    overflow: hidden;
    white-space: nowrap;
  }
  .btn-group {
    margin-top: 4px;
    margin-right: -3px;
  }
  .btn-group > .btn {
    background: #444;
    border-color: #333;
    padding: 6px 6px 7px 6px;
    margin-top: -5px;
    border-radius: 0px;
    margin-left: -1px;
  }
  .btn-group > .btn:hover {
    background: #555;
    border-color: #333;
  }
</style>