/*
 * Copyright (C) 2017 Eduardo Viciana Gamez
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


function GenerateVoltGraph(week) {
  var Options = { 
    strokeWidth: 1,
    labels: ["Variation", "Time"],
    labelsDiv: "GridVoltageLegend",
    stepPlot: true,
    plotter: multiColumnBarPlotter,
    colors: ["#31708f"],
    gridLineColor: "#999",
    gridLinePattern: [4, 4],
    dateWindow: [-0.5, Voltages.length - 1.5],
    interactionModel: {},
    axes: {
      y: { pixelsPerLabel: 24,
           axisLabelWidth: 24,
           valueFormatter: function (x) {
             return Round2(x * 100.0 / WeekStats[week].VoltageSamples) + ' %';
           },
           axisLabelFormatter: function (x) {
             return '<div style="font-size: 10px; margin-top: -2px">' + Math.round(x * 100 / WeekStats[week].VoltageSamples, 0) + '%</div>';
           }
         },
      x: {  pixelsPerLabel: 30,
            axisLabelWidth: 30,
            valueFormatter: function (x) {
              if((x < 0) || (x >= Voltages.length - 1)) return "";
              if(x === 0) {
                return "Less than " + Round2(Analyzers[Analyzer].NominalVoltage * (1 + Voltages[x + 1] / 100.0)) + ' V';
              } else if(x == (Voltages.length - 2)) {
                return "More than " + Round2(Analyzers[Analyzer].NominalVoltage * (1 + Voltages[x] / 100.0)) + ' V';
              } else {
                return "From " + Round2(Analyzers[Analyzer].NominalVoltage * (1 + Voltages[x] / 100.0)) + " to " + Round2(Analyzers[Analyzer].NominalVoltage * (1 + Voltages[x + 1] / 100.0)) + ' V';
              }
            },
            axisLabelFormatter: function (x) {
              if((x < 0) || (x >= Voltages.length - 1)) return "";
              if(x === 0) {
                return '<div style="font-size: 10px; line-height: 10px">≤' + Round2(Analyzers[Analyzer].NominalVoltage * (1 + Voltages[x + 1] / 100.0))+ '</div>';
              } else if(x == (Voltages.length - 2)) {
                return '<div style="font-size: 10px; line-height: 10px">≥' + Round2(Analyzers[Analyzer].NominalVoltage * (1 + Voltages[x] / 100.0))+ '</div>';
              } else {
                return '<div style="font-size: 10px; line-height: 10px">' + Round2(Analyzers[Analyzer].NominalVoltage * (1 + Voltages[x] / 100.0)) + "<br/>" + Round2(Analyzers[Analyzer].NominalVoltage * (1 + Voltages[x + 1] / 100.0)) + '</div>';
              }
            }
          }
    }
  };
  var In  = (WeekStats[week].VoltageIN / WeekStats[week].VoltageSamples) * 100.0;
  var ToShow = [];
  for(i = 0; i < WeekStats[week].Voltage.length; i++) ToShow.push([i, WeekStats[week].Voltage[i]]);
  if(In < 95) {
    $('#VoltageWarning').removeClass('hidden');
  } else {
    $('#VoltageWarning').addClass('hidden');
  }
  $('#VoltageInRange').css('width', (In - 90) * 10 +'%');
  $('#VoltageInRangeText').html(Round2(In) + '% of time in range');
  if(VoltageChart === null) {
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"), ToShow, Options);
  } else {
    Options.file = ToShow;
    VoltageChart.updateOptions(Options);
  }
}
function GenerateTHDGraph(week) {
  var Options = {
    strokeWidth: 1,
    labels: ["Distorsion", "Time"],
    labelsDiv: "THDLegend",
    stepPlot: true,
    plotter: multiColumnBarPlotter,
    colors: ["#31708f"],
    gridLineColor: "#999",
    gridLinePattern: [4, 4],
    dateWindow: [-0.5, THDs.length - 1.5],
    interactionModel: {},
    axes: {
      y: { pixelsPerLabel: 24,
           axisLabelWidth: 24,
           valueFormatter: function (x) {
             return Round2(x * 100.0 / WeekStats[week].THDSamples) + ' %';
           },
           axisLabelFormatter: function (x) {
             return '<div style="font-size: 10px; margin-top: -2px">' + Math.round(x * 100 / WeekStats[week].THDSamples, 0) + '%</div>';
           }
         },
      x: {  pixelsPerLabel: 30,
            axisLabelWidth: 30,
            valueFormatter: function (x) {
              if((x < 0) || (x >= THDs.length - 1)) return "";
              if(x == (THDs.length - 2)) {
                return "More than " + THDs[x] + ' %';
              } else {
                return "From " + THDs[x] + " to " + THDs[x + 1] + ' %';
              }
            },
            axisLabelFormatter: function (x) {
              if((x < 0) || (x >= THDs.length - 1)) return "";
              if(x == (THDs.length - 2)) {
                return '<div style="font-size: 10px; line-height: 10px">≥' + THDs[x] + '</div>';
              } else {
                return '<div style="font-size: 10px; line-height: 10px">' + THDs[x] + "<br/>" + THDs[x + 1] + '</div>';
              }
           }
         }
    }
  };
  var In = (WeekStats[week].THDIN / WeekStats[week].THDSamples) * 100.0;
  var ToShow = [];
  for(i = 0; i < WeekStats[week].THD.length; i++) ToShow.push([i, WeekStats[week].THD[i]]);
  if(In < 100) {
    $('#THDWarning').removeClass('hidden');
  } else {
    $('#THDWarning').addClass('hidden');
  }
  $('#THDInRange').css('width', (In - 90) * 10 +'%');
  $('#THDInRangeText').html(Round2(In) + '% of time in range');
  if(THDChart === null) {
    THDChart = new Dygraph(document.getElementById("THDChart"), ToShow, Options);
  } else {
    Options.file = ToShow;
    THDChart.updateOptions(Options);
  }  
}
function GenerateHarmonicsGraph(week) {
  var In = (WeekStats[week].HarmonicsIN / WeekStats[week].HarmonicsSamples) * 100.0;
  $('#HarmonicsInRange').css('width', (In - 90) * 10 +'%');
  $('#HarmonicsInRangeText').html(Round2(In) + '% of time in range');
}
function GenerateEventsGraph(week) {
  var DipProgress = WeekStats[week].Dip * 100 / 1000;
  $('#DipEvents').css('width', DipProgress  +'%');
  $('#DipEventsText').text(WeekStats[week].Dip + ' events');
  if(WeekStats[week].Dip > 1000) $('#DipEvents').css('background-color', '#d9534f"');
  var ShortProgress = WeekStats[week].Short * 100 / 100;
  $('#ShortEvents').css('width', ShortProgress  +'%');
  $('#ShortEventsText').text(WeekStats[week].Short + ' events');
  if(WeekStats[week].Short > 100) $('#ShortEvents').css('background-color', '#d9534f"');
  var LongProgress = WeekStats[week].Long * 100 / 50;
  $('#LongEvents').css('width', LongProgress  +'%');
  $('#LongEventsText').text(WeekStats[week].Long + ' events');
  if(WeekStats[week].Long > 100) $('#LongEvents').css('background-color', '#d9534f"');  
}

