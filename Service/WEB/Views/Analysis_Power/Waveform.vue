<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ (MouseOver || paused) ? '(Paused)' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :multi="false" :analyzer="analyzer" v-model="Phase"></toolbar-phases>
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
        <graph :options="GraphOptions" :height="Expanded?DialogHeight:300" :paused="paused"></graph>
      </div>
    </portal>
    <dlg-fs v-if="Expanded" title="Waveform" :height.sync="DialogHeight"></dlg-fs>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';
  export default {
    props: { analyzer: Object, data: Object, paused: Boolean },
    data() {
      return { MouseOver: false, Phase: null, Expanded: false, DialogHeight: 300, CurrentMult: 1.0 };
    },
    computed: {
      RmsCurrent: function() { return (this.data == null) ? 0.0 : oZm.ArrayAbsMax(this.data.Current); },
      GraphOptions: function() {
        var Opt = {};
        Opt.plugins = [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ];
        Opt.interactionModel = Graph.FixedRangeInteractionModel(this);
        Opt.drawAxesAtZero = true;
        Opt.legend = 'follow';
        Opt.showRangeSelector = false;
        Opt.strokeWidth = 0.5;
        Opt.legendFormatter = Graph.LegendFormatter_Follow;
        if((this.analyzer == null) || (this.Phase == null) || (this.data == null)) return Opt;
        Opt.series = { 'Voltage': { axis: 'y2' }, 'Current': { axis: 'y2' }, 'Power': { fillGraph: true }, 'Power SUM': { axis: 'y', strokeWidth: 2 } };
        Opt.labels = ['Sample', 'Voltage', 'Current', 'Power'].concat(this.analyzer.VoltageChannels > 1 ? ['Power SUM'] : []);
        Opt.colors = [this.analyzer.Color.Voltage, this.analyzer.Color.Current, this.analyzer.Color.Active_Power].concat(this.analyzer.VoltageChannels > 1 ? [this.analyzer.Color.Active_Power] : []);
        Opt.axes = { y: {}, y2: {}, x: {} };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.file = [];
        for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
          var Row = [i, this.data.Voltage_Samples[this.Phase][i], this.data.Current_Samples[this.Phase][i]];
          Row.push(Row[1] * Row[2]);
          if(this.analyzer.VoltageChannels > 1) {
            var Sum = 0;
            for(var j = 0; j < this.analyzer.VoltageChannels; j++) Sum = Sum + this.data.Voltage_Samples[j][i] * this.data.Current_Samples[j][i];
            Row.push(Sum);
          }
          Opt.file.push(Row);
        }
        var VoltageRange = this.MatrixColsRange(Opt.file, [false, true], { defMin: 0, defMax: 1, percent: 0.05, minWidth: 10 });
        var CurrentRange = this.MatrixColsRange(Opt.file, [false, false, true], { defMin: 0, defMax: 1, percent: 0.05, minWidth: 10 });
        var MaxVoltage = Math.max(Math.abs(VoltageRange[0]), Math.abs(VoltageRange[1]));
        var MaxCurrent = Math.max(Math.abs(CurrentRange[0]), Math.abs(CurrentRange[1]));
        this.CurrentMult = MaxVoltage / MaxCurrent;
        for(var i = 0; i < Opt.file.length; i++) Opt.file[i][2] = Opt.file[i][2] * this.CurrentMult;
        Opt.axes.y2.valueRange = [-MaxVoltage, MaxVoltage];
        var PowerRange = this.MatrixColsRange(Opt.file, [false, false, false, true, true], { defMin: 0, defMax: 1, percent: 0.05, minWidth: 10 });
        var MaxY1      = Math.max(Math.abs(PowerRange[0]), Math.abs(PowerRange[1]));
        Opt.valueRange = [-MaxY1, MaxY1];
        Opt.axes.y.valueFormatter = (x) => { var Result = oZm.Series['ACTIVE_POWER'].FormatValue(x); return Result.Value + ' ' + Result.Units; };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return (g.axes_[0].valueRange == null) ? '' : this.Round(this.ScaleUnits(x, this.BestUnits(Math.max(Math.abs(g.axes_[0].valueRange[0]), Math.abs(g.axes_[0].valueRange[1])))), 2, true); }
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.axisLabelWidth = 72;
        Opt.axes.y2.valueFormatter = (x, opts, seriesName) => { return oZm.Round2((seriesName == 'Voltage') ? x : (x / this.CurrentMult)) + ((seriesName == 'Voltage') ? ' V' : ' A'); };
        Opt.axes.y2.axisLabelFormatter = (x) => { return '<div style="display: inline-block; text-align: right; width: 36px; color: ' + oZm.Series['VOLTAGE'].Color + ' ">' + oZm.Round2(x) + '</div><div style="display: inline-block; text-align: right; width: 36px; color: ' + oZm.Series['CURRENT'].Color + ' ">' + oZm.Round2(x / this.CurrentMult) + '</div>'; };
        Opt.axes.x.ticker = (min, max) => { return Graph.WaveTimeTicks(max, this.analyzer); };
        Opt.axes.x.valueFormatter = (x) => { return oZm.Round2((x * 1000) / this.analyzer.SampleRate) + ' ms'; };
        return Object.seal(Opt);
      },
      Export: function() {
        if((this.data == null) || (this.analyzer == null)) return [];
        var Header = ['Sample']
          .concat(this.analyzer.GetSeriesNames('Voltage', this.analyzer.VoltageChannels))
          .concat(this.analyzer.GetSeriesNames('Current', this.analyzer.VoltageChannels))
          .concat(this.analyzer.GetSeriesNames('Power', this.analyzer.VoltageChannels));
        var Data = [Header];
        for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
          var Row = [i];
          for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.data.Voltage_Samples[n][i]);
          for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.data.Current_Samples[n][i]);
          for(var n = 0; n < this.data.Voltage_Samples.length; n++) Row.push(this.data.Voltage_Samples[n][i] * this.data.Current_Samples[n][i]);
          Data.push(Row);
        }
        return Object.freeze(Data);
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