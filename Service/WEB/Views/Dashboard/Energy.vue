<template>
  <q-card>
    <q-toolbar class="bg-indigo-9 text-white q-pr-none">
      <portal to="dlg-toolbar" :disabled="!Expanded">
        <q-toolbar-title>Energy consumption</q-toolbar-title>
        <toolbar-aggreg storage="Dashboard.Aggreg" :options="['15M', '1H']" v-model="Aggreg" />
        <q-separator dark vertical />
        <toolbar-period storage="Period" v-model="Range"></toolbar-period>
        <q-separator dark vertical />
        <toolbar-export file="Energy" :rows="Export" />
        <q-separator dark vertical />
        <toolbar-expand v-model="Expanded" />
      </portal>
    </q-toolbar>
    <q-card-section class="row q-pa-xs">
      <counters class="col-auto" :height.sync="Height" :data="Data" />
      <div class="col-grow">
        <portal to="dlg-body" :disabled="!Expanded">
          <!--spinner v-if="TransferGraph.Data==null"></spinner-->
          <div @mouseenter="MouseOver=true" @mouseleave="MouseOver=false" class="q-ma-xs">
            <graph :options="GraphOptions" :height="Expanded?DialogHeight:Height-10" :paused="MouseOver"></graph>
          </div>
                <!--energy :height="" :data="TransferGraph.Data" @range="Range=$event"></energy-->
                  </portal>
      </div>
    </q-card-section>
    <dialog-fullscreen :modelValue="Expanded" color="bg-indigo-9" :height.sync="DialogHeight" />
  </q-card>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import Counters from './Counters.vue';
  export default {
    data: function() {
      return { Aggreg: null, Range: null, Height: 200, Expanded: false, DialogHeight: 200, MouseOver: false,
               //TransferGraph: this.PriceTransfer(['Active_Energy', 'Cost', 'Color', 'Period', 'Bill'])
               Data: this.Analyzer.Queries.Suscribe('Costs', ['Active_Energy', 'Cost', 'Color', 'Period', 'Bill'], 60000)
      };
    },
    unmounted: function() {
      this.Data.Stop();
    },
    watch: {
      Aggreg: function(val) {
        this.Data.SetAggreg(val);
      },
      Range: function(val) {

        this.Data.SetRange(val);
      }
    },
    computed: {
      GraphOptions: function() {
        var Opt = { file: [], axes: { y: {}, x: {} } };
        Opt.panEdgeFraction = 1;
        Opt.interactionModel = Graph.InteractionModels.Scrollable(this);
        Opt.strokeWidth = 0.5;
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.legend = 'follow';
//        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.plotter = (e) => { Graph.Plotters.BarOffset(e, ((this.Data.Rows != null) && (this.Aggreg == '1H')) ? 3600000 : 900000); };
        Opt.labels = ['Time', 'Energy'];
        Opt.colors = ['rgb(40, 53, 147)'];
        Opt.xAxisHeight = 12;
        Opt.underlayCallback = (ctx, area, dygraph) => { Graph.Callbacks.Underlay_DrawPeriods(ctx, area, dygraph, this.data); };
        Opt.drawHighlightPointCallback = Graph.Callbacks.DrawHighlightPoint_HighlightBar;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x, opts, seriesName) => { return Graph.Formatters.Y_UnitScale(x, 'Wh', 2, true);  };
        //Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return Graph.Formatters.Y_UnitScaleFull(x, gran, opts, g, '', 2, true); };
        //Opt.axes.x.valueFormatter = (x) => { return Graph.Formatters.X_Hourly(x, this.analyzer.TimeZone,  ((this.data != null) && (this.data.Aggreg == '1H')) ? false : true); };
        Opt.axes.x.ticker = (min, max, pixels) => { return Graph.Formatters.X_HourlyFull(min, max, pixels, (this.analyzer != null) ? this.analyzer.TimeZone : null); };
        if(this.Data.Headers != null) {
          var TimeCol = this.Data.Headers.indexOf('Time');
          var EnergyCol = this.Data.Headers.indexOf('Active_Energy');
          for(var n in this.Data.Rows) Opt.file.push([this.Data.Rows[n][TimeCol], this.Data.Rows[n][EnergyCol]]);
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true], { incZero: 10 });
//          Opt.dateWindow = [this.data.From, this.data.To];
        }
        return Opt;
      }
    },
    methods: {
      Export: function() {
        if(this.TransferGraph.Data == null) return [];
        var Data = [['Time', 'Active Energy', 'Period expense']];
        var TimeCol = this.TransferGraph.Data.Headers.indexOf('Time');
        var ActiveCol = this.TransferGraph.Data.Headers.indexOf('Active_Energy');
        var ExpenseCol = this.TransferGraph.Data.Headers.indexOf('Cost');
        for(var i = 0; i < this.TransferGraph.Data.Rows.length; i++) {
          if(this.TransferGraph.Data.Rows[i][ActiveCol] == null) continue;
          Data.push([this.analyzer.FormatDateTime(this.TransferGraph.Data.Rows[i][TimeCol]), this.Round(this.TransferGraph.Data.Rows[i][ActiveCol], 3), this.Round(this.TransferGraph.Data.Rows[i][ExpenseCol], 3)]);
        }
        return Object.seal(Data);
      }

    },
    components: { Counters, Graph }
  }
</script>