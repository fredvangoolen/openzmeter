<template>
  <div class="panel panel-other"> 
    <div class="panel-heading">By duration</div>
    <div class="panel-body">
      <graph ref="graph" :options="GraphOptions"></graph>
    </div>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    data: function() {
      return {};
    },
    created: function() {
      this.Durations = [[0, '20ms'], [0.020, '50ms'], [0.050, '100ms'], [0.100, '200ms'], [0.200, '500ms'], [0.500, '1s'], [1, '2s'], [2, '5s'], [5, '10s' ], [10, '20s' ], [20, '50s'], [50, '100s'], [Infinity, 'Others']];
      this.GraphOptions = { strokeWidth: 0.5, labels: ["Date", "Events"], legend: 'follow', plotter: Graph.multiColumnBarPlotter, colors: ["#DF842A"], dateWindow: [-1, this.Durations.length], interactionModel: {},
        drawHighlightPointCallback: Graph.DrawHighlightBarChartCallback,
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 16,
               valueFormatter: function (x) { return x; },
               axisLabelFormatter: function (x) { return ((x % 1) !== 0) ? "" : x; }
             },
          x: {  pixelsPerLabel: 40, axisLabelWidth: 40,
                valueFormatter: (x) => { return ((x < 0) || (x >= this.Durations.length)) ? "" : this.Durations[x][1]; },
                axisLabelFormatter: (x) => { return ((x < 0) || (x >= this.Durations.length)) ? "" : this.Durations[x][1]; }
             }
        }
      };
    },
    methods: {
      SetData: function(start, end, events) {
        var ByDuration = [];
        for(var i = 0; i < this.Durations.length; i++) ByDuration.push([i, 0]);
        for(var i = 0; i < events.StartTime.length; i++) {
          if((events.StartTime < start) || (events.StartTime > end)) continue;
          for(var j = 0; j < this.Durations.length; j++) {
            if(((events.Duration[i] / events.SampleRate[i]) >= this.Durations[j][0]) && ((events.Duration[i] / events.SampleRate[i]) < this.Durations[j + 1][0])) {
              ByDuration[j][1] = ByDuration[j][1] + 1;
              break;
            }
          }
        }
        this.$refs.graph.SetOptions({ file: ByDuration });
      }
    },
    components: { Graph }
  };
</script>