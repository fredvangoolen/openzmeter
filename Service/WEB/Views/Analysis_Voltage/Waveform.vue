<template>
  <q-tab-panel class="q-pa-none">
  <q-toolbar class="bg-grey-3 q-pr-none">
    <portal to="dlg-toolbar" :disabled="!Expanded">
      <q-toolbar-title>{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ paused ? '(Paused)' : MouseOver ? '(Graph paused)' : '' }}</q-toolbar-title>
      <div v-if="RmsCurrent<Analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <q-separator vertical />
      <toolbar-phases-multi storage="Analysis_Voltage.Waveform.Phases" :current="true" v-model="Phases" v-show="Analyzer.VoltageChannels() > 1" />
      <q-separator vertical v-if="Analyzer.VoltageChannels() > 1" />
        <!--toolbar-options storage="DetailsOptions" :options="['Current']" v-model="Options"></toolbar-options-->
        <button type="button" class="btn btn-default btn-xs" @click="ShowStream=true" title="Stream waves in realtime"><img :src="require('Imgs/diskette.png')"></button>
      <q-separator vertical />
      <toolbar-export file="Waveform" :rows="Export" />
      <q-separator vertical />
      <toolbar-pause :value="paused" @input="$emit('update:paused', $event)" />
      <q-separator vertical />
      <toolbar-expand v-model="Expanded" />
    </portal>
  </q-toolbar>
  <q-separator />
  <portal to="dlg-body" :disabled="!Expanded">
    <!--spinner v-if="data==null"></spinner-->
    <div @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
      <!--graph :analyzer="Analyzer" :options="GraphOptions" :height="Expanded?DialogHeight:300" :paused="MouseOver"></graph-->
    </div>
  </portal>
  <wave-stream v-if="ShowStream" @close="ShowStream=false"></wave-stream>
    <!--dlg-fs v-if="Expanded" title="Waveform" :height.sync="DialogHeight"></dlg-fs-->
  <dialog-fullscreen :modelValue="Expanded" color="bg-red-9" />
  </q-tab-panel>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import WaveStream from './WaveStream.vue';
  import Dygraph from 'JS/dygraph.js';
//  import Graph from 'Components/Graph.vue';
  export default {
    props: { data: Object, paused: Boolean },
    data: function() {
      return { MouseOver: false, Options: null, Phases: [], Expanded: false, DialogHeight: 300, ShowStream: false,
               Transfer: this.Analyzer.NowSeries.Suscribe(['Current', 'Voltage_Samples', 'Current_Samples'])
      };
    },
    beforeUnmount: function() {
      this.Analyzer.NowSeries.Unsuscribe(['Current', 'Voltage_Samples', 'Current_Samples']);
    },
    computed: {
      RmsCurrent: function() { return (this.data == null) ? 0.0 : this.ArrayAbsMax(this.data.Current); },
      GraphOptions: function() {
        var Opt = {file: [], axes: { y: {}, y2: {}, x: {} } };
        Opt.plugins = [ Graph.Plugins.Vertical_Crosshair() ];
        Opt.interactionModel = Graph.InteractionModels.Fixed(this);
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.drawAxesAtZero = true;
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.showRangeSelector = false;
        Opt.axes.x.ticker = (min, max) => { return Graph.Tickers.WaveTimeTicks(max, this.Analyzer); };
        if((this.Phases == null) || (this.Options == null)) return Opt;
        Opt.visibility = [];
        var VoltageNames = this.analyzer.SerieNames('Voltage');
        var CurrentNames = this.analyzer.SerieNames('Current');
        for(var n in VoltageNames) Opt.visibility.push(this.Phases[n]);
        for(var n in CurrentNames) Opt.visibility.push(this.Phases[n] && this.Options['Current']);
        Opt.series = {};
        for(var n in CurrentNames) Opt.series[CurrentNames[n]] = { axis: 'y2' };
        Opt.axes = { y: {}, y2: {}, x: {} };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { return this.Round(x, 2, true) + ' V'; };
        Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(x, 2, true); };
        Opt.axes.y.strokeWidth = (this.Options['Current'] == true) ? 2 : 0.5;;
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.drawAxis = this.Options['Current'];
        Opt.axes.y2.axisLabelWidth = this.Options['Current'] ? 34 : 0;
        Opt.axes.y2.strokeWidth = 0.5;
        Opt.axes.y2.valueFormatter = (x) => { return this.Round(x, 2, true) + ' A'; };
        Opt.axes.y2.axisLabelFormatter = this.Options['Current'] ? (x) => { return this.Round(x, 2, true); } : (x)  => { return ''};
        Opt.axes.x.valueFormatter = (x) => { return this.Round((x * 1000) / this.analyzer.SampleRate, 2, true) + ' ms'; };
        Opt.labels = ['Sample'].concat(VoltageNames).concat(CurrentNames);
        Opt.colors = this.analyzer.SerieColors('Voltage').concat(this.analyzer.SerieColors('Current'));
        if(this.data != null) {
          for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
            var Row = [i];
            for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.data.Voltage_Samples[n][i]);
            for(var n = 0; n < this.data.Current_Samples.length; n++) Row.push(this.data.Current_Samples[n][i]);
            Opt.file.push(Row);
          }
          var V = [false], I = [false];
          for(var n in VoltageNames) { V.push(this.Phases[n]); I.push(false); }
          for(var n in CurrentNames) { I.push(this.Phases[n]); V.push(false); }
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, V, { wave: 1.0 });
          Opt.axes.y2.valueRange = this.MatrixColsRange(Opt.file, I, { wave: 1.0 });
          Opt.dateWindow = [0, Opt.file.length];
        }
        return Object.seal(Opt);
      }
    },
    methods: {
      Export: function() {
        var Rows = [['Sample'].concat(this.analyzer.SerieNames('Voltage').concat(this.analyzer.SerieNames('Current')))];
        if(this.data != null) {
          for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
            var Row = [i];
            for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.Round(this.data.Voltage_Samples[n][i], 3));
            for(var n = 0; n < this.data.Current_Samples.length; n++) Row.push(this.Round(this.data.Current_Samples[n][i], 3));
            Rows.push(Row);
          }
        }
        return Object.seal(Rows);
      }
    },
    components: { WaveStream }
  }
</script>