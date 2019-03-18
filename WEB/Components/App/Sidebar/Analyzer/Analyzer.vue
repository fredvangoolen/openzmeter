<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <analyzer-table ref="Table"></analyzer-table>
      <analyzer-not-found ref="NotFound"></analyzer-not-found>
      <analyzer-tariffs ref="Tariffs"></analyzer-tariffs>
    </div>
  </div>
</template>

<script>
  import AnalyzerNotFound from './AnalyzerNotFound.vue';
  import AnalyzerTable from './AnalyzerTable.vue';
  import AnalyzerConfigure from './AnalyzerConfigure.vue';
  import AnalyzerTariffs from './AnalyzerTariffs';
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
            this.$refs.Table.Load(this.Analyzers);
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