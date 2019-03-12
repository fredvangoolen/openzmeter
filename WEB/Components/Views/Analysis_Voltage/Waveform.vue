<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ Time }}</div>
        <div class="header-buttons" ref="Toolbar">
          <toolbar-phases variable="DetailsPhaseMulti" :multi="true" v-on:PhasesChanged="SetPhases"></toolbar-phases>
          <toolbar-options variable="DetailOptions" :options="['Current']" v-on:OptionChanged="SetOption"></toolbar-options>
          <toolbar-tools variable="To" ref="Tools" :options="['EXPORT', 'MAXIMIZE', 'PLAY']"></toolbar-tools>
        </div>
      </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div ref="Graph" class="chart big_chart"></div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'JS/moment.js';
  import Dygraph from 'JS/dygraph.js';
  import Spinner from 'Components/Spinner.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarOptions from 'Components/ToolbarOptions.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  function GraphWaveTimeTicks(max, analyzer) {
    var ticks = [];
    for(var i = 0; i <= Math.floor(max / analyzer.SampleFreq * analyzer.NominalFreq); i++) {
      ticks.push({ v: i * analyzer.SampleFreq / analyzer.NominalFreq, label: oZm.Round2(i / analyzer.NominalFreq * 1000) });
    }
    return ticks;
  }
  export default {
    data() {
      return { ShowSpinner: true, Time: 'Loading data...', Graph: null, Data: null, VoltagePhases: null, ShowCurrent: null };
    },
    methods: {
      Redraw() {
        this.$nextTick(function() {    
          $(this.Graph.graphDiv).parent().removeAttr('style');
          window.dispatchEvent(new Event('resize'));
        });
      },
      SetPhases: function(phases) {
        this.VoltagePhases = phases;
        this.CalcOptions();
      },
      SetOption: function(option, val) {
        if(option === 'Current') this.ShowCurrent = val;
        this.CalcOptions();
      },
      SetData(result) {
        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
        this.Data = [];
        for(var i = 0; i < result.Voltage_Samples[0].length; i++) {
          var Row = [i];
          for(var n = 0; n < result.Voltage_Samples.length; n++) Row.push(result.Voltage_Samples[n][i]);
          for(var n = 0; n < result.Current_Samples.length; n++) Row.push(result.Current_Samples[n][i]);
          this.Data.push(Row);
        }
        this.CalcOptions();
        this.ShowSpinner = false;
      },
      CalcOptions() {
        if((this.$root.SelectedAnalyzer === null) || (this.VoltagePhases === null) || (this.ShowCurrent === null) || (this.Data === null)) return;
        var Analyzer = this.$root.SelectedAnalyzer, Series = {}, CurrentPhases = [];
        for(var n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length; n++) {
          CurrentPhases.push((this.VoltagePhases[n] === true) && (this.ShowCurrent === true));
          if((this.VoltagePhases[n] === true) && (this.ShowCurrent === true)) Series[oZm.Series['CURRENT'].Names(Analyzer)[n]] = { axis: 'y2' };
        }
        var Y1Range = oZm.Series['VOLTAGE_SAMPLES'].Range(this.Data, this.VoltagePhases);
        var Y2Range = oZm.Series['CURRENT_SAMPLES'].Range(this.Data, Array(this.VoltagePhases.length).fill(false).concat(CurrentPhases));
        var Options = {
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: oZm.mouseUp, dblclick: oZm.dblClick_Restore },
          legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow, series: Series, visibility: this.VoltagePhases.concat(CurrentPhases),
          labels: ['Sample'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)).concat(oZm.Series['CURRENT'].Names(Analyzer)), 
          colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length).concat(oZm.ColorArray(oZm.Series['CURRENT'].Color, oZm.Series['CURRENT'].Names(Analyzer).length)), 
          plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ], showRangeSelector: false,
          axes: {
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, //valueRange: Y1Range, 
                 valueFormatter: function (x) { return oZm.Round2(x) + ' <em class="x-small">V</em>'; },
                 axisLabelFormatter: function (x) { return oZm.Round2(x); }
            },
            y2: { pixelsPerLabel: 15, axisLabelWidth: (this.ShowCurrent) ? 34 : 0, valueRange: Y2Range, 
                  valueFormatter: function (x) { return oZm.Round2(x) + ' <em class="x-small">A</em>'; },
                  axisLabelFormatter: function (x) { return (Object.keys(Series).length > 0) ? oZm.Round2(x) : ''; }
            },
            x: { ticker: function(min, max) { return GraphWaveTimeTicks(max, Analyzer); },
                 valueFormatter: function (x) { return oZm.Round2((x * 1000) / Analyzer.SampleFreq) + ' <em class="x-small">ms</em>'; }
               }
          },
          file: this.Data
        };
        this.Graph.updateOptions({ showRangeSelector: true}); 
        this.Graph.updateOptions(Options);
      }
    },
    mounted: function() {
      this.Graph = new Dygraph(this.$refs.Graph, [[0, 0]], { });
    },
    components: { Spinner, ToolbarPhases, ToolbarOptions, ToolbarTools }
  }
</script>