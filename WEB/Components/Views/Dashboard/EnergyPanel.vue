<template>
  <div class="panel panel-active-energy dashboard-energy">
    <div class="panel-heading">
      <div class="huge header-text">Energy consumption</div>
      <div class="header-buttons clearfix" ref="Toolbar">
        <toolbar-aggregation variable="AggregationEnergy" v-on:AggregationChanged="AggregationChanged" :options="['15M', '1H']"></toolbar-aggregation>
        <toolbar-period variable="Period" v-on:PeriodChanged="PeriodChange" ref="period"></toolbar-period>
        <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE', 'NOW']" v-on:NowStatus="SetAutoUpdate" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <div class="col-md-12 col-lg-4 col-xl-3" ref="counters">
          <div class="subpanel">
            <div class="huge-small" id="labelEnergyPrimary">Loading data...</div>
            <div><span id="labelEnergyPrimaryValue" class="huge">?</span>&nbsp;<em class="huge-small-unit" id="labelEnergyPrimaryUnits"></em></div>
            <div id="labelEnergyPrimaryPhases"></div>
            <div><span id="labelEnergySecondary">Loading data...</span>&nbsp;<span id="labelEnergySecondaryValue"></span></div>
          </div>
          <div class="subpanel">
            <div class="huge-small" id="labelBudgetPrimary">Loading data...</div>
            <span id="labelBudgetPrimaryValue" class="huge">?</span>&nbsp;<em class="huge-small-unit" id="labelBudgetUnits"></em>
            <div><span id="labelBudgetSecondary">Loading data...</span>&nbsp;<span id="labelBudgetSecondaryValue"></span></div>
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
      return { ShowSpinner: true };
    },
    created: function() {
      this.Periods = [];
      this.Graph = null;
      this.Data = [];
      this.LastSample = 0;
      this.Timer = null;
      this.Aggreg = null;
      this.Period = null;
      this.DateWindow = [0, 0];
      this.AutoUpdate = null;
      this.GraphOptions = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow',
        labels: ['Time', 'Energy'], interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: this.MouseUpEvt, dblclick: this.DoubleClickEvt },
        colors: [oZm.Series['ACTIVE_ENERGY'].Color], xAxisHeight: 24, legendFormatter: oZm.LegendFormatter_Follow, underlayCallback: this.DrawPeriods,
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
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      $(window).bind('resize', this.ResizeHandler.bind(this));
      window.dispatchEvent(new Event('resize'));
      this.UpdateGraph();
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
      ResizeHandler: function() {
        if(typeof this.$refs.graph !== 'undefined') this.$refs.graph.Redraw('100%', $(this.$refs.counters).height());
      },
      MaximizeClick() {
        $(window).unbind('resize', this.ResizeHandler);
        this.$refs.tools.Maximize('Energy consumption', this.$refs.graph.Graph, this.$refs.Toolbar);
      },
      MinimizeClick() {
        $(window).bind('resize', this.ResizeHandler.bind(this));
      },
      UpdateGraph() {
        if((this.$root.SelectedAnalyzer === null) || (this.Aggreg === null) || (this.Period === null) || (this.AutoUpdate === null)) return;
        $.ajax({ url: 'getPrices', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: this.Aggreg, Series: ['ACTIVE_POWER'], To: this.$refs.tools.ReadTo(), LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.ID, Period: this.Period }), 
          success: function(result) {
            if((result['Sampletime'] === null) || (result['Sampletime'].length === 0)) return;
            if(this.LastSample === 0) this.Data = [];
            if(this.LastSample === 0) this.Periods = [];
            this.Data = oZm.GraphDataAppend(result, 'Active_Energy', this.Data);
            this.DateWindow = [result['From'], result['To']];
            this.$refs.tools.WriteTo(result['To']);
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
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
              this.Periods = this.Periods.filter(period => (period.To > this.DateWindow[0]) || (period.From < this.DateWindow[1]) ).sort(function(a, b) { if(a.To < b.To) return -1; if(a.To > b.To) return 1; return 0; });
            }
            this.$refs.graph.SetOptions( { dateWindow: this.DateWindow, file: this.Data, valueRange: oZm.Series['ACTIVE_ENERGY'].Range(this.Data, [true]) } );
            this.ShowSpinner = false;
          }.bind(this),
          complete: function() {
            if(this.Timer !== null) clearTimeout(this.Timer);
            if(this.AutoUpdate === true) this.Timer = setTimeout(this.UpdateGraph.bind(this), 5000);
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
          canvas.fillStyle = this.Periods[i].Color;
          canvas.fillRect(Left, area.y, Right, 13);
          if(((Right - Left) > 20) && (this.Periods[i].Name !== null)) {
            canvas.fillStyle = '#000';
            canvas.font = "10px monospace";
            canvas.fillText(this.Periods[i].Name, Left + 1, 10);
          }
//              var Y = g.toDomYCoord(this.Periods[i].Power);
//              canvas.strokeStyle = '#d9534f';
//              canvas.lineWidth = 1;
//              canvas.beginPath();
//              if(i > 0) {
//                canvas.moveTo(Left, g.toDomYCoord(this.Periods[i -1 ].Power));
//                canvas.lineTo(Left, Y);
//              } else {
//                canvas.moveTo(Left, Y);
//              }
//              canvas.lineTo(Right, Y);
//              canvas.stroke();
        }
        area.y = 13;
        area.h = area.h - 13;
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
  //    $(Self.Opt.NowBtn).removeClass('active');
  //    localStorage.setItem('GraphEnd', maxDate);
  //    localStorage.setItem('GraphPeriod', maxDate - minDate);
  //    Self.Range = [minDate, maxDate];
        this.LastSample = 0;
  //    Self.GetSeries();
      }
    },
    beforeDestroy: function() {
      if(this.Timer !== null) clearInterval(this.Timer);
      $(window).off('resize', this.ResizeHandler);
    },
    components: { ToolbarAggregation, ToolbarPeriod, ToolbarTools, Spinner, Graph }
  }
