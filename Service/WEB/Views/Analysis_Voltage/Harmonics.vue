<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ paused ? '(Paused)' : MouseOver ? '(Graph paused)' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="true" :current="true" v-model="Phases"></toolbar-phases>
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
    props: { analyzer: Object, data: Object, paused: Boolean },
    data() {
      return { MouseOver: false, Options: null, Phases: null, VOffset: 0, COffset: 0, Expanded: false, DialogHeight: 300 };
    },
    computed: {
      RmsCurrent: function() { return (this.data == null) ? 0.0 : oZm.ArrayAbsMax(this.data.Current); },
      GraphOptions: function() {
        var Opt = { series: {}, axes: { y: {}, y2: {}, x: {} }, file: [] };
        Opt.strokeWidth = 0.5;
        Opt.showRangeSelector = false;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.interactionModel = { };
        Opt.plotter = Graph.Plotters.MultiBar;
        Opt.drawHighlightPointCallback = Graph.Callbacks.DrawHighlightPoint_HighlightBar;
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        if((this.Phases == null) || (this.Options == null)) return { };
        Opt.visibility = [];
        var VoltageStyle = oZm.SerieStyle(oZm.Series['VOLTAGE'], this.analyzer, 0);
        var CurrentStyle = oZm.SerieStyle(oZm.Series['CURRENT'], this.analyzer, 0);
        for(var c in CurrentStyle.Colors) CurrentStyle.Colors[c] = chroma(CurrentStyle.Colors[c]).darken();
        var VoltageNames = oZm.Series['VOLTAGE'].Names(this.analyzer);
        var CurrentNames = oZm.Series['CURRENT'].Names(this.analyzer);
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
        Opt.axes.x.axisLabelFormatter = (x) => { return oZm.Series['HARMONICS'].Names[x]; };
        Opt.axes.x.valueFormatter = (y) => { return oZm.Series['HARMONICS'].Names[y] + ' Harmonic (' + (this.analyzer.NominalFrequency * (y + 1)) + 'Hz)'; };
        Opt.labels = ['Harmonic'].concat(VoltageNames).concat(CurrentNames);
        Opt.colors = VoltageStyle.Colors.concat(CurrentStyle.Colors);
        if(this.data != null) {
          for(var i = 0; i < 50; i++) {
            if((this.Options['1st Harmonic'] == false) && (i == 0)) continue;
            var Row = [i];
            for(var n = 0; n < this.data.Voltage_Harmonics.length; n++) Row.push(this.Options['Log'] ? 20.0 * Math.log10(Math.max(0.001, this.BinPower(this.data.Voltage_Harmonics[n][i]))) : this.BinPower(this.data.Voltage_Harmonics[n][i]));
            for(var n = 0; n < this.data.Current_Harmonics.length; n++) Row.push(this.Options['Log'] ? 20.0 * Math.log10(Math.max(0.001, this.BinPower(this.data.Current_Harmonics[n][i]))) : this.BinPower(this.data.Current_Harmonics[n][i]));
            Opt.file.push(Row);
          }
          Opt.dateWindow = [Opt.file[0][0] - 0.5, 50.5];
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
        for(var i = 0; i < oZm.Series['VOLTAGE'].Names(this.analyzer).length; i++) {
          Header.push(oZm.Series['VOLTAGE'].Names(this.analyzer)[i]);
          Header.push(oZm.Series['VOLTAGE'].Names(this.analyzer)[i] + ' (dB)');
          Header.push(oZm.Series['VOLTAGE'].Names(this.analyzer)[i] + ' (Complex)');
        }
        for(var i = 0; i < oZm.Series['CURRENT'].Names(this.analyzer).length; i++) {
          Header.push(oZm.Series['CURRENT'].Names(this.analyzer)[i]);
          Header.push(oZm.Series['CURRENT'].Names(this.analyzer)[i] + ' (dB)');
          Header.push(oZm.Series['CURRENT'].Names(this.analyzer)[i] + ' (Complex)');
        }
        Data.push(Header);
        for(var i = 0; i < oZm.Series['HARMONICS'].Names.length; i++) {
          var Row = [oZm.Series['HARMONICS'].Names[i]];
          for(var n = 0; n < oZm.Series['VOLTAGE'].Names(this.analyzer).length; n++) {
            Row.push(this.Round(this.BinPower(this.data.Voltage_Harmonics[n][i]), 3));
            Row.push(this.Round(20.0 * Math.log10(Math.max(0.001, this.BinPower(this.data.Voltage_Harmonics[n][i]))), 3));
            Row.push(this.Round(this.data.Voltage_Harmonics[n][i][0], 3) + ((this.data.Voltage_Harmonics[n][i][1] > 0) ? '+' + this.Round(this.data.Voltage_Harmonics[n][i][1], 3) : this.Round(this.data.Voltage_Harmonics[n][i][1], 3)) + 'i');
          }
          for(var n = 0; n < oZm.Series['CURRENT'].Names(this.analyzer).length; n++) {
            Row.push(this.Round(this.BinPower(this.data.Current_Harmonics[n][i]), 3));
            Row.push(this.Round(20.0 * Math.log10(Math.max(0.001, this.BinPower(this.data.Current_Harmonics[n][i]))), 3));
            Row.push(this.Round(this.data.Current_Harmonics[n][i][0], 3) + ((this.data.Current_Harmonics[n][i][1] > 0) ? '+' + this.Round(this.data.Current_Harmonics[n][i][1], 3) : this.Round(this.data.Current_Harmonics[n][i][1], 3)) + 'i');
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
  .title-warning {
    line-height: 20px;
    font-size: 14px;
  }
  .title-warning img {
    margin: -4px 4px 0px 0px;
  }
</style>