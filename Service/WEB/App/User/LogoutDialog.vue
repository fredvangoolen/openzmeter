<template>
  <dlg>
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Login</h4></div>
    </template>
    <p>The current session has expired due to inactivity. Please re-login</p>
    <p><a href="/Logout">Login</a></p>
  </dlg>
</template>

<script>
  export default {
    data() {
      return { };
    }
  }
</script>

<style scoped>
  .modal-dialog p {
    text-align: center;
  }
  .modal-dialog a {
    font-size: 24px;
  }
</style>