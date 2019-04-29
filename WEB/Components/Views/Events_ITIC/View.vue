<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-other">
          <div class="panel-heading">
            <span class="header-text">ITIC Curve (CBEMA)</span>
            <div class="header-buttons">
              <span class="small">Time scale:</span><br>
              <div class="btn-group">
                <button type="button" class="btn btn-default btn-xs" id="btnShowEventYear"  data-toggle="tooltip" title="Select exactly a year period from current time">Year</button>
                <button type="button" class="btn btn-default btn-xs" id="btnShowEventMonth" data-toggle="tooltip" title="Select exactly a month period from current time">Month</button>
                <button type="button" class="btn btn-default btn-xs" id="btnShowEventWeek"  data-toggle="tooltip" title="Select exactly a week period from current time">Week</button>
                <button type="button" class="btn btn-default btn-xs" id="btnShowEventDay"   data-toggle="tooltip" title="Select exactly a day period from current time">Day</button>
              </div>
            </div>
            <div class="pull-right">
              <span class="huge-small-unit" id="EventsSelection"></span><br>
            </div>
          </div>
          <div class="panel-body panel-bigheader">
            <div class="panel-info panel-heading">
              <div style="display:inline-block" class="pull-right small" id="RangeLabel">Loading...</div>
              <graph  class="chart" style="height: 50px"></graph>
            </div>
            <div class="row">
              <div class="col-md-12 col-lg-6"><curve></curve></div> 
              <div class="col-md-12 col-lg-6">
                <div class="row" ref="subgraphs">
                  <div class="col-xs-12 col-md-6"><by-date></by-date></div>
                  <div class="col-xs-12 col-md-6"><by-type></by-type></div>
                  <div class="col-xs-12 col-md-6"><by-rms></by-rms></div>
                  <div class="col-xs-12 col-md-6"><by-duration></by-duration></div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Curve from './Curve.vue';
  import ByDate from './ByDate.vue';
  import ByType from './ByType.vue';
  import ByRms from './ByRMS.vue';
  import ByDuration from './ByDuration.vue';
  import Graph from 'Components/Graph.vue';
  export default {
    data: function() {
      return { ShowSpinner: true };
    },
    components: { Graph, Curve, ByDate, ByType, ByRms, ByDuration }
  }
</script>

<style>
</style>

var VoltageGraph = null;
var VoltageData = [];
var EventsData = [];
var ITICGraph = null;
var ByDateGraph = null;
var ByRMSGraph = null;
var ByTypeGraph = null;
var ByDurationGraph = null;

