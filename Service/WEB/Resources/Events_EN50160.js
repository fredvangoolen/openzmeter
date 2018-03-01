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
var Spinner            = new Spinner(SpinnerOpts);
var WeekStats          = [];
var FrequencyYearChart = null;
var VoltageYearChart   = null;
var THDYearChart       = null;
var FrequencyWeekChart = null;
var VoltageWeekChart   = null;
var THDWeekChart       = null;
var Frequencies        = [ -Math.INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, Math.INFINITY ];
var Voltages           = [-Math.INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, Math.INFINITY];
var THDs               = [0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, Math.INFINITY];

//var LoadWeek;
//var LastWeek;
//var LastYear;
//function SetWeekEvents(result) {
//  var Table = $('#weeks_table tbody');
//  var VoltagePercent = (result['Samples'] - result['Voltage']) * 100 / result['Samples'];
//  var THDPercent = (result['Samples'] - result['THD']) * 100 / result['Samples'];
//  var HarmonicsPercent = (result['Samples'] - result['Harmonics']) * 100 / result['Samples'];
//  var Row = '<tr><td class="text-center"><strong>' + result['Week'] + "</strong>&nbsp;(" + result['Year'] + ')</td>' +
//            '<td class="text-center">' + Round2(result['Samples'] * 100 / 1008) + '%</td>';
//  if(result['Samples'] > 0) {
//    Row += '<td>' +
//           '  <div class="progress text-center" style="margin-bottom:0px; position: relative">' +
//           '    <div class="progress-bar" role="progressbar" style="width:' + Math.min(100, VoltagePercent * 20) + '%; background-color: #5cb85c"></div>' +
//           '    <span style="top: 2px; left: 0px; position: absolute; width: 100%; mix-blend-mode: multiply">' + Round2(VoltagePercent) + '% of time outside range</span>' +
//           '  </div>' +
//           '</td>' +
//           '<td>' +
//           '  <div class="progress text-center" style="margin-bottom:0px; position: relative">' +
//           '    <div class="progress-bar" role="progressbar" style="width:' + Math.min(100, THDPercent * 20) + '%; background-color: #5cb85c"></div>' +
//           '    <span style="top: 2px; left: 0px; position: absolute; width: 100%; mix-blend-mode: multiply">' + Round2(THDPercent) + '% of time outside range</span>' +
//           '  </div>' +
//           '</td>' +
//           '<td>' +
//           '  <div class="progress text-center" style="margin-bottom:0px; position: relative">' +
//           '    <div class="progress-bar" role="progressbar" style="width:' + Math.min(100, HarmonicsPercent * 20) + '%; background-color: #5cb85c"></div>' +
//           '    <span style="top: 2px; left: 0px; position: absolute; width: 100%; mix-blend-mode: multiply">' + Round2(HarmonicsPercent) + '% of time outside range</span>' +
//           '  </div>' +
//           '</td>';
//  } else {
//    Row += '<td colspan="4" class="text-center">No samples for this week</td>';
//  }
//  Table.append(Row + '</tr>');
//  LastWeek = result['Week'];
//  LastYear = result['Year'];
//  if(result['Week'] === 1) {
//    $('#weeks_table tfoot').hide();
//  } else if(LoadWeek === 1) {
//    $('#RowLoading').addClass('hide');
//    $('#RowButton').removeClass('hide');
//  } else {
//    LoadWeek--;
//    GetWeekInfo(result['Year'], LastWeek - 1);
//  }
//}
//function GetWeekInfo(reqyear, reqweek) {
//  $('#RowLoading').removeClass('hide');
//  $('#RowButton').addClass('hide');
//  $.ajax({ url: "events/getWeekEvents",
//           timeout: 20000,
//           type: "POST",
//           dataType: "json",
//           contentType: 'application/json',
//           data: JSON.stringify({ Year: reqyear, Week: reqweek }),
//           success: SetWeekEvents
//         });
//}
//function GetYearInfo(reqyear) {
//  if(reqyear === 0) {
//    var End = GetEndYearTimestamp() - 365 * 24 * 3600 * 1000;
//    var Start = GetYearTimestamp() - 365 * 24 * 3600 * 1000;
//  }
//  $.ajax({ url: "getEvents",
//           timeout: 20000,
//           type: "POST",
//           dataType: "json",
//           contentType: 'application/json',
//           data: JSON.stringify({ Node : Node, To: End, Period: End - Start }),
//           success: SetYearEvents
//         });
//}

