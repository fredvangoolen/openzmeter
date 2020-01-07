<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ MouseOver ? 'Paused while over graph' : Time }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <div class="header-buttons" ref="Toolbar">
        <toolbar-phases storage="DetailsPhaseMulti" :analyzer="analyzer" :multi="true" :current="true" :value="phases" @input="$emit('update:phases', $event)"></toolbar-phases>
        <toolbar-options storage="DetailOptions" :options="['Voltage']" v-model="Options"></toolbar-options>
        <div class="btn-group">
          <button-pause :value="paused" @input="$emit('update:paused', $event)"></button-pause>
        <!--toolbar-tools variable="To" ref="tools" :options="['EXPORT', 'MAXIMIZE', 'PLAY']" v-on:PlayStatus="$parent.PlayStatusChanged" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="Redraw"></toolbar-tools-->
        </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="data.Current==undefined"></spinner>
      <graph :options="GraphOptions" v-on:mouseOver="MouseOver=$event"></graph>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';
  export default {
    props: { analyzer: { type: Object, required: true }, data: { type: Object, required: true }, phases: { type: Array, required: true }, paused: { type: Boolean, required: true } },
    data() {
      return { MouseOver: false, Options: {} };
    },
    computed: {
      RmsCurrent: function() { return (this.data == null) ? 0.0 : oZm.ArrayAbsMax(this.data.Current); },
      Time: function() { return (this.data == null) ?  'Loading data...' : 'Last update at ' + moment(this.data.Sampletime).format('HH:mm:ss.SSS'); },
      GraphOptions: function() {
        var VoltageStyle = oZm.SerieStyle(oZm.Series['VOLTAGE'], this.analyzer, 0);
        var CurrentStyle = oZm.SerieStyle(oZm.Series['CURRENT'], this.analyzer, ((this.Options['Voltage'] == true) ? 1 : 0));
        var VoltageNames = oZm.Series['VOLTAGE'].Names(this.analyzer);
        var CurrentNames = oZm.Series['CURRENT'].Names(this.analyzer);
        var Opt = {};
        Opt.visibility = [];
        Opt.series = {};
        for(var n in CurrentNames) Opt.visibility.push(this.phases[n]);
        for(var n in VoltageNames) Opt.visibility.push(this.phases[n] && this.Options['Current']);
        for(var n in VoltageNames) Opt.series[CurrentNames[n]] = { axis: 'y2' };
        Opt.drawAxesAtZero = true;
        Opt.plugins = [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ];
        Opt.interactionModel = Graph.FixedRangeInteractionModel(this);
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.LegendFormatter_Follow;
        Opt.showRangeSelector = false;
        Opt.axes = { y: {}, y2: {}, x: {} };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        // Opt.axes.y.valueRange = oZm.Series['CURRENT_SAMPLES'].Range(this.Data, CurrentPhases);
        Opt.axes.y.strokeWidth = CurrentStyle.Width;
        Opt.axes.y.valueFormatter = (x) => { return oZm.Round2(x) + ' A'; };
        Opt.axes.y.axisLabelFormatter = (x) => { return oZm.Round2(x); };
        Opt.axes.y2.drawAxis = this.Options['Voltage'];
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.axisLabelWidth = this.Options['Voltage'] ? 34 : 0;
//        Opt.axes.y2.valueRange = Y2Range;
        Opt.axes.y2.strokeWidth = VoltageStyle.Width;
        Opt.axes.y2.valueFormatter = (x) => { return oZm.Round2(x) + ' V'; };
        Opt.axes.y2.axisLabelFormatter = this.Options['Voltage'] ? (x) => { return oZm.Round2(x); } : (x)  => { return ''};
        Opt.axes.x.ticker = (min, max) => { return Graph.WaveTimeTicks(max, this.analyzer); };
        Opt.axes.x.valueFormatter = (x) => { return oZm.Round2((x * 1000) / this.analyzer.SampleRate) + ' ms'; };
        Opt.labels = ['Sample'].concat(CurrentNames).concat(VoltageNames);
        Opt.colors = CurrentStyle.Colors.concat(VoltageStyle.Colors);
        Opt.file = [];
        if(this.data.Current_Samples != undefined) {
          for(var i = 0; i < this.data.Current_Samples[0].length; i++) {
            var Row = [i];
            for(var n = 0; n < this.data.Current_Samples.length; n++) Row.push(this.data.Current_Samples[n][i]);
            for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.data.Voltage_Samples[n][i]);
            Opt.file.push(Row);
          }
        }
        return Opt;





      }
    },

    methods: {
//      Redraw: function() {
//        this.$nextTick(function() {
//          this.$refs.graph.Redraw('100%', '300');
//        });
//      },
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
        var VoltageStyle = oZm.SerieStyle(oZm.Series['VOLTAGE'], Analyzer, 0);
        var CurrentStyle = oZm.SerieStyle(oZm.Series['CURRENT'], Analyzer, ((this.ShowVoltage) ? 1 : 0));
        var Options = {
          legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow, series: Series, visibility: this.CurrentPhases.concat(VoltagePhases),
          labels: ['Sample'].concat(oZm.Series['CURRENT'].Names(Analyzer)).concat(oZm.Series['VOLTAGE'].Names(Analyzer)),
          colors: CurrentStyle.Colors.concat(VoltageStyle.Colors), showRangeSelector: false,
          axes: {
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: Y1Range, strokeWidth: CurrentStyle.Width,
                 valueFormatter: function (x) { return oZm.Round2(x) + ' <em class="x-small">A</em>'; },
                 axisLabelFormatter: function (x) { return oZm.Round2(x); }
            },
            y2: { pixelsPerLabel: 15, axisLabelWidth: (this.ShowVoltage) ? 34 : 0, valueRange: Y2Range, strokeWidth: VoltageStyle.Width,
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
    components: { Graph }
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