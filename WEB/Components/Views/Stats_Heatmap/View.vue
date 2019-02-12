<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy stats-headmap">
          <!--div id="StatsSpinner" class="spinnerdiv"></div-->
          <div class="panel-heading">
              <span class="huge header-text">Energy Heatmap</span>
              <div class="header-buttons">
                <color-bar width="140px" height="10px"></color-bar>
                <!--toolbar-aggregation></toolbar-aggregation>
                <toolbar-years></toolbar-years-->
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
  import ColorBar from 'Components/ColorBar.vue';
  import Toolbar from 'Components/Toolbar.vue';
  import ToolbarYears from 'Components/ToolbarYears.vue';
  export default {
    data() {
      return { };
    },
    mounted: function() {
      oZm.UpdateStatsHeatmap();
    },
    components: { Toolbar, ToolbarYears, HeatmapTable, ColorBar }
}
</script>

<style>
  .stats-headmap .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
