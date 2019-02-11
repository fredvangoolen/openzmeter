<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <AnalyzerTable v-if="Frame == 'Table'" :analyzers="analyzers"></AnalyzerTable>
      <AnalyzerNotFound v-if="Frame == 'NotFound'" :analyzers="analyzers"></AnalyzerNotFound>
      <AnalyzerTariffs v-if="Frame == 'Tariffs'" :analyzers="analyzers"></AnalyzerTariffs>
    </div>
  </div>
</template>

<script>
  import AnalyzerNotFound from './AnalyzerNotFound.vue';
  import AnalyzerTable from './AnalyzerTable.vue';
  import AnalyzerConfigure from './AnalyzerConfigure.vue';
  import AnalyzerTariffs from './AnalyzerTariffs';
  export default {
    props: ['analyzers'],
    data() {
      return { Frame: null, EditingAnalyzerID: null };
    },
    methods: {
      showTariffs(analyzer) {
        this.EditingAnalyzerID = analyzer;
        this.Frame = 'Tariffs';
        alert(analyzer);
      }
    },
    mounted: function() {
      $(this.$el).modal('show');
      if(this.$parent.AvaliableAnalyzers.length === 0) {
        this.Frame = 'NotFound';
      } else {
        this.Frame = 'Table';
      }
    },
    components: { AnalyzerNotFound, AnalyzerTariffs, AnalyzerConfigure, AnalyzerTable }
  }
</script>

<style>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
</style>