<template>
  <div id="WaveformPanel">
    <div id="WaveformMaximize">
      <div id="WaveformChart" class="chart big_chart"></div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'dygraphs';
  export default {
    data() {
      return { Graph: null, Data: [[0, 0]], VoltagePhases: [true], CurrentPhases: [false] };
    },
    methods: {
      Export() {
        
      },
      SetPhases(voltage, current) {
        this.VoltagePhases = voltage;
        this.CurrentPhases = current;
        this.Graph.updateOptions(this.CalcOptions());
      },
      SetData(result) {
        this.Data = [];
        for(var i = 0; i < result.Voltage_Samples[0].length; i++) {
          var Row = [i];
          for(var n = 0; n < result.Voltage_Samples.length; n++) Row.push(result.Voltage_Samples[n][i]);
          for(var n = 0; n < result.Current_Samples.length; n++) Row.push(result.Current_Samples[n][i]);
          this.Data.push(Row);
        }
        this.Graph.updateOptions(this.CalcOptions(), true);
        this.Graph.updateOptions({ file: this.Data });
      },
      CalcOptions() {
        if(this.$root.SelectedAnalyzer === null) return {};
        var Analyzer = this.$root.SelectedAnalyzer, Series = {}, Visibility = [], Y1Range = [], Y2Range = []
//          var VoltageVisibility = [], CurrentVisibility = [], , ;
//  var ShowCurrent = (localStorage.getItem("VoltageDetailCurrent") === 'true');
//  GroupButtons($('#groupDetailPhasesMulti'), 'VoltageDetailVisiblePhasesMulti', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false, 
//    function(VisibleSeries, Visibles) {
//      VoltageVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Voltage').length);
//      CurrentVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Current').length);
//      if(ShowCurrent === false) CurrentVisibility.fill(false);
//      Y1Range = CalcRanges(WaveformDataset, Visibles);
//      Y2Range = CalcRanges(WaveformDataset, Visibles.map(function(val) { return val + Analyzer.GetSeriesNames('Voltage').length }));
//      Analyzer.GetSeriesNames('Current').map(function(val) {Series[val] = { axis: 'y2' }; });

        
        
        return { 
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: oZm.mouseUp, dblclick: oZm.dblClick_Restore },
          legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow, series: Series, visibility: this.VoltagePhases.concat(this.CurrentPhases),
          labels: ['Sample'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)).concat(oZm.Series['CURRENT'].Names(Analyzer)), 
          colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length).concat(oZm.ColorArray(oZm.Series['CURRENT'].Color, oZm.Series['CURRENT'].Names(Analyzer).length)), 
//          plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
/*          axes: {
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: Y1Range, 
                 valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">V</em>'; },
                 axisLabelFormatter: function (x) { return Round2(x); }
            },
            y2: { pixelsPerLabel: 15, axisLabelWidth: ShowCurrent ? 34 : 0, valueRange: Y2Range, 
                  valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">A</em>'; },
                  axisLabelFormatter: function (x) { return (ShowCurrent == true) ? Round2(x) : ''; }
            },
            x: { ticker: function(min, max) { return GraphWaveTimeTicks(max); },
                 valueFormatter: function (x) { return Round2((x * 1000) / Analyzers[Analyzer]['SampleFreq']) + ' <em class="x-small">ms</em>'; }
               }
          }*/
      };
      }
    },
    mounted: function() {
      this.Graph = new Dygraph(document.getElementById("WaveformChart"), this.Data, this.CalcOptions());
    }
  }
</script>

<style>
    
</style>

    function UpdateWaveformChart() {
  var VoltageVisibility = [], CurrentVisibility = [], Y1Range = [], Y2Range = [], ;
  var ShowCurrent = (localStorage.getItem("VoltageDetailCurrent") === 'true');
  GroupButtons($('#groupDetailPhasesMulti'), 'VoltageDetailVisiblePhasesMulti', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false, 
    function(VisibleSeries, Visibles) {
      VoltageVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Voltage').length);
      CurrentVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Current').length);
      if(ShowCurrent === false) CurrentVisibility.fill(false);
      Y1Range = CalcRanges(WaveformDataset, Visibles);
      Y2Range = CalcRanges(WaveformDataset, Visibles.map(function(val) { return val + Analyzer.GetSeriesNames('Voltage').length }));
      Analyzer.GetSeriesNames('Current').map(function(val) {Series[val] = { axis: 'y2' }; });
      
      if(WaveformChart === null) {
        
      } else {
        WaveformChart.updateOptions(Options, true);
        WaveformChart.updateOptions({ file: WaveformDataset });
      }
    }, undefined);
}