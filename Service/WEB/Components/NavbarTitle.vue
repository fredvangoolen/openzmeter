<template>
  <b-navbar>
    <b-navbar-brand>
      <embed src="Imgs/Logo2.svg" type="image/svg+xml"/><br/>
      <div>Node time:&nbsp; {{ time }}</div>
    </b-navbar-brand>
    <b-navbar-toggle target="nav_collapse"><img src="Imgs/application_side_expand.png"></b-navbar-toggle>
    <!--b-button type="button" class="navbar-toggle" id="sidenav-toggle">
      <span class="glyphicon glyphicon-menu-hamburger"></span>
    </b-button-->
  </b-navbar>
</template>

<script>
  export default {
    methods: {
      setValue: function(val) {
        if(val === null) {
          time = 'No analyzer selected';
        } else {
          time = moment(val).format('DD/MM/YYYY HH:mm:ss');
        }
      }
    },
    data() {
      return {time: 'Offline'};
    }
  }
</script>
  
<style>
  .navbar {
    background-color: #474544;
    height: 56px;
  }
  .navbar-brand > embed {
    width: 100%;
  }
  .navbar-brand > div {
    font-size: 12px; 
    color: #FFF; 
    margin-top: -5px;
  }
  .navbar-brand {
    text-align: left;
    width: 260px;
    padding: 0px;
    height: 50px;     
  }
  .navbar-toggler {
    padding: 9px 5px;
    right: 2px;
    position: absolute;
  }
</style>