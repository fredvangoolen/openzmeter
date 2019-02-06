<template>
  <div class="sidenav-header">
    <div>{{ analyzerName }}</div>
    <b-button-group>
      <b-button><img src="Imgs/magnifier.png"></b-button>
      <b-button><img src="Imgs/exclamation.png"></b-button>
    </b-button-group>
    <table>
      <tr><td>Disk:</td><td><b-progress :value="disk"></b-progress></td></tr>
      <tr><td>Battery:</td><td><b-progress :value="battery"></b-progress></td></tr>
      <tr><td>Usage:</td><td>{{ usage }}</td></tr>              
      <tr><td>Uptime:</td><td>{{ uptime }}</td></tr>
    </table>
  </div> 
</template> 

<script>
  export default {
    data() {
      return { analyzerName: 'test', disk: 10, battery: 10, usage: 'poco', uptime: 'mucho'}
    }
  }
</script>

<style>
  .sidenav-header .progress {
    height: 12px;
    background-color: #474544;
  }
  .sidenav-header .progress > div {
    background-color: #337ab7;
  }
  .sidenav-header td {
    text-align: right;
  }
  .sidenav-header > table {
    width: 100%;
    font-size: 11px;
    line-height: 12px;
  }
  .sidenav-header > table tr td:nth-child(2) {
    width: 100%;
    padding: 0px 0px 0px 10px;
    text-align: center;
  }
</style>