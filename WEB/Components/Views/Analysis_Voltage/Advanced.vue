<template>
  <div id="FFTPanel" v-bind:class="{ hide: !visible }">
    <div id="FFTMaximize">
      <div id="FFTChart" class="chart big_chart"></div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'dygraphs';
  export default {
    props: ['visible'],
    data() {
      return { Graph: null, Data: [[0, 0]], VoltagePhases: [true], CurrentPhases: [true], Log: false, VOffset: 0, COffset: 0 };
    },
    methods: {
      SetOptions(log, voltage, current) {
        this.VoltagePhases = voltage;
        this.CurrentPhases = current;
        this.Log = log;
        this.CalcOptions();
      },
      SetData(result) {
        this.Data = [];
        for(var i = 0; i < result.Voltage_FFT[0].length; i++) {
          var Row = [];
          for(var n = 0; n < result.Voltage_FFT.length; n++) Row.push(result.Voltage_FFT[n][i]);
          for(var n = 0; n < result.Current_FFT.length; n++) Row.push(result.Current_FFT[n][i]);
          this.Data.push(Row);
        }
        this.CalcOptions();
      },
      CalcOptions() {
        if(this.$root.SelectedAnalyzer === null) return {};
        var Analyzer = this.$root.SelectedAnalyzer, ToShow = [], Series = {}, Visibility = [], Y1Range = [], Y2Range = [];
        for(var n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length; n++) Visibility.push(this.VoltagePhases[n] === true);
        for(var n = 0; n < oZm.Series['CURRENT'].Names(Analyzer).length; n++) {
          Visibility.push(this.CurrentPhases[n] === true);
          if(this.CurrentPhases[n] === true) Series[oZm.Series['CURRENT'].Names(Analyzer)[n]] = { axis: 'y2' };
        }
        for(var i = 0; i < this.Data.length; i++) {
          var Row = [i];
          for(var n = 0; n < this.Data[i].length; n++) Row.push(this.Log ? 20.0 * Math.log10(Math.max(0.001, this.Data[i][n])) : this.Data[i][n]);
          ToShow.push(Row);
        }
        var Options = {
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], fillGraph: true, interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup  : oZm.mouseUp, dblclick : oZm.dblClick_Restore },
          legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow, labels: ['Bin'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)).concat(oZm.Series['CURRENT'].Names(Analyzer)), series: Series, visibility: Visibility,
          //plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
          colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length).concat(oZm.ColorArray(oZm.Series['CURRENT'].Color, oZm.Series['CURRENT'].Names(Analyzer).length)),
          axes: { 
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, //valueRange: [Y1Range[0] + FFTVOffset, Y1Range[1] + FFTVOffset], 
//                 ticker: this.Log ? function(min, max) { return oZm.GraphDBTicks(min, max, FFTVOffset); } : Dygraph.numericTicks,
//                 valueFormatter: function(x) { return Round2(x - HarmonicsVOffset) + '<em class="x-small">' + (Log ? 'dB' : 'V') + '</em>'; },
                 axisLabelFormatter: function (x) { return oZm.Round2(x); }
               },
//            y2: { pixelsPerLabel: 15, axisLabelWidth: ShowCurrent ? 34 : 0, valueRange: [Y2Range[0] + FFTCOffset, Y2Range[1] + FFTCOffset],
//                  valueFormatter: function(x) { return Round2(x - FFTCOffset) + ' <em class="x-small">' + (Log ? 'dB' : 'A') + '</em>'; },
//                  ticker: (Log || ShowCurrent === false) ? function(min, max) { return (ShowCurrent === true) ? GraphDBTicks(min, max, FFTCOffset) : []; } : Dygraph.numericTicks,
//                  axisLabelFormatter: function (x) { return Round2(x); }
//                },
            x: { drawGrid: true,  
                 valueFormatter: function (y) { return Math.round(y / 10 * Analyzers.NominalFreq) + '<em class="small"> Hz</em>'; },
                 axisLabelFormatter: function (y) {
                   y = (y / 10 * Analyzer.NominalFreq);
                   if(y >= 1000) {
                     y = y / 1000;
                     if((y % 1) !== 0) return "";
                     return y + '<em class="small"> kHz</em>';
                   } else {
                     if((y % 1) !== 0) return "";
                     return y + '<em class="small"> Hz</em>';
                   }
                 }
               }
          },
          file: ToShow
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
      this.Graph = new Dygraph(document.getElementById("FFTChart"), this.Data, {});
    }
  }
</script>



  
  /* Grafica de FFT */
//function UpdateFFTChart() {
//  var ToShow      = [];
//  var ShowCurrent = (localStorage.getItem("VoltageDetailCurrent") === 'true');
//  var Log         = (localStorage.getItem('VoltageDetailLog') === 'true');


//  var VoltageVisibility = [], CurrentVisibility = [], Y1Range = [], Y2Range = [], Series = {};
//  GroupButtons($('#groupDetailPhasesMulti'), 'VoltageDetailVisiblePhasesMulti', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false, 
//    function(VisibleSeries, Visibles) {
//      VoltageVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Voltage').length);
//      CurrentVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Current').length);
//      if(ShowCurrent === false) CurrentVisibility.fill(false);
//      Y1Range = CalcRanges(ToShow, Visibles, !Log);
//      Y2Range = CalcRanges(ToShow, Visibles.map(function(val) { return val + Analyzer.GetSeriesNames('Voltage').length }), !Log);
//      FFTVOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
//      FFTCOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
//      for(i = 0; i < ToShow.length; i++) {
//        var Row = [i];
//        for(n = 0; n < Analyzer.GetSeriesNames('Voltage').length; n++) Row.push(ToShow[i][n + 1] + FFTVOffset);
//        for(n = 0; n < Analyzer.GetSeriesNames('Current').length; n++) Row.push(ToShow[i][n + 1 + Analyzer.GetSeriesNames('Voltage').length] + FFTCOffset);
//        ToShow[i] = Row;
//      }
//      Analyzer.GetSeriesNames('Current').map(function(val) {Series[val] = { axis: 'y2' }; });
//        
//    }
//  };
//    if(FFTChart === null) {
//      
//    } else {
//      FFTChart.updateOptions(Options, true);
//      FFTChart.updateOptions({ file: ToShow });
//    }
//  }, undefined);
//}
  
