<template>
  <div class="panel panel-active-power">
    <div class="panel-heading">
      <div class="huge header-text">Active power</div>
      <div class="header-buttons clearfix" ref="Toolbar">
        <toolbar-aggregation variable="AggregationEnergy" :options="['15M', '1H']" v-on:AggregationChanged="AggregationChanged" ref="aggregation"></toolbar-aggregation>
        <toolbar-period variable="Period" v-on:PeriodChanged="PeriodChange" ref="period"></toolbar-period>
        <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE', 'NOW']" v-on:NowStatus="SetAutoUpdate" v-on:ExportClick="$emit('ExportClick')" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
    <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <div class="col-md-6 col-lg-4 col-xl-3" ref="counters">
          <div class="subpanel clearfix">
            <div class="counters-title">Last value:</div>
            <div class="counters-aggreg">From 3s seconds aggregation<br>{{ Time3S }}</div>
            <div class="clearfix"></div>
            <div>
              <div class="current-value" v-html="CurrentValue3S"></div>
              <div class="counters-phases" v-html="CurrentValue3SPhases" v-if="CurrentValue3SPhases != ''"></div>
            </div>
          </div>
          <div class="subpanel clearfix">
            <div class="counters-title">Last value:</div>
            <div class="counters-aggreg">From 1 minute aggregation<br>{{ Time1M }}</div>
            <div class="clearfix"></div>
            <div>
              <div class="current-value" v-html="CurrentValue1M"></div>
              <div class="counters-phases" v-html="CurrentValue1MPhases" v-if="CurrentValue1MPhases != ''"></div>
            </div>
          </div>
          <div class="subpanel clearfix">
            <div class="counters-title">Period stats:</div>
            <div class="counters-aggreg">{{ CountersAggregation }}<br>From {{ TimeStatsFrom }} to {{ TimeStatsTo }}</div>
            <div class="line">
              <div class="float-left">
                <div class="counters-stat-type">Min value:</div>
                <div class="counters-stat-time">{{ MinTime }}</div>
              </div>
              <div class="counters-stat-val" v-html="MinValue"></div>
            </div>
            <div class="line line-border">
              <div class="float-left">
                <div class="counters-stat-type">Max value:</div>
                <div class="counters-stat-time">{{ MaxTime }}</div>
              </div>
              <div class="counters-stat-val" v-html="MaxValue"></div>
            </div>
          </div>
        </div>
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
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data () {
      return { CountersAggregation: 'Loading data...', MinTime: '', MinValue: '', MaxTime: '', MaxValue: '',
               Time3S: '', CurrentValue3S: '', CurrentValue3SPhases: '', 
               Time1M: '', CurrentValue1M: '', CurrentValue1MPhases: '', 
               TimeStatsFrom: '...', TimeStatsTo: '...',
               ShowSpinner: true };
    },
    created: function() {
      this.GraphOptions = {
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
    },
    mounted: function() {
      $(window).on('resize', this.ResizeHandler.bind(this));
    },
    beforeDestroy: function() {
      $(window).off('resize', this.ResizeHandler);
    },
    methods: {
      SetAutoUpdate: function(autoUpdate) {
        this.$emit('SetAutoUpdate', autoUpdate);
      },
      AggregationChanged: function(aggreg) {
        this.$emit('AggregationChanged', aggreg);
      },
      PeriodChange: function(time) {
        this.$emit('PeriodChange', time);
      },
      ResizeHandler: function() {
        if(typeof this.$refs.graph !== 'undefined') this.$refs.graph.Redraw('100%', $(this.$refs.counters).height());
      },
      MaximizeClick: function() {
        $(window).unbind('resize', this.ResizeHandler);
        this.$refs.tools.Maximize('Active Power', this.$refs.graph.Graph, this.$refs.Toolbar);
      },
      MinimizeClick: function() {
        $(window).bind('resize', this.ResizeHandler.bind(this));
      },
      FormatPhases: function(values) {
        var Return = '';
        if(values.length > 1) {
          for(var n = 0; n < values.length; n++) {
            var Format = oZm.Series['ACTIVE_POWER'].FormatValue(values[n]);
            Return += Format.Value + '<em>' + Format.Units +'</em><br>';
          }
        }
        return Return;
      },
      SumPhases: function(values) {
        var Sum = 0;
        for(var n = 0; n < values.length; n++) Sum += values[n];
        return Sum;
      },
      FormatAggregBlock(result) {
        if(result['Sampletime'].length === 0) return ['', 'No data', ''];
        var Pos = result['Sampletime'].length - 1;
        var Time = 'At ' + moment(result.Sampletime[Pos]).format('HH:mm:ss');
        var Value = 'No data';
        var Phases = '';
        if(result['Active_Power'][Pos] !== null) {
          Phases = this.FormatPhases(result['Active_Power'][Pos]);
          var Format = oZm.Series['ACTIVE_POWER'].FormatValue(this.SumPhases(result['Active_Power'][Pos]));
          Value = Format.Value + '<em>' + Format.Units +'</em>';
        }
        return [Time, Value, Phases];
      },
      NowValue: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['ACTIVE_POWER'], To: 0, Aggreg: '3S', Period: 6000, Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            [this.Time3S, this.CurrentValue3S, this.CurrentValue3SPhases] = this.FormatAggregBlock(result);
          }.bind(this),
          complete: function() {
            clearTimeout(this.NowTimer);
            this.NowTimer = setTimeout(this.NowValue.bind(this), 2000);
            this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
          }.bind(this), 
          error: function() {
            [this.Time3S, this.CurrentValue3S, this.CurrentValue3SPhases] = ['', 'No data', ''];
          }.bind(this)
        });
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['ACTIVE_POWER'], To: 0, Aggreg: '1M', Period: 120000, Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            [this.Time1M, this.CurrentValue1M, this.CurrentValue1MPhases] = this.FormatAggregBlock(result);
          }.bind(this),
          error: function() {
            [this.Time1M, this.CurrentValue1M, this.CurrentValue1MPhases] = ['', 'No data', ''];
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
        this.$emit('ZoomEvt', minDate, maxDate);
      }
    },
    components: { Spinner, ToolbarAggregation, ToolbarPeriod, ToolbarTools, Graph }
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

