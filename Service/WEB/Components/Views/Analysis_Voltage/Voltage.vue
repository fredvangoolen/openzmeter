<template>
  <div class="panel panel-voltage">
    <div class="panel-heading">
      <div class="header-text">Voltage</div>
      <div class="header-buttons clearfix" ref="Toolbar">
        <toolbar-aggregation variable="Aggregation" :options="['3S', '1M', '10M', '1H']" v-on:AggregationChanged="AggregationChanged" ref="aggregation"></toolbar-aggregation>
        <toolbar-period variable="Period" v-on:PeriodChanged="UpdateGraph" ref="period"></toolbar-period>
        <toolbar-phases variable="VoltageViewPhases" type="VOLTAGE" :multi="true" v-on:PhasesChanged="PhasesChanged"></toolbar-phases>
        <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <counters class="col-md-6 col-lg-4 col-xl-3" ref="counters"></counters>
        <div class="col-md-6 col-lg-8 col-xl-9">
          <graph ref="graph" :options="GraphOptions"></graph>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
  import ToolbarPeriod from 'Components/ToolbarPeriod.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import Counters from './Counters.vue';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    data() {
      return { ShowSpinner: true };
    },
    created: function() {
      this.Data = [];
      this.GraphOptions = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow,
        zoomCallback: this.ZoomEvt.bind(this), interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: this.MouseUpEvt.bind(this), dblclick: this.DoubleClickEvt.bind(this) }, xAxisHeight: 24, 
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Result = oZm.Series['VOLTAGE'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
               axisLabelFormatter: function (x) { var Result = oZm.Series['VOLTAGE'].FormatValue(x); return Result.Value; }
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
      UpdateGraph(to, period, lastsample, aggreg) {
        if(this.LastSample === 0) {
          this.Data = [];
          this.ShowSpinner = true;
        }
        var Analyzer = this.$root.SelectedAnalyzer;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Aggreg: aggreg, Series: ['VOLTAGE'], To: to, LastSample: lastsample, Analyzer: Analyzer.ID, Period: period }), 
          success: function(result) {
            if((result['Sampletime'] === null) || (result['Sampletime'].length === 0)) return;
            this.Data = oZm.GraphDataAppend(result, 'Voltage', this.Data);
            lastsample = oZm.LastSampletime(result, lastsample);
            this.$refs.graph.SetOptions({ dateWindow: [result.From, result.To], file: this.Data, valueRange: oZm.Series['VOLTAGE'].Range(this.Data, this.Phases), 
                                          labels: ['Time'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)), colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length) } );
            this.$refs.counters.SetStats([result.From, result.To], this.Data, result["Aggregation_Source"]);
          }.bind(this),
          complete: function() {
            this.ShowSpinner = false;
            this.$refs.period.UpdateCompleted(lastsample);
          }.bind(this),
          error: function(response) {
            if(response.status === 400) this.$refs.aggregation.AggregationFail();
            this.ShowSpinner = true;
          }.bind(this)
        });
      },
      MouseUpEvt(event, g, contex) {
        oZm.mouseUp(event, g, contex);
        if((contex.isPanning) && (contex.regionWidth > 0)) this.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
      },
      DoubleClickEvt(event, g, context) {
        oZm.dblClick(event, g, context);
        this.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
      },
      ZoomEvt(minDate, maxDate, yRanges) {
        this.$refs.tools.WriteTo(maxDate, true);
        this.$refs.period.SetPeriod(maxDate - minDate);
        this.DateWindow = [minDate, maxDate];
        this.LastSample = 0;
//        this.UpdateGraph();
      }
    },
    components: { Spinner, ToolbarAggregation, ToolbarPeriod, ToolbarPhases, ToolbarTools, Counters, Graph }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>