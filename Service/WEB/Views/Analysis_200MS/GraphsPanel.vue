<template>
  <div class="panel" :class="PanelClass">
    <div class="panel-heading">
      <div class="header-text">{{ PanelTitle }} {{ paused ? '(Paused)' : '' }}</div>
      <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
        <toolbar-phases :analyzer="analyzer" v-model="Phases" v-if="serie != 'Frequency'" :storage="'Dashboard_' + serie" :current="serie=='Current'" :multi="true"></toolbar-phases>
        <z-button-group>
          <button-export :rows="Export" :file="PanelTitle"></button-export>
          <button-expand v-model="Expanded"></button-expand>
          <z-button @click="$emit('remove', serie)" :image="require('Imgs/cross.png')" tooltip="Remove this serie" />
        </z-button-group>
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
               TransferGraph: this.SeriesTransfer([this.serie])
      };
    },
    created: function() {
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
      }
    },
    computed: {
      PanelClass: function() {
        if(this.serie == 'Voltage') return 'panel-voltage';
        if(this.serie == 'Current') return 'panel-current';
        if(this.serie == 'Active_Power') return 'panel-active-power';
        if(this.serie == 'Frequency') return 'panel-frequency';
      },
      PanelTitle: function() {
        if(this.serie == 'Voltage') return 'Voltage';
        if(this.serie == 'Current') return 'Current';
        if(this.serie == 'Active_Power') return 'Active Power';
        if(this.serie == 'Frequency') return 'Frequency';
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
        Opt.labels = ['Time'].concat(this.analyzer.SerieNames(this.serie));
        Opt.colors = this.analyzer.SerieColors(this.serie);
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
          if(this.serie == 'Voltage') {
            for(var i in this.TransferGraph.Data.Voltage[n]) Row.push(this.TransferGraph.Data.Voltage[n][i]);
          } else if(this.serie == 'Current') {
            for(var i in this.TransferGraph.Data.Current[n]) Row.push(this.TransferGraph.Data.Current[n][i]);
          } else if(this.serie == 'Active_Power') {
            for(var i in this.TransferGraph.Data.Active_Power[n]) Row.push(this.TransferGraph.Data.Active_Power[n][i]);
          } else if(this.serie == 'Frequency') {
            Row.push(this.TransferGraph.Data.Frequency[n]);
          }
          Opt.file.push(Row);
        }
        if((this.serie == 'Voltage') || (this.serie == 'Current')) {
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.Phases), { rmsRange: 1.0 });
        } else if(this.serie == 'Active_Power') {
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.Phases), { incZero: 1.0 });
        } else if(this.serie == 'Frequency') {
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true], { rmsRange: 0.1 });
        }
        var Units = this.BestUnits(Math.max(Math.abs(Opt.axes.y.valueRange[0]), Math.abs(Opt.axes.y.valueRange[1])));
        Opt.axes.y.valueFormatter = (x) => { return Graph.Formatters.Y_UnitScale(x, this.analyzer.SerieUnits(this.serie), 2, true); };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return Graph.Formatters.Y_UnitScaleFull(x, gran, opts, g, '', 2, true); };
        return Object.seal(Opt);
      }
    },
    methods: {
      Export: function() {
        if(this.data == null) return [];
        var Header = [['Time'].concat(this.analyzer.SerieNames(this.serie))];
        for(var n in this.data.Sampletime) {
          var Row = [this.data.Sampletime[n]];
          if(this.serie == 'Voltage') {
            for(var i in this.data.Voltage[n]) Row.push(this.Round(this.data.Voltage[n][i], 3));
          } else if(this.serie == 'Current') {
            for(var i in this.data.Current[n]) Row.push(this.Round(this.data.Current[n][i], 3));
          } else if(this.serie == 'Active_Power') {
            for(var i in this.data.Active_Power[n]) Row.push(this.Round(this.data.Active_Power[n][i], 3));
          } else if(this.serie == 'Frequency') {
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