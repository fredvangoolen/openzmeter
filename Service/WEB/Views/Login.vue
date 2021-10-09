<template>
  <q-page class="bg-grey-9 window-height window-width row justify-center items-center">
    <q-card class="q-pa-sm">
      <q-card-section class="text-center">
        <img src="Logo.png" class="q-ma-sm">
        <form>
          <q-input v-model="UserName" label="Username" name="user"></q-input>
          <q-input v-model="Password" label="Password" type="password" name="password" autocomplete="on"></q-input>
          <q-btn unelevated type="submit" class="q-mt-sm full-width" color="orange-9" @click="Login">Sign in</q-btn>
        </form>
      </q-card-section>
      <q-card-section class="row q-py-none">
        <div class="col-6">
          Visit us at<br><a href="http://www.openzmeter.com" style="color: black">www.openzmeter.com</a>
        </div>
        <div class="col-6 text-right self-end">
          <a href="http://www.zred.es"><img src="zRed.png"></a>
        </div>
      </q-card-section>
    </q-card>
  </q-page>
</template>

<script>
  export default {
    data() {
      return { UserName: null, Password: null };
    },
    methods: {
      Login: function(event) {
        event.preventDefault();
        this.logIn(this.UserName, this.Password).then(
          () => this.$router.replace('/Dashboard'),
          () => this.$q.notify({ message: 'Failed to login', caption: 'Invalid user name or password', timeout: 3000, color: 'red', position: 'bottom-right' })
        );
      }
    }
  }
</script>