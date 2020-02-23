<template>
  <div>
    <graph :options="GraphOptions" :height="height"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    props: { data: { type: Object }, serie: { type: String }, height: { type: Number, default: 300 }, analyzer: { type: Object } },
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
        Opt.plotter = Graph.Plotters.MultiBar;
        Opt.interactionModel = Graph.InteractionModels.Scrollable(this);
        Opt.drawHighlightPointCallback = Graph.Callbacks.drawHighlightPoint_HighlightBar;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.labels = (this.serie == 'Active_Energy') ? ['Time', 'Active Energy'] : ['Time', 'Reactive Energy'];
        Opt.colors = [this.analyzer.Color[this.serie]];
        Opt.xAxisHeight = 24;
        Opt.selectMode = 'euclidian';
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatDateTime(x, '<br/>'); };
        Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatDateTime(x); };
        Opt.axes.y.valueFormatter = (x) => { return this.Round(this.ScaleUnits(x, this.BestUnits(x)), 2, false) + ((this.serie == 'Active_Energy') ? ' Wh' : 'VArh');  };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return (g.axes_[0].valueRange == null) ? '' : this.Round(this.ScaleUnits(x, this.BestUnits(Math.max(Math.abs(g.axes_[0].valueRange[0]), Math.abs(g.axes_[0].valueRange[1])))), 2, true); };
        if(this.data != null) {
          var Div = (this.data.Aggregation_Source == '1H') ? 1.0 : 4.0;
          for(var n in this.data.Sampletime) Opt.file.push([this.data.Sampletime[n], this.data[(this.serie == 'Active_Energy') ? 'Active_Power' : 'Reactive_Power'][n] / Div]);
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