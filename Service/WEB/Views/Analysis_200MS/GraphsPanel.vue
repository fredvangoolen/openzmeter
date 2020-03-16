<template>
  <div class="panel" :class="PanelClass">
    <div class="panel-heading">
      <div class="header-text">{{ PanelTitle }} {{ paused ? '(Paused)' : '' }}</div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases :analyzer="analyzer" v-model="Phases" v-if="serie != 'FREQUENCY'" :storage="'Dashboard_' + serie" :current="serie=='CURRENT'" :multi="true"></toolbar-phases>
        <div class="btn-group">
          <button-export :rows="Export" :file="PanelTitle"></button-export>
          <button-expand v-model="Expanded"></button-expand>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('remove', serie)"><img :src="require('Imgs/cross.png')" alt="Remove this serie"></button>
        </div>
      </portal>
    </div>
    <portal to="fs-body" :disabled="!Expanded" class="panel-body">
      <spinner v-if="TransferGraph.Data==null"></spinner>
      <div @mouseenter="$emit('update:paused', true)" @mouseleave="$emit('update:paused', false)">
        <graph ref="graph" :options="GraphOptions" :height="Expanded?DialogHeight:150" :paused="paused"></graph>
      </div>
    </portal>
    <dlg-fs v-if="Expanded" :title="PanelTitle" :height.sync="DialogHeight"></dlg-fs>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';
  import 'JS/dygraph-crosshair.js';
  export default {
    props: { serie: String, analyzer: Object, paused: Boolean, range: [String, Array] },
    data() {
      return { ShowSpinner: true, Phases: null, Expanded: false, DialogHeight: 300,
               TransferGraph: this.CreateSeriesTransfer(this, [this.serie])
      };
    },
    created: function() {
      this.SerieProps = oZm.Series[this.serie];
//      this.TransferGraph.Series = [];
      this.TransferGraph.Update(this.analyzer, '200MS', this.range);
    },
    beforeDestroy: function() {
      this.TransferGraph.Destroy();
    },
    watch: {
      analyzer: function(val) {
        this.TransferGraph.Update(this.analyzer, '200MS', this.range);
      },
      range: function(val, prev) {
        this.TransferGraph.Update(this.analyzer, '200MS', this.range);
      },

    },
    computed: {
      PanelClass: function() {
        if(this.serie == 'VOLTAGE') return 'panel-voltage';
        if(this.serie == 'CURRENT') return 'panel-current';
        if(this.serie == 'ACTIVE_POWER') return 'panel-active-power';
        if(this.serie == 'FREQUENCY') return 'panel-frequency';
      },
      PanelTitle: function() {
        if(this.serie == 'VOLTAGE') return 'Voltage';
        if(this.serie == 'CURRENT') return 'Current';
        if(this.serie == 'ACTIVE_POWER') return 'Active Power';
        if(this.serie == 'FREQUENCY') return 'Frequency';
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
        Opt.labels = ['Time'].concat(this.SerieProps.Names(this.analyzer));
        Opt.colors = oZm.SerieStyle(this.SerieProps, this.analyzer, 0).Colors;
        Opt.selectMode = 'euclidian';
        Opt.xAxisHeight = 12;
        Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatTime(x); };
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatTime(x); };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 30;
        if(this.Phases != null) Opt.visibility =  this.Phases;
        if((this.TransferGraph.Data == null)) return Opt;
        Opt.dateWindow = [this.TransferGraph.Data.From, this.TransferGraph.Data.To];
        for(var n in this.TransferGraph.Data.Sampletime) {
          var Row = [this.TransferGraph.Data.Sampletime[n]];
          if(this.serie == 'VOLTAGE') {
            for(var i in this.TransferGraph.Data.Voltage[n]) Row.push(this.TransferGraph.Data.Voltage[n][i]);
          } else if(this.serie == 'CURRENT') {
            for(var i in this.TransferGraph.Data.Current[n]) Row.push(this.TransferGraph.Data.Current[n][i]);
          } else if(this.serie == 'ACTIVE_POWER') {
            for(var i in this.TransferGraph.Data.Active_Power[n]) Row.push(this.TransferGraph.Data.Active_Power[n][i]);
          } else if(this.serie == 'FREQUENCY') {
            Row.push(this.TransferGraph.Data.Frequency[n]);
          }
          Opt.file.push(Row);
        }
        if((this.serie == 'VOLTAGE') || (this.serie == 'CURRENT')) {
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.Phases), { rmsRange: 1.0 });
        } else if(this.serie == 'ACTIVE_POWER') {
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.Phases), { incZero: 1.0 });
        } else if(this.serie == 'FREQUENCY') {
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true], { rmsRange: 0.1 });
        }
        var Units = this.BestUnits(Math.max(Math.abs(Opt.axes.y.valueRange[0]), Math.abs(Opt.axes.y.valueRange[1])));
        Opt.axes.y.valueFormatter = (x) => {
          var Result = oZm.FormatSerieValue(x, this.SerieProps);
          return Result.ValuePlain + ' ' + Result.Units;
        };
        Opt.axes.y.axisLabelFormatter = (x) => {
          return oZm.FormatSerieValue(x, this.SerieProps, Units).ValuePlain;
        };
        return Object.seal(Opt);
      }
    },
    methods: {
      Export: function() {
        if(this.data == null) return [];
        var Header = [['Time'].concat(this.SerieProps.Names(this.analyzer))];
        for(var n in this.data.Sampletime) {
          var Row = [this.data.Sampletime[n]];
          if(this.serie == 'VOLTAGE') {
            for(var i in this.data.Voltage[n]) Row.push(this.Round(this.data.Voltage[n][i], 3));
          } else if(this.serie == 'CURRENT') {
            for(var i in this.data.Current[n]) Row.push(this.Round(this.data.Current[n][i], 3));
          } else if(this.serie == 'ACTIVE_POWER') {
            for(var i in this.data.Active_Power[n]) Row.push(this.Round(this.data.Active_Power[n][i], 3));
          } else if(this.serie == 'FREQUENCY') {
            Row.push(this.Round(this.data.Frequency[n], 3));
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
    font-size: 18px;
    line-height: 22px;
  }
</style>