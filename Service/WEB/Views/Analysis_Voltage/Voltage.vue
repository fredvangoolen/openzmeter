<template>
  <div>
    <graph :options="GraphOptions"></graph>
  </div>
</template>

<script>
  import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
  import ToolbarPeriod from 'Components/ToolbarPeriod.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { analyzer: {type: Object, required: true }, data: { type: Object, required: true }, phases: { type: Array, required: true } },
    data() {
      return { };
    },
    created: function() {
      this.Data = [];
      this.GraphOptions = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow,
        interactionModel: Graph.ScrollableInteractionModel(this), xAxisHeight: 24, selectMode: 'euclidian',
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Result = oZm.FormatSerieValue(x, oZm.Series['VOLTAGE']); return Result.ValueHTML + ' <em class="x-small">' + Result.Units + '</em>'; },
               axisLabelFormatter: function (x) { return oZm.FormatSerieValue(x, oZm.Series['VOLTAGE']).ValueHTML; }
             },
          x: { axisLabelFormatter: function(x) { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); },
               valueFormatter: function (x) { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); }
             }
        }
      };
    },
    mounted: function() {
      $(window).on('resize', this.ResizeHandler.bind(this));
      window.dispatchEvent(new Event('resize'));
    },
    beforeDestroy: function() {
      $(window).off('resize', this.ResizeHandler);
    },
    methods: {
      AggregationChanged(aggreg) {
        this.$refs.period.SetAggregation(aggreg);
      },
      ResizeHandler: function() {
        if(typeof this.$refs.graph !== 'undefined') this.$refs.graph.Redraw('100%', $(this.$refs.counters.$el).height());
      },
      MaximizeClick: function() {
        $(window).unbind('resize', this.ResizeHandler);
        this.$refs.tools.Maximize('Voltage', this.$refs.graph.Graph, this.$refs.Toolbar);
      },
      MinimizeClick: function() {
        $(window).bind('resize', this.ResizeHandler.bind(this));
      },
      PhasesChanged(phases) {
        this.Phases = phases;
        this.$refs.graph.SetOptions( { visibility: this.Phases } );
      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS('Voltage.csv', [['Time'].concat(oZm.Series['VOLTAGE'].Names(Analyzer))].concat(this.Data));
      },
      UpdateGraph: function(to, period, lastsample, aggreg) {
        if(lastsample === 0) {
          this.Data = [];
          this.ShowSpinner = true;
        }
        this.$parent.$refs.details.$refs.events.GetEvents(to, period, lastsample);
        var Analyzer = this.$root.SelectedAnalyzer;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Aggreg: aggreg, Series: ['VOLTAGE'], To: to, LastSample: lastsample, Analyzer: Analyzer.Analyzer, Period: period }),
          success: function(result) {
            if((result['Sampletime'] === null) || (result['Sampletime'].length === 0)) return;
            this.Data = oZm.GraphDataAppend(result, 'Voltage', this.Data);
            lastsample = oZm.LastSampletime(result, lastsample);
            this.$refs.graph.SetOptions({ dateWindow: [result.From, result.To], file: this.Data, valueRange: oZm.Series['VOLTAGE'].Range(this.Data, this.Phases),
                                          labels: ['Time'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)), colors: oZm.SerieStyle(oZm.Series['VOLTAGE'], Analyzer, 0).Colors } );
            this.$refs.counters.SetStats([result.From, result.To], this.Data, result["Aggregation_Source"]);
          }.bind(this),
          complete: function() {
            this.ShowSpinner = false;
            this.$refs.period.Updating = false;
          }.bind(this),
          error: function(response) {
            if(response.status === 400) this.$refs.aggregation.AggregationFail();
            this.ShowSpinner = true;
          }.bind(this)
        });
      },
      WindowRestore: function() {
        this.$refs.period.PopState();
      },
      WindowChange: function(minDate, maxDate) {
        this.$refs.period.SetCustom(minDate, maxDate, true);
      }
    },
    components: { Spinner, ToolbarAggregation, ToolbarPeriod, ToolbarPhases, ToolbarTools, Graph }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>