</script>
  
<style>
  .dashboard-energy .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>


//function Scrollable(opt) {

//  this.Opt = Object.assign({NowBtn: '#btnScrollableNow', AggregBtn: '#btnScrollableAggreg', PeriodBtn: '#btnSearchAnalyzercrollablePeriod', Spinners: [], Series: '', AggregVariable: 'GraphAggregation'}, opt);
//  this.CountersTimes = null;
//  var Self = this;
//  Self.XAxis = { valueFormatter: function (x) { return new Date(x).toLocaleDateString() + ' - ' + new Date(x).toLocaleTimeString(); },
//                 axisLabelFormatter: function (x) { return new Date(x).toLocaleTimeString() + '<br/>' + new Date(x).toLocaleDateString(); }
//               };

//  $(Self.Opt.PeriodBtn + ' + ul li a').click(function(event) {
//    event.preventDefault();
//    var PeriodName = $(this).text();
//    var Period = GetMSFromPeriodName(PeriodName);
//    localStorage.setItem('GraphPeriod', Period);
//    Self.LastSample = 0;
//    Self.GetSeries();
//  });
//  $(Self.Opt.AggregBtn + ' + ul li a').click(function(event) {
//    event.preventDefault();
//    localStorage.setItem(Self.Opt.AggregVariable, GetAggregShortName($(this).text()));
//    Self.LastSample = 0;
//    Self.GetSeries();
//  });




//}


        //  Self.GetCountersTimePeriod = function(name) {
//    if(parseInt(localStorage.getItem('GraphEnd')) === 0) {
//      if(name === 'Year')  return Object.assign(GetYearTimePeriod(),  { PreferedAggreg: localStorage.getItem(Self.Opt.AggregVariable) });
//      if(name === 'Month') return Object.assign(GetMonthTimePeriod(), { PreferedAggreg: localStorage.getItem(Self.Opt.AggregVariable) });
//      if(name === 'Week')  return Object.assign(GetWeekTimePeriod(),  { PreferedAggreg: localStorage.getItem(Self.Opt.AggregVariable) });
//      if(name === 'Day')   return Object.assign(GetDayTimePeriod(),   { PreferedAggreg: localStorage.getItem(Self.Opt.AggregVariable) });
//    }
//    var Result = {};
//    Result.PrimaryPeriod = parseInt(localStorage.getItem('GraphPeriod'));
//    Result.PrimaryTo = parseInt(localStorage.getItem('GraphEnd'));
//    Result.PreferedAggreg = localStorage.getItem(Self.Opt.AggregVariable);
//    Result.PrimaryText = 'Displayed period';
//    if(Result.PrimaryTo === 0) {
//      Result.SecondaryText = 'Last ' + moment.duration(Result.PrimaryPeriod / 1000, "seconds").format("d [days] h [hours] m [minutes]");
//    } else {
//      var From = new Date(Result.PrimaryTo - Result.PrimaryPeriod);
//      var To = new Date(Result.PrimaryTo);
//      Result.SecondaryText = 'From ' + From.toLocaleDateString() + ' - ' + From.toLocaleTimeString() + ' to ' + To.toLocaleDateString() + ' - ' + To.toLocaleTimeString();
//    }
//    return Result;
//  };
