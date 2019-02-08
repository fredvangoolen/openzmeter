<template>
          <div id="wrapper">
      <div class="row">
        <div class="col-lg-12">
          <div class="panel panel-frequency" id="FrequencyPanel">
            <div id="FrequencySpinner" class="spinnerdiv"></div>
            <div class="panel-heading">
              <span class="huge header-text">Frequency</span>
              <div class="header-buttons" id="toolbarFrequency">
                <div class="btn-group" data-load-block="ScrollableAggreg_Block"></div>
                <span>&nbsp;</span>
                <div class="btn-group" data-load-block="ScrollableTime_Block"></div>
                <span>&nbsp;</span>
                <div class="btn-group" data-load-block="GraphToolsPlay_Block"></div>
              </div>
              <span class="clearfix"></span>
            </div>
            <div class="panel-body">
              <div class="row">
                <div class="col-md-6 col-lg-4 col-xl-3">
                  <div class="subpanel">
                    <h4 class="huge-small" id="labelFrequencyAvgPrimary">Loading data...</h4>
                    <div class="huge-small" id="labelFrequencyAvgPrimaryValue">?</div>
                    <div><span id="labelFrequencyAvgSecondary">Loading data...</span>&nbsp;<span id="labelFrequencyAvgSecondaryValue"></span></div>
                  </div>
                  <div class="subpanel">
                    <h4 class="huge-small" id="labelFrequencyMaxPrimary">Loading data...</h4>
                    <div class="huge-small" id="labelFrequencyMaxPrimaryValue">?</div>
                    <div><span id="labelFrequencyMaxSecondary">Loading data...</span>&nbsp;<span id="labelFrequencyMaxSecondaryValue"></span></div>
                  </div>
                  <div class="subpanel">
                    <h4 class="huge-small" id="labelFrequencyMinPrimary">Loading data...</h4>
                    <div class="huge-small" id="labelFrequencyMinPrimaryValue">?</div>
                    <div><span id="labelFrequencyMinSecondary">Loading data...</span>&nbsp;<span id="labelFrequencyMinSecondaryValue"></span></div>
                  </div>
                </div>
                <div class="col-md-6 col-lg-8 col-xl-9" id="FrequencyCol">
                  <div id="FrequencyMaximizeDiv">
                    <div id="FrequencyChart" class="chart big_chart"></div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
</template>

<script>
export default {
  name: 'Dashboard',
  data () {
    return {
      msg: 'Welcome to Your Vue.js App'
    }
  }
}
</script>

<style>
</style>
