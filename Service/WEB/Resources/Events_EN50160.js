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
var Spinner        = new Spinner(SpinnerOpts);
var Timestamps     = [];
var EventsData     = [];
var StatsFrequency = [];
var FrequencySamples;
var VoltageSamples;
var THDSamples;
var MaxTimestamp;
var StatsVoltage   = [];
var StatsTHD       = [];
var StatsHarmonics = [];
var SelectedYear;
var FrequencyChart = null;
var VoltageChart   = null;
var THDChart   = null;
var Frequencies = [ -Math.INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, Math.INFINITY ];
var Voltages    = [-Math.INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, Math.INFINITY];
var THDs        = [0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, Math.INFINITY];

var LoadWeek;
var LastWeek;
var LastYear;
function SetWeekEvents(result) {
  var Table = $('#weeks_table tbody');
  var VoltagePercent = (result['Samples'] - result['Voltage']) * 100 / result['Samples'];
  var THDPercent = (result['Samples'] - result['THD']) * 100 / result['Samples'];
  var HarmonicsPercent = (result['Samples'] - result['Harmonics']) * 100 / result['Samples'];
  var Row = '<tr><td class="text-center"><strong>' + result['Week'] + "</strong>&nbsp;(" + result['Year'] + ')</td>' +
            '<td class="text-center">' + Round2(result['Samples'] * 100 / 1008) + '%</td>';
  if(result['Samples'] > 0) {
    Row += '<td>' +
           '  <div class="progress text-center" style="margin-bottom:0px; position: relative">' +
           '    <div class="progress-bar" role="progressbar" style="width:' + Math.min(100, VoltagePercent * 20) + '%; background-color: #5cb85c"></div>' +
           '    <span style="top: 2px; left: 0px; position: absolute; width: 100%; mix-blend-mode: multiply">' + Round2(VoltagePercent) + '% of time outside range</span>' +
           '  </div>' +
           '</td>' +
           '<td>' +
           '  <div class="progress text-center" style="margin-bottom:0px; position: relative">' +
           '    <div class="progress-bar" role="progressbar" style="width:' + Math.min(100, THDPercent * 20) + '%; background-color: #5cb85c"></div>' +
           '    <span style="top: 2px; left: 0px; position: absolute; width: 100%; mix-blend-mode: multiply">' + Round2(THDPercent) + '% of time outside range</span>' +
           '  </div>' +
           '</td>' +
           '<td>' +
           '  <div class="progress text-center" style="margin-bottom:0px; position: relative">' +
           '    <div class="progress-bar" role="progressbar" style="width:' + Math.min(100, HarmonicsPercent * 20) + '%; background-color: #5cb85c"></div>' +
           '    <span style="top: 2px; left: 0px; position: absolute; width: 100%; mix-blend-mode: multiply">' + Round2(HarmonicsPercent) + '% of time outside range</span>' +
           '  </div>' +
           '</td>';
  } else {
    Row += '<td colspan="4" class="text-center">No samples for this week</td>';
  }
  Table.append(Row + '</tr>');
  LastWeek = result['Week'];
  LastYear = result['Year'];
  if(result['Week'] === 1) {
    $('#weeks_table tfoot').hide();
  } else if(LoadWeek === 1) {
    $('#RowLoading').addClass('hide');
    $('#RowButton').removeClass('hide');
  } else {
    LoadWeek--;
    GetWeekInfo(result['Year'], LastWeek - 1);
  }
}
function GetWeekInfo(reqyear, reqweek) {
  $('#RowLoading').removeClass('hide');
  $('#RowButton').addClass('hide');
  $.ajax({ url: "events/getWeekEvents",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Year: reqyear, Week: reqweek }),
           success: SetWeekEvents
         });
}

function GetYearInfo(reqyear) {
  if(reqyear === 0) {
    var End = GetEndYearTimestamp() - 365 * 24 * 3600 * 1000;
    var Start = GetYearTimestamp() - 365 * 24 * 3600 * 1000;
  }
  $.ajax({ url: "getEvents",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Node : Node, To: End, Period: End - Start }),
           success: SetYearEvents
         });
}


