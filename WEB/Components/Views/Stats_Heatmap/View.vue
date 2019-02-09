<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div id="StatsSpinner" class="spinnerdiv"></div>
          <div class="panel-heading">
              <span class="huge header-text">Energy Heatmap</span>
              <div class="header-buttons">
                <div style="display: inline-block; width: 140px" class="btn-group">
                  <div style="float: left" id="MapMin">?</div>
                  <div style="float: right" id="MapMax">?</div>
                  <div style="text-align: center"><em id="MapUnits"></em></div>
                  <div id="CalendarScale" style="height: 22px; width: 140px; display: flex; flex-direction:row; border: 1px solid #EEE"></div>
                </div>  
                <div class="btn-group" data-load-block="Years_Block"></div>
                <div class="btn-group" data-load-block="BlockSize_Block"></div>
                <div class="btn-group" data-load-block="PhaseV_Block" id="groupEnergyPhases"></div>
              </div>
              <span class="clearfix"> </span>
            </div>
            <div class="panel-body">
              <heatmap-table></heatmap-table>
            </div>
          </div>
        </div>
      </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.Stats_Heatmap.js';
  import HeatmapTable from './HeatmapTable.vue';
  export default {
    data() {
      return { };
    },
    mounted: function() {
      oZm.UpdateStatsHeatmap();
    },
    components: { HeatmapTable }
}
</script>

<style>
</style>
