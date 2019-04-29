<template>
  <div class="panel panel-default"> 
    <div class="panel-heading">By type</div>
    <div class="panel-body">
      <graph :options="GraphOptions"></graph>
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
      this.GraphOptions = { strokeWidth: 1, 
        labels: ["Type", "Events"],
        labelsDiv: "ByTypeLegend",
        stepPlot: true,
//        plotter: multiColumnBarPlotter,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        dateWindow: [-1, this.Types.length],
        interactionModel: {},
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 14,
               valueFormatter: function (x) {
                 return x;
               },
               axisLabelFormatter: function (x) {
                 if((x % 1) !== 0) return "";
                 return '<div style="font-size: 10px; margin-top: -2px">' + x + '</div>';
               }
             },
          x: {  pixelsPerLabel: 26,
                valueFormatter: function (x) { return (((x % 1) !== 0) || (x < 0) || (x >= this.Types[x])) ? "" : this.Types[x]; }.bind(this),
               axisLabelFormatter: function (x) { return (((x % 1) !== 0) || (x < 0) || (x >= this.Types[x])) ? "" : this.Types[x]; }.bind(this)
             }
        }
      };
    },
    methods: {
      SetData: function() {
//    ByTypeGraph.updateOptions({'file': ByType});
  
        
      }
    },
    components: { Graph }
  };
</script>