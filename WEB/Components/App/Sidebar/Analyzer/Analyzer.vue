<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <analyzer-table ref="Table"></analyzer-table>
      <analyzer-not-found ref="NotFound"></analyzer-not-found>
      <analyzer-configure ref="Configure"></analyzer-configure>
      <analyzer-tariffs ref="Tariffs"></analyzer-tariffs>
      <analyzer-tariff ref="Tariff"></analyzer-tariff>
      <analyzer-tariff-periods ref="Periods"></analyzer-tariff-periods>
    </div>
  </div>
</template>

<script>
  import AnalyzerNotFound from './AnalyzerNotFound.vue';
  import AnalyzerTable from './AnalyzerTable.vue';
  import AnalyzerConfigure from './AnalyzerConfigure.vue';
  import AnalyzerTariffs from './AnalyzerTariffs';
  import AnalyzerTariff from './AnalyzerTariff.vue';
  import AnalyzerTariffPeriods from './AnalyzerTariffPeriods.vue';
  export default {
    data() {
      return { ShowDialog: false };
    },
    methods: {
      Show: function(analyzers) {
        this.ShowDialog = true;
        this.Analyzers = analyzers;
        this.$nextTick(function() {     
          if(analyzers.length === 0) {
            this.$refs.NotFound.Load();
          } else {
            this.$refs.Table.Load();
          }
          $(this.$el).modal('show');
        });
      },
      Close: function() {
        $(this.$el).modal('hide');
        this.ShowDialog = false;
      }
    },
    mounted: function() {
      this.$root.$el.append(this.$el);
    },
    destroyed: function() {
      this.$root.$el.removeChild(this.$el);
    },
    components: { AnalyzerNotFound, AnalyzerTariffs, AnalyzerConfigure, AnalyzerTable, AnalyzerTariff, AnalyzerTariffPeriods, AnalyzerConfigure }
  }
</script>

<style>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
</style>