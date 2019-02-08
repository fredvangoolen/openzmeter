<template>
  <div>
    <div class="row">
      <div class="col-lg-12">
          <div class="panel panel-active-energy" id="EnergyPanel">
            <!--div id="EnergySpinner" class="spinnerdiv"></div-->
            <div class="panel-heading">
              <div class="huge header-text">Energy consumption</div>
              <div class="header-buttons" id="toolbarEnergy">
                <div class="btn-group" data-load-block="ScrollableAggregEnergy_Block"></div>
                <div class="btn-group" data-load-block="ScrollableTime_Block"></div>
                <div class="btn-group hide" id="groupEnergyPhases" data-load-block="PhaseV_Block"></div>
                <div class="btn-group" data-load-block="GraphToolsPlay_Block"></div>
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
      </div>
    </div>
    <div class="row">
      <div class="col-lg-6 col-md-12"><dashboard-view type="VOLTAGE"></dashboard-view></div>
      <div class="col-lg-6 col-md-12"><dashboard-view type="CURRENT"></dashboard-view></div>
      <div class="col-lg-6 col-md-12"><dashboard-view type="ACTIVE_POWER"></dashboard-view></div>
      <div class="col-lg-6 col-md-12"><dashboard-view type="FREQUENCY"></dashboard-view></div>
    </div>
    <div class="row">
      <div class="col-lg-12"><dashboard-variables></dashboard-variables></div>
    </div>
  </div>
</template>

<script>
  import DashboardView from './DashboardView.vue';
  import DashboardVariables from './DashboardVariables.vue';
  export default {
    data () {
      return { }
    },
    components: { DashboardView, DashboardVariables }
  }
</script>

<style>
</style>
