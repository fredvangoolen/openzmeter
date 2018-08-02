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
function AjaxGetVoltage() {
  GetSeries(7200000, 0, "3s", LastVoltageSample, 'rms_v, flag', AjaxVoltage, function() { setTimeout(AjaxGetVoltage, 5000); });
}
function AjaxVoltage(result) {
  if(result['Sampletime'] === null) return;
  if(LastVoltageSample === 0) $('#VoltageSpinner').hide();
  VoltageData = GraphDataAppend(result, 'rms_v', VoltageData);
  LastVoltageSample = LastSampletime(result, LastVoltageSample);
  $('#labelVoltage').html(Array_Round2(result['rms_v'][result['rms_v'].length - 1], '&nbsp;', ColorArray($('#VoltagePanel').css('border-color'), 3)));
  $('#labelVoltageAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  if(VoltageChart === null) {
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"), VoltageData,
      { strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: false, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {},
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(VoltageNames), colors: ColorArray($('#VoltagePanel').css('border-color'), 3), dateWindow: [result['From'], result['To']],
        underlayCallback: function(canvas, area, g) {
          var NV = Analyzers[Analyzer].NominalVoltage, Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
          GraphHBackgroundRanges([[NV * 1.1, NV * 1.07, Yellow], [NV * 0.93, NV * 0.9, Yellow], [Infinity, NV * 1.1, Red], [NV * 0.9, -Infinity, Red]], g, canvas);
        },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">V</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return x.toLocaleTimeString(); }
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
var LastCurrentSample   = 0;
var CurrentData         = [];
function AjaxGetCurrent() {
  GetSeries(7200000, 0, "3s", LastCurrentSample, 'rms_i', AjaxCurrent, function() { setTimeout(AjaxGetCurrent, 5000); });
}
function AjaxCurrent(result) {
  if(result['Sampletime'] === null) return;
  if(LastCurrentSample === 0) $('#CurrentSpinner').hide();
  CurrentData = GraphDataAppend(result, 'rms_i', CurrentData);
  LastCurrentSample = LastSampletime(result, LastCurrentSample);
  $('#labelCurrent').html(Array_Round2(result['rms_i'][result['rms_i'].length - 1], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 4)));
  $('#CurrentAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  if(CurrentChart == null) {
    CurrentChart = new Dygraph(document.getElementById("CurrentChart"), CurrentData,
      { strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: true, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {}, 
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(VoltageNames), colors: ColorArray($('#CurrentPanel').css('border-color'), 4), dateWindow: [result['From'], result['To']],
        underlayCallback: function(canvas, area, g) {
          var MC = Analyzers[Analyzer]['MaxCurrent'], Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
          GraphHBackgroundRanges([[MC * 0.7, MC * 0.6, Yellow], [Infinity, MC * 0.7, Red]], g, canvas);
        },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">A</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return x.toLocaleTimeString(); }
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
function AjaxGetPower() {
  GetSeries(7200000, 0, "3s", LastPowerSample, 'active_power', AjaxPower, function() { setTimeout(AjaxGetPower, 5000); });
}
function AjaxPower(result) {
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastPowerSample === 0) $('#PowerSpinner').hide();
  PowerData = GraphDataAppend(result, 'active_power', PowerData);
  LastPowerSample = LastSampletime(result, LastPowerSample);
  PowerChartUnits = GetBestUnits(ArrayGraphMax(PowerData));
  $('#labelPowerUnits').text(PowerChartUnits + 'W');
  $('#labelPower').html(Array_Round2(ScaleArrayToUnits(result['active_power'][result['active_power'].length - 1], PowerChartUnits), '&nbsp;&nbsp;', ColorArray($('#PowerPanel').css('border-color'), 3)));
  $('#PowerAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  if(PowerChart == null) {
    PowerChart = new Dygraph(document.getElementById("PowerChart"), PowerData, 
      { strokeWidth: 0.5, stepPlot: true, stackedGraph: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: true, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {},
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(ActivePowerNames), colors: ColorArray($('#PowerPanel').css('border-color'), 3), dateWindow: [result['From'], result['To']],
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(ScaleToUnits(x, PowerChartUnits)) + ' <em class="x-small">' + PowerChartUnits + 'W</em>'; },
               axisLabelFormatter: function (x) { return Round2(ScaleToUnits(x, PowerChartUnits)); }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return x.toLocaleTimeString(); }
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
  var Annotations = [];
  var Result = ArrayGraphMinMax(PowerChart, true);
  Annotations.push( { series: Result.VisibleNames[0], xval: Result.MaxTime, shortText: 'MAX', text: 'Active power peak', width: 28, cssClass: 'annotation' });
  Annotations.push( { series: Result.VisibleNames[0], xval: Result.MinTime, shortText: 'MIN', text: 'Active power peak', width: 28, cssClass: 'annotation' });
  PowerChart.setAnnotations(Annotations);
}
$('#groupPowerPhases').on('click', 'button', PowerSeries);
$('#btnExportPower').click(function() {
  exportToCsv("Power_" + Node + '_' + LastPowerSample + '.csv', [['Time'].concat(ActivePowerNames)].concat(PowerData));
});

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

/* Grafica de frequencia */
var FrequencyChart      = null;
var LastFrequencySample = 0;
var FrequencyData       = [];
function AjaxGetFrequency() {
  GetSeries(7200000, 0, "3s", LastFrequencySample, 'freq', AjaxFrequency, function() { setTimeout(AjaxGetFrequency, 5000); });
}
function AjaxFrequency(result) {
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastFrequencySample === 0) $('#FrequencySpinner').hide();
  FrequencyData = GraphDataAppend(result, 'freq', FrequencyData);
  LastFrequencySample = LastSampletime(result, LastFrequencySample);
  $('#labelFrequency').html(Round2(result['freq'][result['freq'].length - 1]));
  $('#FrequencyAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  if(FrequencyChart == null) {
    FrequencyChart = new Dygraph(document.getElementById("FrequencyChart"), FrequencyData,
      { strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: true, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {},
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ["Time", "Frequency"], colors: [$('#FrequencyPanel').css('border-color')], valueRange: CalcRanges(FrequencyData, [1]), dateWindow: [result['From'], result['To']],
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">Hz</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return x.toLocaleTimeString(); }
             }
        }
      }
    );
  } else {
    FrequencyChart.updateOptions({'file': FrequencyData, valueRange: CalcRanges(FrequencyData, [1]), dateWindow: [result['From'], result['To']]});
  }
  if((VoltageChart != null) && (CurrentChart != null) && (PowerChart != null) && (FrequencyChart != null)) Dygraph.synchronize([VoltageChart, CurrentChart, PowerChart, FrequencyChart], { range: false, zoom: false } );
}
$('#btnExportFrequency').click(function() {
  exportToCsv("Frequency_" + Node + '_' + LastFrequencySample + '.csv', ([['DateTime', 'Frequency']]).concat(FrequencyData));
});

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

/* Grafico de energia */
var EnergyData       = [];
var EnergyChart      = null;
var ScrollableSeries = "active_energy";
function Scrollable_ShowSpinner() {
  $('#EnergySpinner').show();
}
function Scrollable_StopSpinner() {
  $('#EnergySpinner').hide();
}
function Scrollable_OnSuccess(result) {
  if(ScrollableLastSample === 0) EnergyData = [Array(ActiveEnergyNames.length + 1).fill(0)];
  EnergyData = GraphDataAppend(result, 'active_energy', EnergyData);
  $('#EnergyAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');  
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
          x: { valueFormatter: function (x) { return new Date(x).toLocaleDateString() + ' - ' + new Date(x).toLocaleTimeString(); },
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
  GraphPhaseButtons(EnergyChart, $('#groupEnergyPhases'), 'VisibleDashboardEnergyPhases', false, function(series) { return CalcRanges(EnergyData, series, true, false, true); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupEnergyPhases').on('click', 'button', EnergySeries);
$('#btnExportEnergy').click(function() {
  exportToCsv("ActiveEnergy_" + Node + '_' + LastEnergySample + '.csv', ((Nodes['Devices'][Node]['Type'] == '3Phase') ? [['DateTime', 'Energy R', 'Energy S', 'Energy T']] : [['DateTime', 'Energy']]).concat(EnergyData));
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

/* Contadores de consumo */
var EnergyCountersTimer  = null;
function Scrollable_UpdateCounters() {
  if(EnergyCountersTimer !== null) clearInterval(EnergyCountersTimer);
  EnergyCountersTimer = setTimeout(Scrollable_UpdateCounters, 60000);
  var Times = GetCountersTimeDuration();
  $('#labelEnergyPeriod').text(Times.CurrentText);
  $('#labelPrevEnergyPeriod').text(Times.PrevText);
  GetSeriesAggreg(Times.CurrentPeriod, Times.CurrentEnd, Times.PreferedAggreg, 'sum(active_energy)', EnergyUpdateCountersCurrent);
  GetSeriesAggreg(Times.PrevPeriod, Times.PrevEnd, Times.PreferedAggreg, 'sum(active_energy)', EnergyUpdateCountersPrevs);
//  $.ajax({ url: "getBudget",
//           timeout: 20000,
//           type: "POST",
//           dataType: "json",
//           contentType: 'application/json',
//           data: JSON.stringify({ Period: Period, To: Current, Analyzer: Analyzers[Analyzer].ID }),
//           success: EnergyUpdateCountersBudget
//  });
}
function EnergyUpdateCountersCurrent(result) {
  EnergyCountersTimer = setTimeout(AjaxGetEnergyCounters, 60000);
  var CountersPeriod = localStorage.getItem('CountersPeriod');
  if(CountersPeriod === 'Week') {
//    $('#labelEnergyPeriod').text('This week');
  } else if(CountersPeriod === 'Month') {
    $('#labelEnergyPeriod').text('This month');
  } else if(CountersPeriod === 'Year') {
    $('#labelEnergyPeriod').text('This year');
  } else if(CountersPeriod === 'Day') {
//    $('#labelEnergyPeriod').text('Today');
  }
  var Units = 'Wh';
  for(i = 0; i < result['sum_active_energy'].length; i++) {
    if(result['sum_active_energy'][i] > 1000) Units = 'kWh';
  }
  $('#labelEnergyUnits').text(Units);
  if(Units == 'kWh') {
    for(i = 0; i < result['sum_active_energy'].length; i++) result['sum_active_energy'][i] = result['sum_active_energy'][i] / 1000;
  }
  if(Analyzers[Analyzer]['Mode'] === '1Phase') {  
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
//    $('#labelPrevEnergyPeriod').text('Previous week');
  } else if(CountersPeriod === 'Month') {
    $('#labelPrevEnergyPeriod').text('Previous month');
  } else if(CountersPeriod === 'Year') {
    $('#labelPrevEnergyPeriod').text('Previous year');
  } else if(CountersPeriod === 'Day') {
//    $('#labelPrevEnergyPeriod').text('Yesterday');
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

/* Parameters */
var ReactiveChart       = null;
var ReactiveData        = [];
var PowerFactorChart    = null;
var PowerFactorData     = [];
var PhiChart            = null;
var PhiData             = [];
var THDVChart           = null;
var THDVData            = [];
var THDIChart           = null;
var THDIData            = [];
var LastEvolutionSample = 0;
function AjaxGetMainParameters() {
  GetSeries(86400000, 0, "1m", LastEvolutionSample, 'reactive_power, power_factor, phi, thd_v, thd_i', AjaxMainParameters, function() { setTimeout(AjaxGetMainParameters, 60000); });
}
function AjaxMainParameters(result) {
  if((result === null) || (result['Sampletime'] === null)) return;
  if(LastEvolutionSample === 0) {
    $('#MainParametersSpinner').hide();
    $('#labelParametersAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  }
  ReactiveData = GraphDataAppend(result, 'reactive_power', ReactiveData);
  PowerFactorData = GraphDataAppend(result, 'power_factor', PowerFactorData);
  PhiData  = GraphDataAppend(result, 'phi', PhiData);
  THDVData = GraphDataAppend(result, 'thd_v', THDVData);
  THDIData = GraphDataAppend(result, 'thd_i', THDIData);
  LastEvolutionSample = LastSampletime(result, LastEvolutionSample);
  if(ReactiveChart === null) {
    ReactiveChart    = AppendCubismGraph($('#EvolutionPanel'), ReactiveData,    "Reactive Power", [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + Round2(ScaleToUnits(x, Units)) + '<em>' + Units + 'VAr</em></div>'; });
    PowerFactorChart = AppendCubismGraph($('#EvolutionPanel'), PowerFactorData, "Power Factor",   [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + Round2(x) + '</div>'; });
    PhiChart         = AppendCubismGraph($('#EvolutionPanel'), PhiData,         "Phi",            [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + Round2(Math.abs(x)) + '<em>' + ((x < 0) ? 'º CAP' : (x > 0) ? 'º IND' : 'º') + '</em></div>'; });
    THDVChart        = AppendCubismGraph($('#EvolutionPanel'), THDVData,        "Voltage THD",    [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + Round2(x) + '<em>%</em></div>'; });
    THDIChart        = AppendCubismGraph($('#EvolutionPanel'), THDIData,        "Current THD",    [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + Round2(x) + '<em>%</em></div>'; });
    Dygraph.synchronize([ReactiveChart, PowerFactorChart, PhiChart, THDVChart, THDIChart], { range: false, zoom: false });
  } else {
    ReactiveChart.updateOptions({ file: ReactiveData, dateWindow: [result['From'], result['To']] });
    PowerFactorChart.updateOptions({ file: PowerFactorData, dateWindow: [result['From'], result['To']] });
    PhiChart.updateOptions({ file: PhiData, dateWindow: [result['From'], result['To']] });
    THDVChart.updateOptions({ file: THDVData, dateWindow: [result['From'], result['To']] });
    THDIChart.updateOptions({ file: THDIData, dateWindow: [result['From'], result['To']] });
  }
  ParametersSeries();
}
function ParametersSeries(event) {
  GraphPhaseButtons(ReactiveChart,    $('#groupParametersPhases'), 'VisibleDashboardParametersPhases', true, function(series) { return CalcRanges(ReactiveData, series, false); }, (event !== undefined) ? event.target : undefined); 
  GraphPhaseButtons(PowerFactorChart, $('#groupParametersPhases'), 'VisibleDashboardParametersPhases', true, function(series) { return CalcRanges(PowerFactorData, series, false); }, undefined); 
  GraphPhaseButtons(PhiChart,         $('#groupParametersPhases'), 'VisibleDashboardParametersPhases', true, function(series) { return CalcRanges(PhiData, series, false); }, undefined); 
  GraphPhaseButtons(THDVChart,        $('#groupParametersPhases'), 'VisibleDashboardParametersPhases', true, function(series) { return CalcRanges(THDVData, series, false); }, undefined); 
  GraphPhaseButtons(THDIChart,        $('#groupParametersPhases'), 'VisibleDashboardParametersPhases', true, function(series) { return CalcRanges(THDIData, series, false); }, undefined); 
}
$('#groupParametersPhases').on('click', 'button', ParametersSeries);

var DialogElement           = null;
var PrevDialogContainer     = null;
var DialogResizeHandler     = null;
var DialogPrevResizeHandler = null;

$('#GraphDialogClose').click(function() {
  $(window).unbind('resize', DialogResizeHandler);
  $(window).bind('resize', DialogPrevResizeHandler);
  $(PrevDialogContainer).append(DialogElement);
  $('#GraphDialog').modal('hide');
  window.dispatchEvent(new Event('resize'));
});

function UpdatePage() {
  GetSeriesScrollable();
  Scrollable_UpdateCounters();
  AjaxGetVoltage();
  AjaxGetCurrent();
  AjaxGetPower();
  AjaxGetFrequency();
  AjaxGetMainParameters();
}//1267
