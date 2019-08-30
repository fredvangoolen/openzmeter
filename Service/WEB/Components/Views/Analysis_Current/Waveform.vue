<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ MouseOver ? 'Paused while over graph' : Time }}</div> <div v-if="ShowWarning" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <div class="header-buttons" ref="Toolbar">
        <toolbar-phases variable="DetailsPhaseMulti" ref="phases" :multi="true" v-on:PhasesChanged="$parent.PhasesChangedMulti" type="CURRENT"></toolbar-phases>
        <toolbar-options variable="DetailOptions" :options="['Voltage']" v-on:OptionChanged="SetOption"></toolbar-options>
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
  import ToolbarOptions from 'Components/ToolbarOptions.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  export default {
    data() {
      return { ShowWarning: true, MouseOver: false, ShowSpinner: true, Time: 'Loading data...', GraphOptions: { interactionModel: Graph.FixedRangeInteractionModel(this), selectMode: 'euclidian' } };
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
        this.CalcOptions();
      },
      SetData: function(result) {
        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
        this.Data = [];
        for(var i = 0; i < result.Voltage_Samples[0].length; i++) {
          var Row = [i];
          for(var n = 0; n < result.Current_Samples.length; n++) Row.push(result.Current_Samples[n][i]);
          for(var n = 0; n < result.Voltage_Samples.length; n++) Row.push(result.Voltage_Samples[n][i]);
          this.Data.push(Row);
        }
        this.MaxCurrent = oZm.ArrayAbsMax(result.Current);
        this.CalcOptions();
        this.ShowSpinner = false;
      },
      CalcOptions() {
        if((this.$root.SelectedAnalyzer === null) || (this.CurrentPhases === null) || (this.ShowVoltage === null) || (Array.isArray(this.Data) === false)) return;
        var Analyzer = this.$root.SelectedAnalyzer, Series = {}, VoltagePhases = [];
        this.ShowWarning = (this.MaxCurrent < Analyzer.MinCurrent); 
        for(var n = 0; n < oZm.Series['CURRENT'].Names(Analyzer).length; n++) {
          VoltagePhases.push((this.CurrentPhases[n] === true) && (this.ShowVoltage === true));
          if((this.CurrentPhases[n] === true) && (this.ShowVoltage === true)) Series[oZm.Series['VOLTAGE'].Names(Analyzer)[n]] = { axis: 'y2' };
        }
        var Y1Range = oZm.Series['CURRENT_SAMPLES'].Range(this.Data, this.CurrentPhases);
        var Y2Range = oZm.Series['VOLTAGE_SAMPLES'].Range(this.Data, Array(this.CurrentPhases.length).fill(false).concat(VoltagePhases));
        var Options = {
          interactionModel: { mousedown: Graph.mouseDown, mousemove: Graph.mouseMove, mouseup: oZm.mouseUp, dblclick: oZm.dblClick_Restore },
          legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow, series: Series, visibility: this.CurrentPhases.concat(VoltagePhases),
          labels: ['Sample'].concat(oZm.Series['CURRENT'].Names(Analyzer)).concat(oZm.Series['VOLTAGE'].Names(Analyzer)), 
          colors: oZm.ColorArray(oZm.Series['CURRENT'].Color, oZm.Series['CURRENT'].Names(Analyzer).length).concat(oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length)), 
          plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ], showRangeSelector: false,
          axes: {
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: Y1Range, 
                 valueFormatter: function (x) { return oZm.Round2(x) + ' <em class="x-small">A</em>'; },
                 axisLabelFormatter: function (x) { return oZm.Round2(x); }
            },
            y2: { pixelsPerLabel: 15, axisLabelWidth: (this.ShowVoltage) ? 34 : 0, valueRange: Y2Range, 
                  valueFormatter: function (x) { return oZm.Round2(x) + ' <em class="x-small">V</em>'; },
                  axisLabelFormatter: function (x) { return (Object.keys(Series).length > 0) ? oZm.Round2(x) : ''; }
            },
            x: { ticker: function(min, max) { return Graph.WaveTimeTicks(max, Analyzer); },
                 valueFormatter: function (x) { return oZm.Round2((x * 1000) / Analyzer.SampleRate) + ' <em class="x-small">ms</em>'; }
               }
          },
          file: this.Data
        };
        this.$refs.graph.SetOptions({ showRangeSelector: true}); 
        this.$refs.graph.SetOptions(Options);
      }, 
      ExportClick() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS('Waveform.csv', [['Time'].concat(oZm.Series['CURRENT'].Names(Analyzer).concat(oZm.Series['VOLTAGE'].Names(Analyzer)))].concat(this.Data));
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
      this.MaxCurrent = 0.0;
      this.Data = null;
      this.CurrentPhases = null;
      this.ShowVoltage = null;
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