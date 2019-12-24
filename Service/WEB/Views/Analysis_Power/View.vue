<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><power-graphs :analyzer="analyzer"></power-graphs></div>
    </div>
    <div class="row">
      <div class="col-lg-12"><details-panel :analyzer="analyzer"></details-panel></div>
    </div>
  </div>
</template>

<script>
  import PowerGraphs from './PowerGraphs.vue';
  import DetailsPanel from './Details.vue';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { };
    },
    components: { PowerGraphs, DetailsPanel }
  }
</script>