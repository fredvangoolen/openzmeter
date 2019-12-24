<template>
  <div>
    <graph :options="GraphOptions" :data="GraphData"></graph>
  </div>
</template>

<script>
  import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
  import ToolbarPeriod from 'Components/ToolbarPeriod.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: { type: Object } },
    data() {
      return { 
        Aggreg: null, ShowSpinner: true, NowData: null, GraphData: null, GraphOptions: {},
        NowTimer: null, GraphTimer: null,
      CountersAggregation: 'Loading data...', TimeStatsFrom: '...', TimeStatsTo: '...', MinTime: '?', MaxTime: '?', MinValue: '?', MaxValue: '?', AvgValue: '?', AvgSamples: '?', StdPercent: '?', StdValue: '?' };
    },
    watch: {
      analyzer: function() { this.UpdateNowValues(); this.UpdateGraph(); },
      Aggreg: function() {  this.UpdateGraph(); }
    },
    created: function() {
      this.GraphOptions.strokeWidth = 0.5;
      this.GraphOptions.stepPlot = true;
      this.GraphOptions.drawGapEdgePoints = true;
      this.GraphOptions.connectSeparatedPoints = false;
      this.GraphOptions.fillGraph = false;
      this.GraphOptions.gridLineColor = '#BBB';
      this.GraphOptions.gridLinePattern = [4, 4];
//      , legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow,
//        labels: ['Time', 'Frequency'], interactionModel: Graph.ScrollableInteractionModel(this), colors: [oZm.Series['FREQUENCY'].Color], xAxisHeight: 24, 
//        axes: {
//          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
//               valueFormatter: function (x) { return oZm.Round3(x) + ' <em class="x-small">Hz</em>'; },
//               axisLabelFormatter: function (x) { return oZm.Round3(x); }
//             },
//          x: { axisLabelFormatter: function(x) { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); },
//               valueFormatter: function (x) { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); }
//             }
//        }
//      };

      //this.Data = [];
      //this.NowTimer = null;
    },
//    mounted: function() {
//      $(window).on('resize', this.ResizeHandler.bind(this));
//      window.dispatchEvent(new Event('resize'));
//      this.NowValue();
//    },
//    beforeDestroy: function() {
//      $(window).off('resize', this.ResizeHandler);
//    },
    methods: {
//      AggregationChanged: function(aggreg) {
//        this.$refs.period.SetAggregation(aggreg);
//      },
//      ResizeHandler: function() {
//        if(typeof this.$refs.graph !== 'undefined') this.$refs.graph.Redraw('100%', $(this.$refs.counters).height());
//      },
      MaximizeClick: function() {
        $(window).unbind('resize', this.ResizeHandler);
        this.$refs.tools.Maximize('Energy consumption', this.$refs.graph.Graph, this.$refs.Toolbar);
      },
      MinimizeClick: function() {
        $(window).bind('resize', this.ResizeHandler.bind(this));
      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        this.$refs.tools.SaveCVS('Frequency.csv', [['Time', 'Frequency']].concat(this.Data));
      },
      UpdateCounters: function(aggreg, from, to) {
        this.CountersAggregation = 'From ' + oZm.GetAggregLongName(aggreg) + ' aggregation';        
        this.TimeStatsFrom = moment(from).format('DD/MM/YYYY HH:mm:ss');
        this.TimeStatsTo = moment(to).format('DD/MM/YYYY HH:mm:ss');
        var AvgSamples = 0, MinValue = +Infinity, MaxValue = -Infinity, AvgValue = 0, MinCount = 0, MaxCount = 0, StdPercent = 0, StdValue = 0, MinTime = null, MaxTime = null;
        for(var x = 0; x < this.Data.length; x++) {
          if(this.Data[x][1] === null) continue;
          if(this.Data[x][1] < MinValue) {
            MinValue = this.Data[x][1];
            MinCount = 0;
            MinTime = this.Data[x][0];
          } else if(this.Data[x][1] === MinValue) {
            MinCount++;
          }
          if(this.Data[x][1] > MaxValue) {
            MaxValue = this.Data[x][1];
            MaxCount = 0;
            MaxTime = this.Data[x][0];
          } else if(this.Data[x][1] === MaxValue) {
            MaxCount++;
          }
          if((this.Data[x][1] > 49) && (this.Data[x][1] < 51)) StdPercent++;
          AvgValue += this.Data[x][1];
          StdValue += Math.pow(this.Data[x][1], 2.0); 
          AvgSamples++;
        }
        this.AvgSamples = AvgSamples;
        this.StdValue = oZm.Round3(Math.sqrt((StdValue / AvgSamples) - Math.pow(AvgValue / AvgSamples, 2))) + ' <em>Hz</em>';
        this.MinTime = moment(MinTime).format("hh:mm:ss - DD/MM/YYYY") + ((MinCount > 1) ? ' (and ' + MinCount + ' times more)' : '');
        this.MaxTime = moment(MaxTime).format("hh:mm:ss - DD/MM/YYYY") + ((MaxCount > 1) ? ' (and ' + MaxCount + ' times more)' : '');
        this.StdPercent = oZm.Round2(StdPercent / AvgSamples * 100) + '% within range';
        this.MinValue = oZm.Round3(MinValue) + ' <em>Hz</em>';
        this.MaxValue = oZm.Round3(MaxValue) + ' <em>Hz</em>';
        this.AvgValue = oZm.Round3(AvgValue / AvgSamples) + ' <em>Hz</em>';
        this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
      },

      WindowRestore: function() {
        this.$refs.period.PopState();
      },
      WindowChange: function(minDate, maxDate) {
        this.$refs.period.SetCustom(minDate, maxDate, true);
      }
    },
    components: { Spinner, ToolbarAggregation, ToolbarPeriod, ToolbarTools, Graph }
  }
</script>