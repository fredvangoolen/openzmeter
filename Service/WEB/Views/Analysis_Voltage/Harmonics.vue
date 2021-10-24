<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ paused ? '(Paused)' : MouseOver ? '(Graph paused)' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :multi="true" :current="true" v-model="Phases"></toolbar-phases>
        <toolbar-options storage="DetailsOptions" :options="['1st Harmonic', 'Current', 'Log']" v-model="Options"></toolbar-options>
        <div class="btn-group">
          <button-export file="Harmonics" :rows="Export"></button-export>
          <button-expand v-model="Expanded"></button-expand>
          <button-pause :value="paused" @input="$emit('update:paused', $event)"></button-pause>
        </div>
      </portal>
    </div>
    <portal to="fs-body" :disabled="!Expanded" class="panel-body">
      <spinner v-if="data==null"></spinner>
      <div @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
        <graph :options="GraphOptions" :paused="MouseOver" :height="Expanded?DialogHeight:300"></graph>
      </div>
    </portal>
    <dlg-fs v-if="Expanded" title="Harmonics" :height.sync="DialogHeight"></dlg-fs>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import chroma from 'chroma-js';
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';
  export default {
    props: { data: Object, paused: Boolean },
    data: function() {
      return { MouseOver: false, Options: null, Phases: null, VOffset: 0, COffset: 0, Expanded: false, DialogHeight: 300 };
    },
    computed: {
      RmsCurrent: function() { return (this.data == null) ? 0.0 : oZm.ArrayAbsMax(this.data.Current); },
      GraphOptions: function() {
        var Opt = { series: {}, axes: { y: {}, y2: {}, x: {} }, file: [] };
        Opt.strokeWidth = 0.5;
        Opt.showRangeSelector = false;
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.interactionModel = { };
        Opt.plotter = Graph.Plotters.MultiBar;
        Opt.drawHighlightPointCallback = Graph.Callbacks.DrawHighlightPoint_HighlightBar;
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        if((this.Phases == null) || (this.Options == null)) return { };
        Opt.visibility = [];
        var VoltageNames = this.analyzer.SerieNames('Voltage');
        var CurrentNames = this.analyzer.SerieNames('Current');
        for(var n in VoltageNames) Opt.visibility.push(this.Phases[n]);
        for(var n in CurrentNames) Opt.visibility.push(this.Phases[n] && this.Options['Current']);
        for(var n in CurrentNames) Opt.series[CurrentNames[n]] = { axis: 'y2' };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { return this.Round(x - this.VOffset, 2, true) + ' ' + (this.Options['Log'] ? 'dB' : 'V'); };
        Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(x - this.VOffset, 2, true); };
        Opt.axes.y.ticker = this.Options['Log'] ? (min, max) => { return Graph.Tickers.DBTicks(min, max, this.VOffset); } : Graph.Tickers.Default;
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.drawAxis = this.Options['Current'];
        Opt.axes.y2.axisLabelWidth = this.Options['Current'] ? 34 : 0;
        Opt.axes.y2.valueFormatter = (x) => { return this.Round(x - this.COffset, 2, true) + ' ' + (this.Options['Log'] ? 'dB' : 'A'); };
        Opt.axes.y2.axisLabelFormatter = (x) => { return this.Round(x - this.COffset, 2, true); };
        Opt.axes.y2.ticker = this.Options['Current'] ? this.Options['Log'] ? (min, max) => { return Graph.Tickers.DBTicks(min, max, this.COffset) } : Graph.Tickers.Default : () => { return []; };
        Opt.axes.x.drawGrid = false;
        Opt.axes.x.ticker = () => { return Graph.Tickers.OddHarmonicsTicks(this.analyzer); };
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.HarmonicsNames[x]; };
        Opt.axes.x.valueFormatter = (y) => { return this.analyzer.HarmonicsNames[y] + ' Harmonic (' + (this.analyzer.NominalFrequency * (y + 1)) + 'Hz)'; };
        Opt.labels = ['Harmonic'].concat(VoltageNames).concat(CurrentNames);
        Opt.colors = this.analyzer.SerieColors('Voltage').concat(this.analyzer.SerieColors('Current').map((val) => { return chroma(val).darken() }));
        if(this.data != null) {
          for(var i = 0; i < 50; i++) {
            if((this.Options['1st Harmonic'] == false) && (i == 0)) continue;
            var Row = [i];
            for(var n = 0; n < this.data.Voltage_Harmonics_Complex.length; n++) Row.push(this.Options['Log'] ? 20.0 * Math.log10(Math.max(0.001, this.BinPower(this.data.Voltage_Harmonics_Complex[n][i]))) : this.BinPower(this.data.Voltage_Harmonics_Complex[n][i]));
            for(var n = 0; n < this.data.Current_Harmonics_Complex.length; n++) Row.push(this.Options['Log'] ? 20.0 * Math.log10(Math.max(0.001, this.BinPower(this.data.Current_Harmonics_Complex[n][i]))) : this.BinPower(this.data.Current_Harmonics_Complex[n][i]));
            Opt.file.push(Row);
          }
          Opt.dateWindow = [Opt.file[0][0] - 0.5, 49.5];
          var V = [false], I = [false];
          for(var n in VoltageNames) { V.push(this.Phases[n]); I.push(false); }
          for(var n in CurrentNames) { I.push(this.Phases[n]); V.push(false); }
          var Y1Range = this.MatrixColsRange(Opt.file, V, { defMin: 0, defMax: 1, upperPercent: 0.05 });
          var Y2Range = this.MatrixColsRange(Opt.file, I, { defMin: 0, defMax: 1, upperPercent: 0.05 });
          if(this.Options['Log'] == true) { Y1Range[0] = -60; Y2Range[0] = -60; }
          this.VOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
          this.COffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
          Opt.axes.y.valueRange = [Y1Range[0] + this.VOffset, Y1Range[1] + this.VOffset];
          Opt.axes.y2.valueRange = [Y2Range[0] + this.COffset, Y2Range[1] + this.COffset];
          for(var i in Opt.file) {
            for(var n in Opt.file[i]) {
              if(V[n] == true) Opt.file[i][n] += this.VOffset;
              if(I[n] == true) Opt.file[i][n] += this.COffset;
            }
          }
        }
        return Object.seal(Opt);
      },
    },
    methods: {
      Export: function() {
        var Data = [];
        if(this.data == null) return Data;
        var Header = ['Harmonic'];
        var VoltageNames = this.analyzer.SerieNames('Voltage');
        for(var i = 0; i < VoltageNames.length; i++) {
          Header.push(VoltageNames[i]);
          Header.push(VoltageNames[i] + ' (dB)');
          Header.push(VoltageNames[i] + ' (Complex)');
        }
        var CurrentNames = this.analyzer.SerieNames('Current');
        for(var i = 0; i < CurrentNames.length; i++) {
          Header.push(CurrentNames[i]);
          Header.push(CurrentNames[i] + ' (dB)');
          Header.push(CurrentNames[i] + ' (Complex)');
        }
        Data.push(Header);
        for(var i = 0; i < this.analyzer.HarmonicsNames.length; i++) {
          var Row = [this.analyzer.HarmonicsNames[i]];
          for(var n = 0; n < VoltageNames.length; n++) {
            Row.push(this.Round(this.BinPower(this.data.Voltage_Harmonics_Complex[n][i]), 3));
            Row.push(this.Round(20.0 * Math.log10(Math.max(0.001, this.BinPower(this.data.Voltage_Harmonics_Complex[n][i]))), 3));
            Row.push(this.Round(this.data.Voltage_Harmonics_Complex[n][i][0], 3) + ((this.data.Voltage_Harmonics_Complex[n][i][1] >= 0) ? '+' + this.Round(this.data.Voltage_Harmonics_Complex[n][i][1], 3) : this.Round(this.data.Voltage_Harmonics_Complex[n][i][1], 3)) + 'i');
          }
          for(var n = 0; n < CurrentNames.length; n++) {
            Row.push(this.Round(this.BinPower(this.data.Current_Harmonics_Complex[n][i]), 3));
            Row.push(this.Round(20.0 * Math.log10(Math.max(0.001, this.BinPower(this.data.Current_Harmonics_Complex[n][i]))), 3));
            Row.push(this.Round(this.data.Current_Harmonics_Complex[n][i][0], 3) + ((this.data.Current_Harmonics_Complex[n][i][1] >= 0) ? '+' + this.Round(this.data.Current_Harmonics_Complex[n][i][1], 3) : this.Round(this.data.Current_Harmonics_Complex[n][i][1], 3)) + 'i');
          }
          Data.push(Row);
        }
        return Object.seal(Data);
      }
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
</style>