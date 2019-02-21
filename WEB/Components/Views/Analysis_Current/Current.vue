<template>
  <div class="panel panel-current">
    <div class="panel-heading">
      <div class="huge header-text">Current</div>
      <div class="header-buttons clearfix"  id="toolbarCurrent">
        <toolbar-aggregation variable="Aggregation" :options="['3S', '1M', '10M', '1H']" v-on:AggregationChanged="AggregationChanged" ref="aggregation"></toolbar-aggregation>
        <toolbar-period variable="Period" v-on:PeriodChanged="PeriodChange" ref="period"></toolbar-period>
        <toolbar-phases variable="VoltageViewPhases" type="VOLTAGE" :multi="true" v-on:PhasesChanged="PhasesChanged"></toolbar-phases>
        <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE', 'NOW']" v-on:NowStatus="SetAutoUpdate" ref="tools"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <div class="col-md-6 col-lg-4 col-xl-3">
          <div class="subpanel">
            <div class="huge-small" id="labelCurrentAvgPrimary">Loading data...</div>
            <div class="huge-small" id="labelCurrentAvgPrimaryValue">?</div>
            <div><span id="labelCurrentAvgSecondary">Loading data...</span>&nbsp;<span id="labelCurrentAvgSecondaryValue"></span></div>
          </div>
          <div class="subpanel">
            <div class="huge-small" id="labelCurrentMaxPrimary">Loading data...</div>
            <div class="huge-small" id="labelCurrentMaxPrimaryValue">?</div>
            <div><span id="labelCurrentMaxSecondary">Loading data...</span>&nbsp;<span id="labelCurrentMaxSecondaryValue"></span></div>
          </div>
          <div class="subpanel">
            <div class="huge-small" id="labelCurrentMinPrimary">Loading data...</div>
            <div class="huge-small" id="labelCurrentMinPrimaryValue">?</div>
            <div><span id="labelCurrentMinSecondary">Loading data...</span>&nbsp;<span id="labelCurrentMinSecondaryValue"></span></div>
          </div>
        </div>
        <div class="col-md-6 col-lg-8 col-xl-9">
          <div class="chart" ref="graph"></div>
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
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'dygraphs';
  export default {
    data() {
      return { Graph: null, Data: [], ShowSpinner: true, LastSample: 0, Timer: null, Phases: [], Aggreg: null, Period: null, DateWindow: [0, 0], AutoUpdate: null };
    },
    created: function() {
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      this.Graph = new Dygraph(this.$refs.graph, [[0, 0]], {});
      this.UpdateGraph();
      this.Graph.resize('100%', $(this.$refs.indicators).height());
    },
    methods: {
      SetAnalyzer() {
        this.LastSample = 0;
        this.UpdateGraph();
      },
      SetAutoUpdate(autoUpdate) {
        this.AutoUpdate = autoUpdate;
        clearInterval(this.Timer);
        this.LastSample = 0;
        if(autoUpdate === false) this.$refs.tools.WriteTo(this.DateWindow[1]);
        if((autoUpdate === true) || (this.Data === [])) this.UpdateGraph();
      },
      PhasesChanged(phases) {
        this.Phases = phases;
        this.CalcOptions();
      },
      AggregationChanged(aggreg) {
        this.Aggreg = aggreg;
        this.LastSample = 0;
        this.UpdateGraph();
      },
      PeriodChange(time) {
        this.Period = time;
        this.LastSample = 0;
        this.UpdateGraph();
      },
      CalcOptions() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Range = oZm.Series['CURRENT'].Range(this.Data, this.Phases);
        var Options = {
          strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow',
          legendFormatter: oZm.LegendFormatter_Follow, labels: ['Time'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)), dateWindow: this.DateWindow, file: this.Data, visibility: this.Phases,
          zoomCallback: this.ZoomEvt, interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: this.MouseUpEvt, dblclick: this.DoubleClickEvt },
//        underlayCallback: function(context, area, dygraph) { GraphHightlightWeekends("#EEE", context, area, dygraph); }, 
          colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length), xAxisHeight: 24, valueRange: Range,
          axes: {
            y: { pixelsPerLabel: 15, axisLabelWidth: 34,
                 valueFormatter: function (x) { var Result = oZm.Series['VOLTAGE'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
                 axisLabelFormatter: function (x) { return oZm.Round2(x); }
               },
            x: { axisLabelFormatter: function(x) { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); },
                 valueFormatter: function (x) { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); }
               }
          }
        };
        this.Graph.updateOptions(Options);
        window.dispatchEvent(new Event('resize'));
      },
      UpdateGraph() {
        if((this.$root.SelectedAnalyzer === null) || (this.Aggreg === null) || (this.Period === null) || (this.AutoUpdate === null)) return;
        if(this.LastSample === 0) this.Data = [];
        this.$refs.tools.BeginUpdate();
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Aggreg: this.Aggreg, Series: ['CURRENT'], From: 0, LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.ID, Period: this.Period }), 
          success: function(result) {
            this.ShowSpinner = false;
            if(result['Sampletime'] === null) return;
            if(result['Sampletime'].length === 0) return;
            this.Data = oZm.GraphDataAppend(result, 'Current', this.Data);
            this.DateWindow = [result['From'], result['To']];
            this.$refs.tools.WriteTo(result['To']);
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
            this.CalcOptions();
          }.bind(this),
          complete: function() {
            this.$refs.tools.EndUpdate();
            if(this.Timer !== null) clearTimeout(this.Timer);
            if(this.AutoUpdate === true) this.Timer = setTimeout(this.UpdateGraph, 5000);
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
        clearInterval(this.Timer);
        this.$refs.tools.WriteTo(maxDate, true);
        this.$refs.period.SetPeriod(maxDate - minDate);
        this.DateWindow = [minDate, maxDate];
        this.LastSample = 0;
        this.UpdateGraph();
      }
    },
    components: { Spinner, ToolbarAggregation, ToolbarPeriod, ToolbarPhases, ToolbarTools }
  }
</script>