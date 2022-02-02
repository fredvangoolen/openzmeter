<template>
  <q-toolbar class="bg-red-9 text-white q-pr-none">
    <portal to="dlg-toolbar" :disabled="!Expanded">
      <q-toolbar-title>Voltage</q-toolbar-title>
      <q-separator dark vertical />
      <toolbar-aggreg storage="Analysis_Voltage.Aggreg" :options="['3S', '1M', '10M', '1H']" v-model="Aggreg" />
      <q-separator dark vertical />
      <!--toolbar-period storage="Period" v-model="Range" ></toolbar-period>
      <q-separator dark vertical /-->
      <toolbar-phases-multi storage="Analysis_Voltage.Phases" :current="false" v-model="Phases" v-show="Analyzer.VoltageChannels() > 1" />
      <q-separator dark vertical v-if="Analyzer.VoltageChannels() > 1" />

      <toolbar-export file="Voltage" :rows="Export" />
      <q-separator dark vertical />
      <toolbar-expand v-model="Expanded" />
    </portal>
  </q-toolbar>
  <q-card-section class="row q-pa-xs">
        <!--counters class="col-auto" :data10M="Transfer10M.LastData" :dataGraph="TransferGraph.Data" :height.sync="Height"></counters-->
        <div class="col">
          <portal to="dlg-body" :disabled="!Expanded">
                  blau
                  <!--spinner v-if="TransferGraph.Data==null"></spinner-->
                  <!--voltage :phases="Phases" :data="TransferGraph.Data" :height="Expanded?DialogHeight:Height" @range="Range=$event"></voltage-->
                    <div @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
    <!--graph :options="GraphOptions" :height="height" :paused="MouseOver"></graph-->
  </div>
      </portal>
    </div>
  </q-card-section>
  <dialog-fullscreen :modelValue="Expanded" color="bg-red-9" />
</template>

<script>
  import Graph from 'Components/Graph.vue';
//  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  import Counters from './Counters.vue';
  export default {
    props: { data: Object, phases: Array, height: Number },
    data: function() {
      return { MouseOver: false, Aggreg: null, Phases: [], Expanded: false,    Height: 300,  DialogHeight: 300,
      TransferGraph: this.SeriesTransfer(['Voltage']),
       };
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
        Opt.axes.y.valueFormatter = (x) => { return Graph.Formatters.Y_UnitScale(x, 'V', 2, true); };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return Graph.Formatters.Y_UnitScaleFull(x, gran, opts, g, '', 2, true); };
        Opt.axes.x.ticker = (min, max, pixels) => { return Graph.Formatters.X_HourlyFull(min, max, pixels, (this.analyzer != null) ? this.analyzer.TimeZone : null); };
        Opt.axes.x.valueFormatter = (x) => { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); };
        Opt.labels = ['Time'].concat(this.analyzer.SerieNames('Voltage'));
        Opt.colors = this.analyzer.SerieColors('Voltage')
        if(this.data != null) {
          var TimeCol = this.data.Headers.indexOf('Time');
          var ValueCol = this.data.Headers.indexOf('Voltage');
          for(var n = 0; n < this.data.Rows.length; n++) Opt.file.push([this.data.Rows[n][TimeCol]].concat(this.data.Rows[n][ValueCol]));
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.phases), { defMin: 0, defMax: 1, percent: 0.01, minWidth: 10 });
          Opt.dateWindow = [this.data.From, this.data.To];
        }
        return Object.seal(Opt);
      }
    },
    methods: {
            Export: function() {
        if(this.TransferGraph.Data == null) return [];
        var Rows = [['Time'].concat(this.analyzer.SerieNames('Voltage'))];
        var TimeCol = this.TransferGraph.Data.Headers.indexOf('Time');
        var VoltageCol = this.TransferGraph.Data.Headers.indexOf('Voltage');
        for(var n = 0; n < this.TransferGraph.Data.Rows.length; n++) {
          var Row = [this.analyzer.FormatDateTime(this.TransferGraph.Data.Rows[n][TimeCol])];
          for(var i = 0; i < this.TransferGraph.Data.Rows[n][VoltageCol].length; i++) Row.push(this.Round(this.TransferGraph.Data.Rows[n][VoltageCol][i], 3));
          Rows.push(Row);
        }
        return Object.seal(Rows);
      }
    },
    components: { Counters }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>