<template>
  <q-toolbar class="bg-blue-6 text-white q-pr-none">
    <portal to="dlg-toolbar" :disabled="!Expanded">
      <q-toolbar-title>Current</q-toolbar-title>
      <q-separator dark vertical />
      <toolbar-aggreg storage="Analysis_Current.Aggreg" :options="['3S', '1M', '10M', '1H']" v-model="Aggreg" />
      <q-separator dark vertical />
      <!--toolbar-period storage="Period" v-model="Range" ></toolbar-period>
      <q-separator dark vertical /-->
      <toolbar-phases-multi storage="Analysis_Voltage.Phases" :current="true" v-model="Phases" v-if="Analyzer.VoltageChannels() > 1" />
      <q-separator dark vertical v-if="Analyzer.VoltageChannels() > 1" />
      <toolbar-export file="Current" :rows="Export" />
      <q-separator dark vertical />
      <toolbar-expand v-model="Expanded" />
    </portal>
  </q-toolbar>
  <q-card-section class="row q-pa-xs">
    <!--counters class="col-md-6 col-lg-4 col-xl-3" :data1M="Transfer1M.LastData" :data3S="Transfer3S.LastData" :dataGraph="TransferGraph.Data" :height.sync="Height"></counters-->
    <div class="col">
      <portal to="dlg-body" :disabled="!Expanded">
      blau
                  <!--spinner v-if="TransferGraph.Data==null"></spinner-->
                  <!--current :phases="Phases" :data="TransferGraph.Data" :height="Expanded?DialogHeight:Height" @range="Range=$event"></current-->
                  <!--graph :options="GraphOptions" :height="height"></graph-->
                </portal>
              </div>
  </q-card-section>
  <dialog-fullscreen :modelValue="Expanded" color="bg-blue-6" />
</template>

<script>
  import Graph from 'Components/Graph.vue';
//  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { data: Object, phases: Array, height: Number },
    data: function() {
      return {Aggreg: null, Range: null, Phases: [], Height: 300, Paused: false, Expanded: false, DialogHeight: 300, };
    },
    computed: {
      GraphOptions: function(val) {
        var Opt = { file: [], axes: { x: {}, y: {} } };
        Opt.panEdgeFraction = 1;
        Opt.interactionModel = Graph.InteractionModels.Scrollable(this);
        Opt.strokeWidth = 0.5;
        Opt.stepPlot = true;
        Opt.showRangeSelector = false;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.xAxisHeight = 12;
        Opt.selectMode = 'euclidian';
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.visibility = this.phases;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { return Graph.Formatters.Y_UnitScale(x, 'A', 2, true); };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return Graph.Formatters.Y_UnitScaleFull(x, gran, opts, g, '', 2, true); };
        Opt.axes.x.ticker = (min, max, pixels) => { return Graph.Formatters.X_HourlyFull(min, max, pixels, (this.analyzer != null) ? this.analyzer.TimeZone : null); };
        Opt.axes.x.valueFormatter = (x) => { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); };
        Opt.labels = ['Time'].concat(this.analyzer.SerieNames('Current'));
        Opt.colors = this.analyzer.SerieColors('Current');
        if(this.data != null) {
          var TimeCol = this.data.Headers.indexOf('Time');
          var ValueCol = this.data.Headers.indexOf('Current');
          for(var n = 0; n < this.data.Rows.length; n++) Opt.file.push([this.data.Rows[n][TimeCol]].concat(this.data.Rows[n][ValueCol]));
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.phases), { defMin: 0, defMax: 1, percent: 0.01, minWidth: 10 });
          Opt.dateWindow = [this.data.From, this.data.To];
        }
        return Object.seal(Opt);
      }
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>