var RMSs      = [-100, -80, -50, -20, -15, -10, -5, 0, 5, 10, 15, 20, 50, 100];
var Dates     = [];
function GenerateMonths(min, max) {
  var Start = new Date(min), End = new Date(max);
  Start = new Date(Start.getFullYear(), Start.getMonth(), 1);
  Dates = [];
  while(Start <= End) {
    Dates.push([new Date(Start.getFullYear(), Start.getMonth(), 1).getTime(),new Date(Start.getFullYear(), Start.getMonth() + 1, 0).getTime(), (Start.getMonth() + 1) + '/' + Start.getFullYear()]);
    Start.setMonth(Start.getMonth() + 1);
  }
  return;
}
function GenerateDays(min, max) {
  var Start = new Date(min), End = new Date(max);
  Dates = [];
  while(Start <= End) {
    Dates.push([new Date(Start.getFullYear(), Start.getMonth(), Start.getDate()).getTime(),new Date(Start.getFullYear(), Start.getMonth(), Start.getDate() + 1).getTime(), Start.getDate() + '/' + (Start.getMonth() + 1)]);
    Start.setDate(Start.getDate() + 1);
  }
}
function GenerateGraphs(min, max) {
  var StartDate = new Date(min);
  var EndDate = new Date(max);
  $('#RangeLabel').text('From ' + StartDate.toLocaleDateString() + ' at ' + StartDate.toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'}) + ' to ' + EndDate.toLocaleDateString() + ' at ' + EndDate.toLocaleTimeString([], {hour: '2-digit', minute:'2-digit'}));
  EventsCount = 0;
  GenerateMonths(min, max);
  if(Dates.length < 3) GenerateDays(min, max);
  var ByDuration = [];
  var ByRMS = [];
  var ByDate = [];
  var ByITIC = [[0, 0]];
  var ByType = [[0, 0], [1, 0], [2, 0], [3, 0]];
  for(i = 0; i < Durations.length; i++) ByDuration.push([i, 0]);
  for(i = 0; i < RMSs.length; i++) ByRMS.push([i, 0]);
  for(i = 0; i < Dates.length; i++) ByDate.push([i, 0]);
  var DatesPos = 0;
  for(i = 0; i < EventsData.length; i++) {
    if((EventsData[i][0] < min) || (EventsData[i][0] > max)) continue;
    ByITIC.push([EventsData[i][1], 100 + EventsData[i][4]]);
    EventsCount++;
    for(j = 0; j < Durations.length; j++) {
      if((EventsData[i][1] >= Durations[j][0]) && (EventsData[i][1] < Durations[j + 1][0])) {
        ByDuration[j][1] += 1;
        break;
      }
    }
    for(j = 0; j < RMSs.length; j++) {
      if((EventsData[i][4] >= RMSs[j]) && (EventsData[i][4] < RMSs[j + 1])) {
        ByRMS[j][1] += 1;
        break;
      }
    }
    for(j = 0; j < Types.length; j++) {
      if(EventsData[i][2] === Types[j]) {
        ByType[j][1] += 1;
        break;
      }
    }
    for(; DatesPos < Dates.length; DatesPos++) {
      if((EventsData[i][0] >= Dates[DatesPos][0]) && (EventsData[i][0] < Dates[DatesPos][1])) {
        ByDate[DatesPos][1] += 1;
        break;
      }
    }
  }
  $('#EventsSelection').text(EventsCount + ' events in selection');
  if(ByDurationGraph === null) {
    ByDurationGraph = new Dygraph(document.getElementById("byduration_chart"),
      ByDuration,
      { strokeWidth: 1,
        labels: ["Duration", "Events"],
        labelsDiv: "ByDurationLegend",
        stepPlot: true,
        plotter: multiColumnBarPlotter,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        dateWindow: [-1, Durations.length],
        interactionModel: {},
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 14,
               valueFormatter: function (x) {
                 return x;
               },
               axisLabelFormatter: function (x) {
                 if((x % 1) !== 0) return "";
                 return '<div style="font-size: 10px; margin-top: -2px">' + x + '</div>';
               }
             },
          x: {  pixelsPerLabel: 26,
                valueFormatter: function (x) {
                  if((x < 0) || (x >= Durations.length)) return "";
                  return Durations[x][1];
               },
               axisLabelFormatter: function (x) {
                 if((x < 0) || (x >= Durations.length)) return "";
                 return '<div style="font-size: 10px; margin-top: -4px">' + Durations[x][1] + '</div>';
               }
             }
        }
      }
    );
  } else {
    ByDurationGraph.updateOptions({'file': ByDuration});
  }
  if(ByRMSGraph === null) {
    ByRMSGraph = new Dygraph(document.getElementById("byrms_chart"),
      ByRMS,
      { strokeWidth: 1,
        labels: ["Variation", "Events"],
        labelsDiv: "ByRMSLegend",
        stepPlot: true,
        plotter: multiColumnBarPlotter,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        dateWindow: [-1, RMSs.length],
        interactionModel: {},
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 14,
               valueFormatter: function (x) {
                 return x;
               },
               axisLabelFormatter: function (x) {
                 if((x % 1) !== 0) return "";
                 return '<div style="font-size: 10px; margin-top: -2px">' + x + '</div>';
               }
             },
          x: {  pixelsPerLabel: 26,
                valueFormatter: function (x) {
                  if((x < 0) || (x >= RMSs.length)) return "";
                  if(RMSs[x] === 0) return 'Low';
                  if(RMSs[x] < 0) return '<' + (100 + RMSs[x]) + '%';
                  if(RMSs[x] > 0) return '>' + (100 + RMSs[x]) + '%';
               },
               axisLabelFormatter: function (x) {
                 if((x < 0) || (x >= RMSs.length)) return "";
                 if(RMSs[x] === 0) return '<div style="font-size: 10px; margin-top: -4px">Low</div>';
                 if(RMSs[x] < 0) return '<div style="font-size: 10px; margin-top: -4px">' + (100 + RMSs[x]) + '</div>';
                 if(RMSs[x] > 0) return '<div style="font-size: 10px; margin-top: -4px">' + (100 + RMSs[x]) + '</div>';
               }
             }
        }
      }
    );
  } else {
    ByRMSGraph.updateOptions({'file': ByRMS});
  }
  if(ByDateGraph === null) {
    ByDateGraph = new Dygraph(document.getElementById("bydate_chart"),
      ByDate,
      { strokeWidth: 1,
        labels: ["Date", "Events"],
        labelsDiv: "ByDateLegend",
        stepPlot: true,
        plotter: multiColumnBarPlotter,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        dateWindow: [-1, Dates.length],
        interactionModel: {},
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 14,
               valueFormatter: function (x) {
                 return x;
               },
               axisLabelFormatter: function (x) {
                 if((x % 1) !== 0) return "";
                 return '<div style="font-size: 10px; margin-top: -2px">' + x + '</div>';
               }
             },
          x: {  pixelsPerLabel: 40,
                axisLabelWidth: 40,
                valueFormatter: function (x) {
                  if((x < 0) || (x >= Dates.length)) return "";
                  return Dates[x][2];
               },
               axisLabelFormatter: function (x) {
                 if((x % 1) !== 0) return "";
                 if((x < 0) || (x >= Dates.length)) return "";
                 return '<div style="font-size: 10px; margin-top: -4px">' + Dates[x][2] + '</div>';
               }
             }
        }
      }
    );
  } else {
    ByDateGraph.updateOptions({'file': ByDate, dateWindow: [-1, Dates.length]});
  }
  
  $('#EventSpinner').hide();
}
function CreateVoltageRangeSelector(result) {
  VoltageData.push([new Date(result['From']), Analyzers[Analyzer].NominalVoltage]);
  for(i = 0; i < result['Sampletime'].length; i++) {
    var Max = -Infinity;
    for(n = 0; n < result['rms_v'][i].length; n++) Max = Math.max(Max, result['rms_v'][i][n]);
    VoltageData.push([new Date(result['Sampletime'][i]), Max]);
  }
  VoltageGraph = new Dygraph(document.getElementById("VoltageRangeChart"),
    VoltageData,
    { labels: ["Time", "Voltage"],
      stepPlot: true,
      dateWindow: [(result['From']), (result['To'])],
      xAxisHeight: 0,
      showRangeSelector: true,
      rangeSelectorHeight: 40,
      legend: "never",
      axisLineColor: "#CCC",
      series: {
        'Voltage': {
          strokeWidth: 0,
          showInRangeSelector: true
        }
      },
      rangeSelectorPlotFillColor: "#FFFFFF",
      rangeSelectorPlotStrokeColor: "#d9534f",
      drawCallback: function(dygraph, is_initial) {
        if(is_initial) return;
        GenerateGraphs(dygraph.dateWindow_[0], dygraph.dateWindow_[1]);
        $('#btnShowEventYear').removeClass('active');
        $('#btnShowEventMonth').removeClass('active');
        $('#btnShowEventWeek').removeClass('active');
        $('#btnShowEventDay').removeClass('active');
      },
      axes: {
        x: { 
          drawAxis: false
        },
        y: { 
          drawAxis: false
        }
      }
    }
  );
  GenerateGraphs((result['From']), (result['To']));
  ButtonsClick();
}
function GetEvents() {
  Ajax.GetEvents(31536000000, 0, 0, function(result) {
    if(result['Starttime'] !== null) {
      for(i = 0; i < result['Starttime'].length; i++) EventsData.push([result['Starttime'][i], result['Duration'][i] / 15.6250, result['Type'][i], result['Voltage'][i], result['Percentage'][i]]);
    }
    Ajax.GetSeries(31536000000, 0, '1h', 0, 'rms_v', CreateVoltageRangeSelector);
  });  
}
/* Seleccion rapida de periodo */
function ButtonsClick() {
  $('#btnShowEventYear').removeClass('active');
  $('#btnShowEventMonth').removeClass('active');
  $('#btnShowEventWeek').removeClass('active');
  $('#btnShowEventDay').removeClass('active');
  var Range = VoltageGraph.getOption('dateWindow');
  if(this.innerText === 'Day') {
    localStorage.setItem('ITICPeriod', 'Day');
  } else if(this.innerText === 'Week') {
    localStorage.setItem('ITICPeriod', 'Week');
  } else if(this.innerText === 'Month') {
    localStorage.setItem('ITICPeriod', 'Month');
  } else if(this.innerText === 'Year') {
    localStorage.setItem('ITICPeriod', 'Year');
  }
  var Period = localStorage.getItem('ITICPeriod');
  if(Period === 'Day') {
    Period = 86400000;
    $('#btnShowEventDay').addClass('active');
  } else if(Period === 'Week') {
    Period = 604800000;
    $('#btnShowEventWeek').addClass('active');
  } else if(Period === 'Year') {
    Period = 31536000000;
    $('#btnShowEventYear').addClass('active');
  } else {
    Period = 2592000000;
    localStorage.setItem('ITICPeriod', 'Month');
    $('#btnShowEventMonth').addClass('active');
  }
  VoltageGraph.updateOptions({dateWindow: [Range[1] - Period, Range[1]]});
  GenerateGraphs(Range[1] - Period, Range[1]);
}
$('#btnShowEventYear').click(ButtonsClick);
$('#btnShowEventMonth').click(ButtonsClick);
$('#btnShowEventWeek').click(ButtonsClick);
$('#btnShowEventDay').click(ButtonsClick);

function UpdatePage() {
  GetEvents();
}