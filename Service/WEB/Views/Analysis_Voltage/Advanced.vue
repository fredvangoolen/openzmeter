<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ paused ? '(Paused)' : MouseOver ? '(Graph paused)' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="true" :current="true" v-model="Phases"></toolbar-phases>
        <toolbar-options storage="DetailsOptions" :options="['Current', 'Log']" v-model="Options"></toolbar-options>
        <div class="btn-group">
          <button-export file="FFT" :rows="Export"></button-export>
          <button-expand v-model="Expanded"></button-expand>
          <button-pause :value="paused" @input="$emit('update:paused', $event)"></button-pause>
        </div>
      </portal>
    </div>
    <portal to="fs-body" :disabled="!Expanded" class="panel-body">
      <spinner v-if="data==null"></spinner>
      <div @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
        <graph :options="GraphOptions" :paused="MouseOver" :height="Expanded?DialogHeight:300" :analyzer="analyzer"></graph>
      </div>
    </portal>
    <dlg-fs v-if="Expanded" title="Advanced" :height.sync="DialogHeight"></dlg-fs>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
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
        Opt.plugins = [ Graph.Plugins.Vertical_Crosshair() ];
        Opt.interactionModel = Graph.InteractionModels.Fixed(this);
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrayBorder;
        if((this.Phases == null) || (this.Options == null)) return Opt;
        Opt.visibility = [];
        var VoltageStyle = oZm.SerieStyle(oZm.Series['VOLTAGE'], this.analyzer, ((this.Options['Current'] == true) ? 1 : 0));
        var CurrentStyle = oZm.SerieStyle(oZm.Series['CURRENT'], this.analyzer, 0);
        var VoltageNames = oZm.Series['VOLTAGE'].Names(this.analyzer);
        var CurrentNames = oZm.Series['CURRENT'].Names(this.analyzer);
        for(var n in VoltageNames) Opt.visibility.push(this.Phases[n]);
        for(var n in CurrentNames) Opt.visibility.push(this.Phases[n] && this.Options['Current']);
        for(var n in CurrentNames) Opt.series[CurrentNames[n]] = { axis: 'y2' };
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { return this.Round(x - this.VOffset, 2, true) + ' ' + (this.Options['Log'] ? 'dB' : 'V'); };
        Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(x - this.VOffset, 2, true); };
        Opt.axes.y.ticker = this.Options['Log'] ? (min, max) => { return Graph.Tickers.DBTicks(min, max, this.VOffset); } : Dygraph.numericTicks;
        Opt.axes.y.strokeWidth = VoltageStyle.Width;
        Opt.axes.y2.strokeWidth = CurrentStyle.Width;
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.drawAxis = this.Options['Current'];
        Opt.axes.y2.axisLabelWidth = this.Options['Current'] ? 34 : 0;
        Opt.axes.y2.valueFormatter = (x) => { return this.Round(x - this.COffset, 2, true) + ' ' + (this.Options['Log'] ? 'dB' : 'A'); };
        Opt.axes.y2.axisLabelFormatter = (x) => { return this.Round(x - this.COffset, 2, true); };
        Opt.axes.y2.ticker = this.Options['Current'] ? this.Options['Log'] ? (min, max) => { return Graph.Tickers.DBTicks(min, max, this.COffset) } : Dygraph.numericTicks : () => { return []; };
        Opt.axes.x.drawGrid = false;
        Opt.fillGraph = true;
        Opt.selectMode = 'euclidian';
        Opt.axes.x.axisLabelFormatter = (y) => {
          y = (y / 10 * this.analyzer.NominalFrequency);
          if(y >= 1000) {
            return (((y / 1000) % 1) !== 0) ? "" : (y / 1000) + ' kHz';
          } else {
            return ((y % 1) !== 0) ? "" : y + ' Hz';
          }
        };
        Opt.axes.x.valueFormatter = (y) => { return this.Round(y / 10 * this.analyzer.NominalFrequency, 0, true) + ' Hz'; };
        Opt.labels = ['Bin'].concat(VoltageNames).concat(CurrentNames);
        Opt.colors = VoltageStyle.Colors.concat(CurrentStyle.Colors);
        if(this.data != null) {
          for(var i = 0; i < this.data.Voltage_FFT[0].length; i++) {
          var Row = [i];
          for(var n = 0; n < this.data.Voltage_FFT.length; n++) Row.push(this.Options['Log'] ? 20.0 * Math.log10(Math.max(0.001, this.data.Voltage_FFT[n][i])) : this.data.Voltage_FFT[n][i]);
          for(var n = 0; n < this.data.Current_FFT.length; n++) Row.push(this.Options['Log'] ? 20.0 * Math.log10(Math.max(0.001, this.data.Current_FFT[n][i])) : this.data.Current_FFT[n][i]);
          Opt.file.push(Row);
          }
          Opt.dateWindow = [0, Opt.file.length];
          var V = [false], I = [false];
          for(var n in VoltageNames) { V.push(this.Phases[n]); I.push(false); }
          for(var n in CurrentNames) { I.push(this.Phases[n]); V.push(false); }
          var Y1Range = this.MatrixColsRange(Opt.file, V, { wave: 1.0 });
          var Y2Range = this.MatrixColsRange(Opt.file, I, { wave: 1.0 });
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
        return Opt;
      },
      Export: function() {
        var Data = [];
        if(this.data == null) return Data;
        var Header = ['Index'];
        for(var i = 0; i < oZm.Series['VOLTAGE'].Names(this.analyzer).length; i++) {
          Header.push(oZm.Series['VOLTAGE'].Names(this.analyzer)[i]);
          Header.push(oZm.Series['VOLTAGE'].Names(this.analyzer)[i] + ' (dB)');
        }
        for(var i = 0; i < oZm.Series['CURRENT'].Names(this.analyzer).length; i++) {
          Header.push(oZm.Series['CURRENT'].Names(this.analyzer)[i]);
          Header.push(oZm.Series['CURRENT'].Names(this.analyzer)[i] + ' (dB)');
        }
        Data.push(Header);
        for(var i = 0; i < this.data['Voltage_FFT'][0].length; i++) {
          var Row = [i];
          for(var n = 0; n < oZm.Series['VOLTAGE'].Names(this.analyzer).length; n++) {
            Row.push(this.data.Voltage_FFT[n][i]);
            Row.push(20.0 * Math.log10(Math.max(0.001, this.data.Voltage_FFT[n][i])));
          }
          for(var n = 0; n < oZm.Series['CURRENT'].Names(this.analyzer).length; n++) {
            Row.push(this.data.Current_FFT[n][i]);
            Row.push(20.0 * Math.log10(Math.max(0.001, this.data.Current_FFT[n][i])));
          }
          Data.push(Row);
        }
        return Data;
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