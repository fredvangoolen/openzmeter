<template>
  <div>
    <graph :options="GraphOptions" :height="height"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
<<<<<<< HEAD
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
=======
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
  export default {
    props: { data: { type: Object }, serie: { type: String }, height: { type: Number, default: 300 }, analyzer: { type: Object } },
    data() {
      return { };
    },
    computed: {
      GraphOptions: function() {
<<<<<<< HEAD
        var Opt = {};
=======
        var Opt = { file: [], axes: { y: {}, x: {} } };
        Opt.panEdgeFraction = 1;
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
        Opt.strokeWidth = 0.5;
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
<<<<<<< HEAD
        Opt.legendFormatter = Graph.LegendFormatter_Follow;
        Opt.plotter = Graph.multiColumnBarPlotter;
        Opt.interactionModel = Graph.ScrollableInteractionModel(this);
        Opt.xAxisHeight = 24;
        Opt.drawHighlightPointCallback = Graph.DrawHighlightBarChartCallback;
        Opt.labels = (this.serie == 'ACTIVE_ENERGY') ? ['Time', 'Active Energy'] : ['Time', 'Reactive Energy'];
        Opt.colors = [oZm.Series[this.serie].Color];
        Opt.axes = { y: {}, x: {}};
        Opt.axes.x.axisLabelFormatter = (x) => { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); };
        Opt.axes.x.valueFormatter = (x) => { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        if(this.serie == 'ACTIVE_ENERGY') {
          Opt.axes.y.valueFormatter = (x) => { var Result = oZm.Series['ACTIVE_ENERGY'].FormatValue(x); return Result.Value + ' ' + Result.Units; };
          Opt.axes.y.axisLabelFormatter = (x) => { var Result = oZm.Series['ACTIVE_ENERGY'].FormatValue(x); return Result.Value; };
        } else {
          Opt.axes.y.valueFormatter = (x) => {  var Result = oZm.Series['REACTIVE_ENERGY'].FormatValue(x); return Result.Value + ' ' + Result.Units; };
          Opt.axes.y.axisLabelFormatter = (x) => { var Result = oZm.Series['REACTIVE_ENERGY'].FormatValue(x); return Result.Value;  };
        }
        Opt.file = [];
        if(this.data != null) {
          var Divisor = (this.data.Aggregation_Source == '15M') ? 4 : 1;
          for(var n in this.data.Sampletime) Opt.file.push([this.data.Sampletime[n], this.data[(this.serie == 'ACTIVE_ENERGY') ? 'Active_Power' : 'Reactive_Power'][n] / Divisor]);
        } else {
          Opt.file.push([0, 0]);
        }

=======
        Opt.drawAxesAtZero = true;
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.plotter = Graph.Plotters.MultiBar;
        Opt.interactionModel = Graph.InteractionModels.Scrollable(this);
        Opt.drawHighlightPointCallback = Graph.Callbacks.DrawHighlightPoint_HighlightBar;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.labels = (this.serie == 'Active_Energy') ? ['Time', 'Active Energy'] : ['Time', 'Reactive Energy'];
        Opt.colors = [this.analyzer.Color[this.serie]];
        Opt.xAxisHeight = 24;
        Opt.selectMode = 'euclidian';
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatDateTime(x, '<br/>'); };
        Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatDateTime(x); };
        Opt.axes.y.valueFormatter = (x) => { return this.Round(this.ScaleUnits(x, this.BestUnits(x)), 2, false) + ((this.serie == 'Active_Energy') ? ' Wh' : 'VArh');  };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return (g.axes_[0].valueRange == null) ? '' : this.Round(this.ScaleUnits(x, this.BestUnits(Math.max(Math.abs(g.axes_[0].valueRange[0]), Math.abs(g.axes_[0].valueRange[1])))), 2, true); };
        if(this.data != null) {
          var Div = (this.data.Aggregation_Source == '1H') ? 1.0 : 4.0;
          for(var n in this.data.Sampletime) Opt.file.push([this.data.Sampletime[n], this.data[(this.serie == 'Active_Energy') ? 'Active_Power' : 'Reactive_Power'][n] / Div]);
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true], { incZero: 10 });
          Opt.dateWindow = [this.data.From, this.data.To];
        }
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
        return Opt;
      }
    },
    created: function() {
      this.Periods = [];
      this.Bills = [];
      this.ActiveData = [];
      this.ReactiveData = [];
      this.NowTimer = null;
<<<<<<< HEAD
    },
//    mounted: function() {
//      $(window).on('resize', this.ResizeHandler.bind(this));
//      window.dispatchEvent(new Event('resize'));
//      this.NowValue();
//    },
//    beforeDestroy: function() {
//      $(window).off('resize', this.ResizeHandler);
//      clearInterval(this.NowTimer);
//    },
    methods: {
//      SetAnalyzer: function() {
//        this.NowValue();
//      },
//      AggregationChanged: function(aggreg) {
//        this.$refs.period.SetAggregation(aggreg);
//      },
      ResizeHandler: function() {
        if(typeof this.$refs.graphActive !== 'undefined') this.$refs.graphActive.Redraw('100%', $(this.$refs.activeCounters.$el).height());
        if(typeof this.$refs.graphReactive !== 'undefined') this.$refs.graphReactive.Redraw('100%', $(this.$refs.reactiveCounters.$el).height());
      },
      MaximizeActiveClick: function() {
        $(window).unbind('resize', this.ResizeHandler);
        this.$refs.tools.Maximize('Active Energy', this.$refs.graphActive.Graph, this.$refs.ToolbarActive);
      },
      MaximizeReactiveClick: function() {
        $(window).unbind('resize', this.ResizeHandler);
        this.$refs.reactiveTools.Maximize('Fund. Reactive Power', this.$refs.graphReactive.Graph, this.$refs.ToolbarReactive);
      },
      MinimizeClick: function() {
        $(window).bind('resize', this.ResizeHandler.bind(this));
      },
      ExportClickActive: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS('Active_Energy.csv', [['Time'].concat(oZm.Series['ACTIVE_ENERGY'].Names(Analyzer))].concat(this.ActiveData));
      },
      ExportClickReactive: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS('Reactive_Energy.csv', [['Time'].concat(oZm.Series['REACTIVE_ENERGY'].Names(Analyzer))].concat(this.ReactiveData));
      },
      WindowRestore: function() {
        this.$refs.period.PopState();
      },
      WindowChange: function(minDate, maxDate) {
        this.$refs.period.SetCustom(minDate, maxDate, true);
      }
    },
    components: { Graph }
=======
    }
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
  }
</script>