<template>
  <div>
    <div class="row">
      <div class="col-lg-12"><energy></energy></div>
    </div>
    <div class="row">
      <div class="col-lg-6 col-md-12"><graphs type="VOLTAGE"></graphs></div>
      <div class="col-lg-6 col-md-12"><graphs type="CURRENT"></graphs></div>
      <div class="col-lg-6 col-md-12"><graphs type="ACTIVE_POWER"></graphs></div>
      <div class="col-lg-6 col-md-12"><graphs type="FREQUENCY"></graphs></div>
    </div>
    <div class="row">
      <div class="col-lg-12"><variables></variables></div>
    </div>
  </div>
</template>

<script>
  import Energy from './EnergyPanel.vue'
  import Graphs from './GraphsPanel.vue';
  import Variables from './VariablesPanel.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data () {
      return { };
    },
    components: { Energy, Graphs, Variables }
  }
</script>

<style>
</style>
