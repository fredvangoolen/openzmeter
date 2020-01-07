<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ MouseOver ? 'Paused while over graph' : Time }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
        <div class="header-buttons" ref="Toolbar">
          <toolbar-phases storage="DetailsPhaseMulti" :analyzer="analyzer" :multi="true" :current="true" :value="phases" @input="$emit('update:phases', $event)"></toolbar-phases>
          <toolbar-options storage="DetailOptions" :options="['Current', 'Log']" v-model="Options"></toolbar-options>
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


        for(var i in this.data) {
//          var Row = [i];
//          for(var n in this.data[i].length; n++) Row.push(this.Log ? 20.0 * Math.log10(Math.max(0.001, this.Data[i][n])) : this.Data[i][n]);
//          ToShow.push(Row);
        }
//        for(var n = 0; n < this.Phases.length; n++) {
//          CurrentPhases.push((this.Phases[n] === true) && (this.ShowCurrent === true));
//          if((this.Phases[n] === true) && (this.ShowCurrent === true)) Series[oZm.Series['CURRENT'].Names(Analyzer)[n]] = { axis: 'y2' };
//        }
//        var Y1Range = oZm.Series['VOLTAGE'].Range(ToShow, VoltagePhases);
//        if(this.Log) Y1Range[0] = -60;
//        var Y2Range = oZm.Series['CURRENT'].Range(ToShow, Array(VoltagePhases.length).fill(false).concat(CurrentPhases));
//        if(this.Log) Y2Range[0] = -60;
//        this.VOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
//        this.COffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
//        for(var i = 0; i < this.Data.length; i++) {
//          var Row = [i];
//          for(n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length; n++) Row.push(ToShow[i][n + 1] + this.VOffset);
//          for(n = 0; n < oZm.Series['CURRENT'].Names(Analyzer).length; n++) Row.push(ToShow[i][n + 1 + oZm.Series['VOLTAGE'].Names(Analyzer).length] + this.COffset);
//          ToShow[i] = Row;
//        }






        var Opt = {};
        Opt.visibility = [];
        Opt.series = {};
        for(var n in VoltageNames) Opt.visibility.push(this.phases[n]);
        for(var n in CurrentNames) Opt.visibility.push(this.phases[n] && this.Options['Current']);
        for(var n in CurrentNames) Opt.series[CurrentNames[n]] = { axis: 'y2' };
        Opt.plugins = [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ];
        Opt.interactionModel = Graph.FixedRangeInteractionModel(this);
        Opt.selectMode = 'euclidian';
        Opt.strokeWidth = 0.5;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.fillGraph = true;
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.LegendFormatter_Follow;
        Opt.showRangeSelector = false;
        Opt.axes = { y: {}, y2: {}, x: {} };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        //Opt.axes.y.valueRange = [Y1Range[0] + this.VOffset, Y1Range[1] + this.VOffset],
        Opt.axes.y.strokeWidth = VoltageStyle.Width;
        Opt.axes.y.valueFormatter = (x) => { return oZm.Round2(x - this.VOffset) + '<em class="x-small">' + (this.Log ? 'dB' : 'V') + '</em>'; };
        Opt.axes.y.ticker = this.Options['Log'] ? (min, max) => { return Graph.DBTicks(min, max, this.VOffset); } : Dygraph.numericTicks;
        Opt.axes.y.axisLabelFormatter = (x) => { return oZm.Round2(x); };
        Opt.axes.y2.drawAxis = this.Options['Current'];
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.axisLabelWidth = this.Options['Current'] ? 34 : 0;
        //Opt.axes.y2.valueRange = [Y2Range[0] + this.COffset, Y2Range[1] + this.COffset];
        Opt.axes.y2.strokeWidth = CurrentStyle.Width;
        Opt.axes.y2.valueFormatter = (x) => { return oZm.Round2(x - this.COffset) + ' <em class="x-small">' + (this.Options['Log'] ? 'dB' : 'A') + '</em>'; };
        Opt.axes.y2.ticker = (this.Log || this.ShowCurrent === false) ? function(min, max) { return (this.ShowCurrent === true) ? Graph.DBTicks(min, max, this.COffset) : []; }.bind(this) : Dygraph.numericTicks;
        Opt.axes.y2.axisLabelFormatter = (x) => { return oZm.Round2(x - this.COffset); };
//        Opt.axes.x.drawGrid: true,
        Opt.axes.x.valueFormatter = (y) => { return Math.round(y / 10 * Analyzer.NominalFreq) + '<em class="small"> Hz</em>'; };
        Opt.axes.x.axisLabelFormatter = function (y) {
                   y = (y / 10 * Analyzer.NominalFreq);
                   if(y >= 1000) {
                     y = y / 1000;
                     if((y % 1) !== 0) return "";
                     return y + '<em class="small"> kHz</em>';
                   } else {
                     if((y % 1) !== 0) return "";
                     return y + '<em class="small"> Hz</em>';
                   }
                 };
        Opt.labels = ['Bin'].concat(VoltageNames).concat(CurrentNames);
        Opt.colors = VoltageStyle.Colors.concat(CurrentStyle.Colors);
        Opt.file = [];




        for(var i = 0; i < result.Voltage_FFT[0].length; i++) {
          var Row = [];
          for(var n = 0; n < result.Voltage_FFT.length; n++) Row.push(result.Voltage_FFT[n][i]);
          for(var n = 0; n < result.Current_FFT.length; n++) Row.push(result.Current_FFT[n][i]);
          this.Data.push(Row);
        }
        return Opt;
      }
    },
    created: function() {
      this.MaxCurrent = 0;
      this.Data = null;
      this.Phases = null;
      this.ShowCurrent = null;
      this.Log = null;
      this.VOffset = 0;
      this.COffset = 0;
    },
    methods: {
      Redraw: function() {
        this.$nextTick(function() {
          this.$refs.graph.Redraw('100%', '300');
        });
      },
//      SetPhases: function(phases) {
//        this.$refs.phases.SetPhases(phases);
//        this.Phases = phases;
//        this.CalcOptions();
//      },
//      SetOption: function(option, val) {
///        if(option === 'Current') this.ShowCurrent = val;
//        if(option === 'Log') this.Log = val;
//        this.CalcOptions();
//      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Data = [];
        var Header = ['Index'];
        for(var i = 0; i < oZm.Series['VOLTAGE'].Names(Analyzer).length; i++) {
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i]);
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i] + ' (dB)');
        }
        for(var i = 0; i < oZm.Series['CURRENT'].Names(Analyzer).length; i++) {
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i]);
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i] + ' (dB)');
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
      },
      MaximizeClick: function() {
        this.$refs.tools.Maximize('Advanced', this.$refs.graph.Graph, this.$refs.Toolbar);
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