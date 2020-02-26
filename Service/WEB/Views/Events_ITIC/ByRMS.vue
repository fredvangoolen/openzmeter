<template>
  <div class="panel panel-other">
    <div class="panel-heading">By RMS variation (%)</div>
    <div class="panel-body">
      <spinner v-if="data==null"></spinner>
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
      this.RMSs = [-100, -80, -50, -20, -15, -10, -5, 0, 5, 10, 15, 20, 50, 100];
    },
    computed: {
      GraphOptions: function() {
        var Opt = { axes: { x: {}, y: {} }, file: [] };
        Opt.strokeWidth = 0.5;
        Opt.showRangeSelector = false;
        Opt.labels = ["Variation", "Events"];
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.plotter = Graph.Plotters.MultiBar;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.colors = ["#DF842A"];
        Opt.interactionModel = {};
        Opt.dateWindow = [-0.5, this.RMSs.length - 0.5];
        Opt.drawHighlightPointCallback = Graph.Callbacks.DrawHighlightPoint_HighlightBar;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 24;
        Opt.axes.y.valueFormatter = (x) => { return x; };
        Opt.axes.y.axisLabelFormatter = (x) => { return ((x % 1) != 0) ? "" : x; };
        Opt.axes.x.pixelsPerLabel = 26;
        Opt.axes.x.valueFormatter = (x) => { return ((x < 0) || (x >= this.RMSs.length)) ? "" : (this.RMSs[x] == 0) ? 'Low' : (this.RMSs[x] < 0) ? '<' + (100 + this.RMSs[x]) + '%' : '>' + (100 + this.RMSs[x]) + '%'; };
        Opt.axes.x.axisLabelFormatter = (x) => { return (((x % 1 == 0)) && (x >= 0) && (x < this.RMSs.length)) ? (this.RMSs[x] == 0) ? 'Low' : (this.RMSs[x] < 0) ? (100 + this.RMSs[x]) : (100 + this.RMSs[x]) : ''; };
        for(var i = 0; i < this.RMSs.length; i++) Opt.file.push([i, 0]);
        if(this.data != null) {
          for(var i in this.data.StartTime) {
            var Diff = this.MaxDiff(this.data.Peak[i]);
            for(var j in this.RMSs) {
              if((Diff >= this.RMSs[j]) && (Diff < this.RMSs[j + 1])) {
                Opt.file[j][1] = Opt.file[j][1] + 1;
                break;
              }
            }
          }
        }
        return Object.seal(Opt);
      }
    },
    methods: {
      MaxDiff: function(a) {
        var Max = a[0] - this.analyzer.NominalVoltage;
        for(var n = 1; n < a.length; n++) {
          if(Math.abs(a[n] - this.analyzer.NominalVoltage) > Math.abs(Max)) Max = a[n] - this.nalyzer.NominalVoltage;
        }
        return (Max * 100.0) / this.analyzer.NominalVoltage;
      }
    },
    components: { Graph }
  };
</script>