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
    props: { data: Object },
    data: function() {
      return { };
    },
    created: function() {
      this.RMSs = [0, 20, 50, 80, 85, 90, 95, 100, 105, 110, 115, 120, 150, 200];
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
        Opt.gridLineColor = "#DDD";
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
        Opt.axes.x.valueFormatter = (x) => { return ((x < 0) || (x >= this.RMSs.length)) ? "" : (this.RMSs[x] == 100) ? 'Low' : (this.RMSs[x] < 0) ? '<' + this.RMSs[x] + '%' : '>' + this.RMSs[x] + '%'; };
        Opt.axes.x.axisLabelFormatter = (x) => { return (((x % 1 == 0)) && (x >= 0) && (x < this.RMSs.length)) ? (this.RMSs[x] == 100) ? 'Low' : (this.RMSs[x] < 0) ? this.RMSs[x] : this.RMSs[x] : ''; };
        for(var i = 0; i < this.RMSs.length; i++) Opt.file.push([i, 0]);
        if(this.data != null) {
          var PeakCol  = this.data.Headers.indexOf('Peak');
          for(var i = 0; i < this.data.Rows.length; i++) {
            var Diff = this.MaxDiff(this.data.Rows[i][PeakCol]) + 100;
            for(var j = 0; j < this.RMSs.length - 1; j++) {
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
          if(Math.abs(a[n] - this.analyzer.NominalVoltage) > Math.abs(Max)) Max = a[n] - this.analyzer.NominalVoltage;
        }
        return (Max * 100.0) / this.analyzer.NominalVoltage;
      }
    },
    components: { Graph }
  };
</script>