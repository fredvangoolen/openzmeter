<template>
  <div class="panel panel-other">
    <div class="panel-heading">By date</div>
    <div class="panel-body">
      <spinner v-if="data==null"></spinner>
      <graph :options="GraphOptions"></graph>
    </div>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import moment from 'moment';
  export default {
    props: { data: Object },
    data: function() {
      return { };
    },
    created: function() {
      this.Months = moment.months();
    },
    computed: {
      GraphOptions: function() {
        var Opt = { axes: { x: {}, y: {} }, file: [] };
        Opt.strokeWidth = 0.5;
        Opt.showRangeSelector = false;
        Opt.labels = ["Type", "Events"];
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.plotter = Graph.Plotters.MultiBar;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.colors = ["#DF842A"];
        Opt.interactionModel = {};
        Opt.dateWindow = [-0.5, this.Months.length - 0.5];
        Opt.drawHighlightPointCallback = Graph.Callbacks.DrawHighlightPoint_HighlightBar;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 24;
        Opt.axes.y.valueFormatter = (x) => { return x; };
        Opt.axes.y.axisLabelFormatter = (x) => { return ((x % 1) != 0) ? "" : x; };
        Opt.axes.x.pixelsPerLabel = 26;
        Opt.axes.x.valueFormatter = (x) => { return (((x % 1) == 0) && (x >= 0) && (x < this.Months.length)) ? this.Months[x] : ''; };
        Opt.axes.x.axisLabelFormatter = (x) => { return (((x % 1) == 0) && (x >= 0) && (x < this.Months.length)) ? this.Months[x] : ''; };
        for(var i = 0; i < this.Months.length; i++) Opt.file.push([i, 0]);
        if(this.data != null) {
          var StartCol = this.data.Headers.indexOf('StartTime');
          for(var i = 0; i < this.data.Rows.length; i++) {
            var Month = moment(this.data.Rows[i][StartCol]).tz(this.Analyzer.TimeZone).month();
            Opt.file[Month][1] = Opt.file[Month][1] + 1;
          }
        }
        return Object.seal(Opt);
      }
    },
    components: { Graph }
  };
</script>