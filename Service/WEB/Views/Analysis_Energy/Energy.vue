<template>
  <div>
    <graph :options="GraphOptions" :height="height"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    props: { data: Object, serie: String, height: Number, analyzer: Object, periods: Object, bills: Array },
    data() {
      return { };
    },
    computed: {
      GraphOptions: function() {
        var Opt = { file: [], axes: { y: {}, x: {} } };
        Opt.panEdgeFraction = 1;
        Opt.strokeWidth = 0.5;
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.drawAxesAtZero = true;
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.plotter = (e) => { Graph.Plotters.BarOffset(e, ((this.data != null) && (this.data.Aggreg == '1H')) ? 3600000 : 900000); };
        Opt.interactionModel = Graph.InteractionModels.Scrollable(this);
        Opt.drawHighlightPointCallback = Graph.Callbacks.DrawHighlightPoint_HighlightBar;
        Opt.underlayCallback = (ctx, area, dygraph) => { Graph.Callbacks.Underlay_DrawPeriods(ctx, area, dygraph, this.data) };
        Opt.labels = (this.serie == 'Active_Energy') ? ['Time', 'Active Energy'] : ['Time', 'Reactive Energy'];
        Opt.colors = [this.analyzer.SerieColor(this.serie)];
        Opt.xAxisHeight = 12;
        Opt.selectMode = 'euclidian';
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.x.valueFormatter = (x) => { return Graph.Formatters.X_Hourly(x, this.analyzer.TimeZone,  ((this.data != null) && (this.data.Aggreg == '1H')) ? false : true); };
        Opt.axes.x.ticker = (min, max, pixels) => { return Graph.Formatters.X_HourlyFull(min, max, pixels, (this.analyzer != null) ? this.analyzer.TimeZone : null); };
        Opt.axes.y.valueFormatter = (x, opts, seriesName) => { return Graph.Formatters.Y_UnitScale(x, ((this.serie == 'Active_Energy') ? ' Wh' : 'varh'), 2, true);  };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return Graph.Formatters.Y_UnitScaleFull(x, gran, opts, g, '', 2, true); };
        if(this.data != null) {
          var TimeCol = this.data.Headers.indexOf('Time');
          var ValueCol = this.data.Headers.indexOf(this.serie);
          for(var n = 0; n < this.data.Rows.length; n++) Opt.file.push([this.data.Rows[n][TimeCol], this.data.Rows[n][ValueCol]]);
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true], { incZero: 10 });
          Opt.dateWindow = [this.data.From, this.data.To];
        }
        return Opt;
      }
    },
    created: function() {
      this.Periods = [];
      this.Bills = [];
      this.ActiveData = [];
      this.ReactiveData = [];
      this.NowTimer = null;
    }
  }
</script>