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
var WeekStats      = [];
var FrequencyChart = null;
var VoltageChart   = null;
var THDChart       = null;
var Frequencies    = [ -Math.INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, Math.INFINITY ];
var Voltages       = [-Math.INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, Math.INFINITY];
var THDs           = [0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, Math.INFINITY];

function GenerateFreqGraph(week) {
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
             return Round2(x * 100.0 / WeekStats[week].FrequencySamples) + ' %';
           },
           axisLabelFormatter: function (x) {
             return '<div style="font-size: 10px; margin-top: -2px">' + Math.round(x * 100 / WeekStats[week].FrequencySamples, 0) + '%</div>';
           }
         },
      x: { pixelsPerLabel: 30,
           axisLabelWidth: 30,
           valueFormatter: function (x) {
             if((x < 0) || (x >= Frequencies.length - 1)) return "";
             if(x === 0) {
               return "Less than " + Round2(Analyzers[Analyzer].NominalFreq * (1 + Frequencies[x + 1] / 100.0)) + ' Hz';
             } else if(x == (Frequencies.length - 2)) {
               return "More than " + Round2(Analyzers[Analyzer].NominalFreq * (1 + Frequencies[x] / 100.0)) + ' Hz';
             } else {
               return "From " + Round2(Analyzers[Analyzer].NominalFreq * (1 + Frequencies[x] / 100.0)) + " to " + Round2(Nodes['NominalFreq'] * (1 + Frequencies[x + 1] / 100.0)) + ' Hz';
             }
           },
           axisLabelFormatter: function (x) {
             if((x < 0) || (x >= Frequencies.length - 1)) return "";
             if(x === 0) {
               return '<div style="font-size: 10px; line-height: 10px">≤' + Round2(Analyzers[Analyzer].NominalFreq * (1 + Frequencies[x + 1] / 100.0))+ '</div>';
             } else if(x == (Frequencies.length - 2)) {
               return '<div style="font-size: 10px; line-height: 10px">≥' + Round2(Analyzers[Analyzer].NominalFreq * (1 + Frequencies[x] / 100.0))+ '</div>';
             } else {
               return '<div style="font-size: 10px; line-height: 10px">' + Round2(Analyzers[Analyzer].NominalFreq * (1 + Frequencies[x] / 100.0)) + "<br/>" + Round2(Analyzers[Analyzer].NominalFreq * (1 + Frequencies[x + 1] / 100.0)) + '</div>';
             }
           }
         }
    }
  };
  var In  = (WeekStats[week].FrequencyIN / WeekStats[week].FrequencySamples) * 100.0;
  var ToShow = [];
  for(i = 0; i < WeekStats[week].Frequency.length; i++) ToShow.push([i, WeekStats[week].Frequency[i]]);
  if(In < 99.5) {
    $('#FreqWarning').removeClass('hidden');
  } else {
    $('#FreqWarning').addClass('hidden');
  }
  $('#FreqInRange').css('width', (In - 90) * 10 +'%');
  $('#FreqInRangeText').html(Round2(In) + '% of time in range');
  if(FrequencyChart === null) {
    FrequencyChart = new Dygraph(document.getElementById("FrequencyChart"), ToShow, Options);
  } else {
    Options.file = ToShow;
    FrequencyChart.updateOptions(Options);
  }
}
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
function AjaxGetEvents(selected_year) {
  var From = new Date(selected_year, 0, 1, 0, 0, 0, 0).getTime();
  var To = new Date(selected_year, 11, 31, 23, 59, 59, 999).getTime();
  Ajax.GetEvents(To - From, To, 0, function(result) {
             WeekStats = [];
             if(result['Starttime'] != null) {
               if(WeekStats[0] === undefined) InitWeek(0, new Date(selected_year, 0, 1, 0, 0, 0, 0), new Date(selected_year, 11, 31, 23, 59, 59, 999));
               for(i = 0; i < result['Starttime'].length; i++) {
                 var Week = GetWeekOfYear(result['Starttime'][i]);
                 if(WeekStats[Week] === undefined) {
                   var Start = GetDateOfISOWeek(Week, selected_year);
                   var End = new Date(Start.getFullYear(), Start.getMonth(), Start.getDate() + 6);
                   InitWeek(Week, Start, End);
                 }
                 var Duration = result['Duration'][i] / Nodes['Devices'][Node]['SampleFreq'];
                 var Percent = result['Percentage'][i];
                 if((Duration < 60) && ((Percent < -15) || (Percent > -10))) {
                   WeekStats[Week].Dip = WeekStats[Week].Dip + 1;
                   WeekStats[0].Dip = WeekStats[0].Dip + 1;
                 }
                 if((Duration < 180) && (Percent < -99)) {
                   WeekStats[Week].Short = WeekStats[Week].Short + 1;
                   WeekStats[0].Short = WeekStats[0].Short + 1;
                 }
                 if((Duration > 180) && (Percent < -99)) {
                   WeekStats[Week].Long = WeekStats[Week].Long + 1;
                   WeekStats[0].Long = WeekStats[0].Long + 1;
                 }
               }
             }
             GetStatsData(selected_year);
       
         });  
}
function InitWeek(week, start, end) {
  WeekStats[week] = [];
  WeekStats[week].Start            = start;
  WeekStats[week].End              = end;
  WeekStats[week].Frequency        = [];
  WeekStats[week].FrequencySamples = 0;
  WeekStats[week].FrequencyIN      = 0;
  WeekStats[week].Voltage          = [];
  WeekStats[week].VoltageSamples   = 0;
  WeekStats[week].VoltageIN        = 0;
  WeekStats[week].THD              = [];
  WeekStats[week].THDSamples       = 0;
  WeekStats[week].THDIN            = 0;
  WeekStats[week].HarmonicsSamples = 0;
  WeekStats[week].HarmonicsIN      = 0;
  WeekStats[week].Dip              = 0;
  WeekStats[week].Short            = 0;
  WeekStats[week].Long             = 0;
  for(j = 0; j < Frequencies.length - 1; j++) WeekStats[week].Frequency.push(0);
  for(j = 0; j < Voltages.length - 1; j++) WeekStats[week].Voltage.push(0);
  for(j = 0; j < THDs.length - 1; j++) WeekStats[week].THD.push(0);
}
function GetStatsData(selected_year) {
  $('#YearSelector button').removeClass('active');
  $('#btnSelYear' + selected_year).addClass('active');
  var From = new Date(selected_year, 0, 1, 0, 0, 0, 0).getTime();
  var To = new Date(selected_year, 11, 31, 23, 59, 59, 999).getTime();
  Ajax.GetSeries(To - From, To, '1h', 0, 'stat_frequency, stat_voltage, stat_thd, stat_harmonics', function(result) {
             $('#ProgressYear').css('width', ((result['Sampletime'][result['Sampletime'].length - 1] - new Date(Date.UTC(selected_year, 0, 1, 0, 0, 0, 0)).getTime()) * 100 / 31536000000)  +'%');
             if(WeekStats[0] === undefined) InitWeek(0, new Date(selected_year, 0, 1, 0, 0, 0, 0), new Date(selected_year, 11, 31, 23, 59, 59, 999));
             for(i = 0; i < result['Sampletime'].length - 1; i++) {
               var Week = GetWeekOfYear(result['Sampletime'][i]);
               if(WeekStats[Week] === undefined) {
                 var Start = GetDateOfISOWeek(Week, selected_year);
                 var End = new Date(Start.getFullYear(), Start.getMonth(), Start.getDate() + 6);
                 InitWeek(Week, Start, End);
               }
               for(j = 0; j < Frequencies.length - 1; j++) {
                 WeekStats[Week].Frequency[j] = WeekStats[Week].Frequency[j] + result['stat_frequency'][i][j];
                 WeekStats[0].Frequency[j] = WeekStats[0].Frequency[j] +  result['stat_frequency'][i][j];
                 WeekStats[0].FrequencySamples = WeekStats[0].FrequencySamples + result['stat_frequency'][i][j];
                 WeekStats[Week].FrequencySamples = WeekStats[Week].FrequencySamples + result['stat_frequency'][i][j];
                 if((j > 4) && (j < 36)) {
                   WeekStats[0].FrequencyIN = WeekStats[0].FrequencyIN + result['stat_frequency'][i][j];
                   WeekStats[Week].FrequencyIN = WeekStats[Week].FrequencyIN + result['stat_frequency'][i][j];
                 }
               }
               for(j = 0; j < Voltages.length - 1; j++) {
                 WeekStats[Week].Voltage[j] = WeekStats[Week].Voltage[j] + result['stat_voltage'][i][j];
                 WeekStats[0].Voltage[j] = WeekStats[0].Voltage[j] + result['stat_voltage'][i][j];
                 WeekStats[0].VoltageSamples = WeekStats[0].VoltageSamples + result['stat_voltage'][i][j];
                 WeekStats[Week].VoltageSamples = WeekStats[Week].VoltageSamples + result['stat_voltage'][i][j];
                 if((j > 2) && (j < 24)) {
                   WeekStats[0].VoltageIN = WeekStats[0].VoltageIN + result['stat_voltage'][i][j];
                   WeekStats[Week].VoltageIN = WeekStats[Week].VoltageIN + result['stat_voltage'][i][j];
                 }
               }
               for(j = 0; j < THDs.length - 1; j++) {
                 WeekStats[Week].THD[j] = WeekStats[Week].THD[j] + result['stat_thd'][i][j];
                 WeekStats[0].THD[j] = WeekStats[0].THD[j] + result['stat_thd'][i][j];
                 WeekStats[0].THDSamples = WeekStats[0].THDSamples + result['stat_thd'][i][j];
                 WeekStats[Week].THDSamples = WeekStats[Week].THDSamples + result['stat_thd'][i][j];
                 if(j < 16) {
                   WeekStats[0].THDIN = WeekStats[0].THDIN + result['stat_thd'][i][j];
                   WeekStats[Week].THDIN = WeekStats[Week].THDIN + result['stat_thd'][i][j];
                 }
               }
               WeekStats[Week].HarmonicsSamples = WeekStats[Week].HarmonicsSamples + result['stat_harmonics'][i][0] + result['stat_harmonics'][i][1];
               WeekStats[0].HarmonicsSamples = WeekStats[0].HarmonicsSamples + result['stat_harmonics'][i][0] + result['stat_harmonics'][i][1];
               WeekStats[0].HarmonicsIN = WeekStats[0].HarmonicsIN + result['stat_harmonics'][i][1];
               WeekStats[Week].HarmonicsIN = WeekStats[Week].HarmonicsIN + result['stat_harmonics'][i][1];
             }
             for(var Week in WeekStats) {
               if((WeekStats[Week].FrequencySamples == 0) || (WeekStats[Week].VoltageSamples == 0) || (WeekStats[Week].THDSamples == 0) || (WeekStats[Week].HarmonicsSamples == 0)) continue;
               var InRange = (((WeekStats[Week].FrequencyIN / WeekStats[Week].FrequencySamples) * 100.0 >= 99.5) && 
                              ((WeekStats[Week].VoltageIN / WeekStats[Week].VoltageSamples) * 100.0 >= 95.0) && 
                              ((WeekStats[Week].THDIN / WeekStats[Week].VoltageSamples) * 100.0 >= 100.0) &&
                              ((WeekStats[Week].HarmonicsIN / WeekStats[Week].HarmonicsSamples) * 100.0 >= 100.0));
               $('#WeekList').append('<button type="button" class="btn btn-default btn-xs" id="btnWeek_' + Week + '" onclick="LoadWeek(' + Week + ')" data-toggle="tooltip" title="From ' + WeekStats[Week].Start.toLocaleDateString() + " to " + WeekStats[Week].End.toLocaleDateString() + '">' + ((Week == 0) ? 'Totals' : Week) + (InRange ? '' : ' <i class="fa fa-warning"></i>') + '</button>');
             }
             LoadWeek(0);
             $('#EN50160Spinner').hide();
           
         });
}
function LoadWeek(week) {
  $('#WeekList button').removeClass('active');
  GenerateFreqGraph(week);
  GenerateVoltGraph(week);
  GenerateTHDGraph(week);
  GenerateHarmonicsGraph(week);
  GenerateEventsGraph(week);
  $('#btnWeek_' + week).addClass('active');  
}
function UpdatePage() {
  $('#GridNominalFreq').text(Analyzers[Analyzer].NominalFreq);
  $('#GridNominalVoltage').text(Analyzers[Analyzer].NominalVoltage);
  Ajax.GetSeriesRange('1h',
    function(result) {
      for(year = new Date(result['From']).getFullYear(); year <= new Date(result['To']).getFullYear(); year++) $('#YearSelector').append('<button type="button" class="btn btn-default btn-xs" id="btnSelYear' + year + '" onclick="AjaxGetEvents(' + year + ')">' + year + '</button>');
      AjaxGetEvents(new Date(result['To']).getFullYear());
    }
  );
}