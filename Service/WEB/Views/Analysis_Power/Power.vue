<template>
  <div>
    <graph :options="GraphOptions" :height="height"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { data: Object, serie: String, height: Number, analyzer: Object, periods: Object, bills: Array, showContracted: Boolean },
    data () {
      return { };
    },
    computed: {
      GraphOptions: function() {
        var Opt = { file: [], axes: { y: {}, x: {} } };
        Opt.showRangeSelector = false;
        Opt.panEdgeFraction = 1;
        Opt.strokeWidth = 0.5;
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = true;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.interactionModel = Graph.InteractionModels.Scrollable(this);
        Opt.underlayCallback = (ctx, area, dygraph) => { Graph.Callbacks.Underlay_DrawPeriods(ctx, area, dygraph, this.periods, this.bills) };
        Opt.labels = (this.serie == 'Active_Power') ? ['Time', 'Active Power', 'Contracted Power'] : ['Time', 'Reactive Power'];
        Opt.colors = [this.analyzer.Color[this.serie], '#318ce7'];
        Opt.xAxisHeight = 24;
        Opt.selectMode = 'euclidian';
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatDateTime(x, '<br/>'); };
        Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatDateTime(x); };
        Opt.axes.y.valueFormatter = (x) => { return Graph.Formatters.Y_UnitScale(x, (this.serie == 'Active_Power') ? ' W' : 'var', 2, true);  };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return Graph.Formatters.Y_UnitScaleFull(x, gran, opts, g, '', 2, true); };
        Opt.series = { 'Contracted Power': { strokeWidth: 1, fillGraph: false } };
        if(this.data != null) {
          var TimeCol = this.data.Headers.indexOf('Sampletime');
          var ActiveCol = this.data.Headers.indexOf('Active_Power');
          var ReactiveCol = this.data.Headers.indexOf('Reactive_Power');
          var ContractedCol = this.data.Headers.indexOf('Power_Contracted');
          for(var n in this.data.Rows) {
            if(this.serie == 'Active_Power')
              Opt.file.push([this.data.Rows[n][TimeCol], this.data.Rows[n][ActiveCol], this.data.Rows[n][ContractedCol] * 1000]);
            else if(this.serie == 'Reactive_Power')
              Opt.file.push([this.data.Rows[n][TimeCol], this.data.Rows[n][ReactiveCol]]);
            else
              Opt.file.push([this.data.Rows[n][TimeCol], Math.sqrt(Math.pow(this.data.Rows[n][ReactiveCol], 2.0) + Math.pow(this.data.Rows[n][ActiveCol], 2.0))]);
          }
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, ((this.serie == 'Active_Power') && (this.showContracted == true)) ? [false, true, true] : [false, true], { incZero: 10 });
          Opt.dateWindow = [this.data.From, this.data.To];
        }
        return Object.seal(Opt);
      }
    }
  }
</script>