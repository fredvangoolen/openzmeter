<template>
  <dlg>
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Connection lost</h4></div>
    </template>
    <p>Connection to the server is lost, please check your internet connection.</p>
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