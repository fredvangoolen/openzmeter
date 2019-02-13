<template>
  <div id="HarmonicsPanel" v-bind:class="{ hide: !visible }">
    <div id="HarmonicsMaximize">
      <div id="HarmonicsChart" class="chart big_chart"></div>
    </div>
  </div>
</template>
<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'dygraphs';
  export default {
    props: ['visible'],
    data() {
      return { Graph: null, Data: [[0, 0]], VoltagePhases: [true], CurrentPhases: [true], First: true, Log: false, VOffset: 0, COffset: 0 };
    },
    methods: {
      SetOptions(log, first, voltage, current) {
        this.VoltagePhases = voltage;
        this.CurrentPhases = current;
        this.First = first;
        his.Log = log;
        this.CalcOptions();
      },
      SetData(result) {
        this.Data = [];
        for(var i = 0; i < 50; i++) {
          var Row = [];
          for(var n = 0; n < result.Voltage_Harmonics.length; n++) Row.push(result.Voltage_Harmonics[n][i]);
          for(var n = 0; n < result.Current_Harmonics.length; n++) Row.push(result.Current_Harmonics[n][i]);
          this.Data.push(Row);
        }
        this.CalcOptions();
      },
      CalcOptions() {
        if(this.$root.SelectedAnalyzer === null) return {};
        var Analyzer = this.$root.SelectedAnalyzer, Series = {}, Visibility = [], Y1Range = [], Y2Range = [], ToShow = [];
//        var ShowCurrent = (localStorage.getItem("VoltageDetailCurrent") === 'true');
//        var ShowFirst   = (localStorage.getItem('VoltageDetailFirstHarmonic') === 'true');
//        var Log         = (localStorage.getItem('VoltageDetailLog') === 'true');
        for(var i = this.First ? 0 : 1; i < this.Data.length; i++) {
          var Row = [i];
          for(var n = 0; n < this.Data[i].length; n++) Row.push(this.Log ? 20.0 * Math.log10(Math.max(0.001, this.Data[i][n])) : this.Data[i][n]);
          ToShow.push(Row);
        }
        for(var n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length; n++) Visibility.push(this.VoltagePhases[n] === true);
        for(var n = 0; n < oZm.Series['CURRENT'].Names(Analyzer).length; n++) {
          Visibility.push(this.CurrentPhases[n] === true);
          if(this.CurrentPhases[n] === true) Series[oZm.Series['CURRENT'].Names(Analyzer)[n]] = { axis: 'y2' };
        }
        var Options = { 
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { }, plotter: oZm.multiColumnBarPlotter, visibility: Visibility,
          dateWindow: [ToShow[0][0] - 0.5, ToShow[ToShow.length - 1][0] + 0.5], legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow, 
          labels: ['Harmonic'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)).concat(oZm.Series['CURRENT'].Names(Analyzer)), series: Series,
          colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length).concat(oZm.ColorArray(oZm.Series['CURRENT'].Color, oZm.Series['CURRENT'].Names(Analyzer).length)), 
          axes: { 
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, //valueRange: [Y1Range[0] + HarmonicsVOffset, Y1Range[1] + HarmonicsVOffset],
//                     ticker: Log ? function(min, max) { return GraphDBTicks(min, max, HarmonicsVOffset); } : Dygraph.numericTicks,
//                     valueFormatter: function(x) { return Round2(x - HarmonicsVOffset) + '<em class="x-small">' + (Log ? 'dB' : 'V') + '</em>'; },
                 axisLabelFormatter: function (x) { return oZm.Round2(x); }
               },
            y2: { pixelsPerLabel: 15, axisLabelWidth: (Object.keys(Series).length > 0) ? 34 : 0, //valueRange: [Y2Range[0] + HarmonicsCOffset, Y2Range[1] + HarmonicsCOffset],
//                      valueFormatter: function(x) { return Round2(x - HarmonicsCOffset) + ' <em class="x-small">' + (Log ? 'dB' : 'A') + '</em>'; },
//                      ticker: (Log || ShowCurrent === false) ? function(min, max) { return (ShowCurrent === true) ? GraphDBTicks(min, max, HarmonicsCOffset) : []; } : Dygraph.numericTicks,
                  axisLabelFormatter: function (x) { return oZm.Round2(x); }
                },
            x: { drawGrid: false, ticker: oZm.GraphOddHarmonics, axisLabelFormatter: function (x) { return oZm.Series['HARMONICS'].Names[x]; },
                 valueFormatter: function(y) { return oZm.Series['HARMONICS'].Names[y] + ' Harmonic (' + (Analyzer.NominalFreq * (y + 1)) + '<em class="small"> Hz</em>)'; }           
               }
          },
          file: ToShow
        };
        this.Graph.updateOptions(Options);
//      this.Graph.updateOptions({ file: this.Data });
//this.$nextTick(function() { $(this.Graph.graphDiv).parent().removeAttr('style'); });
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
      this.Graph = new Dygraph(document.getElementById("HarmonicsChart"), this.Data, {});
    }
  }
</script>

        
//GroupButtons($('#groupDetailPhasesMulti'), 'VoltageDetailVisiblePhasesMulti', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false, 
//          function(VisibleSeries, Visibles) {
//            Y1Range = CalcRanges(ToShow, Visibles, !Log);
//            Y2Range = CalcRanges(ToShow, Visibles.map(function(val) { return val + Analyzer.GetSeriesNames('Voltage').length }), !Log);
//            HarmonicsVOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
//            HarmonicsCOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
//            for(i = 0; i < ToShow.length; i++) {
//              var Row = [ToShow[i][0]];
//              for(n = 0; n < Analyzer.GetSeriesNames('Voltage').length; n++) Row.push(ToShow[i][n + 1] + HarmonicsVOffset);
//              for(n = 0; n < Analyzer.GetSeriesNames('Current').length; n++) Row.push(ToShow[i][n + 1 + Analyzer.GetSeriesNames('Voltage').length] + HarmonicsCOffset);
//              ToShow[i] = Row;
//            }
//            Analyzer.GetSeriesNames('Current').map(function(val) {Series[val] = { axis: 'y2' }; });
//
//              HarmonicsChart.updateOptions(Options, true);
//              HarmonicsChart.updateOptions({ file: ToShow });
//        }, undefined);
