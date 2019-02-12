<template>
  <div id="HarmonicsPanel">
              <div id="HarmonicsMaximize">              
                <div id="HarmonicsChart" class="chart big_chart"></div>
              </div>
            </div>
</template>
<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'dygraphs';
  export default {
    data() {
      return { Graph: null, Data: [[0, 0]], VoltagePhases: [true], CurrentPhases: [false], VOffset: 0, COffset: 0 };
    },
    methods: {
      SetPhases(voltage, current) {
        this.VoltagePhases = voltage;
        this.CurrentPhases = current;
        this.Graph.updateOptions(this.CalcOptions());
      },
      SetData(result) {
        this.Data = [];
        for(var i = 0; i < 50; i++) {
          var Row = [];
          for(var n = 0; n < result.Voltage_Harmonics.length; n++) Row.push(result.Voltage_Harmonics[n][i]);
          for(var n = 0; n < result.Current_Harmonics.length; n++) Row.push(result.Current_Harmonics[n][i]);
          this.Data.push(Row);
        }
        this.Graph.updateOptions(this.CalcOptions(), true);
        this.Graph.updateOptions({ file: this.Data });
      },
      CalcOptions() {
        if(this.$root.SelectedAnalyzer === null) return {};
        return { 
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { }, 
          //plotter: multiColumnBarPlotter, dateWindow: [ToShow[0][0] - 0.5, ToShow[ToShow.length - 1][0] + 0.5],
          legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow, 
          //labels: ['Harmonic'].concat(Analyzer.GetSeriesNames('Voltage')).concat(Analyzer.GetSeriesNames('Current')), series: Series,
//              colors: ColorArray($('#VoltagePanel').css('border-color'), Analyzer.GetSeriesNames('Voltage').length).concat(ColorArray('#428bca', Analyzer.GetSeriesNames('Current').length)), visibility: VoltageVisibility.concat(CurrentVisibility),
//              axes: { 
//                y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: [Y1Range[0] + HarmonicsVOffset, Y1Range[1] + HarmonicsVOffset],
//                     ticker: Log ? function(min, max) { return GraphDBTicks(min, max, HarmonicsVOffset); } : Dygraph.numericTicks,
//                     valueFormatter: function(x) { return Round2(x - HarmonicsVOffset) + '<em class="x-small">' + (Log ? 'dB' : 'V') + '</em>'; },
//                     axisLabelFormatter: function (x) { return Round2(x); }
//                   },
//                y2: { pixelsPerLabel: 15, axisLabelWidth: ShowCurrent ? 34 : 0, valueRange: [Y2Range[0] + HarmonicsCOffset, Y2Range[1] + HarmonicsCOffset],
//                      valueFormatter: function(x) { return Round2(x - HarmonicsCOffset) + ' <em class="x-small">' + (Log ? 'dB' : 'A') + '</em>'; },
//                      ticker: (Log || ShowCurrent === false) ? function(min, max) { return (ShowCurrent === true) ? GraphDBTicks(min, max, HarmonicsCOffset) : []; } : Dygraph.numericTicks,
//                      axisLabelFormatter: function (x) { return Round2(x); }
//                    },
//                x: { drawGrid: false, ticker: GraphOddHarmonics, axisLabelFormatter: function (x) { return SeriesNames.Harmonics[x]; },
//                     valueFormatter: function(y) { return SeriesNames.Harmonics[y] + ' Harmonic (' + (Analyzers[Analyzer].NominalFreq * (y + 1)) + '<em class="small"> Hz</em>)'; }           
//                   }
//              }
            };
      }
    },
    mounted: function() {
      this.Graph = new Dygraph(document.getElementById("HarmonicsChart"), this.Data, this.CalcOptions());
    }
  }
</script>

        var ToShow      = [];
        var ShowCurrent = (localStorage.getItem("VoltageDetailCurrent") === 'true');
        var ShowFirst   = (localStorage.getItem('VoltageDetailFirstHarmonic') === 'true');
        var Log         = (localStorage.getItem('VoltageDetailLog') === 'true');
        for(i = ShowFirst ? 0 : 1; i < HarmonicsDataset.length; i++) {
          var Row = [i];
          for(n = 0; n < HarmonicsDataset[i].length; n++) Row.push(Log ? 20.0 * Math.log10(Math.max(0.001, HarmonicsDataset[i][n])) : HarmonicsDataset[i][n]);
          ToShow.push(Row);
        }
        var VoltageVisibility = [], CurrentVisibility = [], Y1Range = [], Y2Range = [], Series = {};
        GroupButtons($('#groupDetailPhasesMulti'), 'VoltageDetailVisiblePhasesMulti', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false, 
          function(VisibleSeries, Visibles) {
            VoltageVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Voltage').length);
            CurrentVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Current').length);
            if(ShowCurrent === false) CurrentVisibility.fill(false);
            Y1Range = CalcRanges(ToShow, Visibles, !Log);
            Y2Range = CalcRanges(ToShow, Visibles.map(function(val) { return val + Analyzer.GetSeriesNames('Voltage').length }), !Log);
            HarmonicsVOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
            HarmonicsCOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
            for(i = 0; i < ToShow.length; i++) {
              var Row = [ToShow[i][0]];
              for(n = 0; n < Analyzer.GetSeriesNames('Voltage').length; n++) Row.push(ToShow[i][n + 1] + HarmonicsVOffset);
              for(n = 0; n < Analyzer.GetSeriesNames('Current').length; n++) Row.push(ToShow[i][n + 1 + Analyzer.GetSeriesNames('Voltage').length] + HarmonicsCOffset);
              ToShow[i] = Row;
            }
            Analyzer.GetSeriesNames('Current').map(function(val) {Series[val] = { axis: 'y2' }; });

              HarmonicsChart.updateOptions(Options, true);
              HarmonicsChart.updateOptions({ file: ToShow });
        }, undefined);
