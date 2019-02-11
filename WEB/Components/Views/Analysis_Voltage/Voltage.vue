<template>
  <div class="panel panel-voltage" id="VoltagePanel">
    <div id="VoltageSpinner" class="spinnerdiv"></div>
    <div class="panel-heading">
      <div class="huge header-text">Voltage</div>
      <div class="header-buttons" id="toolbarVoltage">
        <div class="btn-group" data-load-block="ScrollableAggreg_Block"></div>
        <span>&nbsp;</span>
        <div class="btn-group" data-load-block="ScrollableTime_Block"></div>
        <span>&nbsp;</span>
        <div class="btn-group" id="groupVoltagePhases" data-load-block="PhaseV_Block"></div>
        <span>&nbsp;</span>
        <div class="btn-group" data-load-block="GraphToolsPlay_Block"></div>
      </div>
      <span class="clearfix"></span>
    </div>
    <div class="panel-body">
      <div class="row">
        <div class="col-md-6 col-lg-4 col-xl-3">
          <div class="subpanel">
            <div class="huge-small" id="labelVoltageAvgPrimary">Loading data...</div>
            <div class="huge-small" id="labelVoltageAvgPrimaryValue">?</div>
            <div><span id="labelVoltageAvgSecondary">Loading data...</span>&nbsp;<span id="labelVoltageAvgSecondaryValue"></span></div>
          </div>
          <div class="subpanel">
            <div class="huge-small" id="labelVoltageMaxPrimary">Loading data...</div>
            <div class="huge-small" id="labelVoltageMaxPrimaryValue">?</div>
            <div><span id="labelVoltageMaxSecondary">Loading data...</span>&nbsp;<span id="labelVoltageMaxSecondaryValue"></span></div>
          </div>
          <div class="subpanel">
            <div class="huge-small" id="labelVoltageMinPrimary">Loading data...</div>
            <div class="huge-small" id="labelVoltageMinPrimaryValue">?</div>
            <div><span id="labelVoltageMinSecondary">Loading data...</span>&nbsp;<span id="labelVoltageMinSecondaryValue"></span></div> 
          </div>
        </div>
        <div class="col-md-6 col-lg-8 col-xl-9">
          <div>
            <div id="VoltageChart" class="chart big_chart"></div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    data() {
      return {};
    }
  }
</script>

<style>
</style>