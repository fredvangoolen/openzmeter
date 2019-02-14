<template>
  <div class="panel panel-active-energy dashboard-energy">
    <!--div id="EnergySpinner" class="spinnerdiv"></div-->
    <div class="panel-heading">
      <div class="huge header-text">Energy consumption</div>
      <div class="header-buttons">
        <!--toolbar-aggregation></toolbar-aggregation>
        <toolbar-time></toolbar-time>
        <toolbar-phases></toolbar-phases>
        <toolbar-tools></toolbar-tools-->
      </div>
      <span class="clearfix"></span>
    </div>
    <div class="panel-body">
      <div class="row">
        <div class="col-md-12 col-lg-4 col-xl-3">
          <div class="subpanel">
            <div class="huge-small" id="labelEnergyPrimary">Loading data...</div>
            <div><span id="labelEnergyPrimaryValue" class="huge">?</span>&nbsp;<em class="huge-small-unit" id="labelEnergyPrimaryUnits"></em></div>
            <div id="labelEnergyPrimaryPhases"></div>
            <div><span id="labelEnergySecondary">Loading data...</span>&nbsp;<span id="labelEnergySecondaryValue"></span></div>
          </div>
          <div class="subpanel">
            <div class="huge-small" id="labelBudgetPrimary">Loading data...</div>
            <span id="labelBudgetPrimaryValue" class="huge">?</span>&nbsp;<em class="huge-small-unit" id="labelBudgetUnits"></em>
            <div><span id="labelBudgetSecondary">Loading data...</span>&nbsp;<span id="labelBudgetSecondaryValue"></span></div>
          </div>
        </div>
        <div class="col-md-12 col-lg-8 col-xl-9">
          <div id="EnergyChart" class="chart big_chart"></div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import ToolbarTime from 'Components/ToolbarTime.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  export default {
      data() {
        return {};
      },
      components: { ToolbarTime, ToolbarPhases, ToolbarTools }
  }
</script>
  
<style>
  .dashboard-energy .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>