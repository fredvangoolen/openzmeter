<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy stats-headmap">
          <!--div id="StatsSpinner" class="spinnerdiv"></div-->
          <div class="panel-heading">
              <span class="huge header-text">Energy Heatmap</span>
              <div class="header-buttons clearfix">
                <color-bar width="140px" height="10px"></color-bar>
                <toolbar-aggregation variable="HeatmapPeriod" :options="['15M', '1H']"></toolbar-aggregation>
                <toolbar-years @YearChanged="YearChanged"></toolbar-years>
              </div>
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
  import * as oZm from 'JS/openZmeter.js';
  import HeatmapTable from './HeatmapTable.vue';
  import ColorBar from 'Components/ColorBar.vue';
  import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
  import ToolbarYears from 'Components/ToolbarYears.vue';
  export default {
    data() {
      return { Year: 2019 };
    },
    methods: {
      YearChanged(year) {
//  var State = parseInt(localStorage.getItem('StatsBlock'));
//  var NumElements = $('#btnBlocks + ul').children().length;
//  if(isNaN(State) || (State < 0) || (State > (NumElements - 1))) State = 0;
//  $('#btnBlocks + ul > li:nth-child(' + (State + 1) + ')').addClass('active');
//  $('#btnBlocks span:first').text($('#btnBlocks + ul > li:nth-child(' + (State + 1) + ') > a').text());
//  localStorage.setItem('StatsBlock', State);
//  Analyzer.GetSeriesRange('10M', 
//    function(result) {
//  $('#StatsSpinner').show();
//  $('#btnYear + ul > li').removeClass('active');
//  $('#btnYear + ul > li > a:contains(' + selected_year +')').parent().addClass('active');
//  $('#btnYear span:first').text(selected_year);
//  SelectedYear = selected_year;
//  $('#StatsSpinner').show();
      var To = new Date(year, 11, 31, 23, 59, 59, 999).getTime();
      var From = new Date(year, 0, 1, 0, 0, 0, 0).getTime();
      $.ajax({ url: "getSeries", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify( { Period: period, To: to, Aggreg: aggreg, LastSample: lastsample, Series: series, Analyzer: Self.GetSelected().ID } ),
        success: function(result) {
          EnergyData = GraphDataAppend(result, 'Active_Power', []);
          $('#StatsSpinner').hide();
          EnergySeries();
        }
      });

      AjaxGetEnergy(new Date(result['To']).getFullYear());
      
      var ColorScale = chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]);
      ShowColorScale(ColorScale, $('#CalendarScale'));
//    }
//  );
        
        
        
        
      }
    },
    mounted: function() {
//      oZm.UpdateStatsHeatmap();
    },
    components: { ToolbarAggregation, ToolbarYears, HeatmapTable, ColorBar }
}
</script>

<style>
  .stats-headmap .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
