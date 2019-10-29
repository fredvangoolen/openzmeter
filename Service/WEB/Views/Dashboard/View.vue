<template>
  <div v-if="SelectedAnalyzer===true">
    <div class="row">
      <div class="col-lg-12"><energy></energy></div>
    </div>
    <div class="row">
      <div class="col-lg-6 col-md-12"><graphs type="VOLTAGE" ref="voltage"></graphs></div>
      <div class="col-lg-6 col-md-12"><graphs type="CURRENT" ref="current"></graphs></div>
      <div v-bind:class="{'col-lg-6': ($root.SelectedAnalyzer.NominalFreq > 0.0)}" class="col-md-12"><graphs type="ACTIVE_POWER" ref="active_power"></graphs></div>
      <div class="col-lg-6 col-md-12" v-if="$root.SelectedAnalyzer.NominalFreq > 0.0"><graphs type="FREQUENCY" ref="frequency"></graphs></div>
    </div>
    <div class="row" v-if="$root.SelectedAnalyzer.NominalFreq > 0.0">
      <div class="col-lg-12"><variables></variables></div>
    </div>
  </div>
  <div v-else-if="SelectedAnalyzer===false">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
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
      return { SelectedAnalyzer: null };
    },
    created: function() {
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      this.SelectedAnalyzer = (this.$root.SelectedAnalyzer !== null);
    },
    methods: {
      SetAnalyzer(analyzer) {
        this.SelectedAnalyzer = (analyzer !== null);
        this.Sync = false;
      }
    },
    components: { Energy, Graphs, Variables }
  }
</script>

<style>
</style>
