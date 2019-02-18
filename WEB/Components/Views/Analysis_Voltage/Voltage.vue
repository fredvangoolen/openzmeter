<template>
  <div class="panel panel-voltage" id="VoltagePanel">
    <div class="panel-heading">
      <div class="header-text" style="font-size: 24px; line-height: 16px;">Voltage</div>
      <div class="header-buttons clearfix" id="toolbarVoltage">
        <toolbar-aggregation variable="Aggregation" :options="['3S', '1M', '10M', '1H']" v-on:AggregationChanged="AggregationChanged"></toolbar-aggregation>
        <toolbar-period variable="Period" v-on:PeriodChanged="PeriodChange"></toolbar-period>
        <toolbar-phases variable="VoltageViewPhases" type="VOLTAGE" :multi="true" v-on:PhasesChanged="PhasesChanged" ></toolbar-phases>
        <toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE', 'NOW']" v-on:NowStatus="SetAutoUpdate"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <div class="col-md-6 col-lg-4 col-xl-3">
          <div class="subpanel">
            <div class="huge-small" id="labelVoltageAvgPrimary">Loading data...</div>
            <div class="huge-small" id="labelVoltageAvgPrimaryValue">?</div>
            <div><span id="labelVoltageAvgSecondary">Loading data...</span>&nbsp;<span id="labelVoltageAvgSecondaryValue"></span></div>
          </div>
          <div class="subpanel">
            <div class="huge-small" id="labelVoltageMaxPrimary">Loading data...</div>
            <div class="huge-small" id="labelVoltageMaxPrimaryValue">?</div>
            <div><span id="labelVoltageMaxSecondary">Loading data...</span>&nbsp;<span id="labelVoltageMaxSecondaryValue"></span></div>
          </div>
          <div class="subpanel">
            <div class="huge-small" id="labelVoltageMinPrimary">Loading data...</div>
            <div class="huge-small" id="labelVoltageMinPrimaryValue">?</div>
            <div><span id="labelVoltageMinSecondary">Loading data...</span>&nbsp;<span id="labelVoltageMinSecondaryValue"></span></div> 
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
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'dygraphs';
  export default {
    data() {
      return { Graph: null, Data: [], ShowSpinner: true, LastSample: 0, Timer: null, Phases: [], Aggreg: null, Period: null, DateWindow: [0, 0], To: null };
    },
    created: function() {
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      this.Graph = new Dygraph(this.$refs.graph, [[0, 0]], {});
      this.UpdateGraph();
    },
    methods: {
      SetAnalyzer() {
        this.Data = [];
        this.LastSample = 0;
        this.UpdateGraph();
      },
      SetAutoUpdate(autoUpdate) {
        clearInterval(Timer);
        if(autoupdate === true) {
          this.To = 0;
        } else {
          
        }
      
        
        //  $(Self.Opt.NowBtn).click(function() {
//    var End = parseInt(localStorage.getItem('GraphEnd'));
//    clearInterval(Self.Timer);
//    if(End === 0) {
//      $(Self.Opt.NowBtn).removeClass('active');
//      localStorage.setItem('GraphEnd', Self.Range[1]);
//    } else {
//      $(Self.Opt.NowBtn).addClass('active');
//      localStorage.setItem('GraphEnd', 0);
//      Self.LastSample = 0;
//      Self.GetSeries();
//    }
//  });
      },
      PhasesChanged(phases) {
        this.Phases = phases;
        this.CalcOptions();
      },
      AggregationChanged(aggreg) {
        this.Aggreg = aggreg;
        this.Data = [];
        this.LastSample = 0;
        this.UpdateGraph();
      },
      PeriodChange(time) {
        this.Period = time;
        this.Data = [];
        this.LastSample = 0;
        this.UpdateGraph();
      },
      CalcOptions() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
//  if(clear) EnergyData = [new Array(Analyzer.GetSeriesNames('Active_Energy').length + 1).fill(0)];
//  for(var i = 0; i < result['Active_Power'].length; i++) {
//    for(var n = 0; n < result['Active_Power'][i].length; n++) result['Active_Power'][i][n] = result['Active_Power'][i][n] * ((result['Aggregation_Source'] === '1H') ? 1.0 : 0.25);
//  }
//  EnergyData = GraphDataAppend(result, 'Active_Power', EnergyData);
//  if(EnergyChart === null) {
        var Options = {
          strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow',
          legendFormatter: oZm.LegendFormatter_Follow, labels: ['Time'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)), dateWindow: this.DateWindow, file: this.Data, visibility: this.Phases,
          //zoomCallback: MainScrollable.ZoomEvt, 
          interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: this.MouseUpEvt, dblclick: this.DoubleClickEvt },
//        underlayCallback: function(context, area, dygraph) { GraphHightlightWeekends("#EEE", context, area, dygraph); }, 
        colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length), xAxisHeight: 24,
