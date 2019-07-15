<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ Time }}</div>
        <div class="header-buttons" ref="Toolbar">
          <toolbar-phases variable="DetailsPhaseMulti" ref="phases" :multi="true" v-on:PhasesChanged="$parent.PhasesChangedMulti" type="CURRENT"></toolbar-phases>
          <toolbar-options variable="DetailOptions" :options="['1st Harmonic', 'Current', 'Log']" v-on:OptionChanged="SetOption"></toolbar-options>
          <toolbar-tools variable="To" ref="tools" :options="['EXPORT', 'MAXIMIZE', 'PLAY']" v-on:PlayStatus="$parent.PlayStatusChanged" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="Redraw"></toolbar-tools>
        </div>
      </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <graph ref="graph" :options="{}"></graph>
    </div>
  </div>
</template>
<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'JS/moment.js';
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';
  import Spinner from 'Components/Spinner.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarOptions from 'Components/ToolbarOptions.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  export default {
    data() {
      return { ShowSpinner: true, Time: 'Loading data...' };
    },
    methods: {
      BinPower: function(value) {
        return Math.sqrt(Math.pow(value[0], 2.0) + Math.pow(value[1], 2.0)) / Math.SQRT2 * 2.0;
      },
      Redraw: function() {
        this.$nextTick(function() {
          this.$refs.graph.Redraw('100%', '300');
        });
      },
      SetPhases: function(phases) {
        this.$refs.phases.SetPhases(phases);
        this.VoltagePhases = phases;
        this.CalcOptions();
      },
      SetOption: function(option, val) {
        if(option === 'Current') this.ShowCurrent = val;
        if(option === '1st Harmonic') this.First = val;
        if(option === 'Log') this.Log = val;
        this.CalcOptions();
      },
      SetData(result) {
        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
        this.Data = [];
        for(var i = 0; i < 50; i++) {
          var Row = [];
          for(var n = 0; n < result.Voltage_Harmonics.length; n++) Row.push(result.Voltage_Harmonics[n][i]);
          for(var n = 0; n < result.Current_Harmonics.length; n++) Row.push(result.Current_Harmonics[n][i]);
          this.Data.push(Row);
        }
        this.CalcOptions();
        this.ShowSpinner = false;
      },
      CalcOptions() {
        if((this.$root.SelectedAnalyzer === null) || (this.VoltagePhases === null) || (this.ShowCurrent === null) || (this.First === null) || (this.Log === null) || (this.Data === null)) return;
        var Analyzer = this.$root.SelectedAnalyzer, Series = {}, ToShow = [], CurrentPhases = [];
        for(var i = this.First ? 0 : 1; i < this.Data.length; i++) {
          var Row = [i];
          for(var n = 0; n < this.Data[i].length; n++) Row.push(this.Log ? 20.0 * Math.log10(Math.max(0.001, this.BinPower(this.Data[i][n]))) : this.BinPower(this.Data[i][n]));
          ToShow.push(Row);
        }
        for(var n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length; n++) {
          CurrentPhases.push((this.VoltagePhases[n] === true) && (this.ShowCurrent === true));
          if((this.VoltagePhases[n] === true) && (this.ShowCurrent === true)) Series[oZm.Series['CURRENT'].Names(Analyzer)[n]] = { axis: 'y2' };
        }
        var Y1Range = oZm.Series['VOLTAGE_SAMPLES'].Range(ToShow, this.VoltagePhases);
        var Y2Range = oZm.Series['CURRENT_SAMPLES'].Range(ToShow, Array(this.VoltagePhases.length).fill(false).concat(CurrentPhases));
        this.VOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
        this.COffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
        for(var i = 0; i < ToShow.length; i++) {
          var Row = [ToShow[i][0]];
          for(n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length; n++) Row.push(ToShow[i][n + 1] + this.VOffset);
          for(n = 0; n < oZm.Series['CURRENT'].Names(Analyzer).length; n++) Row.push(ToShow[i][n + 1 + oZm.Series['VOLTAGE'].Names(Analyzer).length] + this.COffset);
          ToShow[i] = Row;
        }
        var Options = {
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { }, plotter: Graph.multiColumnBarPlotter, visibility: this.VoltagePhases.concat(CurrentPhases),
          dateWindow: [ToShow[0][0] - 0.5, ToShow[ToShow.length - 1][0] + 0.5], legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow, showRangeSelector: false,
          drawHighlightPointCallback: Graph.DrawHighlightBarChartCallback,
          labels: ['Harmonic'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)).concat(oZm.Series['CURRENT'].Names(Analyzer)), series: Series,
          colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length).concat(oZm.ColorArray(oZm.Series['CURRENT'].Color, oZm.Series['CURRENT'].Names(Analyzer).length)), 
          axes: { 
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: [Y1Range[0] + this.VOffset, Y1Range[1] + this.VOffset],
                 ticker: this.Log ? function(min, max) { return Graph.DBTicks(min, max, this.VOffset); }.bind(this) : Dygraph.numericTicks,
                 valueFormatter: function(x) { return oZm.Round2(x - this.VOffset) + '<em class="x-small">' + (this.Log ? 'dB' : 'V') + '</em>'; }.bind(this),
                 axisLabelFormatter: function (x) { return oZm.Round2(x - this.VOffset); }.bind(this)
               },
            y2: { pixelsPerLabel: 15, axisLabelWidth: (this.ShowCurrent) ? 34 : 0, valueRange: [Y2Range[0] + this.COffset, Y2Range[1] + this.COffset],
                  valueFormatter: function(x) { return oZm.Round2(x - this.COffset) + ' <em class="x-small">' + (this.Log ? 'dB' : 'A') + '</em>'; }.bind(this),
                  ticker: (this.Log || this.ShowCurrent === false) ? function(min, max) { return (this.ShowCurrent === true) ? Graph.DBTicks(min, max, this.COffset) : []; }.bind(this) : Dygraph.numericTicks,
                  axisLabelFormatter: function (x) { return oZm.Round2(x - this.COffset); }.bind(this)
                },
            x: { drawGrid: false, ticker: Graph.OddHarmonicsTicks, axisLabelFormatter: function (x) { return oZm.Series['HARMONICS'].Names[x]; },
                 valueFormatter: function(y) { return oZm.Series['HARMONICS'].Names[y] + ' Harmonic (' + (Analyzer.NominalFreq * (y + 1)) + '<em class="small"> Hz</em>)'; }           
               }
          },
          file: ToShow
        };
        this.$refs.graph.SetOptions({ showRangeSelector: true}); 
        this.$refs.graph.SetOptions(Options);
      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Data = [];
        var Header = ['Harmonic'];
        for(var i = 0; i < oZm.Series['VOLTAGE'].Names(Analyzer).length; i++) { 
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i]);
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i] + ' (dB)');
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i] + ' (Complex)');
        }
        for(var i = 0; i < oZm.Series['CURRENT'].Names(Analyzer).length; i++) {
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i]);
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i] + ' (dB)');
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i] + ' (Complex)');
        }
        Data.push(Header);
        for(i = 0; i < oZm.Series['HARMONICS'].Names.length; i++) {
          var Row = [oZm.Series['HARMONICS'].Names[i]];
          for(var n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length + oZm.Series['CURRENT'].Names(Analyzer).length; n++) {
            Row.push(this.BinPower(this.Data[i][n]));
            Row.push(20.0 * Math.log10(Math.max(0.000000001, this.BinPower(this.Data[i][n]))));
            Row.push(this.Data[i][n][0] + ((this.Data[i][n][1] > 0) ? '+' + this.Data[i][n][1] : this.Data[i][n][1]) + 'i');
          }
          Data.push(Row);
        }
        this.$refs.tools.SaveCVS('Harmonics.csv', Data);
      },
      MaximizeClick: function() {
        this.$refs.tools.Maximize('Harmonics', this.$refs.graph.Graph, this.$refs.Toolbar);
      }
    },
    created: function() {
      this.Data = null;
      this.VoltagePhases = null;
      this.ShowCurrent = null;
      this.First = null;
      this.Log = null;
      this.VOffset = 0;
      this.COffset = 0;
    },
    components: { Spinner, ToolbarPhases, ToolbarOptions, ToolbarTools, Graph }
  }
</script>