function SetYearEvents(result) {
  var Dip = 0;
  var Short = 0;
  var Long = 0;
  for(i = 0; i < EventsData.length; i++) {
    var Event = EventsData[i];
    if((Event[1] < 60) && ((Event[4] < -15) || (Event[4] > -10))) Dip++;
    if((Event[1] < 180) && (Event[4] < -99)) Short++;
    if((Event[1] > 180) && (Event[4] < -99)) Short++;
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
}
function GenerateWeekGraphs() {
  
  
}
function GenerateAnualGraphs() {
  var Frequency = [];
  FrequencySamples = 0;
  FrequencyIn = 0;
  for(i = 0; i < Frequencies.length - 1; i++) Frequency.push([i, 0]);
  for(i = 0; i < StatsFrequency.length; i++) {
    for(j = 0; j < Frequencies.length - 1; j++) {
      Frequency[j][1] = Frequency[j][1] + StatsFrequency[i][j];
      FrequencySamples = FrequencySamples + StatsFrequency[i][j];
      if((j > 4) && (j < 36)) FrequencyIn = FrequencyIn + StatsFrequency[i][j];
    }
  }
  if(FrequencyChart === null) {
    FrequencyChart = new Dygraph(document.getElementById("FrequencyChart"),
      Frequency,
      { strokeWidth: 1,
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
                 return Round2(x * 100.0 / FrequencySamples) + ' %';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Math.round(x * 100 / FrequencySamples, 0) + '%</div>';
               }
             },
          x: {  pixelsPerLabel: 30,
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
      }
    );
  } else {
    FrequencyChart.updateOptions({'file': Frequency});
  }
  FrequencyIn  = (FrequencyIn / FrequencySamples) * 100.0;
  if(FrequencyIn < 99.5) {
    $('#FreqWarning').removeClass('hidden');
  } else {
    $('#FreqWarning').addClass('hidden');
  }
  $('#FreqInRange').css('width', (FrequencyIn - 90) * 10 +'%');
  $('#FreqInRangeText').html(Round2(FrequencyIn) + '% of time in range');

  var Voltage = [];
  VoltageSamples = 0;
  VoltageIn = 0;
  for(i = 0; i < Voltages.length - 1; i++) Voltage.push([i, 0]);
  for(i = 0; i < StatsVoltage.length; i++) {
    for(j = 0; j < Voltages.length - 1; j++) {
      Voltage[j][1] = Voltage[j][1] + StatsVoltage[i][j];
      VoltageSamples = VoltageSamples + StatsVoltage[i][j];
      if((j > 2) && (j < 24)) VoltageIn = VoltageIn + StatsVoltage[i][j];
    }
  }
  if(VoltageChart === null) {
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"),
      Voltage,
      { strokeWidth: 1,
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
                 return Round2(x * 100.0 / VoltageSamples) + ' %';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Math.round(x * 100 / VoltageSamples, 0) + '%</div>';
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
      }
    );
  } else {
    VoltageChart.updateOptions({'file': Voltage});
  }
  VoltageIn  = (VoltageIn / VoltageSamples) * 100.0;
  if(VoltageIn < 95) {
    $('#VoltageWarning').removeClass('hidden');
  } else {
    $('#VoltageWarning').addClass('hidden');
  }
  $('#VoltageInRange').css('width', (VoltageIn - 90) * 10 +'%');
  $('#VoltageInRangeText').html(Round2(VoltageIn) + '% of time in range');

  var THD = [];
  THDSamples = 0;
  THDIn = 0;
  for(i = 0; i < THDs.length - 1; i++) THD.push([i, 0]);
  for(i = 0; i < StatsTHD.length; i++) {
    for(j = 0; j < THDs.length - 1; j++) {
      THD[j][1] = THD[j][1] + StatsTHD[i][j];
      THDSamples = THDSamples + StatsTHD[i][j];
      if(j < 16) THDIn = THDIn + StatsTHD[i][j];
    }
  }
  if(THDChart === null) {
    THDChart = new Dygraph(document.getElementById("THDChart"),
     THD,
      { strokeWidth: 1,
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
      }
    );
  } else {
    THDChart.updateOptions({'file': THD});
  }  
  THDIn = (THDIn / THDSamples) * 100.0;
  if(THDIn < 100) {
    $('#THDWarning').removeClass('hidden');
  } else {
    $('#THDWarning').addClass('hidden');
  }
  $('#THDInRange').css('width', (THDIn - 90) * 10 +'%');
  $('#THDInRangeText').html(Round2(THDIn) + '% of time in range');
  
  var HarmonicsSamples = 0;
  var HarmonicsIn = 0;
  for(i = 0; i < StatsHarmonics.length; i++) {
    HarmonicsSamples = HarmonicsSamples + StatsHarmonics[i][0] + StatsHarmonics[i][1];
    HarmonicsIn = HarmonicsIn + StatsHarmonics[i][1];
  }
  HarmonicsIn = (HarmonicsIn / HarmonicsSamples) * 100.0;
  $('#HarmonicsInRange').css('width', (HarmonicsIn - 90) * 10 +'%');
  $('#HarmonicsInRangeText').html(Round2(HarmonicsIn) + '% of time in range');
}
function AjaxGetEvents(selected_year) {
  SelectedYear = selected_year;
  var To = new Date(Date.UTC(selected_year, 11, 31, 23, 59, 59, 999)).getTime();
  $.ajax({ url: "getEvents",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 31536000000, To: To, LastEvent: 0, Node: 0 }),
           success: function(result) {
             if(result['Starttime'] !== null) {
               for(i = 0; i < result['Starttime'].length; i++) EventsData.push([result['Starttime'][i], result['Duration'][i] / Nodes['Devices'][Node]['SampleFreq'], result['Type'][i], result['Voltage'][i], result['Percentage'][i]]);
             }
             SetYearEvents();
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
             Timestamps     = result['Sampletime'];
             StatsFrequency = result['stat_frequency'];
             StatsVoltage   = result['stat_voltage'];
             StatsTHD       = result['stat_thd'];
             StatsHarmonics = result['stat_harmonics'];
             MaxTimestamp = result['Sampletime'][result['Sampletime'].length - 1];
             var From = new Date(Date.UTC(selected_year, 0, 1, 0, 0, 0, 0)).getTime();
             var Elapsed = (MaxTimestamp - From) * 100 / 31536000000;
             $('#ProgressYear').css('width', Elapsed  +'%');
             
             //Creo listado de semanas del año
             
             
             GenerateAnualGraphs();
             GenerateWeekGraphs();
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