//        axes: {
//          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
//               valueFormatter: function (x) { var Units = GetBestUnits(x); return Round2(ScaleToUnits(x, Units)) + ' <em class="x-small">' + Units + 'Wh</em>'; },
//               axisLabelFormatter: function (x) { return Round2(x); }
//             },
//          x: MainScrollable.XAxis
//          }
          
        };
        this.Graph.updateOptions(Options);

//    );
//    $(window).bind('resize', EnergyResizeEvent);
//    window.dispatchEvent(new Event('resize'));
//    new GraphTools('Active energy', EnergyChart, '#toolbarEnergy', EnergyResizeEvent, function() { exportToCsv('ActiveEnergy.csv', [['Time'].concat(SeriesNames.ActiveEnergy())].concat(EnergyData)); });
//  } else {
//    EnergyChart.updateOptions( { file: EnergyData, dateWindow: [result['From'], result['To']] } );
//  }
//  EnergySeries();
//}
//function EnergySeries(event) {
//  GraphPhaseButtons(EnergyChart, $('#groupEnergyPhases'), 'VisibleDashboardEnergyPhases', false, function(series) { return CalcRanges(EnergyData, series, true, false, true); }, (event !== undefined) ? event.target : undefined); 
//}
//$('#groupEnergyPhases').on('click', 'button', EnergySeries);

      },
      UpdateGraph() {
        if((this.$root.SelectedAnalyzer === null) || (this.Aggreg === null) || (this.Period === null)) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: this.Aggreg, Series: ['VOLTAGE'], From: 0, LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.ID, Period: this.Period }), 
          success: function(result) {
            this.ShowSpinner = false;
            if(result['Sampletime'] === null) return;
            if(result['Sampletime'].length === 0) return;
            this.Data = oZm.GraphDataAppend(result, 'Voltage', this.Data);
            //EMIT señal de cambio de agregacion
            this.DateWindow = [result['From'], result['To']];
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
            this.CalcOptions();
            }.bind(this),
          complete: function() {
            if(this.UpdateGraphTimer !== null) clearTimeout(this.UpdateGraphTimer);
            this.UpdateGraphTimer = setTimeout(this.UpdateGraph, 5000);
          }.bind(this), 
          error: function() {
            this.ShowSpinner = true;
          }.bind(this)
        });
