<template>
  <div @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
    <graph :options="GraphOptions" :height="height" :paused="MouseOver"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { analyzer: Object, height: Number, data: Object, periods: Object, bills: Array },
    data() {
      return { MouseOver: false };
    },
    computed: {
      GraphOptions: function() {
        var Opt = { file: [], axes: { y: {}, x: {} } };
        Opt.panEdgeFraction = 1;
        Opt.interactionModel = Graph.InteractionModels.Scrollable(this);
        Opt.strokeWidth = 0.5;
        Opt.drawGrid = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = true;
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.plotter = Graph.Plotters.MultiBar;
        Opt.labels = ['Time', 'Energy'];
        Opt.colors = [oZm.Series['ACTIVE_ENERGY'].Color];
        Opt.xAxisHeight = 20;
        Opt.underlayCallback = (ctx, area, dygraph) => { Graph.Callbacks.Underlay_DrawPeriods(ctx, area, dygraph, this.periods, this.bills); };
        Opt.drawHighlightPointCallback = Graph.Callbacks.DrawHighlightPoint_HighlightBar;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { var Units = this.BestUnits(x); return this.Round(this.ScaleUnits(x, Units), 2, true) + ' ' + Units + 'Wh'; },
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return (g.axes_[0].valueRange == null) ? '' : this.Round(this.ScaleUnits(x, this.BestUnits(Math.max(Math.abs(g.axes_[0].valueRange[0]), Math.abs(g.axes_[0].valueRange[1])))), 2, true); }
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatDateTime(x, '<br/>'); };
        Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatDateTime(x); };
        if(this.data != null) {
          var TimeCol = this.data.Headers.indexOf('Sampletime');
          var EnergyCol = this.data.Headers.indexOf('Active_Energy');
          for(var n in this.data.Rows) Opt.file.push([this.data.Rows[n][TimeCol], this.data.Rows[n][EnergyCol]]);
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true], { defMin: 0, defMax: 1, upperPercent: 0.1, lowerPercent: 0.1, minUpperWidth: 10, minLowerWidth: 10 });
          Opt.dateWindow = [this.data.From, this.data.To];
        }
        return Opt;
      }
    }
  }
</script>