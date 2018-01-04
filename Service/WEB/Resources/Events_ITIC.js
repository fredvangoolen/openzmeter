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
var Types     = ['RVC', 'DIP', 'SWELL', 'INT'];
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
      zoomCallback: GenerateGraphs,
      axes: {
        x: { 
          drawAxis: false,
        },
        y: { 
          drawAxis: false
        }
      }
    }
  );
  GenerateGraphs((result['From']), (result['To']));
}
function GetEvents() {
  $.ajax({ url: "getEvents",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 31536000000, To: 0, LastEvent: 0, Node: 0 }),
           success: function(result) {
             if(result['Starttime'] != null) {
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
           success: CreateVoltageRangeSelector,
         });
}
function ButtonsClick() {
  if(this.checked == false) return;
  var Range = VoltageGraph.getOption('dateWindow');
  if($(this).val() == 'day') {
    Period = 86400000;
  } else if($(this).val() == 'week') {
    Period = 604800000;
  } else if($(this).val() == 'month') {
    Period = 2592000000;
  } else if($(this).val() == 'year') {
    EnergyTimeScale = 'Year';
    Period = 31536000000;
  }
  VoltageGraph.updateOptions({dateWindow: [Range[1] - Period, Range[1]]});
}
function UpdatePage() {
  GetEvents();
  
}
$(':input[name="chart_range"]').change(ButtonsClick);

