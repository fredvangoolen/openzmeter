<template>
  <div @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
    <graph :options="GraphOptions" :height="height" :paused="MouseOver"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    props: { analyzer: Object, data: Object, height: Number },
    data() {
      return { MouseOver: false };
    },
    computed: {
      GraphOptions: function() {
        var Opt = { axes: { x: {}, y: {} }, file: [] };
        Opt.panEdgeFraction = 1;
        Opt.strokeWidth = 0.5;
        Opt.stepPlot = true;
        Opt.showRangeSelector = false;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.gridLineColor = '#BBB';
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.selectMode = 'euclidian';
        Opt.plugins = [ Graph.Plugins.Vertical_Crosshair() ];
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.labels = ['Time', 'Frequency'];
        Opt.interactionModel = Graph.InteractionModels.Scrollable(this);
        Opt.colors = [this.analyzer.Color.Frequency];
        Opt.xAxisHeight = 24;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { return this.Round(x, 3, false) + ' Hz'; };
        Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(x, 3, false); };
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatDateTime(x, '<br/>'); };
        Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatDateTime(x); };
        if(this.data != null) {
          for(var n in this.data.Sampletime) Opt.file.push([this.data.Sampletime[n], this.data.Frequency[n]]);
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true], { rmsRange: 0.05 });
          Opt.dateWindow = [this.data.From, this.data.To];
        }
        return Object.seal(Opt);
      }
    }
  }
</script>