<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ MouseOver ? '[PAUSED]' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
        <div class="header-buttons" ref="Toolbar">
          <toolbar-phases storage="DetailPhases" :analyzer="analyzer" :multi="true" :current="true" v-model="Phases"></toolbar-phases>
          <toolbar-options storage="DetailOptions" :options="['1st Harmonic', 'Current', 'Log']" v-model="Options"></toolbar-options>
          <div class="btn-group">
            <button-pause :value="paused" @input="$emit('update:paused', $event)"></button-pause>
            <!--toolbar-tools variable="To" ref="tools" :options="['EXPORT', 'MAXIMIZE', 'PLAY']" v-on:PlayStatus="$parent.PlayStatusChanged" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="Redraw"></toolbar-tools-->
          </div>
        </div>
      </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <graph :options="GraphOptions" v-on:mouseOver="MouseOver=$event"></graph>
    </div>
  </div>
</template>
<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarOptions from 'Components/ToolbarOptions.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  export default {
    props: { analyzer: Object, data: Object, paused: Boolean },
    data() {
      return { ShowSpinner: true, MouseOver: false, Options: null, Phases: null };
    },
    computed: {
      RmsCurrent: function() { return (this.data == null) ? 0.0 : oZm.ArrayAbsMax(this.data.Current); },
      Time: function() { return (this.data == null) ?  'Loading data...' : 'Last update at ' + moment(this.data.Sampletime).format('HH:mm:ss.SSS'); },
      GraphOptions: function() {
        if((this.analyzer == null) || (this.Phases == null) || (this.Options == null)) return {};
        var VoltageStyle = oZm.SerieStyle(oZm.Series['VOLTAGE'], this.analyzer, ((this.Options['Current'] == true) ? 1 : 0));
        var CurrentStyle = oZm.SerieStyle(oZm.Series['CURRENT'], this.analyzer, 0);
        var VoltageNames = oZm.Series['VOLTAGE'].Names(this.analyzer);
        var CurrentNames = oZm.Series['CURRENT'].Names(this.analyzer);
        var Opt = { visibility: [], series: {}, axes: { y: {}, y2: {}, x: {} }, file: [] };
        for(var n in VoltageNames) Opt.visibility.push(this.Phases[n]);
        for(var n in CurrentNames) Opt.visibility.push(this.Phases[n] && this.Options['Current']);
        for(var n in CurrentNames) Opt.series[CurrentNames[n]] = { axis: 'y2' };
        Opt.strokeWidth = 0.5;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.interactionModel = { };
        Opt.plotter = Graph.multiColumnBarPlotter;
        Opt.drawHighlightPointCallback = Graph.DrawHighlightBarChartCallback;
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.LegendFormatter_Follow;

//dateWindow: [ToShow[0][0] - 0.5, ToShow[ToShow.length - 1][0] + 0.5],
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        //valueRange: [Y1Range[0] + this.VOffset, Y1Range[1] + this.VOffset];
        Opt.axes.y.ticker = this.Options['Log'] ? (min, max) => { return Graph.DBTicks(min, max, this.VOffset); } : Dygraph.numericTicks;
        Opt.axes.y.valueFormatter = (x) => { return oZm.Round2(x - this.VOffset) + ' ' + (this.Options['Log'] ? 'dB' : 'V'); };
        Opt.axes.y.axisLabelFormatter = (x) => { return oZm.Round2(x - this.VOffset); };
        Opt.axes.y2.drawAxis = this.Options['Current'];
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.axisLabelWidth = this.Options['Current'] ? 34 : 0;
//        Opt.axes.y2.valueRange = [Y2Range[0] + this.COffset, Y2Range[1] + this.COffset];
        Opt.axes.y2.valueFormatter = (x) => { return oZm.Round2(x - this.COffset) + ' ' + (this.Log ? 'dB' : 'A'); };
        Opt.axes.y2.ticker = this.Options['Current'] ? this.Options['Log'] ? (min, max) => { return Graph.DBTicks(min, max, this.COffset) } : Dygraph.numericTicks : () => { return []; };
        Opt.axes.y2.axisLabelFormatter = (x) => { return oZm.Round2(x - this.COffset); };
        Opt.axes.x.drawGrid = false;
        Opt.axes.x.ticker = Graph.OddHarmonicsTicks;
        Opt.axes.x.axisLabelFormatter = (x) => { return oZm.Series['HARMONICS'].Names[x]; };
        Opt.axes.x.valueFormatter = (y) => { return oZm.Series['HARMONICS'].Names[y] + ' Harmonic (' + (this.analyzer.NominalFreq * (y + 1)) + '<em class="small"> Hz</em>)'; };
        Opt.labels = ['Harmonic'].concat(VoltageNames).concat(CurrentNames);
        Opt.colors = VoltageStyle.Colors.concat(CurrentStyle.Colors);
        for(var i = 0; i < 50; i++) {
          if(this.Options['1st Harmonic'] && (i == 0)) continue;
          var Row = [i];
          for(var n = 0; n < this.data.Voltage_Harmonics.length; n++) Row.push(this.Options['Log'] ? 20.0 * Math.log10(Math.max(0.001, this.BinPower(this.data.Voltage_Harmonics[n][i]))) : this.BinPower(this.data.Voltage_Harmonics[n][i]));
          for(var n = 0; n < this.data.Current_Harmonics.length; n++) Row.push(this.Options['Log'] ? 20.0 * Math.log10(Math.max(0.001, this.BinPower(this.data.Current_Harmonics[n][i]))) : this.BinPower(this.data.Current_Harmonics[n][i]));
          Opt.file.push(Row);
        }
        this.ShowSpinner = false;
//Opt.axes.y.valueRange = MatrixColsRange(Opt.file, [false].concat());

//      UpperRange = (UpperRange === -Infinity) ? 1 : UpperRange;
//      LowerRange = (LowerRange === Infinity) ? 0 : LowerRange;
//      if((UpperRange - LowerRange) > 5) {
//        var Diff = Math.max(1, UpperRange - LowerRange);
//        UpperRange = UpperRange + Diff * 0.1;
//        LowerRange = LowerRange - Diff * 0.1;
//      } else {
//        UpperRange = UpperRange + 0.25;
//        LowerRange = LowerRange - 0.25;
//      }
//      if(LowerRange < 0) LowerRange = 0;



//busca rangos
//        var Y1Range = oZm.Series['VOLTAGE'].Range(ToShow, VoltagePhases);
//        if(this.Log) Y1Range[0] = -60;
//        var Y2Range = oZm.Series['CURRENT'].Range(ToShow, Array(VoltagePhases.length).fill(false).concat(CurrentPhases));
//        if(this.Log) Y2Range[0] = -60;
//        this.VOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
//        this.COffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
//aplica los offsets que hagan falta para el log
//for(var i = 0; i < ToShow.length; i++) {
//          var Row = [ToShow[i][0]];
//          for(n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length; n++) Row.push(ToShow[i][n + 1] + this.VOffset);
//          for(n = 0; n < oZm.Series['CURRENT'].Names(Analyzer).length; n++) Row.push(ToShow[i][n + 1 + oZm.Series['VOLTAGE'].Names(Analyzer).length] + this.COffset);
//          ToShow[i] = Row;
//        }
        return Opt;
      }
    },
    methods: {
      BinPower: function(value) {
        return Math.sqrt(Math.pow(value[0], 2.0) + Math.pow(value[1], 2.0));
      },
//      Redraw: function() {
//        this.$nextTick(function() {
//          this.$refs.graph.Redraw('100%', '300');
//        });
//      },
//      SetPhases: function(phases) {
//        this.$refs.phases.SetPhases(phases);
//        this.Phases = phases;
//        this.CalcOptions();
//      },
//      SetOption: function(option, val) {
//        if(option === 'Current') this.ShowCurrent = val;
//        if(option === '1st Harmonic') this.First = val;
//        if(option === 'Log') this.Log = val;
//        this.CalcOptions();
//      },
//      SetData(result) {
//      },
//      CalcOptions() {
//      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Data = [];
        var Header = ['Harmonic'];
        for(var i = 0; i < oZm.Series['VOLTAGE'].Names(Analyzer).length; i++) {
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i]);
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i] + ' (dB)');
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i] + ' (Complex)');
        }
        for(var i = 0; i < oZm.Series['CURRENT'].Names(Analyzer).length; i++) {
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i]);
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i] + ' (dB)');
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i] + ' (Complex)');
        }
        Data.push(Header);
        for(i = 0; i < oZm.Series['HARMONICS'].Names.length; i++) {
          var Row = [oZm.Series['HARMONICS'].Names[i]];
          for(var n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length + oZm.Series['CURRENT'].Names(Analyzer).length; n++) {
            Row.push(this.BinPower(this.Data[i][n]));
            Row.push(20.0 * Math.log10(Math.max(0.000000001, this.BinPower(this.Data[i][n]))));
            Row.push(String(this.Data[i][n][0]) + ((this.Data[i][n][1] > 0) ? '+' + this.Data[i][n][1] : this.Data[i][n][1]) + 'i');
          }
          Data.push(Row);
        }
        this.$refs.tools.SaveCVS('Harmonics.csv', Data);
      },
      MaximizeClick: function() {
        this.$refs.tools.Maximize('Harmonics', this.$refs.graph.Graph, this.$refs.Toolbar);
      }
    },
    created: function() {
      //this.MaxCurrent = 0;
      //this.Data = null;
      //this.Phases = null;
      //this.ShowCurrent = null;
      //this.First = null;
      //this.Log = null;
      //this.VOffset = 0;
      //this.COffset = 0;
    },
    components: { ToolbarPhases, ToolbarOptions, ToolbarTools, Graph }
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