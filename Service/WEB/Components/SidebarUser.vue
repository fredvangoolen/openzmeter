<template>
  <div class="sidenav-header clearfix">
    <div>{{ userName }}</div>
    <b-button-group>
      <b-button data-toggle="tooltip" title="User edit" id="btnUserEdit"><img src="Imgs/user_edit.png"></b-button>
      <b-button data-toggle="tooltip" title="User logout" id="btnUserLogout"><img src="Imgs/user_go.png"></b-button>
    </b-button-group>
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

</style>