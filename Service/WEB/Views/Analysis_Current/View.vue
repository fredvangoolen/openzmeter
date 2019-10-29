<template>
  <div v-if="SelectedAnalyzer===true">
    <div class="row">
      <div class="col-lg-12"><current></current></div>
    </div>
    <div class="row">
      <div class="col-lg-12"><details-panel></details-panel></div>
    </div>
  </div>
  <div v-else-if="SelectedAnalyzer===false">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>  
  </div>
</template>

<script>
  import Current from './Current.vue';
  import DetailsPanel from './Details.vue';
  export default {
    data () {
      return { SelectedAnalyzer: null };
    },
    created: function() {
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      this.SelectedAnalyzer = (this.$root.SelectedAnalyzer !== null);
    },
    methods: {
      SetAnalyzer(analyzer) {
        this.SelectedAnalyzer = (analyzer !== null);
      }
    },
    components: { Current, DetailsPanel }
  }
</script>