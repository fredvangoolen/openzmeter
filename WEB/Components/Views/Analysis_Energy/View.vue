<template>
      <div id="wrapper">
      <div class="row">
        <div class="col-lg-12">
          <div class="panel panel-active-energy" id="ActiveEnergyPanel">
            <div id="ActiveEnergySpinner" class="spinnerdiv"></div>
            <div class="panel-heading">
              <div class="huge header-text">Active energy</div>
              <div class="header-buttons">
                <div class="btn-group" data-load-block="ScrollableAggreg_Block"></div>
                <div class="btn-group" data-load-block="ScrollableTime_Block"></div>
                <div class="btn-group" id="groupEnergyPhases" data-load-block="PhaseV_Block"></div>
                <div class="btn-group" data-load-block="GraphToolsPlay_Block"></div>
              </div>
              <span class="clearfix"> </span> 
            </div>
            <div class="panel-body">
              <div class="row">
                <div class="col-md-6 col-lg-4 col-xl-3" id="panelCounters">
                  <div class="subpanel">
                    <div class="huge-small" id="labelActiveEnergyPrimary">Loading data...</div>
                    <div><span class="huge" id="labelActiveEnergyPrimaryValue">?</span>&nbsp;<em class="huge-small-unit" id="labelActiveEnergyPrimaryUnits"></em></div>
                    
                    <span id="labelCurrentActiveEnergyPhases"></span>
                  </div>
                          <div class="panel panel-lightblue2-info">
                            <h4 id="labelPrevPeriod">Loading...</h4>
                            <span class="huge-small" id="labelPrevActiveEnergy">?</span>&nbsp;<em class="huge-small-unit" id="labelPrevActiveEnergyUnits">Wh</em>
                            <span id="labelPrevActiveEnergyPhases"></span>
                          </div>
                          <div class="panel panel-lightblue2-info">
                            <h4 id="labelSelectedPeriod">Graph view</h4>  
                            <span class="huge-small" id="labelSelectedActiveEnergy">?</span>&nbsp;<em class="huge-small-unit" id="labelSelectedActiveEnergyUnits">Wh</em>
                            <span id="labelSelectedActiveEnergyPhases"></span>
                            <p><span id="labelSelectedPeriodRange">?</span></p>
                          </div>
                     
                </div>
                <div class="col-md-6 col-lg-8 col-xl-9" id="ActiveEnergyCol">
                  <div id="ActiveEnergyMaximizeDiv">
                    <div id="ActiveEnergyChart" class="chart big_chart"></div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="row">
        <div class="col-lg-12">
          <div class="panel panel-reactive-energy" id="ReactiveEnergyPanel">
            <div id="ReactiveEnergySpinner" class="spinnerdiv"></div>
            <div class="panel-heading">
              <div class="header-text"><i class="fa fa-area-chart fa-fw"></i> Reactive energy</div>
              <div class="pull-right btn-group">
                <button type="button" class="btn btn-default btn-xs" id="btnExportReactiveEnergy" data-toggle="tooltip" title="Export data to CSV file"><i class="fa fa-table fa-fw"></i></button>
                <button type="button" class="btn btn-default btn-xs" id="btnMaximizeReactiveEnergy" data-toggle="tooltip" title="Export data to CSV file"><i class="fa fa-expand"></i></button>
              </div>
            </div>
            <div class="panel-body">  
              <div id="ReactiveEnergyCol">
                <div id="ReactiveEnergyMaximizeDiv">
                  <div id="ReactiveEnergyChart" class="chart big_chart"></div>
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
