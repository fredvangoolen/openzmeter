<template>
  <div class="panel" :class="PanelClass">
    <div class="panel-heading">
      <div class="header-text">
        <div>{{ analyzer.Title[serie] }}</div>
        <div class="aggreg-row" v-if="data!=null">From {{ analyzer.FormatAggregation(data.Aggreg) }} aggregation {{ paused ? '(Paused)' : '' }}</div>
      </div>
      <div class="header-buttons">
        <div class="main-value"><span v-html="LastValue.Value"></span> <span class="huge-small-unit">{{ LastValue.Units }}</span></div>
        <div v-if="LastValue.Phases.length > 1" class="phases">
          <div v-for="(val, idx) in LastValue.Phases" :key="idx" v-html="val"></div>
        </div>
      </div>
    </div>
    <portal to="fs-body" :disabled="!Expanded" class="panel-body">
      <spinner v-if="data==null"></spinner>
      <div @mouseenter="$emit('update:paused', true)" @mouseleave="$emit('update:paused', false)">
        <graph ref="graph" :options="GraphOptions" :height="Expanded?DialogHeight:150" :paused="paused"></graph>
      </div>
    </portal>
    <div class="panel-footer clearfix">
      <router-link :to="Link"><span class="pull-left">View Details</span></router-link>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases :analyzer="analyzer" v-model="Phases" v-if="serie != 'FREQUENCY'" :storage="'Dashboard_' + serie" :current="serie=='CURRENT'" :multi="true"></toolbar-phases>
        <div class="btn-group">
          <button-export :rows="Export" :file="analyzer.Title[serie]"></button-export>
          <button-expand v-model="Expanded"></button-expand>
        </div>
      </portal>
    </div>
    <dlg-fs v-if="Expanded" :title="analyzer.Title[serie]" :height.sync="DialogHeight"></dlg-fs>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';
  import 'JS/dygraph-crosshair.js';
  export default {
    props: { serie: String, analyzer: Object, data: Object, last: Object, paused: Boolean },
    data() {
      return { ShowSpinner: true, Phases: null, Expanded: false, DialogHeight: 300 };
    },
    computed: {
      Link: function() {
        return '';
      },
      PanelClass: function() {
        if(this.serie == 'Voltage') return 'panel-voltage';
        if(this.serie == 'Current') return 'panel-current';
        if(this.serie == 'Active_Power') return 'panel-active-power';
        if(this.serie == 'Frequency') return 'panel-frequency';
      },
      LastValue: function() {
        var Val = { Value: '-', Units: '', Phases: [] };
        if(this.last == null) return Val;
        if(this.serie == 'Frequency') {
          Val.Value = this.Round(this.last.Frequency, 3, true);
          Val.Units = 'Hz';
        } else if(this.serie == 'Active_Power') {
          Val.Value = 0.0;
          for(var n = 0; n < this.last.Active_Power.length; n++) Val.Value += this.last.Active_Power[n];
          Val.Units = this.BestUnits(Val.Value);
          Val.Value = this.Round(this.ScaleUnits(Val.Value, Val.Units), 2, true);
          for(var n = 1; n < this.last.Active_Power.length; n++) Val.Phases.push(this.Round(this.ScaleUnits(this.last.Active_Power[n], Val.Units), 2, true) + ' ' + Val.Units + 'W');
          Val.Units += 'W';
        } else if((this.serie == 'Voltage') || (this.serie == 'Current')) {
          Val.Value = 0.0;
          for(var n = 0; n < this.analyzer.VoltageChannels; n++) Val.Value += Math.pow(this.last[this.serie][n], 2.0);
          Val.Value = Math.sqrt(Val.Value / this.analyzer.VoltageChannels);
          Val.Units = this.BestUnits(Val.Value);
          Val.Value = this.Round(this.ScaleUnits(Val.Value, Val.Units), 2, true);
          for(var n = 1; n < this.last[this.serie].length; n++) Val.Phases.push(this.Round(this.ScaleUnits(this.last[this.serie][n], Val.Units), 2, true) + ' ' + Val.Units + this.analyzer.Unit[this.serie]);
          Val.Units += this.analyzer.Unit[this.serie];
        }
        return Val;
      },
      GraphOptions: function() {
        var Opt = { file: [], axes: { x: {}, y: {} }};
        Opt.interactionModel = {};
        Opt.plugins = [ Graph.Plugins.Vertical_Crosshair() ];
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.showRangeSelector = false;
        Opt.strokeWidth = 0.5;
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.legend = 'follow';
        Opt.labels = ['Time'].concat(this.analyzer.GetSeriesNames(this.analyzer.Title[this.serie], this.analyzer.Channels[this.serie]));
        Opt.colors = this.analyzer.Style(this.serie, true).Colors;
        Opt.selectMode = 'euclidian';
        Opt.xAxisHeight = 12;
        Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatTime(x); };
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatTime(x); };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 30;
        if(this.Phases != null) Opt.visibility =  this.Phases;
        if((this.data == null)) return Opt;
        Opt.dateWindow = [this.data.From, this.data.To];
        for(var n = 0; n < this.data.Rows.length; n++) {
          var TimeCol = this.data.Headers.indexOf('Sampletime');
          var ValueCol = this.data.Headers.indexOf(this.serie);
          Opt.file.push([this.data.Rows[n][TimeCol]].concat(this.data.Rows[n][ValueCol]));
        }
        if((this.serie == 'Voltage') || (this.serie == 'Current')) {
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.Phases), { rmsRange: 1.0 });
        } else if(this.serie == 'Active_Power') {
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.Phases), { incZero: 1.0 });
        } else if(this.serie == 'Frequency') {
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true], { rmsRange: 0.1 });
        }
        var Units = this.BestUnits(Math.max(Math.abs(Opt.axes.y.valueRange[0]), Math.abs(Opt.axes.y.valueRange[1])));
        Opt.axes.y.valueFormatter = (x) => { return Graph.Formatters.Y_UnitScale(x, this.analyzer.Unit[this.serie], (this.serie == 'Frequency') ? 3 : 2, true); };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return Graph.Formatters.Y_UnitScaleFull(x, gran, opts, g, '', (this.serie == 'Frequency') ? 3 : 2, true); };
        return Object.seal(Opt);
      }
    },
    methods: {
      Export: function() {
        if((this.data == null) || (this.analyzer == null)) return [];
        var Header = [['Time'].concat(this.analyzer.GetSeriesNames(this.analyzer.Title[this.serie], this.analyzer.Channels[this.serie]))];
        var TimeCol = this.data.Headers.indexOf('Sampletime');
        var ValueCol = this.data.Headers.indexOf(this.serie);
        for(var n = 0; n < this.data.Rows.length; n++) {
          var Row = [this.analyzer.FormatDateTime(this.data.Rows[n][TimeCol])];
          if(this.serie != 'Frequency') {
            for(var i in this.data.Rows[n][ValueCol]) Row.push(this.Round(this.data.Rows[n][ValueCol][i], 3));
          } else {
            Row.push(this.Round(this.data.Rows[n][ValueCol], 3));
          }
          Header.push(Row);
        }
        return Object.seal(Header);
      }
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 20px;
    line-height: 24px;
  }
  .phases {
    display: inline-block;
    border-left: 1px solid #FFF;
    padding-left: 2px;
    line-height: 11px;
    font-size: 12px;
    vertical-align: top;
  }
  .aggreg-row {
    font-size: 14px;
    line-height: 20px;
    margin-top: -2px;
  }
  .panel-heading > .header-buttons {
    margin-top: -3px;
  }
  .main-value {
    display: inline-block;
    font-size: 33px;
    line-height: 48px;
    padding-right: 4px;
    vertical-align: top;
  }
</style>