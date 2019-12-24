<template>
  <div class="panel panel-other"> 
    <div class="panel-heading">By date</div>
    <div class="panel-body">
      <graph ref="graph" :options="GraphOptions"></graph>
    </div>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import moment from 'moment';
  export default {
    data: function() {
      return {};
    },
    created : function() {
      this.Dates = [];
      this.GraphOptions = { strokeWidth: 0.5, labels: ["Duration", "Events"], legend: 'follow', plotter: Graph.multiColumnBarPlotter, colors: ["#DF842A"], interactionModel: {},
        drawHighlightPointCallback: Graph.DrawHighlightBarChartCallback,
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 16,
               valueFormatter: function (x) { return x; },
               axisLabelFormatter: function (x) { return ((x % 1) !== 0) ? "" : x; }
             },
          x: {  pixelsPerLabel: 26,
                valueFormatter: (x) => { return ((x < 0) || (x >= this.Dates.length)) ? "" : moment.months()[moment(this.Dates[x][1]).month()]; },
                axisLabelFormatter: (x) => { return ((x < 0) || (x >= this.Dates.length) || (Number.isInteger(x) === false)) ? "" : moment.monthsShort()[moment(this.Dates[x][1]).month()]; }
             }
        }
      };
    },
    methods: {
      GenerateMonths: function(min, max) {
        var Start = new Date(min), End = new Date(max);
        Start = new Date(Start.getFullYear(), Start.getMonth(), 1);
        this.Dates = [];
        while(Start <= End) {
          this.Dates.push([new Date(Start.getFullYear(), Start.getMonth(), 1).getTime(),new Date(Start.getFullYear(), Start.getMonth() + 1, 0).getTime(), (Start.getMonth() + 1) + '/' + Start.getFullYear()]);
          Start.setMonth(Start.getMonth() + 1);
        }
      },
      GenerateDays: function(min, max) {
        var Start = new Date(min), End = new Date(max);
        this.Dates = [];
        while(Start <= End) {
          Dates.push([new Date(Start.getFullYear(), Start.getMonth(), Start.getDate()).getTime(),new Date(Start.getFullYear(), Start.getMonth(), Start.getDate() + 1).getTime(), Start.getDate() + '/' + (Start.getMonth() + 1)]);
          Start.setDate(Start.getDate() + 1);
        }
      },
      SetData: function(start, end, events) {
        var ByDate = [];
        this.GenerateMonths(start, end);
        if(this.Dates.length < 3) this.GenerateDays(start, end);
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