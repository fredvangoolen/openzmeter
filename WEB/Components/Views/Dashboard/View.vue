<template>
  <div>
    <div class="row">
      <div class="col-lg-12"><energy></energy></div>
    </div>
    <div class="row">
      <div class="col-lg-6 col-md-12"><graphs type="VOLTAGE" ref="voltage"></graphs></div>
      <div class="col-lg-6 col-md-12"><graphs type="CURRENT" ref="current"></graphs></div>
      <div class="col-lg-6 col-md-12"><graphs type="ACTIVE_POWER" ref="active_power"></graphs></div>
      <div class="col-lg-6 col-md-12"><graphs type="FREQUENCY" ref="frequency"></graphs></div>
    </div>
    <div class="row">
      <div class="col-lg-12"><variables></variables></div>
    </div>
  </div>
</template>

<script>
  import Energy from './EnergyPanel.vue'
  import Graphs from './GraphsPanel.vue';
  import 'JS/dygraph-synchronizer.js';
  import Variables from './VariablesPanel.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data () {
      return { };
    },
    mounted: function() {
      //var Graphs = [ this.$refs.voltage.$refs.Chart.Graph ];
      
    },
    components: { Energy, Graphs, Variables }
  }
</script>

<style>
</style>
