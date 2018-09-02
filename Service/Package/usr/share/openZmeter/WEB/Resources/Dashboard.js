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

var VoltageChart = null, VoltageData = [], CurrentChart = null, CurrentData = [], PowerChart = null, PowerData = [], PowerChartUnits = '', FrequencyChart = null, FrequencyData = [];
function PeriodView_OnSuccess(result) {
  VoltageData = GraphDataAppend(result, 'rms_v', VoltageData);
  $('#labelVoltage').html(Array_Round2(result['rms_v'][result['rms_v'].length - 1], '&nbsp;', ColorArray($('#VoltagePanel').css('border-color'), 3)));
  $('#labelVoltageAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  if(VoltageChart === null) {
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"), VoltageData,
      { strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: false, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {},
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(SeriesNames.Voltage()), colors: ColorArray($('#VoltagePanel').css('border-color'), 3), dateWindow: [result['From'], result['To']],
        underlayCallback: function(canvas, area, g) {
          var NV = Analyzers[Analyzer].NominalVoltage, Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
          GraphHBackgroundRanges([[NV * 1.1, NV * 1.07, Yellow], [NV * 0.93, NV * 0.9, Yellow], [Infinity, NV * 1.1, Red], [NV * 0.9, -Infinity, Red]], g, canvas);
        },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">V</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: MainScrollable.XAxis
        }
      }
    );
    new GraphMaximizer('Voltage', VoltageChart, '#btnMaximizeVoltage');
  } else {
    VoltageChart.updateOptions({ file: VoltageData, dateWindow: [result['From'], result['To']]});
  }
  for(n = 0; n < SeriesNames.Voltage().length; n++) {
    AddEvents(result, SeriesNames.Voltage()[n], VoltageChart);
  }
  VoltageSeries();
  CurrentData = GraphDataAppend(result, 'rms_i', CurrentData);
  $('#labelCurrent').html(Array_Round2(result['rms_i'][result['rms_i'].length - 1], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 4)));
  $('#CurrentAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  if(CurrentChart == null) {
    CurrentChart = new Dygraph(document.getElementById("CurrentChart"), CurrentData,
      { strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: true, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {}, 
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(SeriesNames.Current()), colors: ColorArray($('#CurrentPanel').css('border-color'), 4), dateWindow: [result['From'], result['To']],
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
    new GraphMaximizer('Current', CurrentChart, '#btnMaximizeCurrent');
  } else {
    CurrentChart.updateOptions({'file': CurrentData, dateWindow: [result['From'], result['To']]});
  }
  CurrentSeries();
  PowerData = GraphDataAppend(result, 'active_power', PowerData);
  PowerChartUnits = GetBestUnits(ArrayGraphMax(PowerData));
  $('#labelPowerUnits').text(PowerChartUnits + 'W');
  $('#labelPower').html(Array_Round2(ScaleArrayToUnits(result['active_power'][result['active_power'].length - 1], PowerChartUnits), '&nbsp;&nbsp;', ColorArray($('#PowerPanel').css('border-color'), 3)));
  $('#PowerAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  if(PowerChart == null) {
    PowerChart = new Dygraph(document.getElementById("PowerChart"), PowerData, 
      { strokeWidth: 0.5, stepPlot: true, stackedGraph: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: true, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {},
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(SeriesNames.ActivePower()), colors: ColorArray($('#PowerPanel').css('border-color'), 3), dateWindow: [result['From'], result['To']],
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
    new GraphMaximizer('Active power', PowerChart, '#btnMaximizePower');
  } else {
    PowerChart.updateOptions({file: PowerData, dateWindow: [result['From'], result['To']]});
  }
  PowerSeries();
  FrequencyData = GraphDataAppend(result, 'freq', FrequencyData);
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
    new GraphMaximizer('Frequency', FrequencyChart, '#btnMaximizeFrequency');
    Dygraph.synchronize([VoltageChart, CurrentChart, PowerChart, FrequencyChart], { range: false, zoom: false } );
  } else {
    FrequencyChart.updateOptions({'file': FrequencyData, valueRange: CalcRanges(FrequencyData, [1]), dateWindow: [result['From'], result['To']]});
  }
}
function VoltageSeries(event) {
  GraphPhaseButtons(VoltageChart, $('#groupVoltagePhases'), 'VisibleDashboardVoltagePhases', false, function(series) { return CalcRanges(VoltageData, series, false); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupVoltagePhases').on('click', 'button', VoltageSeries);
$('#btnExportVoltage').click(function() {
  exportToCsv('Voltage.csv', [['Time'].concat(SeriesNames.Voltage())].concat(VoltageData));
});
function CurrentSeries(event) {
  GraphPhaseButtons(CurrentChart, $('#groupCurrentPhases'), 'VisibleDashboardCurrentPhases', false, function(series) { return CalcRanges(CurrentData, series, false); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupCurrentPhases').on('click', 'button', CurrentSeries);
$('#btnExportCurrent').click(function() {
  exportToCsv('Current.csv', [['Time'].concat(SeriesNames.Current())].concat(CurrentData));
});
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
  exportToCsv('Power.csv', [['Time'].concat(SeriesNames.ActivePower())].concat(PowerData));
});
$('#btnExportFrequency').click(function() {
  exportToCsv('Frequency.csv', ([['Time', 'Frequency']]).concat(FrequencyData));
});
/* Grafico de energia */
var EnergyData = [], EnergyChart = null;
function Scrollable_OnSuccess(result, clear) {
  if(clear) EnergyData = [new Array(SeriesNames.ActiveEnergy().length + 1).fill(0)];
  EnergyData = GraphDataAppend(result, 'active_energy', EnergyData);
  if(EnergyChart == null) {
    EnergyChart = new Dygraph(document.getElementById("EnergyChart"), EnergyData,
      { strokeWidth: 0.5, stackedGraph: true, stepPlot: true, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], plotter: barChartNoSpacedPlotter,
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(SeriesNames.ActiveEnergy()), dateWindow: [result['From'], result['To']],
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },
        underlayCallback: function(context, area, dygraph) { GraphHightlightWeekends("#EEE", context, area, dygraph); }, colors: ColorArray($('#EnergyPanel').css('border-color'), 3),
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Units = GetBestUnits(x); return Round2(ScaleToUnits(x, Units)) + ' <em class="x-small">' + Units + 'Wh</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: MainScrollable.XAxis
          }
        }
    );
    $(window).bind('resize', EnergyResizeEvent);
    window.dispatchEvent(new Event('resize'));
    new GraphMaximizer('Active energy', EnergyChart, '#btnMaximizeEnergy', EnergyResizeEvent);
  } else {
    EnergyChart.updateOptions( { file: EnergyData, dateWindow: [result['From'], result['To']] } );
  }
  EnergySeries();
}
function EnergyResizeEvent() {
  var Height = $('#EnergyBox').height() + $('#BudgetBox').height();
  if(EnergyChart != null) EnergyChart.resize('100%', Height + 12);
}
function EnergySeries(event) {
  GraphPhaseButtons(EnergyChart, $('#groupEnergyPhases'), 'VisibleDashboardEnergyPhases', false, function(series) { return CalcRanges(EnergyData, series, true, false, true); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupEnergyPhases').on('click', 'button', EnergySeries);
$('#btnExportEnergy').click(function() {
  exportToCsv("ActiveEnergy_" + Node + '_' + LastEnergySample + '.csv', [['Time'].concat(ActiveEnergyNames)].concat(EnergyData));
});

/* Contadores de consumo */
var EnergyCountersTimer  = null;
function Scrollable_UpdateCounters(change) {
  if(EnergyCountersTimer !== null) clearInterval(EnergyCountersTimer);
  EnergyCountersTimer = setTimeout(Scrollable_UpdateCounters, 60000);
  var Times = GetCountersTimePeriod();
  if(change === true) {
    $('#labelEnergyPrimary').text(Times.PrimaryText);
    $('#labelEnergyPrimaryValue').text('Calculating...');
    $('#labelEnergyPrimaryUnits').text('');
    $('#labelEnergyPrimaryPhases').text('');
    $('#labelEnergySecondary').text(Times.SecondaryText);
  }
  Ajax.GetSeriesAggreg(Times.PrimaryPeriod, Times.PrimaryTo, Times.PreferedAggreg, 'sum(active_energy)', 
    function(result) {
      var Sum = ArraySum(result['sum_active_energy']);
      var Units = GetBestUnits(Sum);
      $('#labelEnergyPrimaryValue').html(Round2(ScaleToUnits(Sum, Units)))
      $('#labelEnergyPrimaryUnits').text(Units + 'Wh');
      if(result['sum_active_energy'].length > 1) $('#labelEnergyPrimaryPhases').html('By phases:&nbsp;&nbsp;' + Array_Round2(ScaleArrayToUnits(result['sum_active_energy'], Units), '&nbsp;&nbsp;') + '&nbsp;&nbsp;' + Units + 'Wh');
      window.dispatchEvent(new Event('resize'));
    }
  );
  if(Times.SecondaryTo !== undefined) {
    $('#labelEnergySecondaryValue').text('Calculating...');
    Ajax.GetSeriesAggreg(Times.SecondaryPeriod, Times.SecondaryTo, Times.PreferedAggreg, 'sum(active_energy)', 
      function(result) {
        var Sum = ArraySum(result['sum_active_energy']);
        var Units = GetBestUnits(Sum);
        var Text = Round2(ScaleToUnits(Sum, Units)) + '&nbsp;' + Units + 'Wh';
        if(result['sum_active_energy'].length > 1) Text += '(' + Array_Round2(ScaleArrayToUnits(result['sum_active_energy'], Units), '&nbsp;&nbsp;') + ')';
        $('#labelEnergySecondaryValue').html(Text);
        window.dispatchEvent(new Event('resize'));
      }
    );
  } else {
    $('#labelEnergySecondaryValue').text('');
  }
//  GetSeriesAggreg(Times.PrevPeriod, Times.PrevEnd, Times.PreferedAggreg, 'sum(active_energy)', EnergyUpdateCountersPrevs);
//  $.ajax({ url: "getBudget",
//           timeout: 20000,
//           type: "POST",
//           dataType: "json",
//           contentType: 'application/json',
//           data: JSON.stringify({ Period: Period, To: Current, Analyzer: Analyzers[Analyzer].ID }),
//           success: EnergyUpdateCountersBudget
//  });
}

function EnergyUpdateCountersBudget(result) {
  var date = new Date(result['budget_lastcalc'] * 1000);
  $('#budget_lastcalc').text(date.toLocaleDateString() + ' - ' + date.toLocaleTimeString());
  
  Round2Labels(result['current_budget'], $('#budget_integer'), $('#budget_decimal'));Ar
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
var ReactiveChart = null, ReactiveData = [], PowerFactorChart = null, PowerFactorData = [], PhiChart = null, PhiData = [], THDVChart = null, THDVData = [], THDIChart = null, THDIData = [];
function PeriodViewParams_OnSuccess(result) {
  $('#labelParametersAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  ReactiveData = GraphDataAppend(result, 'reactive_power', ReactiveData);
  PowerFactorData = GraphDataAppend(result, 'power_factor', PowerFactorData);
  PhiData  = GraphDataAppend(result, 'phi', PhiData);
  THDVData = GraphDataAppend(result, 'thd_v', THDVData);
  THDIData = GraphDataAppend(result, 'thd_i', THDIData);
  if(ReactiveChart === null) {
    ReactiveChart    = AppendCubismGraph($('#EvolutionPanel'), ReactiveData,    "Reactive Power", [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return Round2(ScaleToUnits(x, Units)) + '<em>' + Units + 'VAr</em>'; });
    PowerFactorChart = AppendCubismGraph($('#EvolutionPanel'), PowerFactorData, "Power Factor",   [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return Round2(x); });
    PhiChart         = AppendCubismGraph($('#EvolutionPanel'), PhiData,         "Phi",            [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return Round2(Math.abs(x)) + '<em>' + ((x < 0) ? 'º CAP' : (x > 0) ? 'º IND' : 'º') + '</em>'; });
    THDVChart        = AppendCubismGraph($('#EvolutionPanel'), THDVData,        "Voltage THD",    [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return Round2(x) + '<em>%</em>'; });
    THDIChart        = AppendCubismGraph($('#EvolutionPanel'), THDIData,        "Current THD",    [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return Round2(x) + '<em>%</em>'; });
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

var MainScrollable = null;
function UpdatePage() {
  MainScrollable = new Scrollable( {Spinners: ['#EnergySpinner'], Series: 'active_energy', OnSuccess: Scrollable_OnSuccess} );
  Scrollable_UpdateCounters(true);
  new PeriodView( {Spinners: ['#VoltageSpinner', '#CurrentSpinner', '#PowerSpinner', '#FrequencySpinner'], Series: 'rms_v, flag, rms_i, active_power, freq', Period: 7200000, Aggreg: '3s', OnSuccess: PeriodView_OnSuccess} );
  new PeriodView( {Spinners: ['#MainParametersSpinner'], Series: 'reactive_power, power_factor, phi, thd_v, thd_i', Period: 86400000, Aggreg: '1m', OnSuccess: PeriodViewParams_OnSuccess} );
}
