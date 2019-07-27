<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <analyzer-table ref="Table"></analyzer-table>
      <analyzer-not-found ref="NotFound"></analyzer-not-found>
      <analyzer-configure ref="Configure"></analyzer-configure>
      <analyzer-tariffs ref="Tariffs"></analyzer-tariffs>
      <analyzer-tariff ref="Tariff"></analyzer-tariff>
      <analyzer-tariff-periods ref="Periods"></analyzer-tariff-periods>
      <analyzer-tariff-billing ref="Billing"></analyzer-tariff-billing>
      <!--analyzer-acls ref="ACLs"></analyzer-acls>
      <analyzer-acl ref="ACL"></analyzer-acl-->
      <analyzer-maintenance ref="Maintenance"></analyzer-maintenance>
    </div>
  </div>
</template>

<script>
  import AnalyzerNotFound from './AnalyzerNotFound.vue';
  import AnalyzerTable from './AnalyzerTable.vue';
  import AnalyzerConfigure from './AnalyzerConfigure.vue';
  import AnalyzerTariffs from './AnalyzerTariffs';
  import AnalyzerTariff from './AnalyzerTariff.vue';
  import AnalyzerAcls from './AnalyzerACLs.vue';
  import AnalyzerAcl from './AnalyzerACL.vue';
  import AnalyzerTariffPeriods from './AnalyzerTariffPeriods.vue';
  import AnalyzerTariffBilling from './AnalyzerTariffBilling.vue';
  import AnalyzerMaintenance from './AnalyzerMaintenance.vue';
  export default {
    data() {
      return { ShowDialog: false };
    },
    methods: {
      Show: function(analyzers) {
        this.ShowDialog = true;
        this.Analyzers = analyzers;
        this.$nextTick(function() {     
          $(this.$el).modal('show');
          this.$refs.Table.Load();
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
    beforeDestroy: function() {
      this.$root.$el.removeChild(this.$el);
    },
    components: { AnalyzerNotFound, AnalyzerTariffs, AnalyzerConfigure, AnalyzerTable, AnalyzerTariff, AnalyzerTariffPeriods, AnalyzerTariffBilling, AnalyzerAcls, AnalyzerAcl, AnalyzerMaintenance }
  }
</script>

<style scoped>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
</style>