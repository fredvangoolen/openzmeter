<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><power-graphs></power-graphs></div>
    </div>
    <div class="row">
      <div class="col-lg-12"><details-panel ref="details"></details-panel></div>
    </div>
  </div>
</template>

<script>
  import PowerGraphs from './PowerGraphs.vue';
  import DetailsPanel from './Details.vue';
  export default {
    data () {
      return { };
    },
    components: { PowerGraphs, DetailsPanel }
  }
</script>