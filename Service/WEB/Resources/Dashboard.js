/*
 * Copyright (C) 2015 Pedro Sánchez Alguacil
 *               2017 Eduardo Viciana Gamez
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

/* Grafica de tension */
var VoltageChart        = null;
var LastVoltageSample   = 0;
var VoltageData         = [];
var VoltageSpinner      = new Spinner(SpinnerOpts);
function AjaxGetVoltage() {
  if(LastVoltageSample === 0) ShowSpinner(VoltageSpinner, 'VoltageSpinner');
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 7200000, To: 0, Aggreg: "3s", LastSample: LastVoltageSample, Series: 'rms_v, flag', Node: Node }),
           success: AjaxVoltage,
           complete: function() { setTimeout(AjaxGetVoltage, 5000); }
        });
}
function AjaxVoltage(result) {
  if(result['Sampletime'] === null) return;
  if(LastVoltageSample === 0) StopSpinner(VoltageSpinner, 'VoltageSpinner');
  if(result['Sampletime'][0] <= LastVoltageSample) return;
  for(i = 0; i < result['Sampletime'].length; i++) VoltageData.push([new Date(result['Sampletime'][i])].concat(result['rms_v'][i]));
  while(VoltageData.length > result['MaxSamples']) VoltageData.shift();
  LastVoltageSample = result['Sampletime'][result['Sampletime'].length - 1];
  $('#labelVoltage').html(Array_Round2(result['rms_v'][result['rms_v'].length - 1], '&nbsp;&nbsp;', '', ["#d9534f", "#534fd9", "#4fd953"]));
  $('#labelVoltageAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  if(VoltageChart === null) {
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"),
      VoltageData,
      { strokeWidth: 1, 
        labelsDiv: "VoltageLegend",
        labels: (Nodes['Devices'][Node]['Type'] == '3Phase') ? ['Time', 'Voltage (R)', 'Voltage (S)', 'Voltage (T)'] :['Time', 'Voltage'],
        stepPlot: true,
        drawGapEdgePoints: true,
        connectSeparatedPoints: false,
        fillGraph: false,
        colors: ["#d9534f", "#534fd9", "#4fd953"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        animatedZooms: true,
        rollPeriod: 0,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        dateWindow: [result['From'], result['To']],
        interactionModel: {},
        underlayCallback: function(canvas, area, g) {
          var NominalVoltage = Nodes['NominalVoltage'];
          var YellowLowA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.1);
          var YellowHeightA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.07) - YellowLowA;
          var YellowLowB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.1);
          var YellowHeightB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.07) - YellowLowB;
          var RedLowA = g.toDomYCoord(1000);
          var RedHeightA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.10) - RedLowA;
          var RedLowB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.10);
          var RedHeightB = g.toDomYCoord(0) - RedLowB;
          canvas.fillStyle = "rgba(255, 255, 0, 0.15)";
          canvas.fillRect(0, YellowLowA, 10000, YellowHeightA);
          canvas.fillRect(0, YellowLowB, 10000, YellowHeightB);
          canvas.fillStyle = "rgba(180, 0, 0, 0.15)";
          canvas.fillRect(0, RedLowA, 10000, RedHeightA);
          canvas.fillRect(0, RedLowB, 10000, RedHeightB);
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">V</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -4px">' + x.toLocaleTimeString() + '</div>';
               }
             }
        }
      }
    );
  } else {
    VoltageChart.updateOptions({'file': VoltageData, dateWindow: [result['From'], result['To']]});
  }
  AddEvents(result, "Voltage", VoltageChart);
  SetVoltageVisibleSeries();
}
function SetVoltageVisibleSeries() {
  if(Nodes['Devices'][Node]['Type'] == '3Phase') {
    var ShowR = (localStorage.getItem('ShowDashboardVoltageR') === 'true');
    var ShowS = (localStorage.getItem('ShowDashboardVoltageS') === 'true');
    var ShowT = (localStorage.getItem('ShowDashboardVoltageT') === 'true');
    var VisibleSeries = [ShowR, ShowS, ShowT];
    var Visibles = [];
    if(ShowR) Visibles.push(1);
    if(ShowS) Visibles.push(2);
    if(ShowT) Visibles.push(3);
    VoltageChart.updateOptions({ visibility: VisibleSeries, valueRange: CalcRanges(VoltageData, Visibles, false) });
    if(ShowR === true)
      $('#btnShowVoltageRPhase').addClass('active');
    else    
      $('#btnShowVoltageRPhase').removeClass('active');
    if(ShowS === true)
      $('#btnShowVoltageSPhase').addClass('active');
    else    
      $('#btnShowVoltageSPhase').removeClass('active');
    if(ShowT === true)
      $('#btnShowVoltageTPhase').addClass('active');
    else    
      $('#btnShowVoltageTPhase').removeClass('active');
  } else {
    VoltageChart.updateOptions({ valueRange: CalcRanges(VoltageData, [1], false) });
  }
}
function ChooseVoltageChannels() {
  var ShowR = (localStorage.getItem('ShowDashboardVoltageR') === 'true');
  var ShowS = (localStorage.getItem('ShowDashboardVoltageS') === 'true');
  var ShowT = (localStorage.getItem('ShowDashboardVoltageT') === 'true');
  if(this.innerText === 'R') ShowR = !ShowR;
  if(this.innerText === 'S') ShowS = !ShowS;
  if(this.innerText === 'T') ShowT = !ShowT;
  if((this.innerText === 'R') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowR = true;
  if((this.innerText === 'S') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowS = true;
  if((this.innerText === 'T') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowT = true;
  localStorage.setItem('ShowDashboardVoltageR', ShowR);
  localStorage.setItem('ShowDashboardVoltageS', ShowS);
  localStorage.setItem('ShowDashboardVoltageT', ShowT);
  SetVoltageVisibleSeries();
}
$('#btnShowVoltageRPhase').click(ChooseVoltageChannels);
$('#btnShowVoltageSPhase').click(ChooseVoltageChannels);
$('#btnShowVoltageTPhase').click(ChooseVoltageChannels);
$('#maximizeVoltage').click(function() {
  $('#GraphDialogTitle').text('Voltage');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#VoltageCol');
  DialogElement = $('#VoltageMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(VoltageChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    VoltageChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#btnExportVoltage').click(function() {
  var Data = [];
  var Filename = "Voltage_" + Node + '_' + LastVoltageSample + '.csv';
  Data.push([['Index', 'Voltage']]);
  Data = Data.concat(VoltageData);
  exportToCsv(Filename, Data);
  $(this).removeClass('active');
  $(this).blur();
});

/* Grafica de corriente */
var CurrentChart        = null;
var LastCurrentSample   = 0;
var CurrentData         = [];
var CurrentSpinner      = new Spinner(SpinnerOpts);
function AjaxGetCurrent() {
  if(LastCurrentSample === 0) ShowSpinner(CurrentSpinner, 'CurrentSpinner');
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 7200000, To: 0, Aggreg: "3s", LastSample: LastCurrentSample, Series: 'rms_i', Node: Node }),
           success: AjaxCurrent,
           complete: function() { setTimeout(AjaxGetCurrent, 5000); }
        });
}
function AjaxCurrent(result) {
  if(result['Sampletime'] === null) return;
  if(LastCurrentSample === 0) StopSpinner(CurrentSpinner, 'CurrentSpinner');
  if(result['Sampletime'][0] <= LastCurrentSample) return;
  for(i = 0; i < result['Sampletime'].length; i++) CurrentData.push([new Date(result['Sampletime'][i])].concat(result['rms_i'][i]));
  while(CurrentData.length > result['MaxSamples']) CurrentData.shift();
  LastCurrentSample = result['Sampletime'][result['Sampletime'].length - 1];
  $('#labelCurrent').html(Array_Round2(result['rms_i'][result['rms_i'].length - 1], '&nbsp;&nbsp;', '', ["#428bca", "#42ca81", "#ca428b", "#ca8142"]));
  $('#CurrentAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  if(CurrentChart == null) {
    CurrentChart = new Dygraph(document.getElementById("CurrentChart"),
      CurrentData,
      { strokeWidth: 1,
        labelsDiv: "CurrentLegend",
        labels: (Nodes['Devices'][Node]['Type'] == '3Phase') ? ['Time', 'Current (R)', 'Current (S)', 'Current (T)', 'Current (N)'] :['Time', 'Current'],
        stepPlot: true,
        drawGapEdgePoints: true,
        connectSeparatedPoints: false,
        fillGraph: false,
        colors: ["#428bca", "#42ca81", "#ca428b", "#ca8142"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        animatedZooms: true,
        rollPeriod: 0,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        dateWindow: [result['From'], result['To']],
        interactionModel: {},
        underlayCallback: function(canvas, area, g) {
          var MaxCurrent = Nodes['Devices'][Node]['MaxCurrent'];
          var YellowLowA = g.toDomYCoord(MaxCurrent);
          var YellowHeightA = g.toDomYCoord(MaxCurrent - MaxCurrent * 0.10) - YellowLowA;
          var YellowLowB = g.toDomYCoord(-MaxCurrent + MaxCurrent * 0.10);
          var YellowHeightB = g.toDomYCoord(-MaxCurrent) - YellowLowB;
          var RedLowA = g.toDomYCoord(10000);
          var RedHeightA = g.toDomYCoord(MaxCurrent) - RedLowA;
          var RedLowB = g.toDomYCoord(-MaxCurrent);
          var RedHeightB = g.toDomYCoord(-10000) - RedLowB;
          canvas.fillStyle = "rgba(255, 255, 0, 0.15)";
          canvas.fillRect(0, YellowLowA, 10000, YellowHeightA);
          canvas.fillRect(0, YellowLowB, 10000, YellowHeightB);
          canvas.fillStyle = "rgba(180, 0, 0, 0.15)";
          canvas.fillRect(0, RedLowA, 10000, RedHeightA);
          canvas.fillRect(0, RedLowB, 10000, RedHeightB);
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">A</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -4px">' + x.toLocaleTimeString() + '</div>';
               }
             }
        }
      }
    );
  } else {
    CurrentChart.updateOptions({'file': CurrentData, dateWindow: [result['From'], result['To']]});
  }
  SetCurrentVisibleSeries();
}
function SetCurrentVisibleSeries() {
  if(Nodes['Devices'][Node]['Type'] == '3Phase') {
    var ShowR = (localStorage.getItem('ShowDashboardCurrentR') === 'true');
    var ShowS = (localStorage.getItem('ShowDashboardCurrentS') === 'true');
    var ShowT = (localStorage.getItem('ShowDashboardCurrentT') === 'true');
    var ShowN = (localStorage.getItem('ShowDashboardCurrentN') === 'true');
    var VisibleSeries = [ShowR, ShowS, ShowT, ShowN];
    var Visibles = [];
    if(ShowR) Visibles.push(1);
    if(ShowS) Visibles.push(2);
    if(ShowT) Visibles.push(3);
    if(ShowN) Visibles.push(4);
    CurrentChart.updateOptions({ visibility: VisibleSeries, valueRange: CalcRanges(CurrentData, Visibles, false) });
    if(ShowR === true)
      $('#btnShowCurrentRPhase').addClass('active');
    else    
      $('#btnShowCurrentRPhase').removeClass('active');
    if(ShowS === true)
      $('#btnShowCurrentSPhase').addClass('active');
    else    
      $('#btnShowCurrentSPhase').removeClass('active');
    if(ShowT === true)
      $('#btnShowCurrentTPhase').addClass('active');
    else    
      $('#btnShowCurrentTPhase').removeClass('active');
    if(ShowN === true)
      $('#btnShowCurrentNPhase').addClass('active');
    else    
      $('#btnShowCurrentNPhase').removeClass('active');
  } else {
    CurrentChart.updateOptions({ valueRange: CalcRanges(CurrentData, [1], false) });
  }
}
function ChooseCurrentChannels() {
  var ShowR = (localStorage.getItem('ShowDashboardCurrentR') === 'true');
  var ShowS = (localStorage.getItem('ShowDashboardCurrentS') === 'true');
  var ShowT = (localStorage.getItem('ShowDashboardCurrentT') === 'true');
  var ShowN = (localStorage.getItem('ShowDashboardCurrentN') === 'true');
  if(this.innerText === 'R') ShowR = !ShowR;
  if(this.innerText === 'S') ShowS = !ShowS;
  if(this.innerText === 'T') ShowT = !ShowT;
  if(this.innerText === 'N') ShowN = !ShowN;
  if((this.innerText === 'R') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true) && (ShowN !== true)) ShowR = true;
  if((this.innerText === 'S') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true) && (ShowN !== true)) ShowS = true;
  if((this.innerText === 'T') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true) && (ShowN !== true)) ShowT = true;
  if((this.innerText === 'N') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true) && (ShowN !== true)) ShowN = true;
  localStorage.setItem('ShowDashboardCurrentR', ShowR);
  localStorage.setItem('ShowDashboardCurrentS', ShowS);
  localStorage.setItem('ShowDashboardCurrentT', ShowT);
  localStorage.setItem('ShowDashboardCurrentN', ShowN);
  SetCurrentVisibleSeries();
}
$('#btnShowCurrentRPhase').click(ChooseCurrentChannels);
$('#btnShowCurrentSPhase').click(ChooseCurrentChannels);
$('#btnShowCurrentTPhase').click(ChooseCurrentChannels);
$('#btnShowCurrentNPhase').click(ChooseCurrentChannels);
$('#maximizeCurrent').click(function() {
  $('#GraphDialogTitle').text('Current');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#CurrentCol');
  DialogElement = $('#CurrentMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(CurrentChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    CurrentChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});

/* Grafica de potencia */
var PowerChart          = null;
var LastPowerSample     = 0;
var PowerData           = [];
var PowerChartUnits     = 'W';
var PowerSpinner        = new Spinner(SpinnerOpts);
function AjaxGetPower() {
  if(LastPowerSample === 0) ShowSpinner(PowerSpinner, 'PowerSpinner');
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 7200000, To: 0, Aggreg: "3s", LastSample: LastPowerSample, Series: 'active_power', Node: Node }),
           success: AjaxPower,
           complete: function() { setTimeout(AjaxGetPower, 5000); }
        });
}
function AjaxPower(result) {
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastPowerSample === 0) StopSpinner(PowerSpinner, 'PowerSpinner');
  if(result['Sampletime'][0] <= LastPowerSample) return;
  for(i = 0; i < result['Sampletime'].length; i++) PowerData.push([new Date(result['Sampletime'][i])].concat(result['active_power'][i]));
  while(PowerData.length > result['MaxSamples']) PowerData.shift();
  LastPowerSample = result['Sampletime'][result['Sampletime'].length - 1];
  
  var Power = result['active_power'][result['active_power'].length - 1];
  if(ArrayMax(Power) > 1000) {
    for(i = 0; i < Power.length; i++) Power[i] = Power[i] / 1000;
    $('#labelPowerUnits').text('kW');
  } else {
    $('#labelPowerUnits').text('W');
  }
  $('#labelPower').html(Array_Round2(Power, '&nbsp;&nbsp;', '', ["#f0ad4e", "#ad4ef0", "#4ef0ad"]));
  $('#PowerAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  if(PowerChart == null) {
    PowerChart = new Dygraph(document.getElementById("PowerChart"),
      PowerData,
      { strokeWidth: 1,
        labelsDiv: "PowerLegend",
        labels: (Nodes['Devices'][Node]['Type'] == '3Phase') ? ['Time', 'Power (R)', 'Power (S)', 'Power (T)'] :['Time', 'Power'],
        stepPlot: true,
        stackedGraph: true,
        drawGapEdgePoints: true,
        connectSeparatedPoints: false,
        fillGraph: true,
        colors: ["#f0ad4e", "#ad4ef0", "#4ef0ad"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        animatedZooms: true,
        rollPeriod: 0,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        dateWindow: [result['From'], result['To']],
        interactionModel: {},
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">' + PowerChartUnits + '</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -4px">' + x.toLocaleTimeString() + '</div>';
               }
             }
        }
      }
    );
  } else {
    PowerChart.updateOptions({file: PowerData, dateWindow: [result['From'], result['To']]});
  }
  SetPowerVisibleSeries();
}
function SetPowerVisibleSeries() {
  if(Nodes['Devices'][Node]['Type'] == '3Phase') {
    var ShowR = (localStorage.getItem('ShowDashboardPowerR') === 'true');
    var ShowS = (localStorage.getItem('ShowDashboardPowerS') === 'true');
    var ShowT = (localStorage.getItem('ShowDashboardPowerT') === 'true');
    var VisibleSeries = [ShowR, ShowS, ShowT];
    var Visibles = [];
    if(ShowR) Visibles.push(1);
    if(ShowS) Visibles.push(2);
    if(ShowT) Visibles.push(3);
    PowerChart.updateOptions({ visibility: VisibleSeries, valueRange: CalcRanges(PowerData, Visibles, true, false, true) });
    if(ShowR === true)
      $('#btnShowPowerRPhase').addClass('active');
    else    
      $('#btnShowPowerRPhase').removeClass('active');
    if(ShowS === true)
      $('#btnShowPowerSPhase').addClass('active');
    else    
      $('#btnShowPowerSPhase').removeClass('active');
    if(ShowT === true)
      $('#btnShowPowerTPhase').addClass('active');
    else    
      $('#btnShowPowerTPhase').removeClass('active');
  } else {
    PowerChart.updateOptions({ valueRange: CalcRanges(PowerData, [1], false) });
  }
}
function ChoosePowerChannels() {
  var ShowR = (localStorage.getItem('ShowDashboardPowerR') === 'true');
  var ShowS = (localStorage.getItem('ShowDashboardPowerS') === 'true');
  var ShowT = (localStorage.getItem('ShowDashboardPowerT') === 'true');
  if(this.innerText === 'R') ShowR = !ShowR;
  if(this.innerText === 'S') ShowS = !ShowS;
  if(this.innerText === 'T') ShowT = !ShowT;
  if((this.innerText === 'R') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowR = true;
  if((this.innerText === 'S') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowS = true;
  if((this.innerText === 'T') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowT = true;
  localStorage.setItem('ShowDashboardPowerR', ShowR);
  localStorage.setItem('ShowDashboardPowerS', ShowS);
  localStorage.setItem('ShowDashboardPowerT', ShowT);
  SetPowerVisibleSeries();
}
$('#btnShowPowerRPhase').click(ChoosePowerChannels);
$('#btnShowPowerSPhase').click(ChoosePowerChannels);
$('#btnShowPowerTPhase').click(ChoosePowerChannels);

/* Grafica de frequencia */
var FrequencyChart      = null;
var LastFrequencySample = 0;
var FrequencyData       = [];
var FrequencySpinner    = new Spinner(SpinnerOpts);
function AjaxGetFrequency() {
  if(LastFrequencySample === 0) ShowSpinner(FrequencySpinner, 'FrequencySpinner');
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 7200000, To: 0, Aggreg: "3s", LastSample: LastFrequencySample, Series: 'freq', Node: Node }),
           success: AjaxFrequency,
           complete: function() { setTimeout(AjaxGetFrequency, 5000); }
        });
}
function AjaxFrequency(result) {
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastFrequencySample === 0) StopSpinner(FrequencySpinner, 'FrequencySpinner');
  if(result['Sampletime'][0] <= LastFrequencySample) return;
  for(i = 0; i < result['Sampletime'].length; i++) FrequencyData.push([new Date(result['Sampletime'][i]), result['freq'][i]]);
  while(FrequencyData.length > result['MaxSamples']) FrequencyData.shift();
  LastFrequencySample = result['Sampletime'][result['Sampletime'].length - 1];
  $('#labelFrequency').html(Round2(result['freq'][result['freq'].length - 1]));
  $('#FrequencyAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  if(FrequencyChart == null) {
    FrequencyChart = new Dygraph(document.getElementById("FrequencyChart"),
      FrequencyData,
      { strokeWidth: 1,
        labelsDiv: "FrequencyLegend",
        labels: ["Time", "Frequency"],
        stepPlot: true,
        drawGapEdgePoints: true,
        connectSeparatedPoints: false,
        fillGraph: false,
        colors: ["#5cb85c"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        animatedZooms: true,
        rollPeriod: 0,
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        valueRange: CalcRanges(FrequencyData, [1]),
        dateWindow: [result['From'], result['To']],
        interactionModel: {},
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">Hz</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -4px">' + x.toLocaleTimeString() + '</div>';
               }
             }
        }
      }
    );
  } else {
    FrequencyChart.updateOptions({'file': FrequencyData, valueRange: CalcRanges(FrequencyData, [1]), dateWindow: [result['From'], result['To']]});
  }
  if((VoltageChart != null) && (CurrentChart != null) && (PowerChart != null) && (FrequencyChart != null)) Dygraph.synchronize([VoltageChart, CurrentChart, PowerChart, FrequencyChart], { range: false, zoom: false } );
}