//    $(Self.Opt.AggregBtn + ' span:first').text(GetAggregLongName(result['Aggregation_Source']));
//    $(Self.Opt.AggregBtn).next().children().removeClass('active');
//    $(Self.Opt.AggregBtn).next().children(':contains(' + GetAggregLongName(result['Aggregation_Source']) + ')').addClass('active');
//    
//    var Period = result['To'] - result['From'];
//    var PeriodName = GetPeriodNameFromMS(Period);
//    $(Self.Opt.PeriodBtn + ' span:first').text(PeriodName);
//    $(Self.Opt.PeriodBtn).next().children().removeClass('active');
//    $(Self.Opt.PeriodBtn).next().children(':contains(' + PeriodName + ')').addClass('active');
//    if(Self.CountersTimes === null) {
//      Self.CountersTimes = Self.GetCountersTimePeriod(PeriodName);
//      if(Self.Opt.Counters !== undefined) Self.Opt.Counters(Self.CountersTimes, true);
//    } else {
//      var Times = Self.GetCountersTimePeriod(PeriodName);
//      if(Self.Opt.Counters !== undefined) Self.Opt.Counters(Times, (Times.PrimaryTo !== Self.CountersTimes.PrimaryTo) || (Times.PrimaryPeriod !== Self.CountersTimes.PrimaryPeriod));
//      Self.CountersTimes = Times;
//    }
//    if(Self.Opt.Series !== 'Prices') {
//      var GraphAggregation = localStorage.getItem(Self.Opt.AggregVariable);
//      if(GraphAggregation !== result['Aggregation_Source']) {
//        $('#dlgAggregWarning').modal('show');
//        GraphAggregation = result['Aggregation_Source'];
//      }
//      localStorage.setItem(Self.Opt.AggregVariable, GraphAggregation);
//    }      
//    var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
//    if(GraphEnd === 0) {
//      $(Self.Opt.NowBtn).addClass('active');
//    } else {
//      $(Self.Opt.NowBtn).removeClass('active');
//    }

          //}
//        function() { 
//          $(Self.Opt.NowBtn).removeClass('disabled');
//          if(parseInt(localStorage.getItem('GraphEnd')) === 0) Self.Timer = setTimeout(Self.GetSeries, GetPeriodFromAggreg(GraphAggregation));
//        }
//      );
        //  Self.GetSeries = function() {
//    if(Self.Timer !== null) clearInterval(Self.Timer);
//    $(Self.Opt.NowBtn).addClass('disabled');
//    if(Self.LastSample === 0) {
//      for(var i in Self.Opt.Spinners) $(Self.Opt.Spinners[i]).show();
//    }
//    var Period = parseInt(localStorage.getItem('GraphPeriod'));
//    if(!Period) {
//      Period = 86400000;
//      localStorage.setItem('GraphPeriod', Period);
//    }
//    var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
//    if(!GraphEnd) {
//      GraphEnd = 0;
//      localStorage.setItem('GraphEnd', 0);
//    }
//    var GraphAggregation = localStorage.getItem(Self.Opt.AggregVariable);
//    if(!GraphAggregation || (GetAggregLongName(GraphAggregation) === false)) {
//      GraphAggregation = '1H';
//      localStorage.setItem(Self.Opt.AggregVariable, GraphAggregation);
//    }
//    if(Self.Opt.Series === 'Prices') {
//      Ajax.GetPrices(Period, GraphEnd, Self.LastSample,
//        Self.OnSuccess, 
//        function() { 
//          $(Self.Opt.NowBtn).removeClass('disabled');
//          if(parseInt(localStorage.getItem('GraphEnd')) === 0) Self.Timer = setTimeout(Self.GetSeries, GetPeriodFromAggreg(GraphAggregation));
//        }
//      );      
//    } else {

//    }
//  };
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
  //    $(Self.Opt.NowBtn).removeClass('active');
  //    localStorage.setItem('GraphEnd', maxDate);
  //    localStorage.setItem('GraphPeriod', maxDate - minDate);
  //    Self.Range = [minDate, maxDate];
        this.LastSample = 0;
  //    Self.GetSeries();
      }
    },
    components: { Spinner, ToolbarAggregation, ToolbarPeriod, ToolbarPhases, ToolbarTools }
  }
</script>

<style>
</style>


