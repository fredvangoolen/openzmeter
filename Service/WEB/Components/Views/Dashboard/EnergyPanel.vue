<template>
  <div class="panel panel-active-energy dashboard-energy">
    <div class="panel-heading">
      <div class="huge header-text">Energy consumption</div>
      <div class="header-buttons clearfix" ref="Toolbar">
        <toolbar-aggregation variable="AggregationEnergy" v-on:AggregationChanged="AggregationChanged" :options="['15M', '1H']"></toolbar-aggregation>
        <toolbar-period variable="Period" v-on:PeriodChanged="UpdateGraph" ref="period"></toolbar-period>
        <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE', 'NOW']" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <div class="col-md-6 col-lg-4 col-xl-3" ref="counters">
          <div class="subpanel clearfix">
            <div class="counters-title">Period stats:</div>
            <div class="counters-aggreg">{{ CountersAggregation }}<br>From {{ TimeStatsFrom }} to {{ TimeStatsTo }}</div>
            <div class="line line-border">
              <div class="float-left">
                <div class="counters-stat-type">Max value:</div>
                <div class="counters-stat-time">{{ MaxTime }}</div>
              </div>
              <div class="counters-stat-val" v-html="MaxValue"></div>
            </div>
            <div class="line line-border">
              <div class="float-left">
                <div class="counters-stat-type">Total in period:</div>
                <div class="counters-stat-time">From {{ AccumSamples }} samples</div>
              </div>
              <div class="counters-stat-val" v-html="AccumValue"></div>
            </div>
          </div>
          <div class="subpanel clearfix">
            <div class="counters-title">Period costs:</div>
            <div class="clearfix"></div>
            <div class="current-value" v-html="BudgetValue"></div>
          </div>
        </div>
        <div class="col-md-12 col-lg-8 col-xl-9">
          <graph :options="GraphOptions" ref="graph"></graph>
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
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    data() {
      return { ShowSpinner: true, CountersAggregation: 'Loading data...', TimeStatsFrom: '...', TimeStatsTo: '...', MaxTime: '?', MaxValue: '?', AccumSamples: '?', AccumValue: '?', BudgetValue: '?'};
    },
    created: function() {
      this.Periods = [];
      this.Bills = [];
      this.Costs = [];
      this.Graph = null;
      this.Data = [];
      this.GraphOptions = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: true, legend: 'follow', plotter: Graph.multiColumnBarPlotter,
        labels: ['Time', 'Energy'], interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: this.MouseUpEvt, dblclick: this.DoubleClickEvt },
        colors: [oZm.Series['ACTIVE_ENERGY'].Color], xAxisHeight: 24, legendFormatter: oZm.LegendFormatter_Follow, underlayCallback: this.DrawPeriods, selectMode: 'euclidian',
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Result = oZm.Series['ACTIVE_ENERGY'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
               axisLabelFormatter: function (x) { var Units = oZm.GetBestUnits(Math.max(Math.abs(Range[0]), Math.abs(Range[1]))); return oZm.Round2(oZm.ScaleToUnits(x, Units)); }
             },
          x: { axisLabelFormatter: function(x) { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); },
               valueFormatter: function (x) { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); }
             }
          }
      };
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
      UpdateGraph: function(datewindow, to, period, lastsample, aggreg) {
        if(lastsample === 0) {
          this.Data = [];
          this.Periods = [];
          this.Costs = [];
          this.ShowSpinner = true;
        }
        $.ajax({ url: 'getPrices', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: aggreg, Series: ['ACTIVE_POWER'], To: to, LastSample: lastsample, Analyzer: this.$root.SelectedAnalyzer.ID, Period: period }), 
          success: function(result) {
            if((this.$refs.period.Update === false) || (result['Sampletime'] === null) || (result['Sampletime'].length === 0)) return;
            if(result['Aggregation_source'] === '15M') {
              for(var n = 0; n < result['Active_Power'].length; n++) result['Active_Power'][n] = result['Active_Power'][n] / 4.0;
            }
            this.Data = oZm.GraphDataAppend(result, 'Active_Power', this.Data);
            this.Costs = oZm.GraphDataAppend(result, 'Expense', this.Costs);
            this.$refs.period.SetAggregation(result['Aggregation_Source'], false);
            this.$refs.period.SetLastSample(oZm.LastSampletime(result, this.$refs.period.LastSample));
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
              this.Periods = this.Periods.filter(period => (period.To > datewindow[0]) || (period.From < datewindow[1]) ).sort(function(a, b) { if(a.To < b.To) return -1; if(a.To > b.To) return 1; return 0; });
            }
            this.$refs.graph.SetOptions( { dateWindow: datewindow, file: this.Data, valueRange: oZm.Series['ACTIVE_ENERGY'].Range(this.Data, [true]) } );
            this.UpdateCounters(result['Aggregation_Source']);
            this.ShowSpinner = false;
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
      },
      UpdateCounters: function(aggreg) {
        this.CountersAggregation = 'From ' + oZm.GetAggregLongName(aggreg) + ' aggregation';        
        this.TimeStatsFrom = moment(this.$refs.period.DateWindow[0]).format('DD/MM/YYYY HH:mm:ss');
        this.TimeStatsTo = moment(this.$refs.period.DateWindow[1]).format('DD/MM/YYYY HH:mm:ss');
        var AccumSamples = 0, AccumValue = 0.0, MaxValue = -Infinity, MaxCount = 0, MaxTime = null, Cost = 0.0;
        for(var x = 0; x < this.Data.length; x++) {
          if(this.Data[x][1] === null) continue;
          if(this.Data[x][1] > MaxValue) {
            MaxValue = this.Data[x][1];
            MaxCount = 0;
            MaxTime = this.Data[x][0];
          } else if(this.Data[x][1] === MaxValue) {
            MaxCount++;
          }
          if(this.Costs[x][1] !== null) Cost += this.Costs[x][1];
          AccumValue += this.Data[x][1];
          AccumSamples++;
        }
        this.AccumSamples = AccumSamples;
        this.MaxTime = moment(MaxTime).format("hh:mm:ss - DD/MM/YYYY") + ((MaxCount > 1) ? ' (and ' + MaxCount + ' times more)' : '');
        var UnitsMax = oZm.Series['ACTIVE_ENERGY'].FormatValue(Number.isFinite(MaxValue) ? MaxValue : 0.0);    
        this.MaxValue = UnitsMax.Value + ' <em>' + UnitsMax.Units + '</em>';
        var UnitsAccum = oZm.Series['ACTIVE_ENERGY'].FormatValue(AccumValue);    
        this.AccumValue = UnitsAccum.Value + '<em>' + UnitsAccum.Units + '</em>';
        this.BudgetValue = oZm.Round2(Cost) + ' <em>€</em>';
        this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
      }
    },
    beforeDestroy: function() {
      $(window).off('resize', this.ResizeHandler);
    },
    components: { ToolbarAggregation, ToolbarPeriod, ToolbarTools, Spinner, Graph }
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