/* Grafico de energia */
var LastEnergySample    = 0;
var EnergyData          = [];
var EnergyChart         = null;
var EnergySpinner       = new Spinner(SpinnerOpts);
var EnergyTimer         = null;
function AjaxGetEnergy() {
  if(EnergyTimer !== null) clearInterval(EnergyTimer);
  $('#btnEnergyNow').addClass('disabled');
  if(LastEnergySample === 0) ShowSpinner(EnergySpinner, 'EnergySpinner');
  var Period = parseInt(localStorage.getItem('GraphPeriod'));
  if(!Period) {
    Period = 86400000;
    localStorage.setItem('GraphPeriod', Period);
  }
  var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
  if(!GraphEnd) {
    GraphEnd = 0;
    localStorage.setItem('GraphEnd', 0);
  } 
  var GraphAggregation = localStorage.getItem('GraphAggregation');
  if(!GraphAggregation || (GetAggregText(GraphAggregation) === false)) {
    GraphAggregation = '10m';
    localStorage.setItem('GraphAggregation', GraphAggregation);
  }
  $.ajax({ url: "getSeries",
    timeout: 20000,
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data: JSON.stringify({ Period: Period, To: GraphEnd, Aggreg: GraphAggregation, LastSample: LastEnergySample, Series: 'active_energy', Node: Node }),
    success: AjaxEnergy,
    complete: function() { 
      $('#btnEnergyNow').removeClass('disabled');
      if(parseInt(localStorage.getItem('GraphEnd')) === 0) EnergyTimer = setTimeout(AjaxGetEnergy, 3000);
    }
 });
}
function AjaxEnergy(result) {
  if(LastEnergySample === 0) {
    StopSpinner(EnergySpinner, 'EnergySpinner');
    EnergyData = [];
  }
  if(result['Sampletime'] !== null) {
    for(i = 0; i < result['Sampletime'].length; i++) EnergyData.push([new Date(result['Sampletime'][i])].concat(result['active_energy'][i]));
    while(EnergyData.length > result['MaxSamples']) EnergyData.shift();
    LastEnergySample = result['Sampletime'][result['Sampletime'].length - 1];
  }
  $('#EnergyAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');  
  if(EnergyChart == null) {
    var VisibleNames = ["Time"];
    if(Nodes['Devices'][Node]['Type'] == '3Phase') {
      VisibleNames = VisibleNames.concat(['Energy (R)', 'Energy (S)', 'Energy (T)']);
    } else {
      VisibleNames.push('Energy');
    }
    EnergyChart = new Dygraph(document.getElementById("EnergyChart"),
      EnergyData,
      { strokeWidth: 1,
        labelsDiv: "EnergyLegend",
        labels: VisibleNames,
        plotter: barChartNoSpacedPlotter,
        stackedGraph: true,
        stepPlot: true,
        fillGraph: true,
        colors: ["#31708f", "#708f31", "#8f3170"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        dateWindow: [result['From'], result['To']],
        zoomCallback: function(minDate, maxDate, yRanges) {
          clearInterval(EnergyTimer);
          $('#btnEnergyNow').removeClass('active');
          localStorage.setItem('GraphEnd', maxDate);
          localStorage.setItem('GraphPeriod', maxDate - minDate);
          LastEnergySample = 0;
          AjaxGetEnergy();
        },
        underlayCallback: function(context, area, dygraph) {
          HightlightWeekends("#EEE", context, area, dygraph);
        },
        interactionModel: {
          mousedown: mouseDownOZM,
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              clearInterval(EnergyTimer);
              $('#btnEnergyNow').removeClass('active');
              localStorage.setItem('GraphEnd', g.dateWindow_[1]);
              localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastEnergySample = 0;
              AjaxGetEnergy();
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            clearInterval(EnergyTimer);
            $('#btnEnergyNow').removeClass('active');
            localStorage.setItem('GraphEnd', g.dateWindow_[1]);
            localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
            LastEnergySample = 0;
            AjaxGetEnergy();
          }
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 if(x > 1000) {
                   return Round2(x / 1000) + ' <em class="x-small">kWh</em>';
                 } else {
                   return Round2(x) + ' <em class="x-small">Wh</em>';
                 }
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return date.toLocaleDateString() + ' - ' + date.toLocaleTimeString();
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px; line-height: 10px">' + x.toLocaleTimeString() + '<br>' + x.toLocaleDateString() + '</div>';
               }
             }
          }
        }
    );
    $(window).bind('resize', EnergyResizeEvent);
    window.dispatchEvent(new Event('resize'));
  } else {
    EnergyChart.updateOptions({file: EnergyData, dateWindow: [result['From'], result['To']]});
  }
  SetEnergyVisibleSeries();
  $('#btnShowEnergyYear').removeClass('active');
  $('#btnShowEnergyMonth').removeClass('active');
  $('#btnShowEnergyWeek').removeClass('active');
  $('#btnShowEnergyDay').removeClass('active');
  if((result['To'] - result['From']) === 86400000) {
    $('#btnShowEnergyDay').addClass('active');
  } else if((result['To'] - result['From']) === 604800000) {
    $('#btnShowEnergyWeek').addClass('active');
  } else if((result['To'] - result['From']) === 2592000000) {
    $('#btnShowEnergyMonth').addClass('active');
  } else if((result['To'] - result['From']) === 31536000000) {
    $('#btnShowEnergyYear').addClass('active');
  }
  var GraphAggregation = localStorage.getItem('GraphAggregation');
  if(GraphAggregation !== result['Aggregation_Source']) {
    $('#aggreg_modal').modal('show');
    GraphAggregation = result['Aggregation_Source'];
  }
  $('#btnEnergy3S').removeClass('active');
  $('#btnEnergy1M').removeClass('active');
  $('#btnEnergy10M').removeClass('active');
  $('#btnEnergy1H').removeClass('active');
  if(GraphAggregation == '3s') {
    $('#btnEnergy3S').addClass('active');
  } else if(GraphAggregation == '1m') {
    $('#btnEnergy1M').addClass('active');
  } else if(GraphAggregation == '10m') {
    $('#btnEnergy10M').addClass('active');
  } else if(GraphAggregation == '1h') {
    $('#btnEnergy1H').addClass('active');
  }
  var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
  if(GraphEnd === 0) {
    $('#btnEnergyNow').addClass('active');
  } else {
    $('#btnEnergyNow').removeClass('active');
  }
  localStorage.setItem('GraphAggregation', GraphAggregation);
}
function EnergyResizeEvent() {
  var Height = $('#EnergyBox').height() + $('#BudgetBox').height();
  if(EnergyChart != null) EnergyChart.resize($('#EnergyCol').width(), Height - 12);
}
function SetEnergyVisibleSeries() {
  if(Nodes['Devices'][Node]['Type'] == '3Phase') {
    var ShowR = (localStorage.getItem('ShowR') === 'true');
    var ShowS = (localStorage.getItem('ShowS') === 'true');
    var ShowT = (localStorage.getItem('ShowT') === 'true');
    var VisibleSeries = [ShowR, ShowS, ShowT];
    var Visibles = [];
    if(ShowR) Visibles.push(1);
    if(ShowS) Visibles.push(2);
    if(ShowT) Visibles.push(3);
    EnergyChart.updateOptions({ visibility: VisibleSeries, valueRange: CalcRanges(EnergyData, Visibles, true, false, true) });
    if(ShowR === true)
      $('#btnShowEnergyRPhase').addClass('active');
    else    
      $('#btnShowEnergyRPhase').removeClass('active');
    if(ShowS === true)
      $('#btnShowEnergySPhase').addClass('active');
    else    
      $('#btnShowEnergySPhase').removeClass('active');
    if(ShowT === true)
      $('#btnShowEnergyTPhase').addClass('active');
    else    
      $('#btnShowEnergyTPhase').removeClass('active');
  } else {
    VoltageChart.updateOptions({ valueRange: CalcRanges(VoltageData, [1], false) });
  }
}
function ChooseEnergyChannels() {
  var ShowR = (localStorage.getItem('ShowR') === 'true');
  var ShowS = (localStorage.getItem('ShowS') === 'true');
  var ShowT = (localStorage.getItem('ShowT') === 'true');
  if(this.innerText === 'R') ShowR = !ShowR;
  if(this.innerText === 'S') ShowS = !ShowS;
  if(this.innerText === 'T') ShowT = !ShowT;
  if((this.innerText === 'R') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowR = true;
  if((this.innerText === 'S') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowS = true;
  if((this.innerText === 'T') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowT = true;
  localStorage.setItem('ShowR', ShowR);
  localStorage.setItem('ShowS', ShowS);
  localStorage.setItem('ShowT', ShowT);
  SetEnergyVisibleSeries();
}
$('#btnShowEnergyRPhase').click(ChooseEnergyChannels);
$('#btnShowEnergySPhase').click(ChooseEnergyChannels);
$('#btnShowEnergyTPhase').click(ChooseEnergyChannels);
$('#btnEnergy3S').click(function() {
  localStorage.setItem('GraphAggregation', '3s');
  LastEnergySample = 0;
  AjaxGetEnergy();
});
$('#btnEnergy1M').click(function() {
  localStorage.setItem('GraphAggregation', '1m');
  LastEnergySample = 0;
  AjaxGetEnergy();
});
$('#btnEnergy10M').click(function() {
  localStorage.setItem('GraphAggregation', '10m');
  LastEnergySample = 0;
  AjaxGetEnergy();
});
$('#btnEnergy1H').click(function() {
  localStorage.setItem('GraphAggregation', '1h');
  LastEnergySample = 0;
  AjaxGetEnergy();
});
$('#btnShowEnergyYear').click(function() {
  localStorage.setItem('GraphPeriod', 31536000000);
  LastEnergySample = 0;
  AjaxGetEnergy();
  localStorage.setItem('CountersPeriod', 'Year');
  AjaxGetEnergyCounters();
});
$('#btnShowEnergyMonth').click(function() {
  localStorage.setItem('GraphPeriod', 2592000000);
  LastEnergySample = 0;
  AjaxGetEnergy();
  localStorage.setItem('CountersPeriod', 'Month');
  AjaxGetEnergyCounters();
});
$('#btnShowEnergyWeek').click(function() {
  localStorage.setItem('GraphPeriod', 604800000);
  LastEnergySample = 0;
  AjaxGetEnergy();
  localStorage.setItem('CountersPeriod', 'Week');
  AjaxGetEnergyCounters();
});
$('#btnShowEnergyDay').click(function() {
  localStorage.setItem('GraphPeriod', 86400000);
  LastEnergySample = 0;
  AjaxGetEnergy();
  localStorage.setItem('CountersPeriod', 'Day');
  AjaxGetEnergyCounters();
});
$('#btnEnergyNow').click(function() {
  var End = parseInt(localStorage.getItem('GraphEnd'));
  clearInterval(EnergyTimer);
  if(End === 0) {
    $('#btnEnergyNow').removeClass('active');
    localStorage.setItem('GraphEnd', EnergyChart.getOption('dateWindow')[1]);
  } else {
    $('#btnEnergyNow').addClass('active');
    localStorage.setItem('GraphEnd', 0);
    LastEnergySample = 0;
    AjaxGetEnergy();
  }
});
$('#btnMaximizeEnergy').click(function() {
  $('#GraphDialogTitle').text('Energy');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#EnergyCol');
  DialogElement = $('#EnergyMaximizeDiv').detach();
  DialogPrevResizeHandler = EnergyResizeEvent;
  DialogResizeHandler = function() {
    EnergyChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
  };
  $(window).unbind('resize', EnergyResizeEvent);
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#btnExportEnergy').click(function() {
  var Data = [];
  var Filename = "Energy_" + LastEnergyTime + '.csv';
  Data.push([['Index', 'Energy']]);
  Data = Data.concat(EnergyData);
  exportToCsv(Filename, Data);
  $(this).blur();
});

/* Contadores de consumo */
var EnergyCountersTimer  = null;
function AjaxGetEnergyCounters() {
  if(EnergyCountersTimer !== null) clearInterval(EnergyCountersTimer);
  EnergyCountersTimer = setTimeout(AjaxGetEnergyCounters, 60000);
  var CountersPeriod = localStorage.getItem('CountersPeriod');
  var Current, Period;
  if(CountersPeriod === 'Week') {
    Period = 86400000 * 7;
    Current = GetWeekTimestamp();
    Aggreg = '10m';
  } else if(CountersPeriod === 'Month') {
    Period = 86400000 * 30;
    Current = GetMonthTimestamp();
    Aggreg = '1h';
  } else if(CountersPeriod === 'Year') {
    Period = 86400000 * 365;
    Current = GetYearTimestamp();
    Aggreg = '1h';
  } else {
    CountersPeriod = 'Day';
    localStorage.setItem('CountersPeriod', CountersPeriod);
    Period = 86400000;
    Current = GetDayTimestamp();
    Aggreg = '1m';
  }
  $.ajax({ url: "getSeriesAggreg",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: "sum(active_energy)", Aggreg: Aggreg, Period: Period, To: Current + Period, Node: Node }),
           success: EnergyUpdateCountersCurrent
  });
  $.ajax({ url: "getSeriesAggreg",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: "sum(active_energy)", Aggreg: Aggreg, Period: Period, To: Current, Node: Node }),
           success: EnergyUpdateCountersPrevs
  });
  $.ajax({ url: "getBudget",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: Period, To: Current, Node: Node }),
           success: EnergyUpdateCountersBudget
  });
}
function EnergyUpdateCountersCurrent(result) {
  EnergyCountersTimer = setTimeout(AjaxGetEnergyCounters, 60000);
  var CountersPeriod = localStorage.getItem('CountersPeriod');
  if(CountersPeriod === 'Week') {
    $('#labelEnergyPeriod').text('This week');
  } else if(CountersPeriod === 'Month') {
    $('#labelEnergyPeriod').text('This month');
  } else if(CountersPeriod === 'Year') {
    $('#labelEnergyPeriod').text('This year');
  } else if(CountersPeriod === 'Day') {
    $('#labelEnergyPeriod').text('Today');
  }
  var Units = 'Wh';
  for(i = 0; i < result['sum_active_energy'].length; i++) {
    if(result['sum_active_energy'][i] > 1000) Units = 'kWh';
  }
  $('#labelEnergyUnits').text(Units);
  if(Units == 'kWh') {
    for(i = 0; i < result['sum_active_energy'].length; i++) result['sum_active_energy'][i] = result['sum_active_energy'][i] / 1000;
  }
  if(Nodes['Devices'][Node]['Type'] === '1Phase') {  
    $('#labelEnergy').html(Round2(result['sum_active_energy'][0]));
  } else {
    $('#labelEnergy').html(Round2(result['sum_active_energy'][0] + result['sum_active_energy'][1] + result['sum_active_energy'][2]));
    $('#labelEnergyPhases').html('<br/>By phase&nbsp;&nbsp;<span style="font-size:150%">' + Array_Round2(result['sum_active_energy'], '&nbsp;&nbsp;') + '</span>&nbsp;&nbsp;' + Units);
  }
  EnergyResizeEvent();
}
function EnergyUpdateCountersPrevs(result) {
  var CountersPeriod = localStorage.getItem('CountersPeriod');
  if(CountersPeriod === 'Week') {
    $('#labelPrevEnergyPeriod').text('Previous week');
  } else if(CountersPeriod === 'Month') {
    $('#labelPrevEnergyPeriod').text('Previous month');
  } else if(CountersPeriod === 'Year') {
    $('#labelPrevEnergyPeriod').text('Previous year');
  } else if(CountersPeriod === 'Day') {
    $('#labelPrevEnergyPeriod').text('Yesterday');
  }
  var Units = 'Wh';
  for(i = 0; i < result['sum_active_energy'].length; i++) {
    if(result['sum_active_energy'][i] > 1000) Units = 'kWh';
  }
  if(Units == 'kWh') {
    for(i = 0; i < result['sum_active_energy'].length; i++) result['sum_active_energy'][i] = result['sum_active_energy'][i] / 1000;
  }
  if(result['sum_active_energy'].length === 1) {
    $('#labelPrevEnergy').html(Round2(result['sum_active_energy'][0])) + '&nbsp;' + Units;
  } else if(result['sum_active_energy'].length === 3) {
    $('#labelPrevEnergy').html(Round2(result['sum_active_energy'][0] + result['sum_active_energy'][1] + result['sum_active_energy'][2]) + '&nbsp;' + Units + 
      '&nbsp;(' + Array_Round2(result['sum_active_energy'], '&nbsp;&nbsp;') + '</span>&nbsp;&nbsp;By phase)');
  } else {
    $('#labelPrevEnergy').html('No data yet!');
  }
  EnergyResizeEvent();
}
function EnergyUpdateCountersBudget(result) {
  

var date = new Date(result['budget_lastcalc'] * 1000);
  $('#budget_lastcalc').text(date.toLocaleDateString() + ' - ' + date.toLocaleTimeString());
  
  Round2Labels(result['current_budget'], $('#budget_integer'), $('#budget_decimal'));
  if(Math.abs(result['current_energy']) < 1000.0) {
    Round2Labels(result['current_energy'], $('#energy_integer'), $('#energy_decimal'));
    $('#EnergyUnits').text('Wh');
  } else {
    Round2Labels(result['current_energy'] / 1000.0, $('#energy_integer'), $('#energy_decimal'));
    $('#EnergyUnits').text('kWh');
  }
  if(Math.abs(result['prev_energy']) < 1000.0) {
    $('#PrevEnergy').text(Round2(result['prev_energy']));
    $('#PrevEnergyUnits').text('Wh');
  } else {
    $('#PrevEnergy').text(Round2(result['prev_energy'] / 1000.0));
    $('#PrevEnergyUnits').text('kWh');
  }
  
  
  
}
/* Cuadro Main parameters */
var EvolutionDatasets   = [[],[],[],[],[],[]];
var EvolutionCharts     = [];
var LastEvolutionSample = 0;
function AjaxGetMainParameters() {
  if(LastEvolutionSample == 0) {
    $('#MainParamPanel').find("*").prop("disabled", true);
    $('#MainParamPanel').css('opacity', '0.6');
    $('#MainParamPanel').append('<div id="SpinnerDivMain" style="position:absolute; top:0; left:0; width:100%; height:100%; z-index:2;"></div>');
    MainParamSpinner.spin(document.getElementById('SpinnerDivMain'));
  }
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 86400000, To: 0, Aggreg: "1m", LastSample: LastEvolutionSample, Series: 'active_power, fundamental_reactive_power, rms_v, rms_i, power_factor, phi', Node: Node }),
           success: AjaxMainParameters
      });
}
function AjaxMainParameters(result) {
  setTimeout(AjaxGetMainParameters, 60000);
  if(LastEvolutionSample === 0) {
    MainParamSpinner.stop();
    $('#SpinnerDivMain').remove();
    $('#MainParamPanel').css('opacity', '1.0');
    $('#MainParamPanel').find("*").prop("disabled", false);
  }
  for(j = 0; j < result['Sampletime'].length; j++) {
    EvolutionDatasets[0].push([new Date(result['Sampletime'][j]), result.active_power[j][0]]);
    EvolutionDatasets[1].push([new Date(result['Sampletime'][j]), result.fundamental_reactive_power[j][0]]);
    EvolutionDatasets[2].push([new Date(result['Sampletime'][j]), result.rms_v[j][0]]);
    EvolutionDatasets[3].push([new Date(result['Sampletime'][j]), result.rms_i[j][0]]);
    EvolutionDatasets[4].push([new Date(result['Sampletime'][j]), result.power_factor[j][0]]);
    EvolutionDatasets[5].push([new Date(result['Sampletime'][j]), result.phi[j][0]]);
    LastEvolutionSample = result['Sampletime'][j];
  }
  for(i = 0; i < 6; i++) {
    while(EvolutionDatasets[i].length > result['Max_Samples']) EvolutionDatasets[i].shift();
  }
  var SerieNames = ['Active power', 'Reactive power', 'Voltage', 'Current', 'Power factor', 'Phi'];
  if(EvolutionCharts.length == 0) {
    for(i = 0; i < 6; i++) {
      var Str = '<div style="position: relative">' +
                '  <div id="EvolutionChart' + i + '" class="chart" style="height: 50px' + ((i == 0) ? '; margin-top: 16px; border-top: 1px solid #999"' : '"') + '></div>' +       
                '  <div style="pointer-events: none; position: absolute; top: -2px; text-align: left; left: 2px" id="EvolutionChartLegend' + i + '"></div>' +                    
                '</div>';
      $("#EvolutionPanel").append(Str);
      EvolutionCharts[i] = new Dygraph(document.getElementById("EvolutionChart" + i),
        EvolutionDatasets[i],
        { strokeWidth: 0.5,
          labels: ["Time", SerieNames[i]],
          colors: ["#d9534f"],
          gridLineColor: "#999",
          gridLinePattern: [4, 4],
          highlightCircleSize: 0,
          interactionModel: { },
          labelsDiv: 'EvolutionChartLegend' + i,
          legend: 'always',
          legendFormatter: function(data) {
            if(data.x == null) {
              return '<span style="font-size: 18px; font-weight: bold">' + data.series[0].label + '</span>';
            } else {
              return '<span style="font-size: 15px; font-weight: bold">' + data.series[0].label + '</span><br/>' +
                      data.xHTML + data.series[0].yHTML;
            }
          },
          xAxisHeight: 0.1,
          xLabelHeight: 1,
          plotter: barChartPlotterCubism,
          dateWindow: [result['From'], result['To']],
          plugins: [
            new Dygraph.Plugins.Crosshair({
              direction: "vertical"
            })
          ],
          axes: {
            y: { drawAxis: false,
                 drawGrid:false,
                 valueFormatter: function (x) {
                   var Units;
                   if(this.attributes_.labels_[0] == SerieNames[0]) {
                     if(x > 1000) {
                       Units = 'kW';
                       x = x / 1000;
                     } else {
                       Units = 'W';
                     }
                   } else if(this.attributes_.labels_[0] == SerieNames[1]) {
                     if(x > 1000) {
                       Units = 'kVAr';
                       x = x / 1000;
                     } else {
                       Units = 'VAr';
                     }
                   } else if(this.attributes_.labels_[0] == SerieNames[2]) {
                     Units = 'V';
                   } else if(this.attributes_.labels_[0] == SerieNames[3]) {
                     Units = 'A';  
                   } else if(this.attributes_.labels_[0] == SerieNames[4]) {
                     Units = '';
                   } else if(this.attributes_.labels_[0] == SerieNames[5]) {
                     if(x < 0) {
                       Units = 'º CAP';
                       x = -x;
                     } else if(x > 0) {
                       Units = 'º IND';
                     } else {
                       Units = 'º';
                     }
                   }
                   return '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + Round2(x) + '<em>' + Units + '</em></div>';
                 }
               },
            x: { valueFormatter: function (x) {
                   var date = new Date(x);
                   return '<div style="font-size: 10px; line-height: 10px">' + date.toLocaleTimeString() + '</div>';
                 },
                 axisLabelFormatter: function(x, i) {
                   if(this.attributes_.labels_[0] == SerieNames[0]) {
                     var date = new Date(x);
                     if((date.getHours() == 0) && (date.getMinutes() == 0) && (date.getSeconds() == 0)) {
                       return '<div style="font-size: 10px; margin-top: -63px; line-height: 10px; font-weight: bold">' + date.toLocaleDateString() + '</div>';
                     } else {
                       return '<div style="font-size: 10px; margin-top: -63px; line-height: 10px">' + date.toLocaleTimeString() + '</div>';                       
                     }
                   } else {
                     return null;
                   }
                 }
            }
          }
        }
      );
    }
    Dygraph.synchronize(EvolutionCharts, { range: false, zoom: false });
  } else {
    for(i = 0; i < 6; i++) {
      EvolutionCharts[i].updateOptions({ file: EvolutionDatasets[i],dateWindow: [result['From'], result['To']] });
    }
  }
}







