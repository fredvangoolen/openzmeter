<template>
  <div>
    <div class="panel panel-active-power">
      <div class="panel-heading">
        <div class="huge header-text">Active Power</div>
        <div class="header-buttons clearfix" ref="ToolbarActive">
          <toolbar-aggregation variable="AggregationEnergy" :options="['15M', '1H']" v-on:AggregationChanged="AggregationChanged" ref="aggregation"></toolbar-aggregation>
          <toolbar-period variable="Period" v-on:PeriodChanged="UpdateGraph" ref="period"></toolbar-period>
          <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClickActive" v-on:MaximizeClick="MaximizeActiveClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools>
        </div>
      </div>
      <div class="panel-body">
        <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
        <div class="row">
          <counters class="col-md-6 col-lg-4 col-xl-3" ref="activeCounters" variable="ACTIVE_POWER"></counters>
          <div class="col-md-6 col-lg-8 col-xl-9">
            <graph ref="graphActive" :options="GraphOptionsActive"></graph>
          </div>
        </div>
      </div>
    </div>
    <div class="panel panel-reactive-power" id="ReactivePowerPanel">
      <div class="panel-heading" id="ReactivePowerPanel">
        <div class="header-text">Fund. Reactive Power</div>
        <div class="header-buttons clearfix" ref="ToolbarReactive">
          <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClickReactive" v-on:MaximizeClick="MaximizeReactiveClick" v-on:MinimizeClick="MinimizeClick" ref="reactiveTools"></toolbar-tools>
        </div>
      </div>
      <div class="panel-body">
        <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
        <div class="row">
          <counters class="col-md-6 col-lg-4 col-xl-3" ref="reactiveCounters" variable="REACTIVE_POWER"></counters>
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
      this.Periods = [];
      this.Bills = [];
      this.ActiveData = [];
      this.ReactiveData = [];
      this.NowTimer = null;
      this.GraphOptionsActive = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow, underlayCallback: this.DrawPeriods,
        labels: ['Time', 'Active Power'], interactionModel: Graph.ScrollableInteractionModel(this), colors: [oZm.Series['ACTIVE_POWER'].Color], xAxisHeight: 24, selectMode: 'euclidian',
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Result = oZm.Series['ACTIVE_POWER'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
               axisLabelFormatter: function (x) { var Result = oZm.Series['ACTIVE_POWER'].FormatValue(x); return Result.Value; }
             },
          x: { axisLabelFormatter: function(x) { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); },
               valueFormatter: function (x) { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); }
             }
        }
      };
      this.GraphOptionsReactive = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow,
        labels: ['Time', 'Reactive Power'], interactionModel: Graph.ScrollableInteractionModel(this), colors: [oZm.Series['REACTIVE_POWER'].Color], xAxisHeight: 24, selectMode: 'euclidian', 
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) {  var Result = oZm.Series['REACTIVE_POWER'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
               axisLabelFormatter: function (x) { var Result = oZm.Series['REACTIVE_POWER'].FormatValue(x); return Result.Value;  }
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
      DrawPeriods: function(canvas, area, g) {
        canvas.strokeStyle = '#777';
        canvas.lineWidth = 2;
        canvas.beginPath();
        canvas.moveTo(area.x, 0);
        canvas.lineTo(area.x, 13);
        canvas.stroke();  
        for(var i = 0; i < this.Periods.length; i++) {
          var Left = g.toDomXCoord(this.Periods[i].From);
          var Right = g.toDomXCoord(this.Periods[i].To);
          if(Left < (area.x + 1)) Left = (area.x + 1);
          canvas.fillStyle = (this.Periods[i].Color === null) ? "#EEEEEE" : this.Periods[i].Color;
          canvas.fillRect(Left, area.y, Right, 13);
          if(((Right - Left) > 20) && (this.Periods[i].Name !== null)) {
            canvas.fillStyle = '#000000';
            canvas.font = "10px monospace";
            canvas.fillText(this.Periods[i].Name, Left + 1, 10);
          }
          var Y = g.toDomYCoord(this.Periods[i].Power);
          canvas.strokeStyle = '#d9534f';
          canvas.lineWidth = 2;
          canvas.beginPath();
          if(i > 0) {
            canvas.moveTo(Left, g.toDomYCoord(this.Periods[i -1 ].Power));
            canvas.lineTo(Left, Y);
          } else {
            canvas.moveTo(Left, Y);
          }
          canvas.lineTo(Right, Y);
          canvas.stroke();
        }
        for(var i = 0; i < this.Bills.length; i++) {
          canvas.strokeStyle = '#d9534f';
          canvas.lineWidth = 1;
          canvas.beginPath();
          var X = g.toDomXCoord(this.Bills[i].Sampletime);
          canvas.moveTo(X, 13);
          canvas.lineTo(X, area.h);
          canvas.stroke();
        }
        canvas.rect(area.x, area.y + 13, area.w, area.h);
        canvas.clip();
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
        this.$refs.tools.Maximize('Active Power', this.$refs.graphActive.Graph, this.$refs.ToolbarActive);
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
        this.$refs.tools.SaveCVS('Active_Power.csv', [['Time'].concat(oZm.Series['ACTIVE_POWER'].Names(Analyzer))].concat(this.ActiveData));
      },
      ExportClickReactive: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS('Reactive_Power.csv', [['Time'].concat(oZm.Series['REACTIVE_POWER'].Names(Analyzer))].concat(this.ReactiveData));
      },
      NowValue: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['ACTIVE_POWER', 'REACTIVE_POWER'], To: 0, Aggreg: '3S', Period: 6000, Analyzer: this.$root.SelectedAnalyzer.ID }), 
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
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['ACTIVE_POWER', 'REACTIVE_POWER'], To: 0, Aggreg: '1M', Period: 120000, Analyzer: this.$root.SelectedAnalyzer.ID }), 
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
              this.Periods = this.Periods.filter(period => (period.To > result.From) || (period.From < result.To) ).sort(function(a, b) { if(a.To < b.To) return -1; if(a.To > b.To) return 1; return 0; });
            }
            this.$refs.graphActive.SetOptions( { dateWindow: [result.From, result.To], file: this.ActiveData, valueRange: oZm.Series['ACTIVE_POWER'].Range(this.ActiveData, [true]) } );
            this.$refs.graphReactive.SetOptions( { dateWindow: [result.From, result.To], file: this.ReactiveData, valueRange: oZm.Series['REACTIVE_POWER'].Range(this.ReactiveData, [true]) } );
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
      WindowRestore: function() {
        this.$refs.period.PopState();
      },
      WindowChange: function(minDate, maxDate) {
        this.$refs.period.SetCustom(minDate, maxDate, true);
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

