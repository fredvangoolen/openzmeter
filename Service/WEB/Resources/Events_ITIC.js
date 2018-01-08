/*
 * Copyright (C) 2015 Pedro Sánchez Alguacil
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
var VoltageGraph = null;
var VoltageData = [];
var EventsData = [];
var ITICGraph = null;
var ByDateGraph = null;
var ByRMSGraph = null;
var ByTypeGraph = null;
var ByDurationGraph = null;

var Durations = [[0, '<20ms'], [20, '<50mS'], [50, '<100mS'], [100, '<200mS'], [200, '<500mS'], [500, '<1S'], [1000, '<2S'], [2000, '<5S'], [5000, '<10S' ], [10000, '<20S' ], [20000, '<50S'], [50000, '<100S'], [Infinity, 'Others']];
var RMSs      = [-100, -80, -50, -20, -15, -10, -5, 0, 5, 10, 15, 20, 50, 100];
var Dates     = [];
var Types     = ['RVC', 'DIP', 'SWELL', 'INTERRUPTION'];
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
                 if(RMSs[x] < 0) return '<div style="font-size: 10px; margin-top: -4px"><' + (100 + RMSs[x]) + '%</div>';
                 if(RMSs[x] > 0) return '<div style="font-size: 10px; margin-top: -4px">>' + (100 + RMSs[x]) + '%</div>';
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
  if(ByTypeGraph === null) {
    ByTypeGraph = new Dygraph(document.getElementById("bytype_chart"),
      ByType,
      { strokeWidth: 1,
        labels: ["Type", "Events"],
        labelsDiv: "ByTypeLegend",
        stepPlot: true,
        plotter: multiColumnBarPlotter,
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        dateWindow: [-1, Types.length],
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
                  if((x % 1) !== 0) return "";
                  if((x < 0) || (x >= Types.length)) return "";
                  return Types[x];
                },
               axisLabelFormatter: function (x) {
                 if((x % 1) !== 0) return "";
                 if((x < 0) || (x >= RMSs.length)) return "";
                 return '<div style="font-size: 10px; margin-top: -4px">' + Types[x] +'</div>';
               }
             }
        }
      }
    );
  } else {
    ByTypeGraph.updateOptions({'file': ByType});
  }
  if(ITICGraph === null) {
    ITICGraph = new Dygraph(document.getElementById("itic_chart"),
      ByITIC,
      { strokeWidth: 0,
        labels: ["Duration", "Variation"],
        legend: "never",
        drawPoints: true, 
        pointSize: 3,
        colors: ["#d9534f"],
        gridLineColor: "#999",
        gridLinePattern: [1, 4],
        dateWindow: [1, 20000],
        valueRange: [0, 200],
        interactionModel: {},
        underlayCallback: function(canvas, area, g) {
          canvas.save();
          var w = g.toDomXCoord(20000) - g.toDomXCoord(1);
          var c = g.toDomXCoord(1);
          var k = (w - c) / (2000 - 1);
          canvas.lineWidth = 1;
          canvas.fillStyle = '#F7FFF7';
          canvas.fillRect(c, 0, area.w, area.h);
          canvas.beginPath();
          canvas.moveTo(c + (w * Math.log10(1)) / Math.log10(20000), g.toDomYCoord(100));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(100));
          canvas.stroke();
          canvas.lineWidth = 2;
          canvas.beginPath();
          canvas.moveTo(c + (w * Math.log10(1)) / Math.log10(20000), g.toDomYCoord(200));
          canvas.lineTo(c + (w * Math.log10(3)) / Math.log10(20000), g.toDomYCoord(140));
          canvas.lineTo(c + (w * Math.log10(3)) / Math.log10(20000), g.toDomYCoord(120));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(120));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(110));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(110));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(200));
          canvas.fillStyle = '#fff7ff';
          canvas.fill();
          canvas.beginPath();
          canvas.moveTo(c + (w * Math.log10(1)) / Math.log10(20000), g.toDomYCoord(200));
          canvas.lineTo(c + (w * Math.log10(3)) / Math.log10(20000), g.toDomYCoord(140));
          canvas.lineTo(c + (w * Math.log10(3)) / Math.log10(20000), g.toDomYCoord(120));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(120));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(110));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(110));
          canvas.strokeStyle = '#000';
          canvas.stroke();
          canvas.beginPath();
          canvas.moveTo(c + (w * Math.log10(20)) / Math.log10(20000), g.toDomYCoord(0));
          canvas.lineTo(c + (w * Math.log10(20)) / Math.log10(20000), g.toDomYCoord(70));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(70));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(80));
          canvas.lineTo(c + (w * Math.log10(10000)) / Math.log10(20000), g.toDomYCoord(80));
          canvas.lineTo(c + (w * Math.log10(10000)) / Math.log10(20000), g.toDomYCoord(90));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(90));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(0));
          canvas.fillStyle = '#fff7ff';
          canvas.fill();
          canvas.beginPath();
          canvas.moveTo(c + (w * Math.log10(20)) / Math.log10(20000), g.toDomYCoord(0));
          canvas.lineTo(c + (w * Math.log10(20)) / Math.log10(20000), g.toDomYCoord(70));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(70));
          canvas.lineTo(c + (w * Math.log10(500)) / Math.log10(20000), g.toDomYCoord(80));
          canvas.lineTo(c + (w * Math.log10(10000)) / Math.log10(20000), g.toDomYCoord(80));
          canvas.lineTo(c + (w * Math.log10(10000)) / Math.log10(20000), g.toDomYCoord(90));
          canvas.lineTo(c + (w * Math.log10(20000)) / Math.log10(20000), g.toDomYCoord(90));
          canvas.stroke();
          canvas.font = "16px sans-serif";
          canvas.fillStyle = "#555";
          canvas.fillText("No interruption region", c + 5 , g.toDomYCoord(0) - 5);
          canvas.textAlign = "end";
          canvas.fillText("No damage region",  c + w - 5 , g.toDomYCoord(0) - 5);
          canvas.textBaseline = "top";
          canvas.fillText("Prohibited region", c + w - 5 , 5);
          canvas.restore();
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 24,
               valueFormatter: function (x) {
                 return x;
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + x + '%</div>';
               }
             },
          x: {  pixelsPerLabel: 34,
                axisLabelWidth: 34,
                logscale: true,
                valueFormatter: function (x) {
                  if(x < 1000) return x + 'mS';
                  return (x / 1000) + 'S';
                  
               },
               axisLabelFormatter: function (x) {
                 if(x < 1000) return '<div style="font-size: 10px; margin-top: -4px">' + x + 'mS</div>';
                 return '<div style="font-size: 10px; margin-top: -4px">' + (x / 1000) + 'S</div>';
               }
             }
        }
      }
    );
    $(window).bind('resize', ITICResizeEvent);
    window.dispatchEvent(new Event('resize'));
  } else {
    ITICGraph.updateOptions({'file': ByITIC, valueRange: [0, 200], dateWindow: [1, 20000]});
  }
}
function ITICResizeEvent() {
  var Height = $('#SubGraphsDiv').height();
  if(ITICGraph != null) ITICGraph.resize($('#ITICDiv').width(), Height);
}
function CreateVoltageRangeSelector(result) {
  VoltageData.push([new Date(result['From']), Nodes['NominalVoltage']]);
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
      zoomCallback: function(min, max) {
        GenerateGraphs(min, max);
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
  $.ajax({ url: "getEvents",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 31536000000, To: 0, LastEvent: 0, Node: 0 }),
           success: function(result) {
             if(result['Starttime'] !== null) {
               for(i = 0; i < result['Starttime'].length; i++) EventsData.push([result['Starttime'][i], result['Duration'][i] / 15.6250, result['Type'][i], result['Voltage'][i], result['Percentage'][i]]);
             }
             GetVoltageData();
           }
         });  
}
function GetVoltageData() {
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 31536000000, To: 0, Aggreg: "1h", LastSample: 0, Series: 'rms_v', Node: Node }),
           success: CreateVoltageRangeSelector
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