var DialogElement           = null;
var PrevDialogContainer     = null;
var DialogResizeHandler     = null;
var DialogPrevResizeHandler = null;







var MainParamSpinner    = new Spinner(SpinnerOpts);

function AjaxSuccess5S(result) {
  if(result != null && Object.keys(result).length > 0) {
    if(result['sampletime'][0] <= LastTimestamp) return;
    for(i = 0; i < result['sampletime'].length; i++) {
//      VoltageData.push([new Date(result['sampletime'][i] * 1000), result['rms_v'][i][0]]);
//      CurrentData.push([new Date(result['sampletime'][i] * 1000), result['rms_i'][i][0]]);
//      PowerData.push([new Date(result['sampletime'][i] * 1000), result['active_power'][i][0]]);
//      FrequencyData.push([new Date(result['sampletime'][i] * 1000), result['freq'][i]]);
//      while(VoltageData.length > result['max_samples'])   VoltageData.shift();
//      while(CurrentData.length > result['max_samples'])   CurrentData.shift();
//      while(PowerData.length > result['max_samples'])     PowerData.shift();
//      while(FrequencyData.length > result['max_samples']) FrequencyData.shift();
//      LastTimestamp = result['sampletime'][i];
    }
//    UpdateVoltageChart(result);
//    UpdateCurrentChart(result);
//    UpdatePowerChart(result);
//    UpdateFrequencyChart(result);
    result.lastVoltage = VoltageData[VoltageData.length - 1][1];
    result.lastCurrent = CurrentData[CurrentData.length - 1][1];
    result.lastPower = PowerData[PowerData.length - 1][1];
    result.lastFrequency = FrequencyData[FrequencyData.length - 1][1];
  }
  UpdateHeader(result);
}