function GenerateYearFreqGraph(distribution, total_samples, inrange_samples, type) {
  var Options = { 
    strokeWidth: 1,
    labels: ["Variation", "Time"],
    labelsDiv: "GridFrequencyLegend",
    stepPlot: true,
    plotter: multiColumnBarPlotter,
    colors: ["#31708f"],
    gridLineColor: "#999",
    gridLinePattern: [4, 4],
    dateWindow: [-0.5, Frequencies.length - 1.5],
    interactionModel: {},
    axes: {
      y: { pixelsPerLabel: 24,
           axisLabelWidth: 24,
           valueFormatter: function (x) {
             return Round2(x * 100.0 / Samples) + ' %';
           },
           axisLabelFormatter: function (x) {
             return '<div style="font-size: 10px; margin-top: -2px">' + Math.round(x * 100 / Samples, 0) + '%</div>';
           }
         },
      x: { pixelsPerLabel: 30,
           axisLabelWidth: 30,
           valueFormatter: function (x) {
             if((x < 0) || (x >= Frequencies.length - 1)) return "";
             if(x === 0) {
               return "Less than " + Round2(Nodes['NominalFreq'] * (1 + Frequencies[x + 1] / 100.0)) + ' Hz';
             } else if(x == (Frequencies.length - 2)) {
               return "More than " + Round2(Nodes['NominalFreq'] * (1 + Frequencies[x] / 100.0)) + ' Hz';
             } else {
               return "From " + Round2(Nodes['NominalFreq'] * (1 + Frequencies[x] / 100.0)) + " to " + Round2(Nodes['NominalFreq'] * (1 + Frequencies[x + 1] / 100.0)) + ' Hz';
             }
           },
           axisLabelFormatter: function (x) {
             if((x < 0) || (x >= Frequencies.length - 1)) return "";
             if(x === 0) {
               return '<div style="font-size: 10px; line-height: 10px">≤' + Round2(Nodes['NominalFreq'] * (1 + Frequencies[x + 1] / 100.0))+ '</div>';
             } else if(x == (Frequencies.length - 2)) {
               return '<div style="font-size: 10px; line-height: 10px">≥' + Round2(Nodes['NominalFreq'] * (1 + Frequencies[x] / 100.0))+ '</div>';
             } else {
               return '<div style="font-size: 10px; line-height: 10px">' + Round2(Nodes['NominalFreq'] * (1 + Frequencies[x] / 100.0)) + "<br/>" + Round2(Nodes['NominalFreq'] * (1 + Frequencies[x + 1] / 100.0)) + '</div>';
             }
           }
         }
    }
  };
  var In = 0;
  for(i = 5; i < 36; i++) In += distribution[i];
  In  = (In / Samples) * 100.0;
  if(In < 99.5) {
    $('#FreqWarning').removeClass('hidden');
  } else {
    $('#FreqWarning').addClass('hidden');
  }
  $('#FreqInRange').css('width', (In - 90) * 10 +'%');
  $('#FreqInRangeText').html(Round2(In) + '% of time in range');
  var ToShow = [];
  for(i = 0; i < distribution.length; i++) ToShow.push([i, distribution[i]]);
  if(FrequencyYearChart === null) {
    FrequencyYearChart = new Dygraph(document.getElementById("FrequencyChart"), ToShow, Options);
  } else {
    FrequencyYearChart.updateOptions({'file': ToShow}, true);
    FrequencyYearChart.updateOptions(Options);
  }
}
function GenerateYearVoltGraph(distribution) {
  var Samples = 0;
  for(i = 0; i < distribution.length; i++) Samples += distribution[i];
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
             return Round2(x * 100.0 / Samples) + ' %';
           },
           axisLabelFormatter: function (x) {
             return '<div style="font-size: 10px; margin-top: -2px">' + Math.round(x * 100 / Samples, 0) + '%</div>';
           }
         },
      x: {  pixelsPerLabel: 30,
            axisLabelWidth: 30,
            valueFormatter: function (x) {
              if((x < 0) || (x >= Voltages.length - 1)) return "";
              if(x === 0) {
                return "Less than " + Round2(Nodes['NominalVoltage'] * (1 + Voltages[x + 1] / 100.0)) + ' V';
              } else if(x == (Voltages.length - 2)) {
                return "More than " + Round2(Nodes['NominalVoltage'] * (1 + Voltages[x] / 100.0)) + ' V';
              } else {
                return "From " + Round2(Nodes['NominalVoltage'] * (1 + Voltages[x] / 100.0)) + " to " + Round2(Nodes['NominalVoltage'] * (1 + Voltages[x + 1] / 100.0)) + ' V';
              }
            },
            axisLabelFormatter: function (x) {
              if((x < 0) || (x >= Voltages.length - 1)) return "";
              if(x === 0) {
                return '<div style="font-size: 10px; line-height: 10px">≤' + Round2(Nodes['NominalVoltage'] * (1 + Voltages[x + 1] / 100.0))+ '</div>';
              } else if(x == (Voltages.length - 2)) {
                return '<div style="font-size: 10px; line-height: 10px">≥' + Round2(Nodes['NominalVoltage'] * (1 + Voltages[x] / 100.0))+ '</div>';
              } else {
                return '<div style="font-size: 10px; line-height: 10px">' + Round2(Nodes['NominalVoltage'] * (1 + Voltages[x] / 100.0)) + "<br/>" + Round2(Nodes['NominalVoltage'] * (1 + Voltages[x + 1] / 100.0)) + '</div>';
              }
            }
          }
    }
  };
  var In = 0;
  for(i = 3; i < 24; i++) In += distribution[i];
  In  = (In / Samples) * 100.0;
  if(In < 95) {
    $('#VoltageWarning').removeClass('hidden');
  } else {
    $('#VoltageWarning').addClass('hidden');
  }
  $('#VoltageInRange').css('width', (In - 90) * 10 +'%');
  $('#VoltageInRangeText').html(Round2(In) + '% of time in range');
  var ToShow = [];
  for(i = 0; i < distribution.length; i++) ToShow.push([i, distribution[i]]);
  if(VoltageYearChart === null) {
    VoltageYearChart = new Dygraph(document.getElementById("VoltageChart"), ToShow, Options);
  } else {
    VoltageYearChart.updateOptions({'file': ToShow}, true);
    VoltageYearChart.updateOptions(Options);
  }
}
function GenerateYearTHDGraph(distribution) {
  Samples = 0;
  for(i = 0; i < distribution.length; i++) Samples += distribution[i];
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
             return Round2(x * 100.0 / THDSamples) + ' %';
           },
           axisLabelFormatter: function (x) {
             return '<div style="font-size: 10px; margin-top: -2px">' + Math.round(x * 100 / THDSamples, 0) + '%</div>';
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
  var In = 0;
  for(i = 0; i < 16; i++) In += distribution[i];
  In = (In / Samples) * 100.0;
  if(In < 100) {
    $('#THDWarning').removeClass('hidden');
  } else {
    $('#THDWarning').addClass('hidden');
  }
  $('#THDInRange').css('width', (In - 90) * 10 +'%');
  $('#THDInRangeText').html(Round2(In) + '% of time in range');
  var ToShow = [];
  for(i = 0; i < distribution.length; i++) ToShow.push([i, distribution[i]]);
  if(THDYearChart === null) {
    THDYearChart = new Dygraph(document.getElementById("THDChart"), ToShow, Options);
  } else {
    THDYearChart.updateOptions({'file': ToShow}, true);
    THDYearChart.updateOptions(Options);
  }  
}
function GenerateYearHarmonicsGraph(distribution) {  
  var Samples = 0;
  for(i = 0; i < distribution.length; i++) Samples += distribution[i];
  var In = 0;
  for(i = 0; i < StatsHarmonics.length; i++) {
    HarmonicsSamples = HarmonicsSamples + StatsHarmonics[i][0] + StatsHarmonics[i][1];
    HarmonicsIn = HarmonicsIn + StatsHarmonics[i][1];
  }
  HarmonicsIn = (HarmonicsIn / HarmonicsSamples) * 100.0;
  $('#HarmonicsInRange').css('width', (HarmonicsIn - 90) * 10 +'%');
  $('#HarmonicsInRangeText').html(Round2(HarmonicsIn) + '% of time in range');
}
function AjaxGetEvents(selected_year) {
  var To = new Date(Date.UTC(selected_year, 11, 31, 23, 59, 59, 999)).getTime();
  $.ajax({ url: "getEvents",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 31536000000, To: To, LastEvent: 0, Node: 0 }),
           success: function(result) {
             var Dip = 0;
             var Short = 0;
             var Long = 0;
             for(i = 0; i < result['Starttime'].length; i++) {
               var Duration = result['Duration'][i] / Nodes['Devices'][Node]['SampleFreq'];
               var Percent = result['Percentage'][i];
               if((Duration < 60) && ((Percent < -15) || (Percent > -10))) Dip++;
               if((Duration < 180) && (Percent < -99)) Short++;
               if((Duration > 180) && (Percent < -99)) Short++;
             }
             var DipProgress = Dip * 100 / 1000;
             $('#DipEvents').css('width', DipProgress  +'%');
             $('#DipEventsText').text(Dip + ' events');
             if(Dip > 1000) $('#DipEvents').css('background-color', '#d9534f"');
             var ShortProgress = Short * 100 / 100;
             $('#ShortEvents').css('width', ShortProgress  +'%');
             $('#ShortEventsText').text(Short + ' events');
             if(Short > 100) $('#ShortEvents').css('background-color', '#d9534f"');
             var LongProgress = Long * 100 / 50;
             $('#LongEvents').css('width', LongProgress  +'%');
             $('#LongEventsText').text(Long + ' events');
             if(Long > 100) $('#LongEvents').css('background-color', '#d9534f"');  
             GetVoltageData(selected_year);
           }
         });  
}
function GetVoltageData(selected_year) {
  $('#YearSelector button').removeClass('active');
  $('#btnSelYear' + selected_year).addClass('active');
  var To = new Date(Date.UTC(selected_year, 11, 31, 23, 59, 59, 999)).getTime();
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 31536000000, To: To, Aggreg: "1h", LastSample: 0, Series: 'stat_frequency, stat_voltage, stat_thd, stat_harmonics', Node: Node }),
           success: function(result) {
             $('#ProgressYear').css('width', ((result['Sampletime'][result['Sampletime'].length - 1] - new Date(Date.UTC(selected_year, 0, 1, 0, 0, 0, 0)).getTime()) * 100 / 31536000000)  +'%');
             var Frequency = [];
             var Voltage = [];
             for(j = 0; j < Frequencies.length - 1; j++) Frequency.push(0);
             for(j = 0; j < Voltages.length - 1; j++) Voltage.push(0);
             WeekStats = [];
             for(i = 0; i < result['Sampletime'].length - 1; i++) {
               var Week = GetWeekOfYear(result['Sampletime'][i]);
               if(WeekStats[Week] === undefined) {
                 WeekStats[Week] = [];
                 WeekStats[Week].Start = GetDateOfISOWeek(Week, selected_year);
                 WeekStats[Week].End = new Date(WeekStats[Week].Start.getFullYear(), WeekStats[Week].Start.getMonth(), WeekStats[Week].Start.getDate() + 6);
                 WeekStats[Week].Frequency = [];
                 for(j = 0; j < Frequencies.length - 1; j++) WeekStats[Week].Frequency.push(0);
                 WeekStats[Week].Voltage = [];
                 for(j = 0; j < Voltages.length - 1; j++) WeekStats[Week].Voltage.push(0);
                 WeekStats[Week].THD = [];
                 WeekStats[Week].Harmonics = [];
               }
               for(j = 0; j < Frequencies.length - 1; j++) {
                 WeekStats[Week].Frequency[j] = WeekStats[Week].Frequency[j] + result['stat_frequency'][i][j];
                 Frequency[j] = Frequency[j] +  result['stat_frequency'][i][j];
               }
               for(j = 0; j < Voltages.length - 1; j++) {
                 WeekStats[Week].Voltage[j] = WeekStats[Week].Voltage[j] + result['stat_voltage'][i][j];
                 Voltage[j] = Voltage[j] +  result['stat_voltage'][i][j];
               }
             }
             var VoltageSamples = 0;
             for(i = 0; i < Voltage.length; i++) VoltageSamples += Voltage[i];
             
             
             
             GenerateYearFreqGraph(Frequency);
             GenerateYearVoltGraph(Voltage);
             GenerateYearTHDGraph(THD);
             return;
             //             
             //             
             //          
//             FrequencySamples = 0;
//             FrequencyIn = 0;
            for(i = 0; i < Frequencies.length - 1; i++) Frequency.push([i, 0]);
            for(i = 0; i < StatsFrequency.length; i++) {
              for(j = 0; j < Frequencies.length - 1; j++) {
                Frequency[j][1] = Frequency[j][1] + StatsFrequency[i][j];
                FrequencySamples = FrequencySamples + StatsFrequency[i][j];
                if((j > 4) && (j < 36)) FrequencyIn = FrequencyIn + StatsFrequency[i][j];
              }
            }
             
             
             
             
             $('#WeekList').html('');
  for(i = 1; ; i++) {
    var startDate = GetDateOfISOWeek(i, selected_year);
    var endDate = new Date(startDate.getFullYear(), startDate.getMonth(), startDate.getDate() + 6);
    if(endDate.getFullYear() !== selected_year) break;
//               var WeekDay = (WeekDayArray[i] !== undefined) ? WeekDayArray[i] : 0;
//               var WeekEnd = (WeekEndArray[i] !== undefined) ? WeekEndArray[i] : 0;
               $('#WeekList').append('<button type="button" class="btn btn-default btn-xs" id="btnWeek_' + i + '" data-toggle="tooltip" title="From ' + startDate.toLocaleDateString() + " to " + endDate.toLocaleDateString() + '">' + i + '<br/><i class="fa fa-warning" style="color: #d9534f"></i></button>');
  }
             
             
             
//             GenerateAnualGraphs();
//             GenerateWeekGraphs();
             ButtonClick();
             StopSpinner(Spinner, 'EN50160Spinner');
           }
         });
}
function GetDataRanges() {
  ShowSpinner(Spinner, 'EN50160Spinner');
  $.ajax({ url: "getSamplesRange",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Aggreg: '1h', Node: Node}),
           success: function(result) {
             for(year = new Date(result['From']).getFullYear(); year <= new Date(result['To']).getFullYear(); year++) $('#YearSelector').append('<button type="button" class="btn btn-default btn-xs" id="btnSelYear' + year + '" onclick="AjaxGetEvents(' + year + ')">' + year + '</button>');
             AjaxGetEvents(new Date(result['To']).getFullYear());
           }
  }); 
}
function ButtonClick(event) {
  if(this.id === 'btnShowTotals')   localStorage.setItem('EN50160View', 'Totals');
  if(this.id === 'btnShowWeeks')    localStorage.setItem('EN50160View', 'Weeks');
  $('#ShowAnual').addClass('hidden');
  $('#ShowWeeks').addClass('hidden');
  $('#btnShowTotals').removeClass('active');
  $('#btnShowWeeks').removeClass('active');
  switch(localStorage.getItem('EN50160View')) {
    case 'Weeks' : 
      $('#btnShowWeeks').addClass('active');
      $('#ShowWeeks').removeClass('hidden');
      break;
    default :
      $('#btnShowTotals').addClass('active');
      $('#ShowAnual').removeClass('hidden');
      break;
  } 
}
$('#btnShowTotals').click(ButtonClick);
$('#btnShowWeeks').click(ButtonClick);

function UpdatePage() {
  $('#GridNominalFreq').text(Nodes['NominalFreq']);
  $('#GridNominalVoltage').text(Nodes['NominalVoltage']);
  GetDataRanges(); 
}