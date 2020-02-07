<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ (MouseOver || paused) ? '(Paused)' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="true" :current="true" v-model="Phases"></toolbar-phases>
        <toolbar-options storage="DetailsOptions" :options="['Current']" v-model="Options"></toolbar-options>
        <div class="btn-group">
          <button-export file="Waveform" :rows="Export"></button-export>
          <button-expand :expanded.sync="Expanded"></button-expand>
          <button-pause :value="paused" @input="$emit('update:paused', $event)"></button-pause>
        </div>
      </portal>
    </div>
    <portal to="fs-body" :disabled="!Expanded" class="panel-body">
      <spinner v-if="data==null"></spinner>
      <div @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
        <graph :options="GraphOptions" :height="Expanded?DialogHeight:300" :paused="MouseOver"></graph>
      </div>
    </portal>
    <dlg-fs v-if="Expanded" title="Waveform" :height.sync="DialogHeight"></dlg-fs>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';
  export default {
    props: { analyzer: Object, data: Object, paused: Boolean },
    data() {
      return { MouseOver: false, Options: null, Phases: null, Expanded: false, DialogHeight: 300 };
    },
    computed: {
      RmsCurrent: function() { return (this.data == null) ? 0.0 : oZm.ArrayAbsMax(this.data.Current); },
      GraphOptions: function() {
        if((this.analyzer == null) || (this.Phases == null) || (this.Options == null)) return { };
        var Opt = { visibility: [], series: {}, axes: { y: {}, y2: {}, x: {} }, file: [] };
        var VoltageStyle = oZm.SerieStyle(oZm.Series['VOLTAGE'], this.analyzer, ((this.Options['Current'] == true) ? 1 : 0));
        var CurrentStyle = oZm.SerieStyle(oZm.Series['CURRENT'], this.analyzer, 0);
        var VoltageNames = this.analyzer.PhaseNames.slice(0, this.analyzer.VoltageChannels).map((val) => { return 'Voltage' + ' (' + val + ')'});
        var CurrentNames = this.analyzer.PhaseNames.slice(0, this.analyzer.CurrentChannels).map((val) => { return 'Current' + ' (' + val + ')'});
        for(var n in VoltageNames) Opt.visibility.push(this.Phases[n]);
        for(var n in CurrentNames) Opt.visibility.push(this.Phases[n] && this.Options['Current']);
        for(var n in CurrentNames) Opt.series[CurrentNames[n]] = { axis: 'y2' };
        Opt.drawAxesAtZero = true;
        Opt.plugins = [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ];
        Opt.interactionModel = Graph.FixedRangeInteractionModel(this);
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.LegendFormatter_Follow;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { return this.Round(x, 2, true) + ' V'; };
        Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(x, 2, true); };
        Opt.axes.y.strokeWidth = VoltageStyle.Width;
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.drawAxis = this.Options['Current'];
        Opt.axes.y2.axisLabelWidth = this.Options['Current'] ? 34 : 0;
        Opt.axes.y2.strokeWidth = CurrentStyle.Width;
        Opt.axes.y2.valueFormatter = (x) => { return this.Round(x, 2, true) + ' A'; };
        Opt.axes.y2.axisLabelFormatter = this.Options['Current'] ? (x) => { return this.Round(x, 2, true); } : (x)  => { return ''};
        Opt.axes.x.ticker = (min, max) => { return Graph.WaveTimeTicks(max, this.analyzer); };
        Opt.axes.x.valueFormatter = (x) => { return this.Round((x * 1000) / this.analyzer.SampleRate, 2, true) + ' ms'; };
        Opt.labels = ['Sample'].concat(VoltageNames).concat(CurrentNames);
        Opt.colors = VoltageStyle.Colors.concat(CurrentStyle.Colors);
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
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, V, { defMin: 0, defMax: 1, percent: 0.05, minWidth: 10 });
          Opt.axes.y2.valueRange = this.MatrixColsRange(Opt.file, I, { defMin: 0, defMax: 1, percent: 0.05, minWidth: 10 });
        }
        return Opt;
      },
      Export: function() {
        var Rows = [['Sample'].concat(oZm.Series['VOLTAGE'].Names(this.analyzer).concat(oZm.Series['CURRENT'].Names(this.analyzer)))];
        if(this.data != null) {
          for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
            var Row = [i];
            for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.data.Voltage_Samples[n][i]);
            for(var n = 0; n < this.data.Current_Samples.length; n++) Row.push(this.data.Current_Samples[n][i]);
            Rows.push(Row);
          }
        }
        return Rows;
      }
    }
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