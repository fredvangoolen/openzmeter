<template>
  <div>
    <div class="panel panel-active-power">
      <div class="panel-heading">
        <div class="huge header-text">Active power</div>
        <div class="header-buttons clearfix" ref="ToolbarActive">
          <toolbar-aggregation variable="AggregationEnergy" :options="['15M', '1H']" v-on:AggregationChanged="AggregationChanged" ref="aggregation"></toolbar-aggregation>
          <toolbar-period variable="Period" v-on:PeriodChanged="PeriodChange" ref="period"></toolbar-period>
          <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE', 'NOW']" v-on:NowStatus="SetAutoUpdate" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeActiveClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools>
        </div>
      </div>
      <div class="panel-body">
        <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
        <div class="row">
          <counters class="col-md-6 col-lg-4 col-xl-3" ref="activeCounters"></counters>
          <div class="col-md-6 col-lg-8 col-xl-9">
            <graph ref="graphActive" :options="GraphOptionsActive"></graph>
          </div>
        </div>
      </div>
    </div>
    <div class="panel panel-reactive-power" id="ReactivePowerPanel">
      <div class="panel-heading" id="ReactivePowerPanel">
        <div class="header-text">Fund. Reactive Power</div>
        <div class="pull-right btn-group">
          <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="$emit('ExportClick')" v-on:MaximizeClick="MaximizeReactiveClick" v-on:MinimizeClick="MinimizeClick"></toolbar-tools>
        </div>
      </div>
      <div class="panel-body">
        <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
        <div class="row">
          <counters class="col-md-6 col-lg-4 col-xl-3" ref="reactiveCounters"></counters>
          <div class="col-md-6 col-lg-8 col-xl-9">
            <graph ref="graphReactive" :options="GraphOptionsReactive"></graph>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
  import ToolbarPeriod from 'Components/ToolbarPeriod.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import Counters from './Counters.vue';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data () {
      return { ShowSpinner: true };
    },
    created: function() {
      this.ActiveData = [];
      this.ReactiveDate = [];
      this.LastSample = 0;
      this.Timer = null;
      this.NowTimer = null;
      this.Aggreg = null;
      this.Period = null;
      this.DateWindow = [0, 0];
      this.AutoUpdate = null;
      this.To = null;
      this.GraphOptionsActive = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow,
        labels: ['Time', 'Frequency'], zoomCallback: this.ZoomEvt.bind(this), interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: this.MouseUpEvt.bind(this), dblclick: this.DoubleClickEvt.bind(this) }, colors: [oZm.Series['FREQUENCY'].Color], xAxisHeight: 24, 
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return oZm.Round3(x) + ' <em class="x-small">Hz</em>'; },
               axisLabelFormatter: function (x) { return oZm.Round3(x); }
             },
          x: { axisLabelFormatter: function(x) { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); },
               valueFormatter: function (x) { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); }
             }
        }
      };
      this.GraphOptionsReactive = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow,
        labels: ['Time', 'Frequency'], zoomCallback: this.ZoomEvt.bind(this), interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: this.MouseUpEvt.bind(this), dblclick: this.DoubleClickEvt.bind(this) }, colors: [oZm.Series['FREQUENCY'].Color], xAxisHeight: 24, 
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return oZm.Round3(x) + ' <em class="x-small">Hz</em>'; },
               axisLabelFormatter: function (x) { return oZm.Round3(x); }
             },
          x: { axisLabelFormatter: function(x) { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); },
               valueFormatter: function (x) { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); }
             }
        }
      };

      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      $(window).on('resize', this.ResizeHandler.bind(this));
       window.dispatchEvent(new Event('resize'));
      this.UpdateGraph();
      this.NowValue();
    },
    beforeDestroy: function() {
      $(window).off('resize', this.ResizeHandler);
      clearInterval(this.Timer);
      clearInterval(this.NowTimer);
    },
    methods: {
      SetAnalyzer: function() {
        this.LastSample = 0;
        this.UpdateGraph();
        this.NowValue();
      },
      SetAutoUpdate: function(autoUpdate, to) {
        this.AutoUpdate = autoUpdate;
        this.To = to;    
        clearInterval(this.Timer);
        this.LastSample = 0;
        if(autoUpdate === false) this.$refs.tools.WriteTo(this.DateWindow[1]);
        if((autoUpdate === true) || (this.Data === [])) this.UpdateGraph();
      },
      AggregationChanged: function(aggreg) {
        this.Aggreg = aggreg;
        this.LastSample = 0;
        this.UpdateGraph();
      },
      PeriodChange: function(time) {
        this.Period = time;
        this.LastSample = 0;
        this.UpdateGraph();
      },
      ResizeHandler: function() {
        if(typeof this.$refs.graphActive !== 'undefined') this.$refs.graphActive.Redraw('100%', $(this.$refs.countersActive).height());
        if(typeof this.$refs.graphReactive !== 'undefined') this.$refs.graphReactive.Redraw('100%', $(this.$refs.countersReative).height());
      },
      MaximizeActiveClick: function() {
        $(window).unbind('resize', this.ResizeHandler);
        this.$refs.tools.Maximize('Active Power', this.$refs.graphActive.Graph, this.$refs.ToolbarActive);
      },
      MaximizeReactiveClick: function() {
        $(window).unbind('resize', this.ResizeHandler);
        this.$refs.tools.Maximize('Fund. Reactive Power', this.$refs.graphReactive.Graph, this.$refs.ToolbarReactive);
      },
      MinimizeClick: function() {
        $(window).bind('resize', this.ResizeHandler.bind(this));
      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        this.$refs.tools.SaveCVS('Frequency.csv', [['Time', 'Frequency']].concat(this.Data));
      },
      NowValue: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['ACTIVE_POWER'], To: 0, Aggreg: '3S', Period: 6000, Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            this.$refs.activeCounters.Set3SValues(result);
            this.$refs.reactiveCounters.Set3SValues(result);
          }.bind(this),
          complete: function() {
            clearTimeout(this.NowTimer);
            this.NowTimer = setTimeout(this.NowValue.bind(this), 2000);
            this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
          }.bind(this), 
          error: function() {
            this.$refs.activeCounters.Set3SValues(null);
            this.$refs.reactiveCcounters.Set3SValues(null);
          }.bind(this)
        });
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['ACTIVE_POWER'], To: 0, Aggreg: '1M', Period: 120000, Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            this.$refs.activeCounters.Set1MValues(result);
            this.$refs.reactiveCounters.Set1MValues(result);
          }.bind(this),
          error: function() {
            this.$refs.activeCounters.Set1MValues(null);
            this.$refs.reactiveCounters.Set1MValues(null);            
          }.bind(this)
        });
      },
      UpdateGraph: function() {
        if((this.$root.SelectedAnalyzer === null) || (this.Aggreg === null) || (this.Period === null) || (this.AutoUpdate === null) || (this.To === null)) return;
        if(this.LastSample === 0) {
          this.ActiveData = [];
          this.ReactiveDate = [];
          this.ShowSpinner = true;
        }
        this.$refs.tools.BeginUpdate();
        $.ajax({ url: 'getPrices', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Aggreg: this.Aggreg, To: this.To, LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.ID, Period: this.Period }), 
          success: function(result) {
            if(result['Sampletime'] === null) return;
            if(result['Sampletime'].length === 0) return;
//            this.ActiveData = oZm.GraphDataAppend(result, 'Active_Power', this.ActiveData);
//            this.ActiveData = oZm.GraphDataAppend(result, 'Active_Power', this.ActiveData);
            this.DateWindow = [result['From'], result['To']];
            this.To = this.$refs.tools.WriteTo(result['To']);
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
//            this.$refs.graph.SetOptions( { dateWindow: this.DateWindow, file: this.Data, valueRange: oZm.Series['FREQUENCY'].Range(this.Data, [true]) } );
//            this.UpdateCounters(result['Aggregation_Source']);
  //          this.ShowSpinner = false;
          }.bind(this),
          complete: function() {
            this.$refs.tools.EndUpdate();
            if(this.Timer !== null) clearTimeout(this.Timer);
            if(this.AutoUpdate === true) this.Timer = setTimeout(this.UpdateGraph.bind(this), 5000);
          }.bind(this),
          error: function(response) {
            if(response.status === 400) this.$refs.aggregation.AggregationFail();
//            this.ShowSpinner = true;
          }.bind(this)
        });
      },

      MouseUpEvt: function(event, g, contex) {
        oZm.mouseUp(event, g, contex);
        if((contex.isPanning) && (contex.regionWidth > 0)) this.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
      },
      DoubleClickEvt: function(event, g, context) {
        oZm.dblClick(event, g, context);
        this.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
      },
      ZoomEvt: function(minDate, maxDate, yRanges) {
        this.AutoUpdate = false;
        this.To = this.$refs.tools.WriteTo(maxDate, true);
        this.$refs.period.SetPeriod(maxDate - minDate);
        this.Period = maxDate - minDate;
        this.LastSample = 0;
        this.UpdateGraph();
      }
    },
    components: { Spinner, ToolbarAggregation, ToolbarPeriod, ToolbarTools, Graph, Counters }
  }
</script>

<style>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
  .counters-phases {
    text-align: left;
    display: inline-block;
    font-size: 12px;
    line-height: 12px;
    font-weight: bold;
    padding-left: 2px;
    border-left: 1px solid #f0ad4e;
    margin-left: 5px;
  }
  .counters-title {
    float: left;
    text-align: left;
    font-size: 16px;
  }
  .counters-aggreg {
    float: right;
    text-align: right;
    font-size: 12px;
  }
  .counters-main-val {
    font-size: 36px
  }
  .counters-stat-type {
    text-align: left;
    font-weight: bold;
  }
  .counters-stat-time {
    font-size: 12px;
    text-align: left;
  }
  .counters-stat-val {
    font-size: 28px;
    line-height: 37px;
    float: right;
  }
  .current-value {
    font-size: 36px;
    display: inline-block;
  }
  .line {
    clear: both;
  }
  .line-border {
    border-top: 1px solid #FAE7CC;
  }
</style>

