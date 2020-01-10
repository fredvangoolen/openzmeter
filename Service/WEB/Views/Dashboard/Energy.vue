<template>
  <div>
    <graph :options="GraphOptions" :height="height"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
//  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { analyzer: Object, height: { type: Number } },
    data() {
      return { ShowSpinner: true, CountersAggregation: 'Loading data...' };
    },
    computed: {
      GraphOptions: function() {
        var Opt = {};
        Opt.strokeWidth = 0.5;
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = true;
        Opt.legend = 'follow';
        Opt.plotter = Graph.multiColumnBarPlotter;
        Opt.labels = ['Time', 'Energy'];
        Opt.interactionModel = Graph.ScrollableInteractionModel(this);
        Opt.colors = [oZm.Series['ACTIVE_ENERGY'].Color];
        Opt.xAxisHeight = 24;
        Opt.legendFormatter = Graph.LegendFormatter_Follow;
        //underlayCallback: this.DrawPeriods,
        Opt.drawHighlightPointCallback = Graph.DrawHighlightBarChartCallback;
        Opt.axes = { y: {}, x: {} };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = function (x) { var Result = oZm.Series['ACTIVE_ENERGY'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
        Opt.axes.y.axisLabelFormatter = function (x) {
                 if(this.axes_[0].valueRange === undefined) return "";
                 var Units = oZm.GetBestUnits(Math.max(Math.abs(this.axes_[0].valueRange[0]), Math.abs(this.axes_[0].valueRange[1])));
                 return oZm.Round2(oZm.ScaleToUnits(x, Units)); }
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatDateTime(x, '<br/>'); };
        Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatDateTime(x); };

        return Opt;
      }
    },
    created: function() {
      this.Periods = [];
      this.Bills = [];
      this.Costs = [];
      this.Graph = null;
      this.Data = [];
    },
    mounted: function() {
      $(window).bind('resize', this.ResizeHandler.bind(this));
      window.dispatchEvent(new Event('resize'));
    },
    methods: {
      AggregationChanged: function(aggreg) {
        this.$refs.period.SetAggregation(aggreg);
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
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS('Active_Energy.csv', [['Time'].concat(oZm.Series['ACTIVE_ENERGY'].Names(Analyzer))].concat(this.Data));
      },
      UpdateGraph: function(to, period, lastsample, aggreg) {
        if(lastsample === 0) {
          this.Data = [];
          this.Periods = [];
          this.Costs = [];
          this.ShowSpinner = true;
        }
        $.ajax({ url: 'getPrices', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: aggreg, Series: ['ACTIVE_POWER'], To: to, LastSample: lastsample, Analyzer: this.$root.SelectedAnalyzer.Analyzer, Period: period }),
          success: function(result) {
            if((result['Sampletime'] === null) || (result['Sampletime'].length === 0)) return;
            if(result['Aggregation_Source'] === '15M') {
              for(var n = 0; n < result['Active_Power'].length; n++) result['Active_Power'][n] = result['Active_Power'][n] / 4.0;
            }
            this.Data = oZm.GraphDataAppend(result, 'Active_Power', this.Data);
            this.Costs = oZm.GraphDataAppend(result, 'Expense', this.Costs);
            if(aggreg !== result['Aggregation_Source']) this.$refs.period.SetAggregation(result['Aggregation_Source']);
            lastsample = oZm.LastSampletime(result, lastsample);
            this.Bills = result["Bills"];
            //Group periods
            var LastItem = (this.Periods.length > 0) ? this.Periods.slice(-1)[0] : null;
            for(var i = 0; i < result.Sampletime.length; i++) {
              var Period = { Name: result.Period_Name[i], From: result.Sampletime[i] - ((result['Aggregation_Source'] === '15M') ? 899999 : 3599999), To: result.Sampletime[i], Color: result.Period_Color[i] };
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
            this.$refs.graph.SetOptions( { dateWindow: [result.From, result.To], file: this.Data, valueRange: oZm.Series['ACTIVE_ENERGY'].Range(this.Data, [true]) } );
            this.UpdateCounters(result['Aggregation_Source']);
          }.bind(this),
          complete: function() {
            this.ShowSpinner = false;
            this.$refs.period.UpdateCompleted(lastsample);
          }.bind(this),
          error: function() {
            this.ShowSpinner = true;
          }.bind(this)
        });
      },
      DrawPeriods: function(canvas, area, g) {
        canvas.strokeStyle = '#777';
        canvas.lineWidth = 2;
        canvas.beginPath();
        canvas.moveTo(area.x, 0);
        canvas.lineTo(area.x, 13);
        canvas.stroke();
        for(var i = 0; i < this.Periods.length; i++) {
          var Left = g.toDomXCoord(this.Periods[i].From - 1800000);
          var Right = g.toDomXCoord(this.Periods[i].To - 1800000);
          if(Left < (area.x + 1)) Left = (area.x + 1);
          canvas.fillStyle = (this.Periods[i].Color === null) ? "#EEEEEE" : this.Periods[i].Color;
          canvas.fillRect(Left, area.y, Right, 13);
          if(((Right - Left) > 20) && (this.Periods[i].Name !== null)) {
            canvas.fillStyle = '#000000';
            canvas.font = "10px monospace";
            canvas.fillText(this.Periods[i].Name, Left + 1, 10);
          }
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
      WindowRestore: function() {
        this.$refs.period.PopState();
      },
      WindowChange: function(minDate, maxDate) {
        this.$refs.period.SetCustom(minDate, maxDate, true);
      },

    },
//    beforeDestroy: function() {
//      $(window).off('resize', this.ResizeHandler);
//    },
    components: { Graph }
  }
</script>

<style scoped>
  .dashboard-energy .header-text {
    font-size: 24px;
    line-height: 16px;
  }
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
  }
  .line {
    clear: both;
  }
  .line-border {
    border-top: 1px solid #c7c7e6;
  }
</style>