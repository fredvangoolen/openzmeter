<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12"><energy></energy></div>
    </div>
    <div class="row">
      <div class="col-lg-6 col-md-12"><graphs serie="VOLTAGE" :analyzer="analyzer"></graphs></div>
      <div class="col-lg-6 col-md-12"><graphs serie="CURRENT" :analyzer="analyzer"></graphs></div>
      <div v-bind:class="{'col-lg-6': (analyzer.NominalFreq > 0.0)}" class="col-md-12"><graphs serie="ACTIVE_POWER" :analyzer="analyzer"></graphs></div>
      <div class="col-lg-6 col-md-12" v-if="analyzer.NominalFreq > 0.0"><graphs serie="FREQUENCY" :analyzer="analyzer"></graphs></div>
    </div>
    <div class="row" v-if="analyzer.NominalFreq > 0.0">
      <div class="col-lg-12"><variables :analyzer="analyzer"></variables></div>
    </div>
  </div>
  <div v-else-if="analyzer == null">
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
    props: { analyzer: { type: Object } },
    data () {
      return { };
    },
    components: { Energy, Graphs, Variables }
  }
</script>

<style scoped>
</style>
