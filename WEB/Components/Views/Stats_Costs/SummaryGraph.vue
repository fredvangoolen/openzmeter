<template>
  <graph :options="GraphOptions" ref="graph"></graph>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'JS/moment.js';
  export default {
    data () {
      return { };
    },
    created: function() {
      this.GraphOptions = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', 
        xAxisHeight: 12, legendFormatter: oZm.LegendFormatter_Follow, 
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 30,
               valueFormatter: function (x) { var Result = this.SerieProps.FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; }.bind(this),
               axisLabelFormatter: function (x) { var Units = oZm.GetBestUnits(Math.max(Math.abs(Range[0]), Math.abs(Range[1]))); return oZm.Round2(oZm.ScaleToUnits(x, Units)); }
             },
          x: { valueFormatter: function (x) { return moment(x).format('hh:mm:ss'); }, axisLabelFormatter: function (x) { return moment(x).format('hh:mm'); } }
        }
      };
    },
    methods: {
    },
    components: { Graph }
  }
</script>