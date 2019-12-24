<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12"><energy-graphs></energy-graphs></div>
    </div>
  </div>
  <div v-else-if="analyzer == null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>  
  </div>
</template>

<script>
  import EnergyGraphs from './EnergyGraphs.vue';
  export default {
    props: { analyzer: { type: Object } },
    data () {
      return { };
    },
    components: { EnergyGraphs }
  }
</script>