//
//
//function printITICurve() {
//    var line = d3.svg.line()
//            .x(function (d) {
//                return itic_chart.x()(d.x);
//            })
//            .y(function (d) {
//                return itic_chart.y()(d.y);
//            });
//
//    var path = line([{x: 200, y: 500}, {x: 1000, y: 200}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "black")
//            .attr("stroke-width", 2)
//            .attr("fill", "none");
//
//    path = line([{x: 1000, y: 200}, {x: 3000, y: 140}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "black")
//            .attr("stroke-width", 2)
//            .attr("fill", "none");
//
//
//    path = line([{x: 3000, y: 140}, {x: 3000, y: 120}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "black")
//            .attr("stroke-width", 2)
//            .attr("fill", "none");
//
//    path = line([{x: 3000, y: 120}, {x: 500000, y: 120}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "black")
//            .attr("stroke-width", 2)
//            .attr("fill", "none");
//
//    path = line([{x: 500000, y: 120}, {x: 500000, y: 110}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "black")
//            .attr("stroke-width", 2)
//            .attr("fill", "none");
//
//    path = line([{x: 500000, y: 110}, {x: 100000000, y: 110}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "black")
//            .attr("stroke-width", 2)
//            .attr("fill", "none");
//
////Bottom curve
//
//    path = line([{x: 20000, y: 0}, {x: 20000, y: 70}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "black")
//            .attr("stroke-width", 2)
//            .attr("fill", "none");
//
//
//    path = line([{x: 20000, y: 70}, {x: 500000, y: 70}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "black")
//            .attr("stroke-width", 2)
//            .attr("fill", "none");
//
//
//    path = line([{x: 500000, y: 70}, {x: 500000, y: 80}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "black")
//            .attr("stroke-width", 2)
//            .attr("fill", "none");
//
//    path = line([{x: 500000, y: 80}, {x: 10000000, y: 80}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "black")
//            .attr("stroke-width", 2)
//            .attr("fill", "none");
//
//    path = line([{x: 10000000, y: 80}, {x: 10000000, y: 90}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "black")
//            .attr("stroke-width", 2)
//            .attr("fill", "none");
//
//    path = line([{x: 10000000, y: 90}, {x: 100000000, y: 90}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "black")
//            .attr("stroke-width", 2)
//            .attr("fill", "none");
//
//    path = line([{x: 1, y: 100}, {x: 100000000, y: 100}]);
//
//    itic_chart.svg().select('g.chart-body').append("path").attr("d", path).attr("stroke", "red")
//            .attr("stroke-width", 1)
//            .attr("fill", "none");
//
//    var containerWidth = $('#itic_chart').parent().width();
//    itic_chart.svg().select('g.chart-body').append("text").attr('class', 'area_text').attr("x", containerWidth - containerWidth * 0.35).attr("y", 200)
//            .text(function (d) {
//                return "Prohibited region";
//            });
//
//
//    itic_chart.svg().select('g.chart-body').append("text").attr('class', 'area_text').attr("x", containerWidth - containerWidth * 0.35).attr("y", 530)
//            .text(function (d) {
//                return "No damage area";
//            })
//
//
//
//    itic_chart.svg().select('g.chart-body').append("text").attr('class', 'area_text').attr("width", 100).attr("x", 30).attr("y", 325)
//            .text(function (d) {
//                return "No interruption";
//            })
//
//    itic_chart.svg().select('g.chart-body').append("text").attr('class', 'area_text').attr("width", 100).attr("x", 30).attr("y", 340)
//            .text(function (d) {
//                return "in fuction Region";
//            })
//
//
//}
//
//
//function filterByDate(val) {
//    var date = new Date();
//    switch (val) {
//        case 'today':
//            var end = date;
//            end.setHours(23);
//            end.setMinutes(59);
//            end.setSeconds(59);
//            var start = new Date(end.getFullYear(), end.getMonth(), end.getDate());
//            break;
//        case 'yesterday':
//            var end = date;
//            end.setHours(23);
//            end.setMinutes(59);
//            end.setSeconds(59);
//            end = new Date(end.getFullYear(), end.getMonth(), end.getDate() - 1);
//            end.setHours(23);
//            end.setMinutes(59);
//            end.setSeconds(59);
//            start = new Date(end.getFullYear(), end.getMonth(), end.getDate());
//            break;
//        case 'week':
//            end = date;
//            start = new Date(end.getFullYear(), end.getMonth(), end.getDate() - end.getDay());
//            break;
//        case 'month':
//            end = date;
//            start = new Date(end.getFullYear(), end.getMonth(), 1);
//            break;
//    }
//
//    brush = dateSelectorChart.brush().extent([start, end]);
//
//    gBrush = dateSelectorChart.svg().select('.brush')
//            .call(brush);
//
//    gBrush.call(brush.event);
//}
//
//function resetCharts() {
//    dc.filterAll();
//    dc.renderAll();
//    printITICurve();
//    filterByDate($(':input[name="date_chart_range"]:checked').val());
//}
//
//
//var scatterTip = d3.tip()
//        .attr('class', 'd3-tip')
//        //.offset([-10, 0])
//        .html(function (d) {
//            var date = new Date(d.key[2]);
//            return "<p><em>" + getStringEventTypeByNum(d.key[0])
//                    + "</em></p><ul class='small'><li>" + date.toLocaleString() + "." + date.getMilliseconds() + "</li><li>RMS : " + roundTo(d.value, 2).toFixed(2) + "%</li><li>Duration: " + (d.key[1] == 100000000 ? '≥ ' : '') + formatTimeUnit(d.key[1]) + "</li></ul>";
//        });
//
//
//
////var itic_chart = dc.seriesChart('#itic_chart');
//
////var data_bak = [{duration: 1323133, rms: 3, id_event: 9, type: 0, time: +events[0].timestamp}, {duration: 5234432, rms: 453, id_event: 10, type: 1, time: +events[0].timestamp}, {duration: 2344, rms: 200, id_event: 15, type: 1, time: +events[0].timestamp}, {duration: 10, rms: 80, id_event: 15, type: 2, time: +events[0].timestamp}];
//var data = [];// [{duration: 1323133, rms: 3, id_event: 9, type: 0, time: 1434711599000}, {duration: 5234432, rms: 453, id_event: 10, type: 1, time: 1434721599000}, {duration: 2344, rms: 200, id_event: 15, type: 1, time: 1434731599000}, {duration: 10, rms: 80, id_event: 15, type: 2, time: 1434751599000}];
//for (var i in events) {
//    var startTime = new Date(+events[i].timestamp);
//    //var percent = events[i].max_v / 230 * 100;
//    var percent = roundTo(100 - events[i].percent, 2);
//    var duration = events[i].duration * 1e6;
//    if (duration > 100) {
//        duration = 100;
//    }
//    data.push({
//        id_event: events[i].type + '_' + i,
//        duration: +duration * 1000000,
//        complete_duration: +events[i].duration * 1000000,
//        rms: percent + 0.001,
//        type: getStringEventNum(events[i].type),
//        time: +events[i].timestamp
//    });
//}
//
//
//var ndx = crossfilter(data);
//var all = ndx.groupAll();
//var runDimension, runGroup;
//
//runDimension = ndx.dimension(function (d) {
//    return [d.type, d.duration, d.time];
//});
//runGroup = runDimension.group().reduceSum(function (d) {
//    return d.rms;
//});
//
//var symbolScale = d3.scale.ordinal().range(d3.svg.symbolTypes);
//var symbolAccessor = function (d) {
//    return symbolScale(d.key[0]);
//};
//var subChart = function (c) {
//    return dc.scatterPlot(c)
//            .symbol(symbolAccessor)
//            .symbolSize(5)
//            .highlightedSize(40)
//};
//
//itic_chart
//        .width($('#itic_chart').parent().width())
//        .height(650)
//        .margins({top: 20, right: 20, bottom: 40, left: 40})
//        .chart(subChart)
//        .x(d3.scale.log().nice().domain([1, 100000000]))
//        .y(d3.scale.linear().domain([0, 500]))
//        .brushOn(false)
//        .colors(d3.scale.category10())
//        .yAxisLabel("% of Nominal Voltage (RMS)")
//        .xAxisLabel("Steady state")
//        //.clipPadding(10)
//        .renderHorizontalGridLines(true)
//        .renderVerticalGridLines(true)
//        .renderlet(function (chart) {
//            if (d3.selectAll(".symbol")[0].length > 0) {
//                d3.selectAll(".symbol").call(scatterTip);
//                d3.selectAll(".symbol").on('mouseover', scatterTip.show)
//                        .on('mouseout', scatterTip.hide);
//            }
//        })
//        .elasticY(false)
//        .dimension(runDimension)
//        .group(runGroup)
//        //.mouseZoomable(true)
//        .seriesAccessor(function (d) {
//            return "Expt: " + d.key[0];
//        })
//        .keyAccessor(function (d) {
//            return d.key[1];
//        })
//        .valueAccessor(function (d) {
//            return d.value;
//        }).
//        xAxis().tickFormat(function (v) {
//    if (v >= 1000000) {
//        return v / 1000000 + ' s';
//    }
//    if (v >= 1000) {
//        return v / 1000 + ' ms';
//    }
//    return v + ' µs';
//}).tickValues([1, 10, 100, 1000, 10000, 100000, 1000000, 10000000]);
//itic_chart.yAxis().tickFormat(function (v) {
//    return v + ' %';
//});
//
////itic_chart.on("renderlet", function () {
////    alert("AHOLHA");
////})
////itic_chart.margins().left += 40;
//
//
//
//
///**
// * TYPE OF EVENT
// */
//
//var pieTip = d3.tip()
//        .attr('class', 'd3-tip')
//        .offset([-10, 0])
//        .html(function (d) {
//            return "<em>" + getStringEventTypeByNum(d.data.key) + "</em> : " + d.value + (d.value == 1 ? ' events' : ' event');
//        });
//
//
//var typeChart = dc.pieChart('#type_chart');
//
//var eventTypeDimension = ndx.dimension(function (d) {
//    return d.type;
//});
//
//var eventTypeDimensionGroup = eventTypeDimension.group();
//
//
//typeChart
//        .width($('#type_chart').parent().width())
//        .height($('#type_chart').parent().width() - 20) // (optional) define chart height, :default = 200
//        .radius($('#type_chart').parent().width() / 2 - 20) // define pie radius
//        .dimension(eventTypeDimension) // set dimension
//        .group(eventTypeDimensionGroup) // set group
//        .label(function (d) {
//            var label = getStringEventTypeShort(d.key);
//            if (typeChart.hasFilter() && !typeChart.hasFilter(d.key)) {
//                return label + ' (0%)';
//            }
//            if (all.value()) {
//                label += ' (' + Math.floor(d.value / all.value() * 100) + '%)';
//            }
//            return label;
//        }).renderLabel(true)
//        .transitionDuration(500)
//        .title(function (d) {
//            return getStringEventTypeByNum(d.key) + ": " + d.value;
//        })
//        .renderlet(function (chart) {
//            d3.selectAll(".pie-slice").call(pieTip);
//            d3.selectAll(".pie-slice").on('mouseover', pieTip.show)
//                    .on('mouseout', pieTip.hide);
//        })
//        .colors(d3.scale.category10());
//
//
///**
// * Number of events by % voltage RMS
// */
//
//var rmsChart = dc.barChart('#rms_chart');
//
//var rmsDimension = ndx.dimension(function (d) {
//    return parseInt((d.rms) / 50);
//});
//
//var rmsDimensionGroup = rmsDimension.group();
//
//rmsChart
//        .width($('#rms_chart').parent().width())
//        .height(200)
//        .margins({top: 20, right: 20, bottom: 20, left: 20})
//        .dimension(rmsDimension)
//        .group(rmsDimensionGroup)
//        .xAxisPadding(1)
//        .colors(d3.scale.category10())
//        //.gap(10)
//        .renderHorizontalGridLines(true)
//        //.centerBar(true)
//        .elasticY(true)
//        .round(dc.round.floor)
//        .alwaysUseRounding(true)
//        .x(d3.scale.linear().domain([-0.5, 11]))
//        .renderHorizontalGridLines(true)
//        .xAxis().tickFormat(function (v) {
//    return  ((v) * 50);
//});
//
//rmsChart.yAxis().tickFormat(d3.format("d"))
//        .tickSubdivide(0);
//
//
///**
// * Number of events by duration
// */
//
//var barDurationTip = d3.tip()
//        .attr('class', 'd3-tip')
//        //.offset([-10, 0])
//        .html(function (d) {
//            return "<em> Between" + d.x.toLocaleDateString()
//                    + "</em> : " + d.y + (d.y == 1 ? ' events' : ' event');
//        });
//
//
//
//var durationChart = dc.barChart('#duration_chart');
//
//var durationDimension = ndx.dimension(function (d) {
//    var base = parseInt(getBaseLog(10, d.duration));
//    if (base > 8) {
//        base = 8;
//    }
//    return base;
//});
//
//var durationDimensionGroup = durationDimension.group();
//
//durationChart
//        .width($('#duration_chart').parent().width())
//        .height(200)
//        .margins({top: 20, right: 20, bottom: 20, left: 20})
//        .dimension(durationDimension)
//        .group(durationDimensionGroup)
//        .xAxisPadding(2)
//        .colors(d3.scale.category10())
//        //.gap(10)
//        .renderHorizontalGridLines(true)
//        //.centerBar(true)
//        .elasticY(true)
//        .renderlet(function (chart) {
//            if (d3.selectAll(".symbol")[0].lenght > 0) {
//                d3.selectAll(".bar").call(barDurationTip);
//                d3.selectAll(".bar").on('mouseover', barDurationTip.show)
//                        .on('mouseout', barDurationTip.hide);
//            }
//        })
//        .round(dc.round.floor)
//        .alwaysUseRounding(true)
//        .x(d3.scale.linear().domain([-0.5, 9]))
//        .renderHorizontalGridLines(true)
//        .xAxis().tickFormat(function (v) {
//    switch (v) {
//        case 0:
//            return '1 µs';
//        case 1:
//            return '10 µs';
//        case 2:
//            return '100 µs';
//        case 3:
//            return '1 ms';
//        case 4:
//            return '10 ms';
//        case 5:
//            return '100 ms';
//        case 6:
//            return '1 s';
//        case 7:
//            return '10 s';
//        case 8:
//            return '100 s';
//    }
//    ;
//});
//
//
//durationChart.yAxis().tickFormat(d3.format("d"))
//        .tickSubdivide(0);
//
//
///**
// * Events by date
// */
//
//var barDateTip = d3.tip()
//        .attr('class', 'd3-tip')
//        //.offset([-10, 0])
//        .html(function (d) {
//            return "<em>" + d.x.toLocaleDateString()
//                    + "</em> : " + d.y + (d.y == 1 ? ' events' : ' event');
//        });
//
//
//var dateDimension = ndx.dimension(function (d) {
//    return new Date(d.time);
//});
//
//var dateDimensionGroup = dateDimension.group(function (d) {
//    return d3.time.day(d);
//});
//
//var dateSelectorChart = dc.barChart('#date_selector_chart');
//
//var date = new Date();
//dateSelectorChart
//        .width($('#date_selector_chart').parent().width())
//        .height(60)
//        .margins({top: 10, right: 20, bottom: 20, left: 30})
//        .dimension(dateDimension)
//        .group(dateDimensionGroup)
//        .centerBar(true)
//        .gap(1)
//        .x(d3.time.scale().domain([new Date(date.getFullYear() - 1, date.getMonth(), date.getDate()), date]))
//        .round(d3.time.days.round)
//        .renderHorizontalGridLines(true)
//        .alwaysUseRounding(true)
//        .xUnits(d3.time.days)
//        .renderlet(function (chart) {
//            if (d3.selectAll(".symbol")[0].lenght > 0) {
//                d3.selectAll(".bar").call(barDateTip);
//                d3.selectAll(".bar").on('mouseover', barDateTip.show)
//                        .on('mouseout', barDateTip.hide);
//            }
//        })
//        .yAxis().tickFormat(d3.format("d")).ticks(1).tickSubdivide(0);
//
//var dateChart = dc.barChart('#date_chart');
//
//
//dateChart
//        .width($('#date_chart').parent().width())
//        .height(180)
//        .rangeChart(dateSelectorChart)
//        .dimension(dateDimension)
//        .group(dateDimensionGroup)
//        .margins({top: 20, right: 20, bottom: 20, left: 20})
//        .mouseZoomable(true)
//        .centerBar(true)
//        .elasticY(true)
//        .x(d3.time.scale().domain([new Date(date.getFullYear() - 1, date.getMonth(), date.getDate()), date]))
//        .xUnits(d3.time.days)
//        .renderHorizontalGridLines(true)
//        .round(d3.time.days.round)
//        .alwaysUseRounding(true)
//        .brushOn(false)
//        .zoomScale([1, 14]);
//
//dateChart.yAxis().tickFormat(d3.format("d"))
//        .tickSubdivide(0);
///**
// * Month chart (heatmap)
// */
//var heatChart = dc.heatMap("#month_chart");
//
//var runDim = ndx.dimension(function (d) {
//    var date = new Date(d.time);
//    var day = date.getDate();
//    return [day % 7, parseInt(day / 7)];
//});
//
//var runGroup = runDim.group().reduceSum(function (d) {
//    return 1;
//});
//
//heatChart
//        .width($('#month_chart').parent().width())
//        .height($('#month_chart').parent().width())
//        .label(function (d) {
//            var date = new Date(d.time);
//            var day = date.getDate();
//            return day;
//        }).renderLabel(true)
//        .transitionDuration(500)
//        .dimension(runDim)
//        .group(runGroup)
//        .colorAccessor(function (d) {
//            return +d.value;
//        })
//        .title(function (d) {
//            return "Run:   " + d.key[0] + "\n" +
//                    "Expt:  " + d.key[1] + "\n" +
//                    "Speed: " + (299000 + d.value) + " km/s";
//        })
//        .colors(["#ffffd9", "#edf8b1", "#c7e9b4", "#7fcdbb", "#41b6c4", "#1d91c0", "#225ea8", "#253494", "#081d58"])
//        .calculateColorDomain();
//
//
///**
// * DAY OF WEEK
// */
//
//var dayOfWeekChart = dc.rowChart('#weekday_chart');
//
//var dayOfWeek = ndx.dimension(function (d) {
//    var date = new Date(d.time);
//    var day = date.getDay();
//    var name = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
//    return day + '.' + name[day];
//});
//var dayOfWeekGroup = dayOfWeek.group();
//
//
//dayOfWeekChart.width($('#weekday_chart').parent().width())
//        .height($('#weekday_chart').parent().width())
//        .margins({top: 20, left: 10, right: 10, bottom: 20})
//        .group(dayOfWeekGroup)
//        .dimension(dayOfWeek)
//        .ordinalColors(['#3182bd', '#6baed6', '#9ecae1', '#c6dbef', '#dadaeb'])
//        .label(function (d) {
//            return d.key.split('.')[1];
//        }).title(function (d) {
//    return d.value;
//})
//        .elasticX(true)
//        .xAxis().ticks(4);
//
//dc.renderAll();
//
//
//filterByDate('week');
//
///**
// * ITIC Curve
// */
//
//printITICurve();
//
//$(':input[name="date_chart_range"]').change(function () {
//    filterByDate($(this).val());
//});
//
