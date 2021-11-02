<template>
  <q-item clickable class="q-pa-xs" dark @click="ShowDialog = true">
    <q-item-section side class="q-pr-xs">
      <img class="icon" :src="require('Imgs/user32.png')">
    </q-item-section>
    <q-item-section>
      <q-item-label class="ellipsis">{{ User.User ? User.User : '-' }}</q-item-label>
      <q-item-label caption class="ellipsis">{{ User.Name ? User.Name : '-' }}</q-item-label>
    </q-item-section>
    <q-item-section side>
      <q-icon name="menu" />
    </q-item-section>
    <!--div class="btn-group">
      <button type="button" class="btn btn-default btn-xs" title="User logout" @click="Logout"><img :src="require('Imgs/user_go.png')"></button>
    </div>
    <user-edit v-if="ShowDialog" :user="user" @close="ShowDialog=false"></user-edit-->
  </q-item>
</template>

<script>
  import UserDialog from './User/Dialog.vue';
  export default {
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
    components: { UserDialog }
  }
</script>

<style scoped>
  .icon {
    height: 24px;
    width: 24px;
    margin: 0px 3px;
  }
</style>