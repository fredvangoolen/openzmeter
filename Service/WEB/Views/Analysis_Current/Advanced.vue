<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ (MouseOver || paused) ? '(Paused)' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="true" :current="true" v-model="Phases"></toolbar-phases>
        <toolbar-options storage="DetailsOptions" :options="['Voltage', 'Log']" v-model="Options"></toolbar-options>
        <div class="btn-group">
          <button-export file="FFT" :rows="Export"></button-export>
          <button-expand :expanded.sync="Expanded"></button-expand>
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
        Opt.plugins = [ Graph.Plugins.Vertical_Crosshair() ];
        Opt.interactionModel = Graph.InteractionModels.Fixed(this);
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.selectMode = 'euclidian';
        Opt.fillGraph = true;
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        if((this.Phases == null) || (this.Options == null)) return Opt;
        Opt.visibility = [];
        var VoltageStyle = oZm.SerieStyle(oZm.Series['VOLTAGE'], this.analyzer, ((this.Options['Current'] == true) ? 1 : 0));
        var CurrentStyle = oZm.SerieStyle(oZm.Series['CURRENT'], this.analyzer, 0);
        var VoltageNames = oZm.Series['VOLTAGE'].Names(this.analyzer);
        var CurrentNames = oZm.Series['CURRENT'].Names(this.analyzer);
        for(var n in CurrentNames) Opt.visibility.push(this.Phases[n]);
        for(var n in VoltageNames) Opt.visibility.push(this.Phases[n] && this.Options['Voltage']);
        for(var n in VoltageNames) Opt.series[VoltageNames[n]] = { axis: 'y2' };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { return this.Round(x - this.VOffset, 2, true) + ' ' + (this.Options['Log'] ? 'dB' : 'V'); };
        Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(x - this.VOffset, 2, true); };
        Opt.axes.y.ticker = this.Options['Log'] ? (min, max) => { return Graph.Tickers.DBTicks(min, max, this.VOffset); } : Graph.Tickers.Default;
        Opt.axes.y.strokeWidth = VoltageStyle.Width;
        Opt.axes.y2.strokeWidth = CurrentStyle.Width;
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.drawAxis = this.Options['Current'];
        Opt.axes.y2.axisLabelWidth = this.Options['Current'] ? 34 : 0;
        Opt.axes.y2.valueFormatter = (x) => { return this.Round(x - this.COffset, 2, true) + ' ' + (this.Options['Log'] ? 'dB' : 'A'); };
        Opt.axes.y2.axisLabelFormatter = (x) => { return this.Round(x - this.COffset, 2, true); };
        Opt.axes.y2.ticker = this.Options['Current'] ? this.Options['Log'] ? (min, max) => { return Graph.Tickers.DBTicks(min, max, this.COffset) } : Graph.Tickers.Default : () => { return []; };
        Opt.axes.x.drawGrid = false;

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
        return Opt;
      },
      Export: function() {
        return [];
      }
    },
    methods: {
      Redraw: function() {
        this.$nextTick(function() {
          this.$refs.graph.Redraw('100%', '300');
        });
      },
      SetPhases: function(phases) {
        this.$refs.phases.SetPhases(phases);
        this.CurrentPhases = phases;
        this.CalcOptions();
      },
      SetOption: function(option, val) {
        if(option === 'Voltage') this.ShowVoltage = val;
        if(option === 'Log') this.Log = val;
        this.CalcOptions();
      },
      SetData(result) {
        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
        this.Data = [];
        for(var i = 0; i < result.Voltage_FFT[0].length; i++) {
          var Row = [];
          for(var n = 0; n < result.Current_FFT.length; n++) Row.push(result.Current_FFT[n][i]);
          for(var n = 0; n < result.Voltage_FFT.length; n++) Row.push(result.Voltage_FFT[n][i]);
          this.Data.push(Row);
        }
        this.MaxCurrent = oZm.ArrayAbsMax(result.Current);
        this.CalcOptions();
        this.ShowSpinner = false;
      },
      CalcOptions() {
        if((this.$root.SelectedAnalyzer === null) || (this.CurrentPhases === null) || (this.ShowVoltage === null) || (this.Log === null) || (this.Data === null)) return;
        var Analyzer = this.$root.SelectedAnalyzer, ToShow = [], Series = {}, VoltagePhases = [];
        this.ShowWarning = (this.MaxCurrent < Analyzer.MinCurrent);
        for(var i = 0; i < this.Data.length; i++) {
          var Row = [i];
          for(var n = 0; n < this.Data[i].length; n++) Row.push(this.Log ? 20.0 * Math.log10(Math.max(0.001, this.Data[i][n])) : this.Data[i][n]);
          ToShow.push(Row);
        }
        for(var n = 0; n < oZm.Series['CURRENT'].Names(Analyzer).length; n++) {
          VoltagePhases.push((this.CurrentPhases[n] === true) && (this.ShowVoltage === true));
          if((this.CurrentPhases[n] === true) && (this.ShowVoltage === true)) Series[oZm.Series['VOLTAGE'].Names(Analyzer)[n]] = { axis: 'y2' };
        }
        var Y1Range = oZm.Series['CURRENT'].Range(ToShow, this.CurrentPhases);
        if(this.Log) Y1Range[0] = -60;
        var Y2Range = oZm.Series['VOLTAGE'].Range(ToShow, Array(this.CurrentPhases.length).fill(false).concat(VoltagePhases));
        if(this.Log) Y2Range[0] = -60;
        this.COffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
        this.VOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
        for(var i = 0; i < this.Data.length; i++) {
          var Row = [i];
          for(n = 0; n < oZm.Series['CURRENT'].Names(Analyzer).length; n++) Row.push(ToShow[i][n + 1] + this.COffset);
          for(n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length; n++) Row.push(ToShow[i][n + 1 + oZm.Series['CURRENT'].Names(Analyzer).length] + this.VOffset);
          ToShow[i] = Row;
        }
        var VoltageStyle = oZm.SerieStyle(oZm.Series['VOLTAGE'], Analyzer, 0);
        var CurrentStyle = oZm.SerieStyle(oZm.Series['CURRENT'], Analyzer, ((this.ShowVoltage) ? 1 : 0));
        var Options = {
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], fillGraph: true,
          //interactionModel: { mousedown: Graph.mouseDown, mousemove: Graph.mouseMove, mouseup  : oZm.mouseUp, dblclick : oZm.dblClick_Restore },
          legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow, labels: ['Bin'].concat(oZm.Series['CURRENT'].Names(Analyzer)).concat(oZm.Series['VOLTAGE'].Names(Analyzer)), series: Series, visibility: this.CurrentPhases.concat(VoltagePhases),
          showRangeSelector: false, colors: CurrentStyle.Colors.concat(VoltageStyle.Colors),
          axes: {
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: [Y1Range[0] + this.COffset, Y1Range[1] + this.COffset], strokeWidth: CurrentStyle.Width,
                 ticker: this.Log ? function(min, max) { return Graph.DBTicks(min, max, this.COffset); }.bind(this) : Dygraph.numericTicks,
                 valueFormatter: function(x) { return oZm.Round2(x - this.VOffset) + '<em class="x-small">' + (this.Log ? 'dB' : 'A') + '</em>'; }.bind(this),
                 axisLabelFormatter: function (x) { return oZm.Round2(x); }
               },
            y2: { pixelsPerLabel: 15, axisLabelWidth: this.ShowVoltage ? 34 : 0, valueRange: [Y2Range[0] + this.VOffset, Y2Range[1] + this.VOffset], strokeWidth: VoltageStyle.Width,
                  valueFormatter: function(x) { return oZm.Round2(x - this.VOffset) + ' <em class="x-small">' + (this.Log ? 'dB' : 'V') + '</em>'; }.bind(this),
                  ticker: (this.Log || this.ShowCurrent === false) ? function(min, max) { return (this.ShowVoltage === true) ? Graph.DBTicks(min, max, this.VOffset) : []; }.bind(this) : Dygraph.numericTicks,
                  axisLabelFormatter: function (x) { return oZm.Round2(x - this.VOffset); }.bind(this)
                },
            x: { drawGrid: true,
                 valueFormatter: function (y) { return Math.round(y / 10 * Analyzer.NominalFreq) + '<em class="small"> Hz</em>'; },
                 axisLabelFormatter: function (y) {
                   y = (y / 10 * Analyzer.NominalFreq);
                   if(y >= 1000) {
                     y = y / 1000;
                     if((y % 1) !== 0) return "";
                     return y + '<em class="small"> kHz</em>';
                   } else {
                     if((y % 1) !== 0) return "";
                     return y + '<em class="small"> Hz</em>';
                   }
                 }
               }
          },
          file: ToShow
        };
        this.$refs.graph.SetOptions({ showRangeSelector: true});
        this.$refs.graph.SetOptions(Options);
      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Data = [];
        var Header = ['Index'];
        for(var i = 0; i < oZm.Series['CURRENT'].Names(Analyzer).length; i++) {
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i]);
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i] + ' (dB)');
        }
        for(var i = 0; i < oZm.Series['VOLTAGE'].Names(Analyzer).length; i++) {
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i]);
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i] + ' (dB)');
        }
        Data.push(Header);
        for(var i = 0; i < this.Data.length; i++) {
          var Row = [i];
          for(var n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length + oZm.Series['CURRENT'].Names(Analyzer).length; n++) {
            Row.push(this.Data[i][n]);
            Row.push(20.0 * Math.log10(Math.max(0.000000001, this.Data[i][n])));
          }
          Data.push(Row);
        }
        this.$refs.tools.SaveCVS('FFT.csv', Data);
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