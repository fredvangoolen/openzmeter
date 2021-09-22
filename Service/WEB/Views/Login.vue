<template>
  <q-page class="bg-grey-9 window-height window-width row justify-center items-center">
    <q-card class="q-pa-sm">
      <q-card-section class="text-center">
        <img src="Logo.png" class="q-ma-sm">
        <form>
          <q-input v-model="UserName" label="Username" name="user"></q-input>
          <q-input v-model="Password" label="Password" type="password" name="password" autocomplete="on"></q-input>
          <q-btn type="submit" class="q-mt-sm full-width" color="orange-9" @click="Login">Sign in</q-btn>
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
    props: { user: Object },
    data() {
      return { UserName: null, Password: null };
    },
    created: function() {
      if(this.user != null) this.$router.push('/Dashboard');
    },
    methods: {
      Login: function(event) {
        event.preventDefault();
        this.Post('logIn', { User: this.UserName, Password: this.Password, UpdateLastLogin: true }).then(
          (result) => {
            localStorage.setItem('LastLogin', result.LastLogin);
            localStorage.setItem('UserName', this.UserName);
            this.Post('getUser', { User: localStorage.getItem("UserName") }).then(
              (result) => {
                this.$emit('update:user', result);
                this.$router.push('/Dashboard');
                this.$q.notify({ message: 'Logged in', timeout: 3000, color: 'green', position: 'bottom-right' })
              }
            );
          },
          (fail) => this.$q.notify({ message: 'Failed to login', caption: 'Invalid user name or password', timeout: 3000, color: 'red', position: 'bottom-right' })
        );
      }
    }
  }
</script>