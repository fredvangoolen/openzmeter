<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ MouseOver ? 'Paused while over graph' : Time }}</div> <div v-if="ShowWarning" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
        <div class="header-buttons" ref="Toolbar">
          <toolbar-phases variable="DetailsPhaseMulti" ref="phases" :multi="true" v-on:PhasesChanged="$parent.PhasesChangedMulti" type="CURRENT"></toolbar-phases>
          <toolbar-options variable="DetailOptions" :options="['Voltage', 'Log']" v-on:OptionChanged="SetOption"></toolbar-options>
          <toolbar-tools variable="To" ref="tools" :options="['EXPORT', 'MAXIMIZE', 'PLAY']" v-on:PlayStatus="$parent.PlayStatusChanged" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="Redraw"></toolbar-tools>
        </div>
      </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <graph ref="graph" :options="GraphOptions" v-on:mouseOver="MouseOver=$event"></graph>
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
      return { ShowWarning: true, MouseOver: false, ShowSpinner: true, Time: 'Loading data...', GraphOptions: { plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ], interactionModel: Graph.FixedRangeInteractionModel(this), selectMode: 'euclidian' } };
    },
    created: function() {
      this.MaxCurrent = 0;
      this.Data = null;
      this.Phases = null;
      this.ShowVoltage = null;
      this.Log = null;
      this.VOffset = 0;
      this.COffset = 0;
    },
    methods: {
      Redraw: function() {
        this.$nextTick(function() {
          this.$refs.graph.Redraw('100%', '300');
        });
      },
      SetPhases: function(phases) {
        this.$refs.phases.SetPhases(phases);
        this.CurrentPhases = phases;
        this.CalcOptions();
      },
      SetOption: function(option, val) {
        if(option === 'Voltage') this.ShowVoltage = val;
        if(option === 'Log') this.Log = val;
        this.CalcOptions();
      },
      SetData(result) {
        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
        this.Data = [];
        for(var i = 0; i < result.Voltage_FFT[0].length; i++) {
          var Row = [];
          for(var n = 0; n < result.Current_FFT.length; n++) Row.push(result.Current_FFT[n][i]);
          for(var n = 0; n < result.Voltage_FFT.length; n++) Row.push(result.Voltage_FFT[n][i]);
          this.Data.push(Row);
        }
        this.MaxCurrent = oZm.ArrayAbsMax(result.Current);
        this.CalcOptions();
        this.ShowSpinner = false;
      },
      CalcOptions() {
        if((this.$root.SelectedAnalyzer === null) || (this.CurrentPhases === null) || (this.ShowVoltage === null) || (this.Log === null) || (this.Data === null)) return;
        var Analyzer = this.$root.SelectedAnalyzer, ToShow = [], Series = {}, VoltagePhases = [];
        this.ShowWarning = (this.MaxCurrent < Analyzer.MinCurrent); 
        for(var i = 0; i < this.Data.length; i++) {
          var Row = [i];
          for(var n = 0; n < this.Data[i].length; n++) Row.push(this.Log ? 20.0 * Math.log10(Math.max(0.001, this.Data[i][n])) : this.Data[i][n]);
          ToShow.push(Row);
        }
        for(var n = 0; n < oZm.Series['CURRENT'].Names(Analyzer).length; n++) {
          VoltagePhases.push((this.CurrentPhases[n] === true) && (this.ShowVoltage === true));
          if((this.CurrentPhases[n] === true) && (this.ShowVoltage === true)) Series[oZm.Series['VOLTAGE'].Names(Analyzer)[n]] = { axis: 'y2' };
        }  
        var Y1Range = oZm.Series['CURRENT'].Range(ToShow, this.CurrentPhases);
        if(this.Log) Y1Range[0] = -60;
        var Y2Range = oZm.Series['VOLTAGE'].Range(ToShow, Array(this.CurrentPhases.length).fill(false).concat(VoltagePhases));
        if(this.Log) Y2Range[0] = -60;
        this.COffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
        this.VOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
        for(var i = 0; i < this.Data.length; i++) {
          var Row = [i];
          for(n = 0; n < oZm.Series['CURRENT'].Names(Analyzer).length; n++) Row.push(ToShow[i][n + 1] + this.COffset);
          for(n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length; n++) Row.push(ToShow[i][n + 1 + oZm.Series['CURRENT'].Names(Analyzer).length] + this.VOffset);
          ToShow[i] = Row;
        }
        var VoltageStyle = oZm.SerieStyle(oZm.Series['VOLTAGE'], Analyzer, 0);
        var CurrentStyle = oZm.SerieStyle(oZm.Series['CURRENT'], Analyzer, ((this.ShowVoltage) ? 1 : 0));
        var Options = {
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], fillGraph: true,
          //interactionModel: { mousedown: Graph.mouseDown, mousemove: Graph.mouseMove, mouseup  : oZm.mouseUp, dblclick : oZm.dblClick_Restore },
          legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow, labels: ['Bin'].concat(oZm.Series['CURRENT'].Names(Analyzer)).concat(oZm.Series['VOLTAGE'].Names(Analyzer)), series: Series, visibility: this.CurrentPhases.concat(VoltagePhases),
          showRangeSelector: false, colors: CurrentStyle.Colors.concat(VoltageStyle.Colors),
          axes: {
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: [Y1Range[0] + this.COffset, Y1Range[1] + this.COffset], strokeWidth: CurrentStyle.Width,
                 ticker: this.Log ? function(min, max) { return Graph.DBTicks(min, max, this.COffset); }.bind(this) : Dygraph.numericTicks,
                 valueFormatter: function(x) { return oZm.Round2(x - this.VOffset) + '<em class="x-small">' + (this.Log ? 'dB' : 'A') + '</em>'; }.bind(this),
                 axisLabelFormatter: function (x) { return oZm.Round2(x); }
               },
            y2: { pixelsPerLabel: 15, axisLabelWidth: this.ShowVoltage ? 34 : 0, valueRange: [Y2Range[0] + this.VOffset, Y2Range[1] + this.VOffset], strokeWidth: VoltageStyle.Width,
                  valueFormatter: function(x) { return oZm.Round2(x - this.VOffset) + ' <em class="x-small">' + (this.Log ? 'dB' : 'V') + '</em>'; }.bind(this),
                  ticker: (this.Log || this.ShowCurrent === false) ? function(min, max) { return (this.ShowVoltage === true) ? Graph.DBTicks(min, max, this.VOffset) : []; }.bind(this) : Dygraph.numericTicks,
                  axisLabelFormatter: function (x) { return oZm.Round2(x - this.VOffset); }.bind(this)
                },
            x: { drawGrid: true,  
                 valueFormatter: function (y) { return Math.round(y / 10 * Analyzer.NominalFreq) + '<em class="small"> Hz</em>'; },
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
        this.$refs.graph.SetOptions({ showRangeSelector: true}); 
        this.$refs.graph.SetOptions(Options);
      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Data = [];    
        var Header = ['Index'];
        for(var i = 0; i < oZm.Series['CURRENT'].Names(Analyzer).length; i++) {
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i]);
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i] + ' (dB)'); 
        }
        for(var i = 0; i < oZm.Series['VOLTAGE'].Names(Analyzer).length; i++) { 
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i]); 
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i] + ' (dB)');
        }
        Data.push(Header);
        for(var i = 0; i < this.Data.length; i++) {
          var Row = [i];
          for(var n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length + oZm.Series['CURRENT'].Names(Analyzer).length; n++) {
            Row.push(this.Data[i][n]); 
            Row.push(20.0 * Math.log10(Math.max(0.000000001, this.Data[i][n]))); 
          }
          Data.push(Row);
        }
        this.$refs.tools.SaveCVS('FFT.csv', Data);
      },
      MaximizeClick: function() {
        this.$refs.tools.Maximize('Advanced', this.$refs.graph.Graph, this.$refs.Toolbar);
      },
      WindowChange: function() {},
      WindowRestore: function() {
        this.$refs.graph.SetOptions( {dateWindow: [0, this.Data.length]} );
      }
    },
    components: { Spinner, ToolbarPhases, ToolbarOptions, ToolbarTools, Graph }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
  .title-warning {
    line-height: 20px;
    font-size: 14px;
  }
  .title-warning img {
    margin: -4px 4px 0px 0px;
  }
</style>