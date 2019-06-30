<template>
  <div>
    <div class="panel panel-active-energy">
      <div class="panel-heading">
        <div class="huge header-text">Active Energy</div>
        <div class="header-buttons clearfix" ref="ToolbarActive">
          <toolbar-aggregation variable="AggregationEnergy" :options="['15M', '1H']" v-on:AggregationChanged="AggregationChanged" ref="aggregation"></toolbar-aggregation>
          <toolbar-period variable="Period" v-on:PeriodChanged="UpdateGraph" ref="period"></toolbar-period>
          <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClickActive" v-on:MaximizeClick="MaximizeActiveClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools>
        </div>
      </div>
      <div class="panel-body">
        <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
        <div class="row">
          <counters class="col-md-6 col-lg-4 col-xl-3" ref="activeCounters" variable="ACTIVE_ENERGY"></counters>
          <div class="col-md-6 col-lg-8 col-xl-9">
            <graph ref="graphActive" :options="GraphOptionsActive"></graph>
          </div>
        </div>
      </div>
    </div>
    <div class="panel panel-reactive-energy">
      <div class="panel-heading">
        <div class="header-text">Fund. Reactive Energy</div>
        <div class="pull-right btn-group" ref="ToolbarReactive">
          <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClickReactive" v-on:MaximizeClick="MaximizeReactiveClick" v-on:MinimizeClick="MinimizeClick" ref="reactiveTools"></toolbar-tools>
        </div>
      </div>
      <div class="panel-body">  
        <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
        <div class="row">
          <counters class="col-md-6 col-lg-4 col-xl-3" ref="reactiveCounters" variable="REACTIVE_ENERGY"></counters>
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
    data() {
      return { ShowSpinner: true };
    },
    created: function() {
      this.Periods = [];
      this.Bills = [];
      this.ActiveData = [];
      this.ReactiveData = [];
      this.NowTimer = null;
      this.GraphOptionsActive = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow, plotter: Graph.multiColumnBarPlotter,
        labels: ['Time', 'Active Energy'], zoomCallback: this.ZoomEvt, interactionModel: { mousedown: Graph.mouseDown, mousemove: Graph.mouseMove, mouseup: this.MouseUpEvt, dblclick: this.DoubleClickEvt }, colors: [oZm.Series['ACTIVE_ENERGY'].Color], xAxisHeight: 24, 
        drawHighlightPointCallback: Graph.drawHighlightPointCallback,
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Result = oZm.Series['ACTIVE_ENERGY'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
               axisLabelFormatter: function (x) { var Result = oZm.Series['ACTIVE_ENERGY'].FormatValue(x); return Result.Value; }
             },
          x: { axisLabelFormatter: function(x) { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); },
               valueFormatter: function (x) { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); }
             }
        }
      };
      this.GraphOptionsReactive = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow, plotter: Graph.multiColumnBarPlotter,
        labels: ['Time', 'Reactive Energy'], zoomCallback: this.ZoomEvt.bind(this), interactionModel: { mousedown: Graph.mouseDown, mousemove: Graph.mouseMove, mouseup: this.MouseUpEvt.bind(this), dblclick: this.DoubleClickEvt.bind(this) }, colors: [oZm.Series['REACTIVE_ENERGY'].Color], xAxisHeight: 24, 
        drawHighlightPointCallback: Graph.drawHighlightPointCallback,
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) {  var Result = oZm.Series['REACTIVE_ENERGY'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
               axisLabelFormatter: function (x) { var Result = oZm.Series['REACTIVE_ENERGY'].FormatValue(x); return Result.Value;  }
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
      this.NowValue();
    },
    beforeDestroy: function() {
      $(window).off('resize', this.ResizeHandler);
      clearInterval(this.NowTimer);
    },
    methods: {
      SetAnalyzer: function() {
        this.NowValue();
      },
      AggregationChanged: function(aggreg) {
        this.$refs.period.SetAggregation(aggreg);
      },
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
      NowValue: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['ACTIVE_POWER', 'REACTIVE_POWER'], To: 0, Aggreg: '15M', Period: 1800000, Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            this.$refs.activeCounters.Set15MValues(result);
            this.$refs.reactiveCounters.Set15MValues(result);
          }.bind(this),
          complete: function() {
            clearTimeout(this.NowTimer);
            this.NowTimer = setTimeout(this.NowValue.bind(this), 2000);
            this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
          }.bind(this), 
          error: function() {
            this.$refs.activeCounters.Set15MValues(null);
            this.$refs.reactiveCcounters.Set15MValues(null);
          }.bind(this)
        });
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['ACTIVE_POWER', 'REACTIVE_POWER'], To: 0, Aggreg: '1H', Period: 7200000, Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            this.$refs.activeCounters.Set1HValues(result);
            this.$refs.reactiveCounters.Set1HValues(result);
          }.bind(this),
          error: function() {
            this.$refs.activeCounters.Set1HValues(null);
            this.$refs.reactiveCounters.Set1HValues(null);            
          }.bind(this)
        });
      },
      UpdateGraph: function(to, period, lastsample, aggreg) {
        if(lastsample === 0) {
          this.ActiveData = [];
          this.ReactiveDate = [];
          this.ShowSpinner = true;
        }
        var Analyzer = this.$root.SelectedAnalyzer;
        $.ajax({ url: 'getPrices', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Aggreg: aggreg, To: to, LastSample: lastsample, Analyzer: Analyzer.ID, Period: period }), 
          success: function(result) {
            if((result['Sampletime'] === null) || (result['Sampletime'].length === 0)) return;
            if(result['Aggregation_Source'] === '15M') {
              for(var n = 0; n < result['Active_Power'].length; n++) result['Active_Power'][n] = result['Active_Power'][n] / 4.0;
              for(var n = 0; n < result['Reactive_Power'].length; n++) result['Reactive_Power'][n] = result['Reactive_Power'][n] / 4.0;
            }
            this.ActiveData = oZm.GraphDataAppend(result, 'Active_Power', this.ActiveData);
            this.ReactiveData = oZm.GraphDataAppend(result, 'Reactive_Power', this.ReactiveData);
            lastsample = oZm.LastSampletime(result, lastsample);
            this.Bills = result["Bills"];
            //Group periods
            var LastItem = (this.Periods.length > 0) ? this.Periods.slice(-1)[0] : null;
            for(var i = 0; i < result.Sampletime.length; i++) {
              var Period = { Name: result.Period_Name[i], From: result.Sampletime[i] - ((this.Aggreg === '15M') ? 899999 : 3599999), To: result.Sampletime[i], Color: result.Period_Color[i] };
              if(LastItem === null) {
                this.Periods.push(Period);
                LastItem = Period;
              } else if(Period.From === (LastItem.To + 1)) {
                if(Period.Name === LastItem.Name) {
                  LastItem.To = Period.To;
                  this.Periods.pop();
                  this.Periods.push(LastItem);
                } else {
                  this.Periods.push(Period);
                  LastItem = Period;
                }
              }
              this.Periods = this.Periods.filter(period => (period.To > result.from) || (period.From < result.to) ).sort(function(a, b) { if(a.To < b.To) return -1; if(a.To > b.To) return 1; return 0; });
            }
            this.$refs.graphActive.SetOptions( { dateWindow: [result.From, result.To], file: this.ActiveData, valueRange: oZm.Series['ACTIVE_ENERGY'].Range(this.ActiveData, [true]) } );
            this.$refs.graphReactive.SetOptions( { dateWindow: [result.From, result.To], file: this.ReactiveData, valueRange: oZm.Series['REACTIVE_ENERGY'].Range(this.ReactiveData, [true]) } );
            this.$refs.activeCounters.SetStats([result.From, result.To], this.ActiveData, result["Aggregation_Source"]);
            this.$refs.reactiveCounters.SetStats([result.From, result.To], this.ReactiveData, result["Aggregation_Source"]);
          }.bind(this),
          complete: function() {
            this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
            this.ShowSpinner = false;
            this.$refs.period.UpdateCompleted(lastsample);
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
        this.$refs.period.SetCustom(minDate, maxDate);
      }
    },
    components: { Spinner, ToolbarAggregation, ToolbarPeriod, ToolbarTools, Graph, Counters }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }

</style>