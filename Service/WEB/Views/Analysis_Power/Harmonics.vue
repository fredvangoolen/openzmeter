<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ paused ? '(Paused)' : MouseOver ? '(Graph paused)' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
        <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
          <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="true" :current="true" v-model="Phases"></toolbar-phases>
          <toolbar-options storage="DetailsOptions" :options="['1st Harmonic']" v-model="Options"></toolbar-options>
          <z-button-group>
            <button-export file="Harmonics" :rows="Export"></button-export>
            <button-expand v-model="Expanded"></button-expand>
            <button-pause :value="paused" @input="$emit('update:paused', $event)"></button-pause>
          </z-button-group>
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
  import Graph from 'Components/Graph.vue';
  export default {
    props: { analyzer: Object, data: Object, paused: Boolean },
    data() {
      return { MouseOver: false, Options: null, Phases: null, Expanded: false, DialogHeight: 300 };
    },
    computed: {
      RmsCurrent: function() { return (this.data == null) ? 0.0 : oZm.ArrayAbsMax(this.data.Current); },
      GraphOptions: function() {
        var Opt = { series: {}, axes: { y: {}, y2: {}, x: {} }, file: [] };
        Opt.showRangeSelector = false;
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.strokeWidth = 0.5;
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.interactionModel = { };
        Opt.plotter = Graph.Plotters.MultiBar;
        Opt.drawHighlightPointCallback = Graph.Callbacks.DrawHighlightPoint_HighlightBar;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        if((this.Phases == null) || (this.Options == null)) return { };
        Opt.visibility = this.Phases;
        Opt.axes.y.valueFormatter = (x) => { var Units = this.BestUnits(x); return this.Round(this.ScaleUnits(x, Units), 2, true) + ' ' + Units + 'W'; };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return (g.axes_[0].valueRange == null) ? '' : this.Round(this.ScaleUnits(x, this.BestUnits(Math.max(Math.abs(g.axes_[0].valueRange[0]), Math.abs(g.axes_[0].valueRange[1])))), 2, true); }
        Opt.axes.x.drawGrid = false;
        Opt.axes.y.axisLineColor = 'rgba(0, 0, 0, 0)';
        Opt.drawAxesAtZero = true;
        Opt.axes.x.ticker = () => { return Graph.Tickers.OddHarmonicsTicks(this.analyzer); };
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.HarmonicsNames[x]; };
        Opt.axes.x.valueFormatter = (y) => { return this.analyzer.HarmonicsNames[y] + ' Harmonic (' + (this.analyzer.NominalFrequency * (y + 1)) + 'Hz)'; };
        Opt.labels = ['Harmonic'].concat(this.analyzer.SerieNames('Power'));
        Opt.colors = this.analyzer.SerieColors('Active_Power');
        if(this.data != null) {
          for(var i = 0; i < 50; i++) {
            if((this.Options['1st Harmonic'] == false) && (i == 0)) continue;
            var Row = [i];
            for(var n = 0; n < this.data.Power_Harmonics.length; n++) Row.push(this.data.Power_Harmonics[n][i]);
            Opt.file.push(Row);
          }
          Opt.dateWindow = [Opt.file[0][0] - 0.5, 50.5];
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.Phases), { incZero: 1.0 });
        }
        return Object.seal(Opt);
      }
    },
    methods: {
      Export: function() {
        if((this.data == null) || (this.analyzer == null)) return [];
        var Header = ['Harmonic'].concat(this.analyzer.SerieNames('Power'));
        var Data = [Header];
        for(var i = 0; i < this.data.Power_Harmonics[0].length; i++) {
          var Row = [this.analyzer.HarmonicsNames[i]];
          for(var n = 0; n < this.data.Power_Harmonics.length; n++) Row.push(this.Round(this.data.Power_Harmonics[n][i], 3));
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