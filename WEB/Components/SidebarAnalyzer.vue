<template>
  <div class="sidenav-header">
    <div>{{ analyzerName }}</div>
    <div class="btn-group pull-right">
      <button type="button" class="btn btn-default btn-xs disabled" id="btnAnalyzerSearch"><img src="Imgs/magnifier.png"></button>
      <button type="button" class="btn btn-default btn-xs disabled"><img src="Imgs/exclamation.png"></button>
    </div>
    <div class="clearfix"></div>
    <table>
      <tr><td>Disk:</td><td><progress-bar :value="disk"></progress-bar></td></tr>
      <tr><td>Battery:</td><td><progress-bar :value="battery"></progress-bar></td></tr>
      <tr><td>Usage:</td><td>{{ usage }}</td></tr>              
      <tr><td>Uptime:</td><td>{{ uptime }}</td></tr>
    </table>
  </div> 
</template> 

<script>
  import ProgressBar from './ProgressBar.vue';
  export default {
    data() {
      return { analyzerName: 'test', disk: 10, battery: 10, usage: 'poco', uptime: 'mucho'};
    },
    components: { ProgressBar }
  }
</script>

<style>
  .sidenav-header .progress {
    height: 12px;
    background-color: #474544;
    margin-bottom: 2px;
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
    margin-top: 4px;
    margin-bottom: 4px;
  }
  .sidenav-header > table tr td:nth-child(2) {
    width: 100%;
    padding: 0px 0px 0px 10px;
    text-align: center;
  }
</style>