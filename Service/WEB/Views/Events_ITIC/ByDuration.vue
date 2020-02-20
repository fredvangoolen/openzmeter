<template>
  <div class="panel panel-other">
    <div class="panel-heading">By duration</div>
    <div class="panel-body">
      <graph :options="GraphOptions"></graph>
    </div>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    props: { data: Object, analyzer: Object },
    data: function() {
      return { };
    },
    created: function() {
      this.Durations = [[0, '20ms'], [0.020, '50ms'], [0.050, '100ms'], [0.100, '200ms'], [0.200, '500ms'], [0.500, '1s'], [1, '2s'], [2, '5s'], [5, '10s' ], [10, '20s' ], [20, '50s'], [50, '100s'], [Infinity, 'Others']];
    },
    computed: {
      GraphOptions: function() {
        var Opt = { axes: { x: {}, y: {} }, file: [] };
        Opt.strokeWidth = 0.5;
        Opt.showRangeSelector = false;
        Opt.labels = ["Duration", "Events"];
        Opt.legend = 'follow';
        Opt.plotter = Graph.multiColumnBarPlotter;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.colors = ["#DF842A"];
        Opt.interactionModel = {};
        Opt.dateWindow = [-0.5, this.Durations.length - 0.5];
        Opt.drawHighlightPointCallback = Graph.DrawHighlightBarChartCallback;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 24;
        Opt.axes.y.valueFormatter = (x) => { return x; };
        Opt.axes.y.axisLabelFormatter = (x) => { return ((x % 1) != 0) ? "" : x; };
        Opt.axes.x.pixelsPerLabel = 26;
        Opt.axes.x.valueFormatter = (x) => { return (((x % 1) == 0) && (x >= 0) && (x < this.Durations.length)) ? this.Durations[x][1] : ''; };
        Opt.axes.x.axisLabelFormatter = (x) => { return (((x % 1) == 0) && (x >= 0) && (x < this.Durations.length)) ? this.Durations[x][1] : ''; };
        for(var i = 0; i < this.Durations.length; i++) Opt.file.push([i, 0]);
        if(this.data != null) {
          for(var i in this.data.StartTime) {
            for(var j = 0; j < this.Durations.length; j++) {
              if(((this.data.Duration[i] / this.data.SampleRate[i]) >= this.Durations[j][0]) && ((this.data.Duration[i] / this.data.SampleRate[i]) < this.Durations[j + 1][0])) {
                Opt.file[j][1] = Opt.file[j][1] + 1;
                break;
              }
            }
          }
        }
        return Object.seal(Opt);
      }
    },
    components: { Graph }
  };
</script>