<template>
  <q-item class="q-pa-none expandable" dark :class="{ expanded: expanded }">
    <q-item-section side class="q-pa-none">
      <img class="icon" :src="require('Imgs/user32.png')">
    </q-item-section>
    <q-btn dense unelevated color="primary" stretch icon="toc" />
    <q-item-section class="q-ml-xs">
      <q-item-label class="ellipsis">{{ Users.User.User || '-' }}</q-item-label>
      <q-item-label caption class="ellipsis">{{ Users.User.Name || '-' }}</q-item-label>
    </q-item-section>
    <!--div class="btn-group">
      <button type="button" class="btn btn-default btn-xs" title="User logout" @click="Logout"><img :src="require('Imgs/user_go.png')"></button>
    </div>
    <user-edit v-if="ShowDialog" :user="user" @close="ShowDialog=false"></user-edit-->
  </q-item>
</template>

<script>
  import UserEdit from './User/Dialog.vue';
  export default {
    props: { expanded: Boolean },
    data() {
      return { ShowDialog: false };
    },
//    methods: {
//      GetUser: function() {
//        this.ShowDialog = false;
//        this.Post('getUser', { User: localStorage.getItem("UserName") }).then(
//          (result) => this.$emit('update:user', result)
//        );
//      },
//      Logout: function() {
//        localStorage.setItem('UserName', undefined);
//        this.Post('logOut', {}).then(
//          () => { this.$emit('update:user', null); this.$router.replace('Login'); },
//          () => { this.$emit('update:user', null); this.$router.replace('Login'); }
//        );
//      }
//    },
    components: { UserEdit }
  }
</script>

<style scoped>
  .expandable {
    margin-left: 0;
    width: 300px;
    transition: margin-left 0.1s ease-in-out;
    justify-content: start !important;
  }
  .expandable.expanded {
    margin-left: -48px;
    width: 348px;
    transition: margin-left 0.1s ease-in-out;
  }
  .icon {
    height: 24px;
    width: 24px;
    margin: 14px 12px;
  }
</style>