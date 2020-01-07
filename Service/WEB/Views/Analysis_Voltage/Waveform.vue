<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ MouseOver ? 'Paused while over graph' : Time }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <div class="header-buttons" ref="Toolbar">
        <toolbar-phases storage="DetailsPhaseMulti" :analyzer="analyzer" :multi="true" :current="true" :value="phases" @input="$emit('update:phases', $event)"></toolbar-phases>
        <toolbar-options storage="DetailOptions" :options="['Current']" v-model="Options"></toolbar-options>
        <div class="btn-group">
          <button-pause :value="paused" @input="$emit('update:paused', $event)"></button-pause>
          <!--toolbar-tools variable="To" ref="tools" :options="['EXPORT', 'MAXIMIZE', 'PLAY']" v-on:PlayStatus="$parent.PlayStatusChanged" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="Redraw"></toolbar-tools-->
        </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="data.Voltage==undefined"></spinner>
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
        var VoltageStyle = oZm.SerieStyle(oZm.Series['VOLTAGE'], this.analyzer, ((this.Options['Current'] == true) ? 1 : 0));
        var CurrentStyle = oZm.SerieStyle(oZm.Series['CURRENT'], this.analyzer, 0);
        var VoltageNames = oZm.Series['VOLTAGE'].Names(this.analyzer);
        var CurrentNames = oZm.Series['CURRENT'].Names(this.analyzer);
        var Opt = {};
        Opt.visibility = [];
        Opt.series = {};
        for(var n in VoltageNames) Opt.visibility.push(this.phases[n]);
        for(var n in CurrentNames) Opt.visibility.push(this.phases[n] && this.Options['Current']);
        for(var n in CurrentNames) Opt.series[CurrentNames[n]] = { axis: 'y2' };
        Opt.drawAxesAtZero = true;
        Opt.plugins = [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ];
        Opt.interactionModel = Graph.FixedRangeInteractionModel(this);
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.LegendFormatter_Follow;
        Opt.showRangeSelector = false;
        Opt.axes = { y: {}, y2: {}, x: {} };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
       // Opt.axes.y.valueRange = oZm.Series['VOLTAGE_SAMPLES'].Range(this.Data, VoltagePhases);
        Opt.axes.y.strokeWidth = VoltageStyle.Width;
        Opt.axes.y.valueFormatter = (x) => { return oZm.Round2(x) + ' V'; };
        Opt.axes.y.axisLabelFormatter = (x) => { return oZm.Round2(x); };
        Opt.axes.y2.drawAxis = this.Options['Current'];
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.axisLabelWidth = this.Options['Current'] ? 34 : 0;
//        Opt.axes.y2.valueRange = Y2Range;
        Opt.axes.y2.strokeWidth = CurrentStyle.Width;
        Opt.axes.y2.valueFormatter = (x) => { return oZm.Round2(x) + ' <em class="x-small">A</em>'; };
        Opt.axes.y2.axisLabelFormatter = this.Options['Current'] ? (x) => { return oZm.Round2(x); } : (x)  => { return ''};
        Opt.axes.x.ticker = (min, max) => { return Graph.WaveTimeTicks(max, this.analyzer); };
        Opt.axes.x.valueFormatter = (x) => { return oZm.Round2((x * 1000) / this.analyzer.SampleRate) + ' ms'; };
        Opt.labels = ['Sample'].concat(VoltageNames).concat(CurrentNames);
        Opt.colors = VoltageStyle.Colors.concat(CurrentStyle.Colors);
        Opt.file = [];
        if(this.data.Voltage_Samples != undefined) {
          for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
            var Row = [i];
            for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.data.Voltage_Samples[n][i]);
            for(var n = 0; n < this.data.Current_Samples.length; n++) Row.push(this.data.Current_Samples[n][i]);
            Opt.file.push(Row);
          }
        }

//          CurrentPhases.push((this.Phases[n] === true) && (this.ShowCurrent === true));
//          if((this.Phases[n] == true) && (this.ShowCurrent === true)) Series[oZm.Series['CURRENT'].Names(Analyzer)[n]] = { axis: 'y2' };
//        }
//        for(var n in this.phases) {
//          CurrentPhases.push((this.Phases[n] === true) && (this.ShowCurrent === true));
//          if((this.Phases[n] == true) && (this.ShowCurrent === true)) Series[oZm.Series['CURRENT'].Names(Analyzer)[n]] = { axis: 'y2' };
//        }

//, series: Series, visibility: VoltagePhases.concat(CurrentPhases),


//        var Analyzer = this.$root.SelectedAnalyzer, Series = {}, CurrentPhases = [], VoltagePhases = this.Phases.slice(0, oZm.Series['VOLTAGE'].Names(Analyzer).length);

//        for(var n in this.phases) {
//          CurrentPhases.push((this.Phases[n] === true) && (this.ShowCurrent === true));
//          if((this.Phases[n] === true) && (this.ShowCurrent === true)) Series[oZm.Series['CURRENT'].Names(Analyzer)[n]] = { axis: 'y2' };
//        }
//        var Y1Range =
//        var Y2Range = oZm.Series['CURRENT_SAMPLES'].Range(this.Data, Array(VoltagePhases.length).fill(false).concat(CurrentPhases));
//        var Options = {





//        if((this.analyzer === null) || (this.phases === null) || (this.ShowCurrent === null) || (Array.isArray(this.Data) === false)) return ;

//this.$refs.graph.SetOptions({ showRangeSelector: true});
//        this.$refs.graph.SetOptions(Options);
        return Opt;
      }
    },

    methods: {
//      Redraw: function() {
//        this.$nextTick(function() {
//          this.$refs.graph.Redraw('100%', '300');
//        });
//      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS('Waveform.csv', [['Time'].concat(oZm.Series['VOLTAGE'].Names(Analyzer).concat(oZm.Series['CURRENT'].Names(Analyzer)))].concat(this.Data));
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