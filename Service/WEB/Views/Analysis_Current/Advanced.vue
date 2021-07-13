<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ paused ? '(Paused)' : MouseOver ? '(Graph paused)' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="true" :current="true" v-model="Phases"></toolbar-phases>
        <toolbar-options storage="DetailsOptions" :options="['Voltage', 'Log']" v-model="Options"></toolbar-options>
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
        <graph :options="GraphOptions" :paused="MouseOver" :height="Expanded?DialogHeight:300"></graph>
      </div>
    </portal>
    <dlg-fs v-if="Expanded" title="Advanced" :height.sync="DialogHeight"></dlg-fs>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
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
        Opt.showRangeSelector = false;
        Opt.plugins = [ Graph.Plugins.Vertical_Crosshair() ];
        Opt.interactionModel = Graph.InteractionModels.Fixed(this);
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.selectMode = 'euclidian';
        Opt.fillGraph = true;
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        if((this.Phases == null) || (this.Options == null)) return Opt;
        Opt.visibility = [];
        var VoltageNames = this.analyzer.SerieNames('Voltage');
        var CurrentNames = this.analyzer.SerieNames('Current');
        for(var n in CurrentNames) Opt.visibility.push(this.Phases[n]);
        for(var n in VoltageNames) Opt.visibility.push(this.Phases[n] && this.Options['Voltage']);
        for(var n in VoltageNames) Opt.series[VoltageNames[n]] = { axis: 'y2' };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { return this.Round(x - this.VOffset, 2, true) + ' ' + (this.Options['Log'] ? 'dB' : 'A'); };
        Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(x - this.VOffset, 2, true); };
        Opt.axes.y.ticker = this.Options['Log'] ? (min, max) => { return Graph.Tickers.DBTicks(min, max, this.VOffset); } : Graph.Tickers.Default;
        Opt.axes.y.strokeWidth = (this.Options['Voltage'] == true) ? 2 : 0.5;;
        Opt.axes.y2.strokeWidth = 0.5;
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.drawAxis = this.Options['Current'];
        Opt.axes.y2.axisLabelWidth = this.Options['Current'] ? 34 : 0;
        Opt.axes.y2.valueFormatter = (x) => { return this.Round(x - this.COffset, 2, true) + ' ' + (this.Options['Log'] ? 'dB' : 'V'); };
        Opt.axes.y2.axisLabelFormatter = (x) => { return this.Round(x - this.COffset, 2, true); };
        Opt.axes.y2.ticker = this.Options['Current'] ? this.Options['Log'] ? (min, max) => { return Graph.Tickers.DBTicks(min, max, this.COffset) } : Graph.Tickers.Default : () => { return []; };
        Opt.axes.x.drawGrid = false;
        Opt.axes.x.axisLabelFormatter = (y) => { return Graph.Formatters.FFTAxis(this.analyzer, y); };
        Opt.axes.x.valueFormatter = (y) => { return Graph.Formatters.FFTVal(this.analyzer, y); };
        Opt.labels = ['Bin'].concat(VoltageNames).concat(CurrentNames);
        Opt.colors = this.analyzer.SerieColors('Current').concat(this.analyzer.SerieColors('Voltage'));
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
          var Y1Range = this.MatrixColsRange(Opt.file, I, { wave: 1.0 });
          var Y2Range = this.MatrixColsRange(Opt.file, V, { wave: 1.0 });
          if(this.Options['Log'] == true) { Y1Range[0] = -60; Y2Range[0] = -60; }
          this.COffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
          this.VOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
          Opt.axes.y.valueRange = [Y1Range[0] + this.COffset, Y1Range[1] + this.COffset];
          Opt.axes.y2.valueRange = [Y2Range[0] + this.VOffset, Y2Range[1] + this.VOffset];
          for(var i in Opt.file) {
            for(var n in Opt.file[i]) {
              if(V[n] == true) Opt.file[i][n] += this.VOffset;
              if(I[n] == true) Opt.file[i][n] += this.COffset;
            }
          }
        }
        return Object.seal(Opt);
      }
    },
    methods: {
      Export: function() {
        var Data = [];
        if(this.data == null) return Data;
        var Header = ['Index'];
        var CurrentNames = this.analyzer.SerieNames('Current');
        for(var i = 0; i < CurrentNames.length; i++) {
          Header.push(CurrentNames[i]);
          Header.push(CurrentNames[i] + ' (dB)');
        }
        var VoltageNames = this.analyzer.SerieNames('Voltage');
        for(var i = 0; i < VoltageNames.length; i++) {
          Header.push(VoltageNames[i]);
          Header.push(VoltageNames[i] + ' (dB)');
        }
        Data.push(Header);
        for(var i = 0; i < this.data['Voltage_FFT'][0].length; i++) {
          var Row = [i];
          for(var n = 0; n < CurrentNames.length; n++) {
            Row.push(this.Round(this.data.Current_FFT[n][i], 3));
            Row.push(this.Round(20.0 * Math.log10(Math.max(0.001, this.data.Current_FFT[n][i])), 3));
          }
          for(var n = 0; n < VoltageNames.length; n++) {
            Row.push(this.Round(this.data.Voltage_FFT[n][i], 3));
            Row.push(this.Round(20.0 * Math.log10(Math.max(0.001, this.data.Voltage_FFT[n][i])), 3));
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