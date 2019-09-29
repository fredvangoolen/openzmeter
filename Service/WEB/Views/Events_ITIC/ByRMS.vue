<template>
  <div class="panel panel-other"> 
    <div class="panel-heading">By RMS variation (%)</div>
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
      this.RMSs = [-100, -80, -50, -20, -15, -10, -5, 0, 5, 10, 15, 20, 50, 100];
      this.GraphOptions = { strokeWidth: 1, labels: ["Variation", "Events"], legend: 'follow', plotter: Graph.multiColumnBarPlotter, colors: ["#DF842A"], dateWindow: [-1, this.RMSs.length], interactionModel: {},
        drawHighlightPointCallback: Graph.DrawHighlightBarChartCallback,
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 16,
               valueFormatter: function (x) { return x; },
               axisLabelFormatter: function (x) { return ((x % 1) !== 0) ? "" : x; }
             },
          x: {  pixelsPerLabel: 26,
                valueFormatter: function (x) { return ((x < 0) || (x >= this.RMSs.length)) ? "" : (this.RMSs[x] === 0) ? 'Low' : (this.RMSs[x] < 0) ? '<' + (100 + this.RMSs[x]) + '%' : '>' + (100 + this.RMSs[x]) + '%'; }.bind(this),
                axisLabelFormatter: function (x) { return ((x < 0) || (x >= this.RMSs.length)) ? "" : (this.RMSs[x] === 0) ? 'Low' : (this.RMSs[x] < 0) ? (100 + this.RMSs[x]) : (100 + this.RMSs[x]); }.bind(this)
             }
        }
      };
    },
    methods: {
      MaxDiff: function(a) {
        if(this.$root.SelectedAnalyzer === null) return 0;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Max = a[0] - Analyzer.NominalVoltage;
        for(var n = 1; n < a.length; n++) {
          if(Math.abs(a[n] - Analyzer.NominalVoltage) > Math.abs(Max)) Max = a[n] - Analyzer.NominalVoltage;
        }
        return (Max * 100.0) / Analyzer.NominalVoltage;
      },
      SetData: function(start, end, events) {
        var ByRMS = [];
        for(var i = 0; i < this.RMSs.length; i++) ByRMS.push([i, 0]);
        for(var i = 0; i < events.StartTime.length; i++) {
          if((events.StartTime < start) || (events.StartTime > end)) continue;
          var Diff = this.MaxDiff(events.Peak[i]);
          for(var j = 0; j < this.RMSs.length; j++) {
            if((Diff >= this.RMSs[j]) && (Diff < this.RMSs[j + 1])) {
              ByRMS[j][1] = ByRMS[j][1] + 1;
              break;
            }
          }
        }
        this.$refs.graph.SetOptions({ file: ByRMS });
      }
    },
    components: { Graph }
  };
</script>