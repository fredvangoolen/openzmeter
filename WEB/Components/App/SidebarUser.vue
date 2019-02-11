<template>
  <div class="sidenav-header clearfix">
    <div>{{ userName }}</div>
    <div class="btn-group pull-right">
      <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="User edit" id="btnUserEdit"><img src="Imgs/user_edit.png"></button>
      <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="User logout" id="btnUserLogout"><img src="Imgs/user_go.png"></button>
    </div>
  </div>
</template>

<script>
  export default {
    data() {
      return { userName: 'admin' };
    },
    methods: {
      setValue: function(val) {
        userName = val;
      }
    }
  }
</script>
  
<style>
  .sidenav-header > div:first-child {
    width: 180px;
  }
</style>