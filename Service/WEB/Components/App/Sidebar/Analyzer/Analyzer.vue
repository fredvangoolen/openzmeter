<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <analyzer-table ref="Table"></analyzer-table>
      <analyzer-not-found ref="NotFound"></analyzer-not-found>
      <analyzer-configure ref="Configure"></analyzer-configure>
      <analyzer-rates ref="Rates"></analyzer-rates>
      <analyzer-rate ref="Rate"></analyzer-rate>
      <analyzer-rate-periods ref="Periods"></analyzer-rate-periods>
      <analyzer-rate-billing ref="Billing"></analyzer-rate-billing>
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
  import AnalyzerRates from './AnalyzerRates';
  import AnalyzerRate from './AnalyzerRate.vue';
  import AnalyzerAcls from './AnalyzerACLs.vue';
  import AnalyzerAcl from './AnalyzerACL.vue';
  import AnalyzerRatePeriods from './AnalyzerRatePeriods.vue';
  import AnalyzerRateBilling from './AnalyzerRateBilling.vue';
  import AnalyzerMaintenance from './AnalyzerMaintenance.vue';
  export default {
    data() {
      return { ShowDialog: false };
    },
    methods: {
      Show: function() {
        this.ShowDialog = true;
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
    components: { AnalyzerNotFound, AnalyzerRates, AnalyzerConfigure, AnalyzerTable, AnalyzerRate, AnalyzerRatePeriods, AnalyzerRateBilling, AnalyzerAcls, AnalyzerAcl, AnalyzerMaintenance }
  }
</script>

<style scoped>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
</style>