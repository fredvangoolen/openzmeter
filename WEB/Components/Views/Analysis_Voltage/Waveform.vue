<template>
  <div id="WaveformPanel" v-bind:class="{ hide: !visible }">
    <div id="WaveformMaximize">
      <div id="WaveformChart" class="chart big_chart"></div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'dygraphs';
  function GraphWaveTimeTicks(max, analyzer) {
    var ticks = [];
    for(var i = 0; i <= Math.floor(max / analyzer.SampleFreq * analyzer.NominalFreq); i++) {
      ticks.push({ v: i * analyzer.SampleFreq / analyzer.NominalFreq, label: oZm.Round2(i / analyzer.NominalFreq * 1000) });
    }
    return ticks;
  }
  export default {
    props: ['visible'],
    data() {
      return { Graph: null, Data: [[0, 0]], VoltagePhases: [true], CurrentPhases: [true] };
    },
    methods: {
      Export() {
        
      },
      SetOptions(voltage, current) {
        this.VoltagePhases = voltage;
        this.CurrentPhases = current;
        this.CalcOptions();
      },
      SetData(result) {
        this.Data = [];
        for(var i = 0; i < result.Voltage_Samples[0].length; i++) {
          var Row = [i];
          for(var n = 0; n < result.Voltage_Samples.length; n++) Row.push(result.Voltage_Samples[n][i]);
          for(var n = 0; n < result.Current_Samples.length; n++) Row.push(result.Current_Samples[n][i]);
          this.Data.push(Row);
        }
        this.CalcOptions();
      },
      CalcOptions() {
        if(this.$root.SelectedAnalyzer === null) return {};
        var Analyzer = this.$root.SelectedAnalyzer, Series = {}, Visibility = [], Y1Range = [], Y2Range = [];
//  var ShowCurrent = (localStorage.getItem("VoltageDetailCurrent") === 'true');
//  GroupButtons($('#groupDetailPhasesMulti'), 'VoltageDetailVisiblePhasesMulti', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false, 
//    function(VisibleSeries, Visibles) {
//      VoltageVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Voltage').length);
//      CurrentVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Current').length);
//      if(ShowCurrent === false) CurrentVisibility.fill(false);
//      Y1Range = CalcRanges(WaveformDataset, Visibles);
//      Y2Range = CalcRanges(WaveformDataset, Visibles.map(function(val) { return val + Analyzer.GetSeriesNames('Voltage').length }));
//      oZm.Series['CURRENT'].Names.map(function(val) { Series[val] = { axis: 'y2' }; });
        for(var n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length; n++) Visibility.push(this.VoltagePhases[n] === true);
        for(var n = 0; n < oZm.Series['CURRENT'].Names(Analyzer).length; n++) {
          Visibility.push(this.CurrentPhases[n] === true);
          if(this.CurrentPhases[n] === true) Series[oZm.Series['CURRENT'].Names(Analyzer)[n]] = { axis: 'y2' };
        }
        var Options = {
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: oZm.mouseUp, dblclick: oZm.dblClick_Restore },
          legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow, series: Series, visibility: Visibility,
          labels: ['Sample'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)).concat(oZm.Series['CURRENT'].Names(Analyzer)), 
          colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length).concat(oZm.ColorArray(oZm.Series['CURRENT'].Color, oZm.Series['CURRENT'].Names(Analyzer).length)), 
//          plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
          axes: {
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, //valueRange: Y1Range, 
                 valueFormatter: function (x) { return oZm.Round2(x) + ' <em class="x-small">V</em>'; },
                 axisLabelFormatter: function (x) { return oZm.Round2(x); }
            },
            y2: { pixelsPerLabel: 15, axisLabelWidth: (Object.keys(Series).length > 0) ? 34 : 0, //valueRange: Y2Range, 
                  valueFormatter: function (x) { return oZm.Round2(x) + ' <em class="x-small">A</em>'; },
                  axisLabelFormatter: function (x) { return (Object.keys(Series).length > 0) ? oZm.Round2(x) : ''; }
            },
            x: { ticker: function(min, max) { return GraphWaveTimeTicks(max, Analyzer); },
                 valueFormatter: function (x) { return oZm.Round2((x * 1000) / Analyzer.SampleFreq) + ' <em class="x-small">ms</em>'; }
               }
          },
          file: this.Data
        };
        this.Graph.updateOptions(Options);
      }
    },
    watch: {
      visible: function(newVal, oldVal) {
        if(newVal === false) return;      
        this.$nextTick(function() {    
          $(this.Graph.graphDiv).parent().removeAttr('style');
          window.dispatchEvent(new Event('resize'));
        });
      }
    },
    mounted: function() {
      this.Graph = new Dygraph(document.getElementById("WaveformChart"), this.Data, { });
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