<template>
  <div>
    <graph :options="GraphOptions" :height="height"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    props: { data: Object, serie: String, height: Number, analyzer: Object, periods: Object },
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
        Opt.underlayCallback = (ctx, area, dygraph) => { Graph.Callbacks.Underlay_DrawPeriods(ctx, area, dygraph, this.periods) };
        Opt.labels = (this.serie == 'Active_Power') ? ['Time', 'Active Power', 'Contracted Power'] : ['Time', 'Reactive Power'];
        Opt.colors = [this.analyzer.Color[this.serie], '#d9534f'];
        Opt.xAxisHeight = 24;
        Opt.selectMode = 'euclidian';
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatDateTime(x, '<br/>'); };
        Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatDateTime(x); };
        Opt.axes.y.valueFormatter = (x) => { return Graph.Formatters.Y_UnitScale(x, (this.serie == 'Active_Power') ? ' W' : 'var', 2, true);  };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return Graph.Formatters.Y_UnitScaleFull(x, gran, opts, g, '', 2, true); };
        Opt.series = { 'Contracted Power': { strokeWidth: 2, fillGraph:false } };
        if(this.data != null) {
          for(var n in this.data.Sampletime) {
            if(this.serie == 'Active_Power')
              Opt.file.push([this.data.Sampletime[n], this.data['Active_Power'][n], this.data['Period_Power'][n]]);
            else
              Opt.file.push([this.data.Sampletime[n], this.data['Reactive_Power'][n]]);
          }
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true], { incZero: 10 });
          Opt.dateWindow = [this.data.From, this.data.To];
        }
        return Object.seal(Opt);
      }
    }
  }
</script>