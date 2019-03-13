<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><power-graphs></power-graphs></div>
      <div class="col-lg-12"><details-power></details-power></div>
    </div>
  </div>
</template>

<script>
  import PowerGraphs from './PowerGraphs.vue';
  import DetailsPower from './DetailsPower.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data () {
      return { };
    },
    methods: {

    },
    components: { PowerGraphs, DetailsPower }
  }
</script>