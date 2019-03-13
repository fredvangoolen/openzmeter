<template>
  <div class="panel panel-frequency">
    <div class="panel-heading">
      <div class="huge header-text">Frequency</div>
      <div class="header-buttons clearfix" ref="Toolbar">
        <toolbar-aggregation variable="Aggregation" :options="['3S', '1M', '10M', '1H']" v-on:AggregationChanged="AggregationChanged" ref="aggregation"></toolbar-aggregation>
        <toolbar-period variable="Period" v-on:PeriodChanged="PeriodChange" ref="period"></toolbar-period>
        <toolbar-tools variable="ToFreq" :options="['EXPORT', 'MAXIMIZE', 'NOW']" v-on:NowStatus="SetAutoUpdate" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <div class="col-md-6 col-lg-4 col-xl-3" ref="counters">
          <div class="subpanel">
            <div class="counters-title">Current value:</div>
            <div class="counters-aggreg">From 3 seconds aggregation</div>
            <div class="clearfix"></div>
            <div class="current-value" v-html="CurrentValue"></div>
          </div>
          <div class="subpanel clearfix">
            <div class="counters-title">Period stats:</div>
            <div class="counters-aggreg">{{ CountersAggregation }}</div>
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
            <div class="line line-border">
              <div class="float-left">
                <div class="counters-stat-type">Average value:</div>
                <div class="counters-stat-time">From {{ AvgSamples }} samples</div>
              </div>
              <div class="counters-stat-val" v-html="AvgValue"></div>
            </div>
            <div class="line line-border">
              <div class="float-left">
                <div class="counters-stat-type">Std deviation:</div>
                <div class="counters-stat-time" v-html="StdPercent"></div>
              </div>
              <div class="counters-stat-val" v-html="StdValue"></div>
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
    data() {
      return { CountersAggregation: 'Loading data...', CurrentValue: '?', MinTime: '?', MaxTime: '?', MinValue: '?', MaxValue: '?', AvgValue: '?', AvgSamples: '?', StdPercent: '?', StdValue: '?', ShowSpinner: true };
    },
    created: function() {
      this.Data = [];
      this.LastSample = 0;
      this.Timer = null;
      this.NowTimer = null;
      this.Aggreg = null;
      this.Period = null;
      this.DateWindow = [0, 0];
      this.AutoUpdate = null;
      this.To = null;
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
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      $(window).on('resize', this.ResizeHandler.bind(this));
      window.dispatchEvent(new Event('resize'));
      this.UpdateGraph();
      this.NowValue();
    },
    beforeDestroy: function() {
      clearInterval(this.Timer);
       clearInterval(this.NowTimer);
      $(window).off('resize', this.ResizeHandler);
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
        if(typeof this.$refs.graph !== 'undefined') this.$refs.graph.Redraw('100%', $(this.$refs.counters).height());
      },
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
      NowValue: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['FREQUENCY'], Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            this.CurrentValue = ((result['Frequency'] === null) ? 'No data' : oZm.Round3(result['Frequency']) + '<em>Hz</em>');
          }.bind(this),
          complete: function() {
            clearTimeout(this.NowTimer);
            this.NowTimer = setTimeout(this.NowValue.bind(this), 2000);
          }.bind(this), 
          error: function() {
            this.CurrentValue = 'No data';
          }.bind(this)
        });
      },
      UpdateCounters: function(aggreg) {
        this.CountersAggregation = 'From ' + oZm.GetAggregLongName(aggreg) + ' aggregation';        
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
        window.dispatchEvent(new Event('resize'));
      },
      UpdateGraph: function() {
        if((this.$root.SelectedAnalyzer === null) || (this.Aggreg === null) || (this.Period === null) || (this.AutoUpdate === null) || (this.To === null)) return;
        if(this.LastSample === 0) {
          this.Data = [];
          this.ShowSpinner = true;
        }
        this.$refs.tools.BeginUpdate();
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Aggreg: this.Aggreg, Series: ['FREQUENCY'], To: this.To, LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.ID, Period: this.Period }), 
          success: function(result) {
            if(result['Sampletime'] === null) return;
            if(result['Sampletime'].length === 0) return;
            this.Data = oZm.GraphDataAppend(result, 'Frequency', this.Data);
            this.DateWindow = [result['From'], result['To']];
            this.To = this.$refs.tools.WriteTo(result['To']);
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
            this.$refs.graph.SetOptions( { dateWindow: this.DateWindow, file: this.Data, valueRange: oZm.Series['FREQUENCY'].Range(this.Data, [true]) } );
            this.UpdateCounters(result['Aggregation_Source']);
            this.ShowSpinner = false;
          }.bind(this),
          complete: function() {
            this.$refs.tools.EndUpdate();
            if(this.Timer !== null) clearTimeout(this.Timer);
            if(this.AutoUpdate === true) this.Timer = setTimeout(this.UpdateGraph.bind(this), 5000);
          }.bind(this),
          error: function(response) {
            if(response.status === 400) this.$refs.aggregation.AggregationFail();
            this.ShowSpinner = true;
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
        clearInterval(this.Timer);
        this.AutoUpdate = false;
        this.To = this.$refs.tools.WriteTo(maxDate, true);
        this.$refs.period.SetPeriod(maxDate - minDate);
        this.Period = maxDate - minDate;
        this.LastSample = 0;
        this.UpdateGraph();
      }
    },
    components: { Spinner, ToolbarAggregation, ToolbarPeriod, ToolbarTools, Graph }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
  .counters-title {
    float: left;
    text-align: left;
    font-size: 16px;
  }
  .counters-aggreg {
    float: right;
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
  }
  .line {
    clear: both;
  }
  .line-border {
    border-top: 1px solid #b3e0b3;
  }
</style>
