<template>
  <graph :options="GraphOptions" :height="200"></graph>
</template>

<script>
  import Graph from 'Components/Graph.vue';
//  import moment from 'moment';
  export default {
    props: { data: Array },
    data: function() {
      return { Labels: [] };
    },
    computed: {
      GraphOptions: function() {
        var Opt = { axes: { x: {}, y: {} } };
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.strokeWidth = 0.5,
        Opt.stepPlot = true;
        Opt.interactionModel = {};
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.xAxisHeight = 18;
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.plotter = Graph.Plotters.MultiBar;
        Opt.labels = ['Time', 'Cost'];
        Opt.colors = [this.Analyzer.SerieColor('Active_Energy')];
        Opt.includeZero = true;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 30,
        Opt.axes.y.valueFormatter = (x) => { return this.Round(x, 2, true) + ' €'; };
        Opt.axes.y.axisLabelFormatter = (x) => { return Math.round(x); };
        Opt.axes.x.valueFormatter = (x) => { return this.Labels[x]; };
        Opt.axes.x.axisLabelFormatter = (x) => { return this.Labels[x]; };
        if(this.data != null) {
          Opt.file = [];
          this.Labels = [];
          var MonthsNames = moment.months();
          for(var i in this.data) {
            this.Labels.push(MonthsNames[moment.tz(this.data[i].To, this.Analyzer.TimeZone).month()]);
            Opt.file.push([parseInt(i), (this.data[i].Complete == true) ? this.data[i].Total : null]);
          }
          Opt.dateWindow = [-0.5, this.data.length - 0.5];
        }
        return Opt;
      }
    }
  }
</script>