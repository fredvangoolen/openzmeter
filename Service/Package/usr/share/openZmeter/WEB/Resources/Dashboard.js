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
var VoltageSpinner      = new Spinner(SpinnerOpts);
var LastVoltageSample   = 0;
var VoltageData         = [];
function AjaxGetVoltage() {
  GetSeries(7200000, 0, "3s", LastVoltageSample, 'rms_v, flag', AjaxVoltage, function() { setTimeout(AjaxGetVoltage, 5000); });
}
function AjaxVoltage(result) {
  if(result['Sampletime'] === null) return;
  if(LastVoltageSample === 0) StopSpinner(VoltageSpinner, 'VoltageSpinner');
  if(result['Sampletime'][0] <= LastVoltageSample) return;
  VoltageData = GraphDataAppend(result, 'rms_v', VoltageData);
  LastVoltageSample = LastSampletime(result, LastVoltageSample);
  $('#labelVoltage').html(Array_Round2(result['rms_v'][result['rms_v'].length - 1], '&nbsp;&nbsp;', ColorArray($('#VoltagePanel').css('border-color'), 3)));
  $('#labelVoltageAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  if(VoltageChart === null) {
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"), VoltageData,
      { strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: false, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {},
        labelsDiv: "VoltageLegend", labels: ['Time'].concat(VoltageNames), colors: ColorArray($('#VoltagePanel').css('border-color'), 3), dateWindow: [result['From'], result['To']],
        underlayCallback: function(canvas, area, g) {
          var NV = Nodes[Node]['NominalVoltage'], Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
          GraphHBackgroundRanges([[NV * 1.1, NV * 1.07, Yellow], [NV * 0.93, NV * 0.9, Yellow], [Infinity, NV * 1.1, Red], [NV * 0.9, -Infinity, Red]], g, canvas);
        },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">V</em>'; },
               axisLabelFormatter: function (x) { return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>'; }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return '<div style="font-size: 10px; margin-top: -4px">' + x.toLocaleTimeString() + '</div>'; }
             }
        }
      }
    );
  } else {
    VoltageChart.updateOptions({ file: VoltageData, dateWindow: [result['From'], result['To']]});
  }
  for(n = 0; n < VoltageNames.length; n++) {
    AddEvents(result, VoltageNames[n], VoltageChart);
  }
  VoltageSeries();
}
function VoltageSeries(event) {
  GraphPhaseButtons(VoltageChart, $('#groupVoltagePhases'), 'VisibleDashboardVoltagePhases', false, function(series) { return CalcRanges(VoltageData, series, false); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupVoltagePhases').on('click', 'button', VoltageSeries);
$('#btnExportVoltage').click(function() {
  exportToCsv("Voltage_" + Node + '_' + LastVoltageSample + '.csv', [['Time'].concat(VoltageNames)].concat(VoltageData));
});

$('#btnMaximizeVoltage').click(function() {
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

/* Grafica de corriente */
var CurrentChart        = null;
var CurrentSpinner      = new Spinner(SpinnerOpts);
var LastCurrentSample   = 0;
var CurrentData         = [];
function AjaxGetCurrent() {
  GetSeries(7200000, 0, "3s", LastCurrentSample, 'rms_i', AjaxCurrent, function() { setTimeout(AjaxGetCurrent, 5000); });
}
function AjaxCurrent(result) {
  if(result['Sampletime'] === null) return;
  if(LastCurrentSample === 0) StopSpinner(CurrentSpinner, 'CurrentSpinner');
  if(result['Sampletime'][0] <= LastCurrentSample) return;
  CurrentData = GraphDataAppend(result, 'rms_i', CurrentData);
  LastCurrentSample = LastSampletime(result);
  $('#labelCurrent').html(Array_Round2(result['rms_i'][result['rms_i'].length - 1], '&nbsp;&nbsp;', ["#428bca", "#42ca81", "#ca428b", "#ca8142"]));
  $('#CurrentAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  if(CurrentChart == null) {
    CurrentChart = new Dygraph(document.getElementById("CurrentChart"), CurrentData,
      { strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: true, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {}, 
        labelsDiv: "CurrentLegend", labels: ['Time'].concat(VoltageNames), colors: CurrentGraphColors, dateWindow: [result['From'], result['To']],
        underlayCallback: function(canvas, area, g) {
          var MC = Nodes[Node]['MaxCurrent'], Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
          GraphHBackgroundRanges([[MC * 0.7, MC * 0.6, Yellow], [Infinity, MC * 0.7, Red]], g, canvas);
        },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">A</em>'; },
               axisLabelFormatter: function (x) { return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>'; }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return '<div style="font-size: 10px; margin-top: -4px">' + x.toLocaleTimeString() + '</div>'; }
             }
        }
      }
    );
  } else {
    CurrentChart.updateOptions({'file': CurrentData, dateWindow: [result['From'], result['To']]});
  }
  CurrentSeries();
}
function CurrentSeries(event) {
  GraphPhaseButtons(CurrentChart, $('#groupCurrentPhases'), 'VisibleDashboardCurrentPhases', false, function(series) { return CalcRanges(CurrentData, series, false); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupCurrentPhases').on('click', 'button', CurrentSeries);
$('#btnExportCurrent').click(function() {
  exportToCsv("Current_" + Node + '_' + LastCurrentSample + '.csv', [['Time'].concat(CurrentNames)].concat(CurrentData));
});

$('#btnMaximizeCurrent').click(function() {
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
var PowerChartUnits     = '';
var PowerSpinner        = new Spinner(SpinnerOpts);
function AjaxGetPower() {
  GetSeries(7200000, 0, "3s", LastPowerSample, 'active_power', AjaxPower, function() { setTimeout(AjaxGetPower, 5000); });
}
function AjaxPower(result) {
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastPowerSample === 0) StopSpinner(PowerSpinner, 'PowerSpinner');
  if(result['Sampletime'][0] <= LastPowerSample) return;
  for(i = 0; i < result['Sampletime'].length; i++) PowerData.push([new Date(result['Sampletime'][i])].concat(result['active_power'][i]));
  ShiftArray(PowerData, result['MaxSamples']);
  LastPowerSample = LastSampletime(result);
  PowerChartUnits = GetBestUnits(ArrayGraphMax(PowerData));
  $('#labelPowerUnits').text(PowerChartUnits + 'W');
  $('#labelPower').html(Array_Round2(ScaleArrayToUnits(result['active_power'][result['active_power'].length - 1], PowerChartUnits), '&nbsp;&nbsp;', ["#f0ad4e", "#ad4ef0", "#4ef0ad"]));
  $('#PowerAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  if(PowerChart == null) {
    PowerChart = new Dygraph(document.getElementById("PowerChart"), PowerData, 
      { strokeWidth: 0.5, stepPlot: true, stackedGraph: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: true, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {},
        labelsDiv: "PowerLegend", labels: ['Time'].concat(ActivePowerNames), colors: ActivePowerGraphColors, dateWindow: [result['From'], result['To']],
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(ScaleArrayToUnits(x, PowerChartUnits)) + ' <em class="x-small">' + PowerChartUnits + 'W</em>'; },
               axisLabelFormatter: function (x) { return '<div style="font-size: 10px; margin-top: -2px">' + Round2(ScaleArrayToUnits(x, PowerChartUnits)) + '</div>'; }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return '<div style="font-size: 10px; margin-top: -4px">' + x.toLocaleTimeString() + '</div>'; }
             }
        }
      }
    );
  } else {
    PowerChart.updateOptions({file: PowerData, dateWindow: [result['From'], result['To']]});
  }
  PowerSeries();
}
function PowerSeries(event) {
  GraphPhaseButtons(PowerChart, $('#groupPowerPhases'), 'VisibleDashboardPowerPhases', false, function(series) { return CalcRanges(PowerData, series, true, false, true); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupPowerPhases').on('click', 'button', PowerSeries);
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
  var Serie = 'Power';
  if(Nodes['Devices'][Node]['Type'] == '3Phase') {
    if(ShowT) Serie = 'Power (T)';
    if(ShowS) Serie = 'Power (S)';
    if(ShowR) Serie = 'Power (R)';
  }
  var Annotations = [];
  var MaxFound = [0, -Infinity];
  for(i = 0; i < PowerData.length; i++) {
    var Max = (Nodes['Devices'][Node]['Type'] == '3Phase') ? ((ShowR ? PowerData[i][1] : 0) + (ShowS ? PowerData[i][2] : 0) + (ShowT ? PowerData[i][3] : 0)) : PowerData[i][1];
    if(MaxFound[1] <= Max) MaxFound = [PowerData[i][0], Max];
  }
  Annotations.push({
      series: Serie,
      xval: MaxFound[0].getTime(),
      shortText: 'MAX',
      text: 'Active power peak',
      width: 28,
      cssClass: 'annotation'
    });
  PowerChart.setAnnotations(Annotations);
}

$('#btnMaximizePower').click(function() {
  $('#GraphDialogTitle').text('Power');
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
$('#btnExportPower').click(function() {
  exportToCsv("Power_" + Node + '_' + LastPowerSample + '.csv', ((Nodes['Devices'][Node]['Type'] == '3Phase') ? [['DateTime', 'Power R', 'Power S', 'Power T']] : [['DateTime', 'Power']]).concat(PowerData));
});

/* Grafica de frequencia */
var FrequencyChart      = null;
var LastFrequencySample = 0;
var FrequencyData       = [];
var FrequencySpinner    = new Spinner(SpinnerOpts);
function AjaxGetFrequency() {
  GetSeries(7200000, 0, "3s", LastFrequencySample, 'freq', AjaxFrequency, function() { setTimeout(AjaxGetFrequency, 5000); });
}
function AjaxFrequency(result) {
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastFrequencySample === 0) StopSpinner(FrequencySpinner, 'FrequencySpinner');
  if(result['Sampletime'][0] <= LastFrequencySample) return;
  for(i = 0; i < result['Sampletime'].length; i++) FrequencyData.push([new Date(result['Sampletime'][i]), result['freq'][i]]);
  ShiftArray(FrequencyData, result['MaxSamples']);
  LastFrequencySample = LastSampletime(result);
  $('#labelFrequency').html(Round2(result['freq'][result['freq'].length - 1]));
  $('#FrequencyAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  if(FrequencyChart == null) {
    FrequencyChart = new Dygraph(document.getElementById("FrequencyChart"), FrequencyData,
      { strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#999", gridLinePattern: [4, 4], animatedZooms: true, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {},
        labelsDiv: "FrequencyLegend", labels: ["Time", "Frequency"], colors: ["#5cb85c"], valueRange: CalcRanges(FrequencyData, [1]), dateWindow: [result['From'], result['To']],
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">Hz</em>'; },
               axisLabelFormatter: function (x) { return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>'; }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return '<div style="font-size: 10px; margin-top: -4px">' + x.toLocaleTimeString() + '</div>'; }
             }
        }
      }
    );
  } else {
    FrequencyChart.updateOptions({'file': FrequencyData, valueRange: CalcRanges(FrequencyData, [1]), dateWindow: [result['From'], result['To']]});
  }
  if((VoltageChart != null) && (CurrentChart != null) && (PowerChart != null) && (FrequencyChart != null)) Dygraph.synchronize([VoltageChart, CurrentChart, PowerChart, FrequencyChart], { range: false, zoom: false } );
}
$('#btnMaximizeFrequency').click(function() {
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
$('#btnExportFrequency').click(function() {
  exportToCsv("Frequency_" + Node + '_' + LastFrequencySample + '.csv', ([['DateTime', 'Frequency']]).concat(FrequencyData));
});

/* Grafico de energia */
var EnergyData       = [];
var EnergyChart      = null;
var EnergySpinner    = new Spinner(SpinnerOpts);
var ScrollableSeries = "active_energy";
function Scrollable_ShowSpinner() {
  ShowSpinner(EnergySpinner, 'EnergySpinner');
}
function Scrollable_StopSpinner() {
  StopSpinner(EnergySpinner, 'EnergySpinner');
}
function Scrollable_OnSuccess(result) {
  if(ScrollableLastSample === 0) EnergyData = [Array(ActiveEnergyNames.length + 1).fill(0)];
  EnergyData = GraphDataAppend(result, 'active_energy', EnergyData);
  $('#EnergyAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');  
  if(EnergyChart == null) {
    EnergyChart = new Dygraph(document.getElementById("EnergyChart"), EnergyData,
      { strokeWidth: 0.5, stackedGraph: true, stepPlot: true, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], plotter: barChartNoSpacedPlotter,
        labelsDiv: "EnergyLegend", labels: ['Time'].concat(ActiveEnergyNames), dateWindow: [result['From'], result['To']],
        zoomCallback: Scrollable_Zoom, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: Scrollable_MouseUp, dblclick: Scrollable_DoubleClick },
        underlayCallback: function(context, area, dygraph) { HightlightWeekends("#EEE", context, area, dygraph); }, colors: ColorArray($('#EnergyPanel').css('border-color'), 3),
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Units = GetBestUnits(x); return Round2(ScaleToUnits(x, Units)) + ' <em class="x-small">' + Units + 'Wh</em>'; },
               axisLabelFormatter: function (x) { return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>'; }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleDateString() + ' - ' + date.toLocaleTimeString(); },
               axisLabelFormatter: function (x) { '<div style="font-size: 10px; margin-top: -2px; line-height: 10px">' + new Date(x).toLocaleTimeString() + '<br/>' + new Date(x).toLocaleDateString() + '</div>'; }
             }
          }
        }
    );
    $(window).bind('resize', EnergyResizeEvent);
    window.dispatchEvent(new Event('resize'));
  } else {
    EnergyChart.updateOptions( { file: EnergyData, dateWindow: [result['From'], result['To']] } );
  }
  EnergySeries();
}
function EnergyResizeEvent() {
  var Height = $('#EnergyBox').height() + $('#BudgetBox').height();
  if(EnergyChart != null) EnergyChart.resize($('#EnergyCol').width(), Height - 12);
}
function EnergySeries(event) {
  GraphPhaseButtons(PowerChart, $('#groupEnergyPhases'), 'VisibleDashboardEnergyPhases', false, function(series) { return CalcRanges(EnergyData, series, true, false, true); }, (event !== undefined) ? event.target : undefined); 
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
  exportToCsv("ActiveEnergy_" + Node + '_' + LastEnergySample + '.csv', ((Nodes['Devices'][Node]['Type'] == '3Phase') ? [['DateTime', 'Energy R', 'Energy S', 'Energy T']] : [['DateTime', 'Energy']]).concat(EnergyData));
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
           data: JSON.stringify({ Series: "sum(active_energy)", Aggreg: Aggreg, Period: Period, To: Current + Period, Node: Nodes[Node].ID }),
           success: EnergyUpdateCountersCurrent
  });
  $.ajax({ url: "getSeriesAggreg",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: "sum(active_energy)", Aggreg: Aggreg, Period: Period, To: Current, Node: Nodes[Node].ID }),
           success: EnergyUpdateCountersPrevs
  });
  $.ajax({ url: "getBudget",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: Period, To: Current, Node: Nodes[Node].ID }),
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
  window.dispatchEvent(new Event('resize'));
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
    $('#labelPrevEnergy').html(Round2(result['sum_active_energy'][0]) + '&nbsp;' + Units);
  } else if(result['sum_active_energy'].length === 3) {
    $('#labelPrevEnergy').html(Round2(result['sum_active_energy'][0] + result['sum_active_energy'][1] + result['sum_active_energy'][2]) + '&nbsp;' + Units + 
      '&nbsp;(' + Array_Round2(result['sum_active_energy'], '&nbsp;&nbsp;') + '</span>&nbsp;&nbsp;By phase)');
  } else {
    $('#labelPrevEnergy').html('No data yet!');
  }
  window.dispatchEvent(new Event('resize'));
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
           data: JSON.stringify({ Period: 86400000, To: 0, Aggreg: "1m", LastSample: LastEvolutionSample, Series: 'active_power, fundamental_reactive_power, rms_v, rms_i, power_factor, phi', Node: Nodes[Node].ID }),
           success: AjaxMainParameters,
           complete: function() {
             setTimeout(AjaxGetMainParameters, 60000);
           }
      });
}
function AjaxMainParameters(result) {
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


$('#aggreg_modal_close').click(function() {
  $('#aggreg_modal').modal('hide');
});

$('#GraphDialogClose').click(function() {
  $(window).unbind('resize', DialogResizeHandler);
  $(window).bind('resize', DialogPrevResizeHandler);
  $(PrevDialogContainer).append(DialogElement);
  $('#GraphDialog').modal('hide');
  window.dispatchEvent(new Event('resize'));
});



function UpdatePage() {
  GetSeriesScrollable();
//  AjaxGetEnergy();
  AjaxGetEnergyCounters();
  AjaxGetVoltage();
  AjaxGetCurrent();
  AjaxGetPower();
  AjaxGetFrequency();
  AjaxGetMainParameters();
}//1267
