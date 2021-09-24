<template>
  <div class="body">
    <div class="container">
      <div class="card">
        <div class="row">
          <div class="col-sm-12">
            <img src="Logo.png" class="logo">
          </div>
          <form id="login-form">
            <div class="col-xs-12">
              <input type="text" class="form-control" v-model="UserName" placeholder="Username" required autofocus autocomplete="off" autocapitalize="off" autocorrect="off">
            </div>
            <div class="col-xs-12">
              <input type="password" class="form-control" placeholder="Password" v-model="Password" required>
            </div>
            <div class="col-xs-12">
              <button type="submit" class="btn btn-default btn-xs" @click="Login">Sign in</button>
            </div>
          </form>
        </div>
        <div class="row trademark">
          <div class="col-xs-6">
            Visit us at<br><a href="http://www.openzmeter.com" style="color: black">www.openzmeter.com</a>
          </div>
          <div class="col-xs-6">
            <a href="http://www.zred.es"><img src="zRed.png"></a>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    props: { user: Object },
    data() {
      return { UserName: null, Password: null };
    },
    created: function() {
      if(this.user != null) this.$router.replace('/');
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
                this.$router.replace('/');
                this.$toast.open({ type:'success', message: "Logged in", duration: 3000});
              }
            );
          },
          (fail) => { this.$toast.open({ type:'error', message: "Failed to login<br>Invalid user name or password", duration: 3000}); }
        );
      }
    }
  }
</script>

<style scoped>
  .body {
    box-sizing: border-box;
    display: table;
    position: absolute;
    height: 100%;
    width: 100%;
    background-color: #474544;
    background-image: none;
    overflow: hidden;
    margin: 0px;
  }
  .container {
    display: table-cell;
    vertical-align: middle;
    width: 100%;
    position: relative;
    z-index: 10;
    margin-right: auto;
    margin-left: auto;
    padding-left: 15px;
    padding-right: 15px;
  }
  .card {
    max-width: 350px;
    background-color: #F7F7F7;
    padding: 40px 40px 10px 40px;
    margin: auto;
  }
  .logo {
    margin-bottom: 20px;
  }
    .form-control {
    margin: 5px 0px;
  }
  .btn {
    background-color: #E8701C;
    font-weight: 700;
    color: #F7F7F7;
    text-align: center;
    border: 1px solid transparent;
    padding: 6px 12px;
    font-size: 14px;
    width: 100%;
    margin: 10px 0px;
  }
  .trademark {
    line-height: 16px;
  }
  .trademark > div:nth-child(2) {
    text-align: right;
  }
</style>