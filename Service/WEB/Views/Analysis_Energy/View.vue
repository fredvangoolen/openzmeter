<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><energy-graphs></energy-graphs></div>
    </div>
  </div>
</template>

<script>
  import EnergyGraphs from './EnergyGraphs.vue';
  export default {
    data () {
      return { };
    },
    components: { EnergyGraphs }
  }
</script>