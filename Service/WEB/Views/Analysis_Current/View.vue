<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12"><current :analyzer="analyzer"></current></div>
    </div>
    <div class="row">
      <div class="col-lg-12"><details-panel :analyzer="analyzer"></details-panel></div>
    </div>
  </div>
  <div v-else-if="analyzer == null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>  
  </div>
</template>

<script>
  import Current from './Current.vue';
  import DetailsPanel from './Details.vue';
  export default {
    props: { analyzer: { type: Object } },
    data () {
      return { };
    },
    components: { Current, DetailsPanel }
  }
</script>