function Scrollable_OnSuccess(result, clear) {
  if(clear) VoltageData = [];
  AjaxGetEvents(result['From'], result['To'], clear);
  VoltageData = GraphDataAppend(result, 'Voltage', VoltageData);
  if(VoltageChart === null) {
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"), VoltageData,
      { strokeWidth: 0.5, stepPlot: true, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(Analyzer.GetSeriesNames('Voltage')), dateWindow: [result['From'], result['To']],
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: ColorArray($('#VoltagePanel').css('border-color'), 3), plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ], xAxisHeight: 24,
        underlayCallback: function(canvas, area, g) {
          var NV = Analyzer.GetSelected().NominalVoltage, Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
          GraphHBackgroundRanges([[NV * 1.1, NV * 1.07, Yellow], [NV * 0.93, NV * 0.9, Yellow], [Infinity, NV * 1.1, Red], [NV * 0.9, -Infinity, Red]], g, canvas);
        },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">V</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: MainScrollable.XAxis
          }
        }  
    );
    $(window).bind('resize', VoltageResizeEvent);
    window.dispatchEvent(new Event('resize'));
    new GraphTools('Voltage', VoltageChart, '#toolbarVoltage', VoltageResizeEvent, function() { exportToCsv('Voltage.csv', [['Time'].concat(Analyzer.GetSeriesNames('Voltage'))].concat(VoltageData)); });
  } else {
    VoltageChart.updateOptions({file: VoltageData, dateWindow: [result['From'], result['To']]});
  }
  VoltageSeries();
}
function VoltageResizeEvent() { 
  if(VoltageChart !== null) VoltageChart.resize('100%', $('#VoltagePanel > .panel-body > .row > div:first-child').height());
}
function VoltageSeries(event) {
  GraphPhaseButtons(VoltageChart, $('#groupVoltagePhases'), 'VisibleVoltageMainPhases', false, function(series) { return CalcRanges(VoltageData, series, false); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupVoltagePhases').on('click', 'button', VoltageSeries);
function Scrollable_UpdateCounters(times, renew) {
  if(renew === true) {
    $('#labelVoltageAvgPrimary').text(times.PrimaryText + ' average');
    $('#labelVoltageMaxPrimary').text(times.PrimaryText + ' maximun');
    $('#labelVoltageMinPrimary').text(times.PrimaryText + ' minimun');
    $('#labelVoltageAvgPrimaryValue, #labelVoltageMaxPrimaryValue, #labelVoltageMinPrimaryValue').text('Calculating...');
    $('#labelVoltageAvgSecondary, #labelVoltageMaxSecondary, #labelVoltageMinSecondary').text(times.SecondaryText);
    $('#labelVoltageAvgSecondaryValue, #labelVoltageMaxSecondaryValue, #labelVoltageMinSecondaryValue').text((times.SecondaryTo !== undefined) ? 'Calculating...' : '');
    if(times.SecondaryTo !== undefined) {
      Analyzer.GetSeriesAggreg(times.SecondaryPeriod, times.SecondaryTo, times.PreferedAggreg, 'avg(rms_v), max(rms_v), min(rms_v)',
        function(result) {
          if(result['Samples'] !== 0) {
            $('#labelVoltageAvgSecondaryValue').html(Array_Round2(result['avg_rms_v']) + '&nbsp;<em>V</em>');
            $('#labelVoltageMinSecondaryValue').html(Array_Round2(result['min_rms_v']) + '&nbsp;<em>V</em>');
            $('#labelVoltageMaxSecondaryValue').html(Array_Round2(result['max_rms_v']) + '&nbsp;<em>V</em>');
          } else {
            $('#labelVoltageAvgSecondaryValue, #labelVoltageMaxSecondaryValue, #labelVoltageMinSecondaryValue').text('No data'); 
          }
          window.dispatchEvent(new Event('resize'));
        }
      );
    }
  }
  Analyzer.GetSeriesAggreg(times.PrimaryPeriod, times.PrimaryTo, times.PreferedAggreg, 'avg(rms_v), max(rms_v), min(rms_v)', 
    function(result) {
      $('#labelVoltageAvgPrimaryValue').html(Array_Round2(result['avg_rms_v'], ColorArray($('#VoltagePanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">V</em>');
      $('#labelVoltageMinPrimaryValue').html(Array_Round2(result['min_rms_v'], ColorArray($('#VoltagePanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">V</em>');
      $('#labelVoltageMaxPrimaryValue').html(Array_Round2(result['max_rms_v'], ColorArray($('#VoltagePanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">V</em>');
      window.dispatchEvent(new Event('resize'));
    }
  );
}