function AjaxSuccess60S(result) {

  UpdateHeader(result);
}
$('#aggreg_modal_close').click(function() {
  $('#aggreg_modal').modal('hide');
});



$('#maximizePower').click(function() {
  $('#GraphDialogTitle').text('Active power');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#PowerCol');
  DialogElement = $('#PowerMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(PowerChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    PowerChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#maximizeFrequency').click(function() {
  $('#GraphDialogTitle').text('Frequency');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#FrequencyCol');
  DialogElement = $('#FrequencyMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(FrequencyChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    FrequencyChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#GraphDialogClose').click(function() {
  $(window).unbind('resize', DialogResizeHandler);
  $(window).bind('resize', DialogPrevResizeHandler);
  $(PrevDialogContainer).append(DialogElement);
  $('#GraphDialog').modal('hide');
  window.dispatchEvent(new Event('resize'));
});



function UpdatePage() {
  if(Nodes['Devices'][Node]['Type'] === '3Phase') {
    $('#btnShowEnergyRPhase').removeClass('disabled');
    $('#btnShowEnergySPhase').removeClass('disabled');
    $('#btnShowEnergyTPhase').removeClass('disabled');
    var ShowR = (localStorage.getItem('ShowR') === 'true');
    var ShowS = (localStorage.getItem('ShowS') === 'true');
    var ShowT = (localStorage.getItem('ShowT') === 'true');
    if((ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowR = true;
    localStorage.setItem('ShowR', (ShowR === true)); 
    localStorage.setItem('ShowS', (ShowS === true));
    localStorage.setItem('ShowT', (ShowT === true));
    $('#btnShowVoltageRPhase').removeClass('disabled');
    $('#btnShowVoltageSPhase').removeClass('disabled');
    $('#btnShowVoltageTPhase').removeClass('disabled');
    var ShowDashboardVoltageR = (localStorage.getItem('ShowDashboardVoltageR') === 'true');
    var ShowDashboardVoltageS = (localStorage.getItem('ShowDashboardVoltageS') === 'true');
    var ShowDashboardVoltageT = (localStorage.getItem('ShowDashboardVoltageT') === 'true');
    if((ShowDashboardVoltageR !== true) && (ShowDashboardVoltageS !== true) && (ShowDashboardVoltageT !== true)) ShowDashboardVoltageR = true;
    localStorage.setItem('ShowDashboardVoltageR', (ShowDashboardVoltageR === true)); 
    localStorage.setItem('ShowDashboardVoltageS', (ShowDashboardVoltageS === true));
    localStorage.setItem('ShowDashboardVoltageT', (ShowDashboardVoltageT === true));
    $('#btnShowCurrentRPhase').removeClass('disabled');
    $('#btnShowCurrentSPhase').removeClass('disabled');
    $('#btnShowCurrentTPhase').removeClass('disabled');
    $('#btnShowCurrentNPhase').removeClass('disabled');
    var ShowDashboardCurrentR = (localStorage.getItem('ShowDashboardCurrentR') === 'true');
    var ShowDashboardCurrentS = (localStorage.getItem('ShowDashboardCurrentS') === 'true');
    var ShowDashboardCurrentT = (localStorage.getItem('ShowDashboardCurrentT') === 'true');
    var ShowDashboardCurrentN = (localStorage.getItem('ShowDashboardCurrentN') === 'true');
    if((ShowDashboardCurrentR !== true) && (ShowDashboardCurrentS !== true) && (ShowDashboardCurrentT !== true) && (ShowDashboardCurrentN !== true)) ShowDashboardCurrentR = true;
    localStorage.setItem('ShowDashboardCurrentR', (ShowDashboardCurrentR === true)); 
    localStorage.setItem('ShowDashboardCurrentS', (ShowDashboardCurrentS === true));
    localStorage.setItem('ShowDashboardCurrentT', (ShowDashboardCurrentT === true));
    localStorage.setItem('ShowDashboardCurrentN', (ShowDashboardCurrentN === true));
    $('#btnShowPowerRPhase').removeClass('disabled');
    $('#btnShowPowerSPhase').removeClass('disabled');
    $('#btnShowPowerTPhase').removeClass('disabled');
    var ShowDashboardPowerR = (localStorage.getItem('ShowDashboardPowerR') === 'true');
    var ShowDashboardPowerS = (localStorage.getItem('ShowDashboardPowerS') === 'true');
    var ShowDashboardPowerT = (localStorage.getItem('ShowDashboardPowerT') === 'true');
    if((ShowDashboardPowerR !== true) && (ShowDashboardPowerS !== true) && (ShowDashboardPowerT !== true)) ShowDashboardPowerR = true;
    localStorage.setItem('ShowDashboardPowerR', (ShowDashboardPowerR === true)); 
    localStorage.setItem('ShowDashboardPowerS', (ShowDashboardPowerS === true));
    localStorage.setItem('ShowDashboardPowerT', (ShowDashboardPowerT === true));
  }
  AjaxGetEnergy();
  AjaxGetEnergyCounters();
  AjaxGetVoltage();
  AjaxGetCurrent();
  AjaxGetPower();
  AjaxGetFrequency();
  AjaxGetMainParameters();

//$(document).ready(EnergyGetData(86400, 0, true));


//GetValues60S();
//$(document).ready(GetValues60S, 60000);
//GetValues60SMainParameters();
//$(document).ready(setInterval(GetValues60SMainParameters, 60000));
}