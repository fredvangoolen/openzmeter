<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ MouseOver ? 'Paused while over graph' : Time }}</div> <div v-if="ShowWarning" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <div class="header-buttons" ref="Toolbar">
        <toolbar-phases variable="DetailsPhaseSingle" ref="phases" :multi="false" v-on:PhasesChanged="$parent.PhasesChangedSingle" type="VOLTAGE"></toolbar-phases>
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
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  export default {
    data() {
      return { ShowWarning: true, MouseOver: false, ShowSpinner: true, Time: 'Loading data...', GraphOptions: { interactionModel: Graph.FixedRangeInteractionModel(this), selectMode: 'euclidian' }
      };
    },
    methods: {
      Redraw: function() {
        this.$nextTick(function() {
          this.$refs.graph.Redraw('100%', '300');
        });
      },
      SetPhases: function(phase) {
        this.$refs.phases.SetPhases(phase);
        this.Phase = phase;
        this.CalcOptions();
      },
      SetOption: function(option, val) {
        if(option === 'Current') this.ShowCurrent = val;
        this.CalcOptions();
      },
      SetData: function(result) {
        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
        this.Data = result;
        this.CalcOptions();
        this.ShowSpinner = false;
      },
      CalcOptions: function() {
        if((this.$root.SelectedAnalyzer === null) || (this.Phase === null) || (this.Data === null)) return;
        var Analyzer = this.$root.SelectedAnalyzer, Series = { 'Voltage': {axis: 'y2', strokePattern: [1, 2]}, 'Current': {axis: 'y2', strokePattern: [1, 2]}, 'Power': { fillGraph: true }};
        this.ShowWarning = (this.Data.Current[this.Phase] < Analyzer.MinCurrent); 
        var ToShow = [];
        for(var i = 0; i < this.Data.Voltage_Samples[0].length; i++) ToShow.push([i, this.Data.Voltage_Samples[this.Phase][i], this.Data.Current_Samples[this.Phase][i], this.Data.Voltage_Samples[this.Phase][i] * this.Data.Current_Samples[this.Phase][i]]);
        var Y1Range = oZm.Series['VOLTAGE_SAMPLES'].Range(ToShow, [false, false, true]);
        var CurrentRange = oZm.Series['VOLTAGE_SAMPLES'].Range(ToShow, [false, true, false]);
        var VoltageRange = oZm.Series['VOLTAGE_SAMPLES'].Range(ToShow, [true, false, false]);
        var MaxVoltage = Math.max(Math.abs(VoltageRange[0]), Math.abs(VoltageRange[1]));
        var MaxCurrent = Math.max(Math.abs(CurrentRange[0]), Math.abs(CurrentRange[1]));
        var MaxY1      = Math.max(Math.abs(Y1Range[0]), Math.abs(Y1Range[1]));
        VoltageRange = [-MaxVoltage, MaxVoltage];
        Y1Range      = [-MaxY1, MaxY1];
        this.CurrentMult = MaxVoltage / MaxCurrent;
        for(var i = 0; i < ToShow.length; i++) ToShow[i][2] = ToShow[i][2] * this.CurrentMult;
        
        var Options = {
          legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow, series: Series, labels: ['Sample', 'Voltage', 'Current', 'Power'], colors: [oZm.Series['VOLTAGE'].Color, oZm.Series['CURRENT'].Color, oZm.Series['ACTIVE_POWER'].Color], 
          plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ], showRangeSelector: false,
          axes: {
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: Y1Range,
               valueFormatter: function (x) { var Result = oZm.Series['ACTIVE_POWER'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
               axisLabelFormatter: function (x) { var Result = oZm.Series['ACTIVE_POWER'].FormatValue(x); return Result.Value; }
            },
            y2: { pixelsPerLabel: 15, axisLabelWidth: 72, valueRange: VoltageRange,
                  valueFormatter: function (x, opts, seriesName) {
                    return oZm.Round2((seriesName == 'Voltage') ? x : (x / this.CurrentMult)) + ((seriesName == 'Voltage') ? ' V' : ' A'); 
                  }.bind(this),
                  axisLabelFormatter: function (x) { 
                    return '<div style="display: inline-block; text-align: right; width: 36px; color: ' + oZm.Series['VOLTAGE'].Color + ' ">' + oZm.Round2(x) + '</div><div style="display: inline-block; text-align: right; width: 36px; color: ' + oZm.Series['CURRENT'].Color + ' ">' + oZm.Round2(x / this.CurrentMult) + '</div>'; }.bind(this)
            },
            x: { ticker: function(min, max) { return Graph.WaveTimeTicks(max, Analyzer); },
                 valueFormatter: function (x) { return oZm.Round2((x * 1000) / Analyzer.SampleRate) + ' <em class="x-small">ms</em>'; }
               }
          },
          file: ToShow
        };
        this.$refs.graph.SetOptions({ showRangeSelector: true}); 
        this.$refs.graph.SetOptions(Options);
      },
      ExportClick: function() {
        var ToShow = [];
        for(var i = 0; i < this.Data.Voltage_Samples[0].length; i++) ToShow.push([i, this.Data.Voltage_Samples[this.Phase][i], this.Data.Current_Samples[this.Phase][i], this.Data.Voltage_Samples[this.Phase][i] * this.Data.Current_Samples[this.Phase][i]]);
        this.$refs.tools.SaveCVS('PowerWaveform.csv', [['Time', 'VOLTAGE', 'CURRENT', 'INSTANTANEOUS POWER']].concat(ToShow));
      },
      MaximizeClick: function() {
        this.$refs.tools.Maximize('Waveform', this.$refs.graph.Graph, this.$refs.Toolbar);
      },
      WindowChange: function() {},
      WindowRestore: function() {
        this.$refs.graph.SetOptions( {dateWindow: [0, this.Data.length]} );
      }
    },
    created: function() {
      this.CurrentMult = 1.0;
      this.Data = null;
      this.Phase = null;
    },
    components: { Spinner, ToolbarPhases, ToolbarTools, Graph }
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