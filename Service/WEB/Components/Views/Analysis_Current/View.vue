<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><current></current></div>
    </div>
    <div class="row">
      <div class="col-lg-12"><details-panel></details-panel></div>
    </div>
  </div>
</template>

<script>
  import Current from './Current.vue';
  import DetailsPanel from './Details.vue';
  export default {
    data () {
      return { };
    },
    components: { Current, DetailsPanel }
  }
</script>