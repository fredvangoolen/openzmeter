<template>
  <div class="this" :class="{ expanded: expanded }">
    <img class="icon" :src="require('Imgs/user32.png')">
    <div class="btn-group">
      <button type="button" class="btn btn-default btn-xs" title="User edit" @click="ShowDialog=true"><img :src="require('Imgs/user_edit.png')"></button>
      <button type="button" class="btn btn-default btn-xs" title="User logout" @click="Logout"><img :src="require('Imgs/user_go.png')"></button>
    </div>
    <div class="name">{{ (user==null) ? '-' : user.Name }}</div>
    <user-edit v-if="ShowDialog" :user="user" @close="ShowDialog=false"></user-edit>
  </div>
</template>

<script>
  import UserEdit from './User/Dialog.vue';
  export default {
    props: { user: null, expanded: Boolean },
    data() {
      return { ShowDialog: false };
    },
    methods: {
      GetUser: function() {
        this.ShowDialog = false;
        this.Post('getUser', { User: localStorage.getItem("UserName") }).then(
          (result) => this.$emit('update:user', result)
        );
      },
      Logout: function() {
        localStorage.setItem('UserName', undefined);
        this.Post('logOut', {}).then(
          () => { this.$emit('update:user', null); this.$router.replace('Login'); },
          () => { this.$emit('update:user', null); this.$router.replace('Login'); }
        );
      }
    },
    components: { UserEdit }
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
    text-overflow: ellipsis;
    overflow: hidden;
    white-space: nowrap;
  }
  .btn-group {
    margin-top: 4px;
    margin-left: -1px;
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