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
  import Dygraph from 'JS/dygraph.js';
  export default {
    data () {
      return { };
    },
    mounted: function() {
      var Graphs = [this.$refs.voltage.$refs.Chart.Graph, this.$refs.current.$refs.Chart.Graph, this.$refs.active_power.$refs.Chart.Graph, this.$refs.frequency.$refs.Chart.Graph];
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
    },
    components: { Energy, Graphs, Variables }
  }
</script>

<style>
</style>
