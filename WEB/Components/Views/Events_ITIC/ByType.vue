<template>
  <div class="panel panel-other"> 
    <div class="panel-heading">By type</div>
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
      this.Types = ['RVC', 'DIP', 'SWELL', 'INTERRUPTION'];
      this.GraphOptions = { strokeWidth: 0.5, labels: ["Type", "Events"],  legend: 'follow', plotter: Graph.multiColumnBarPlotter, colors: ["#DF842A"], interactionModel: {}, dateWindow: [-1, this.Types.length],
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 14,
               valueFormatter: function (x) { return x; },
               axisLabelFormatter: function (x) { return ((x % 1) !== 0) ? "" : x; }
             },
          x: {  pixelsPerLabel: 26,
                valueFormatter: function (x) { return (((x % 1) !== 0) || (x < 0) || (x >= this.Types[x])) ? "" : this.Types[x]; }.bind(this),
               axisLabelFormatter: function (x) { return (((x % 1) !== 0) || (x < 0) || (x >= this.Types[x])) ? "" : this.Types[x]; }.bind(this)
             }
        }
      };
    },
    methods: {
      SetData: function(start, end, events) {
        var ByType = [];
        for(var i = 0; i < this.Types.length; i++) ByType.push([i, 0]);
        for(var i = 0; i < events.StartTime.length; i++) {
          if((events.StartTime < start) || (events.StartTime > end)) continue;
          for(var j = 0; j < this.Types.length; j++) {
            if(events.Type[i] === this.Types[j]) {
              ByType[j][1] = ByType[j][1] + 1;
              break;
            }
          }
        }
        this.$refs.graph.SetOptions({ file: ByType });
      }
    },
    components: { Graph }
  };
</script>