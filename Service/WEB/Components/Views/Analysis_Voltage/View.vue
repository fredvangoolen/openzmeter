<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><voltage></voltage></div>
    </div>
    <div class="row">
      <div class="col-lg-12"><details-panel ref="details"></details-panel></div>
    </div>
  </div>
</template>

<script>
  import Voltage from './Voltage.vue';
  import DetailsPanel from './Details.vue';  
  export default {
    data() {
      return { };
    },
    components: { Voltage, DetailsPanel }
  }
</script>