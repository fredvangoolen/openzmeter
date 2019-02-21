<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy stats-headmap">
          <!--div id="StatsSpinner" class="spinnerdiv"></div-->
          <div class="panel-heading">
              <span class="huge header-text">Energy Heatmap</span>
              <div class="header-buttons clearfix">
                <color-bar :width="140" :height="10" ref="colorbar"></color-bar>
                <toolbar-aggregation variable="HeatmapPeriod" :options="['15M', '1H']"></toolbar-aggregation>
                <toolbar-years @YearChanged="YearChanged"></toolbar-years>
              </div>
            </div>
            <div class="panel-body" style="min-height: 300px">
              <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
              <heatmap-table ref="table" v-on:RangeChanged="RangeChanged"></heatmap-table>
            </div>
          </div>
        </div>
      </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Spinner from 'Components/Spinner.vue';
  import HeatmapTable from './HeatmapTable.vue';
  import ColorBar from 'Components/ColorBar.vue';
  import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
  import ToolbarYears from 'Components/ToolbarYears.vue';
  import moment from 'moment';
  export default {
    data() {
      return { ShowSpinner: true, Year: 2019 };
    },
    methods: {
      AggregationChanged(aggreg) {
        this.$refs.table.SetAggregation(aggreg);
      },
      RangeChanged(min, max, units) {
        this.$refs.colorbar.SetRange(oZm.Round2(min), oZm.Round2(max), units + 'Wh');
      },
      YearChanged(year) {
        this.ShowSpinner = true;
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Year = moment().year(year);
        $.ajax({ url: "getSeries", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify( { Period: Year.endOf('year').valueOf() - Year.startOf('year').valueOf(), To: Year.endOf('year').valueOf(), Aggreg: '15M', LastSample: 0, Series: ['ACTIVE_POWER'], Analyzer: Analyzer.ID } ),
          success: function(result) {
            this.ShowSpinner = false;
            this.$refs.table.SetData(result);
          }.bind(this)
        });
      }
    },
    components: { Spinner, ToolbarAggregation, ToolbarYears, HeatmapTable, ColorBar }
}
</script>

<style>
  .stats-headmap .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
