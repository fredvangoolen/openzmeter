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
    props: { data: Object, analyzer: Object },
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
        Opt.plotter = Graph.multiColumnBarPlotter;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.colors = ["#DF842A"];
        Opt.interactionModel = {};
        Opt.dateWindow = [-0.5, this.Months.length - 0.5];
        Opt.drawHighlightPointCallback = Graph.DrawHighlightBarChartCallback;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 24;
        Opt.axes.y.valueFormatter = (x) => { return x; };
        Opt.axes.y.axisLabelFormatter = (x) => { return ((x % 1) != 0) ? "" : x; };
        Opt.axes.x.pixelsPerLabel = 26;
        Opt.axes.x.valueFormatter = (x) => { return (((x % 1) == 0) && (x >= 0) && (x < this.Months.length)) ? this.Months[x] : ''; };
        Opt.axes.x.axisLabelFormatter = (x) => { return (((x % 1) == 0) && (x >= 0) && (x < this.Months.length)) ? this.Months[x] : ''; };
        for(var i = 0; i < this.Months.length; i++) Opt.file.push([i, 0]);
        if(this.data != null) {
          for(var i in this.data.StartTime) {
            var Month = moment(this.data.StartTime[i]).tz(this.analyzer.TimeZone).month();
            Opt.file[Month][1] = Opt.file[Month][1] + 1;
          }
        }
        return Object.seal(Opt);
      }
    },
    methods: {
      SetData: function(start, end, events) {
        var ByDate = [];
        this.GenerateMonths(start, end);
        //if(this.Dates.length < 3) this.GenerateDays(start, end);
        for(var i = 0; i < this.Dates.length; i++) ByDate.push([i, 0]);
        for(var i = 0; i < events.StartTime.length; i++) {
          if((events.StartTime[i] < start) || (events.StartTime[i] > end)) continue;
          for(var n = 0; n < this.Dates.length; n++) {
            if((events.StartTime[i] >= this.Dates[n][0]) && (events.StartTime[i] < this.Dates[n][1])) {
              ByDate[n][1] = ByDate[n][1] + 1;
              break;
            }
          }
        }
        this.$refs.graph.SetOptions({ file: ByDate, dateWindow: [-1, this.Dates.length] });
      }
    },
    components: { Graph }
  };
</script>