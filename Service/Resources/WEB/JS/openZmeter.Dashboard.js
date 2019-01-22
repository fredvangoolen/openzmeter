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
  VoltageData = GraphDataAppend(result, 'RMS_V', VoltageData);
  $('#labelVoltage').html(Array_Round2(result['RMS_V'][result['RMS_V'].length - 1], ColorArray($('#VoltagePanel').css('border-color'), 3)));
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
          x: { valueFormatter: function (x) { return new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return x.toLocaleTimeString(); }
             }
        }
      }
    );
    new GraphTools('Voltage', VoltageChart, '#toolbarVoltage', undefined, function() { exportToCsv('Voltage.csv', [['Time'].concat(SeriesNames.Voltage())].concat(VoltageData)); });
  } else {
    VoltageChart.updateOptions({ file: VoltageData, dateWindow: [result['From'], result['To']]});
  }
  for(var n = 0; n < SeriesNames.Voltage().length; n++) AddEvents(result, SeriesNames.Voltage()[n], VoltageChart);
  VoltageSeries();
  CurrentData = GraphDataAppend(result, 'RMS_I', CurrentData);
  $('#labelCurrent').html(Array_Round2(result['RMS_I'][result['RMS_I'].length - 1], ColorArray($('#CurrentPanel').css('border-color'), 4)));
  $('#CurrentAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  if(CurrentChart === null) {
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
    new GraphTools('Current', CurrentChart, '#toolbarCurrent', undefined, function() { exportToCsv('Current.csv', [['Time'].concat(SeriesNames.Current())].concat(CurrentData)); });
  } else {
    CurrentChart.updateOptions({'file': CurrentData, dateWindow: [result['From'], result['To']]});
  }
  CurrentSeries();
  PowerData = GraphDataAppend(result, 'Active_Power', PowerData);
  PowerChartUnits = GetBestUnits(ArrayGraphMax(PowerData));
  $('#labelPowerUnits').text(PowerChartUnits + 'W');
  $('#labelPower').html(Array_Round2(ScaleArrayToUnits(result['Active_Power'][result['Active_Power'].length - 1], PowerChartUnits), ColorArray($('#PowerPanel').css('border-color'), 3)));
  $('#PowerAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  if(PowerChart === null) {
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
    new GraphTools('Active power', PowerChart, '#toolbarPower', undefined, function() { exportToCsv('Active power.csv', [['Time'].concat(SeriesNames.ActivePower())].concat(PowerData)); });
  } else {
    PowerChart.updateOptions({file: PowerData, dateWindow: [result['From'], result['To']]});
  }
  PowerSeries();
  FrequencyData = GraphDataAppend(result, 'Frequency', FrequencyData);
  $('#labelFrequency').html(Array_Round2([result['Frequency'][result['Frequency'].length - 1]], ColorArray($('#FrequencyPanel').css('border-color'), 1)));
  $('#FrequencyAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  if(FrequencyChart === null) {
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
    new GraphTools('Frequency', FrequencyChart, '#toolbarFrequency', undefined, function() { exportToCsv('Frequency.csv', ([['Time', 'Frequency']]).concat(FrequencyData)); });
    Dygraph.synchronize([VoltageChart, CurrentChart, PowerChart, FrequencyChart], { range: false, zoom: false } );
  } else {
    FrequencyChart.updateOptions({'file': FrequencyData, valueRange: CalcRanges(FrequencyData, [1]), dateWindow: [result['From'], result['To']]});
  }
}
function VoltageSeries(event) {
  GraphPhaseButtons(VoltageChart, $('#groupVoltagePhases'), 'DashboardVoltageVisiblePhases', false, function(series) { return CalcRanges(VoltageData, series, false, true); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupVoltagePhases').on('click', 'button', VoltageSeries);
function CurrentSeries(event) {
  GraphPhaseButtons(CurrentChart, $('#groupCurrentPhases'), 'DashboardCurrentVisiblePhases', false, function(series) { return CalcRanges(CurrentData, series, false, true); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupCurrentPhases').on('click', 'button', CurrentSeries);
function PowerSeries(event) {
  GraphPhaseButtons(PowerChart, $('#groupPowerPhases'), 'DashboardPowerVisiblePhases', false, function(series) { return CalcRanges(PowerData, series, true, false, true); }, (event !== undefined) ? event.target : undefined); 
  var Annotations = [];
  var Result = ArrayGraphMinMax(PowerChart, true);
  Annotations.push( { series: Result.VisibleNames[0], xval: Result.MaxTime, shortText: 'MAX', text: 'Active power peak', width: 28, cssClass: 'annotation' });
  Annotations.push( { series: Result.VisibleNames[0], xval: Result.MinTime, shortText: 'MIN', text: 'Active power peak', width: 28, cssClass: 'annotation' });
  PowerChart.setAnnotations(Annotations);
}
$('#groupPowerPhases').on('click', 'button', PowerSeries);
/* Grafico de energia */
var EnergyData = [], EnergyChart = null;
function Scrollable_OnSuccess(result, clear) {
  if(clear) EnergyData = [new Array(SeriesNames.ActiveEnergy().length + 1).fill(0)];
  for(var i = 0; i < result['Active_Power']; i++) {
    for(var n = 0; n < result['Active_Power'][i]; n++) result['Active_Power'][i][n] = result['Active_Power'][i][n] * ((result['Aggregation_Source'] === '1h') ? 1.0 : 0.25);
  }
  EnergyData = GraphDataAppend(result, 'Active_Power', EnergyData);
  if(EnergyChart === null) {
    EnergyChart = new Dygraph(document.getElementById("EnergyChart"), EnergyData,
      { strokeWidth: 0.5, stepPlot: true, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], plotter: barChartNoSpacedPlotter, stackedGraphNaNFill: 'none',
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(SeriesNames.ActiveEnergy()), dateWindow: [result['From'], result['To']],
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },
        underlayCallback: function(context, area, dygraph) { GraphHightlightWeekends("#EEE", context, area, dygraph); }, colors: ColorArray($('#EnergyPanel').css('border-color'), 3), xAxisHeight: 24,
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
    new GraphTools('Active energy', EnergyChart, '#toolbarEnergy', EnergyResizeEvent, function() { exportToCsv('ActiveEnergy.csv', [['Time'].concat(SeriesNames.ActiveEnergy())].concat(EnergyData)); });
  } else {
    EnergyChart.updateOptions( { file: EnergyData, dateWindow: [result['From'], result['To']] } );
  }
  EnergySeries();
}
function EnergyResizeEvent() {
  if(EnergyChart !== null) EnergyChart.resize('100%', $('#EnergyPanel > .panel-body > .row > div:first-child').height());
}
function EnergySeries(event) {
  GraphPhaseButtons(EnergyChart, $('#groupEnergyPhases'), 'VisibleDashboardEnergyPhases', false, function(series) { return CalcRanges(EnergyData, series, true, false, true); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupEnergyPhases').on('click', 'button', EnergySeries);

/* Contadores de consumo */
function Scrollable_UpdateCounters(times, renew) {
  if(renew === true) {
    $('#labelEnergyPrimary').text(times.PrimaryText);
    $('#labelBudgetPrimary').text(times.PrimaryText + ' budget');
    $('#labelEnergyPrimaryValue').text('Calculating...');
    $('#labelEnergyPrimaryUnits #labelEnergyPrimaryPhases, #labelBudgetUnits').text('');
    $('#labelEnergySecondary, #labelBudgetSecondary').text(times.SecondaryText);
    $('#labelEnergySecondaryValue, #labelBudgetSecondaryValue').text((times.SecondaryTo !== undefined) ? 'Calculating...' : '');
    if(times.SecondaryTo !== undefined) {
      Ajax.GetSeriesAggreg(times.SecondaryPeriod, times.SecondaryTo, times.PreferedAggreg, 'sum(active_energy)', 
        function(result) {
          if(result['Samples'] !== 0) {
            var Sum = ArraySum(result['sum_active_energy']);
            var Units = GetBestUnits(Sum);
            var Text = Round2(ScaleToUnits(Sum, Units)) + '&nbsp;' + Units + 'Wh';
            if(result['sum_active_energy'].length > 1) Text += '(' + Array_Round2(ScaleArrayToUnits(result['sum_active_energy'], Units)) + ')';
            $('#labelEnergySecondaryValue').html(Text);
          } else {
            $('#labelEnergySecondaryValue').text('No data');
          }
          window.dispatchEvent(new Event('resize'));
        }
      );
      Ajax.GetPrices(times.SecondaryPeriod, times.SecondaryTo, 0,
        function(result) {
          if(result['Sampletime'].length !== 0) {
            $('#labelBudgetUnits').text('€');
            var Budget = 0.0;
            for(var i = 0; i < result['Sampletime'].length; i++) Budget += result['active_energy'][i] * result['price'][i] / 1000.0;
            $('#labelBudgetSecondaryValue').html(Round2(Budget));
          } else {
            $('#labelBudgetSecondaryValue').text('No data');
          }
        }
      );
    }
  }
  Ajax.GetSeriesAggreg(times.PrimaryPeriod, times.PrimaryTo, times.PreferedAggreg, 'sum(active_energy)', 
    function(result) {
      var Sum = ArraySum(result['sum_active_energy']);
      var Units = GetBestUnits(Sum);
      $('#labelEnergyPrimaryValue').html(Round2(ScaleToUnits(Sum, Units)));
      $('#labelEnergyPrimaryUnits').text(Units + 'Wh');
      if(result['sum_active_energy'].length > 1) $('#labelEnergyPrimaryPhases').html('By phases:&nbsp;' + Array_Round2(ScaleArrayToUnits(result['sum_active_energy'], Units)) + '&nbsp;' + Units + 'Wh');
      window.dispatchEvent(new Event('resize'));
    }
  );
  Ajax.GetPrices(times.PrimaryPeriod, times.PrimaryTo, 0,
    function(result) {
      $('#labelBudgetUnits').text('€');
      var Budget = 0.0;
      for(var i = 0; i < result['Sampletime'].length; i++) Budget += result['active_energy'][i] * result['price'][i] / 1000.0;
      $('#labelBudgetPrimaryValue').html(Round2(Budget));
    }
  );
}
/* Parameters */
var ActiveChart = null, ReactiveChart = null, ActiveData = [], ReactiveData = [], PowerFactorChart = null, PowerFactorData = [], PhiChart = null, PhiData = [], THDVChart = null, THDVData = [], THDIChart = null, THDIData = [];
function PeriodViewParams_OnSuccess(result) {
  $('#labelParametersAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  ActiveData = GraphDataAppend(result, 'Active_Power', ActiveData);
  ReactiveData = GraphDataAppend(result, 'Reactive_Power', ReactiveData);
  PowerFactorData = GraphDataAppend(result, 'Power_Factor', PowerFactorData);
  PhiData  = GraphDataAppend(result, 'Phi', PhiData);
  THDVData = GraphDataAppend(result, 'THD_V', THDVData);
  THDIData = GraphDataAppend(result, 'THD_I', THDIData);
  if(ReactiveChart === null) {
    ActiveChart      = AppendCubismGraph($('#EvolutionPanel'), ActiveData,      "Active Power",   [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return (x === null) ? '~' : (Round2(ScaleToUnits(x, Units)) + '<em>' + Units + 'W</em>'); });
    ReactiveChart    = AppendCubismGraph($('#EvolutionPanel'), ReactiveData,    "Reactive Power", [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return (x === null) ? '~' : (Round2(ScaleToUnits(x, Units)) + '<em>' + Units + 'VAr</em>'); });
    PowerFactorChart = AppendCubismGraph($('#EvolutionPanel'), PowerFactorData, "Power Factor",   [result['From'], result['To']], function (x) { return (x === null) ? '~' : Round2(x); });
    PhiChart         = AppendCubismGraph($('#EvolutionPanel'), PhiData,         "Angle",          [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(Math.abs(x)) + '<em>' + ((x < 0) ? 'º CAP' : (x > 0) ? 'º IND' : 'º') + '</em>'); });
    THDVChart        = AppendCubismGraph($('#EvolutionPanel'), THDVData,        "Voltage THD",    [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(x * 100.0) + '<em>%</em>'); });
    THDIChart        = AppendCubismGraph($('#EvolutionPanel'), THDIData,        "Current THD",    [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(x * 100.0) + '<em>%</em>'); });
    Dygraph.synchronize([ActiveChart, ReactiveChart, PowerFactorChart, PhiChart, THDVChart, THDIChart], { range: false, zoom: false });
  } else {
    ActiveChart.updateOptions({ file: ActiveData, dateWindow: [result['From'], result['To']] });
    ReactiveChart.updateOptions({ file: ReactiveData, dateWindow: [result['From'], result['To']] });
    PowerFactorChart.updateOptions({ file: PowerFactorData, dateWindow: [result['From'], result['To']] });
    PhiChart.updateOptions({ file: PhiData, dateWindow: [result['From'], result['To']] });
    THDVChart.updateOptions({ file: THDVData, dateWindow: [result['From'], result['To']] });
    THDIChart.updateOptions({ file: THDIData, dateWindow: [result['From'], result['To']] });
  }
  ParametersSeries();
}
function ParametersSeries(event) {
  GraphPhaseButtons(ActiveChart,      $('#groupParametersPhases'), 'VisibleDashboardParametersPhases', true, function(series) { var Range = ArrayAbsMax(CalcRanges(ActiveData, series, false)); return [-Range, Range]; }, (event !== undefined) ? event.target : undefined); 
  GraphPhaseButtons(ReactiveChart,    $('#groupParametersPhases'), 'VisibleDashboardParametersPhases', true, function(series) { var Range = ArrayAbsMax(CalcRanges(ReactiveData, series, false)); return [-Range, Range]; }, (event !== undefined) ? event.target : undefined); 
  GraphPhaseButtons(PowerFactorChart, $('#groupParametersPhases'), 'VisibleDashboardParametersPhases', true, function(series) { var Range = ArrayAbsMax(CalcRanges(PowerFactorData, series, false)); return [-Range, Range]; }, undefined); 
  GraphPhaseButtons(PhiChart,         $('#groupParametersPhases'), 'VisibleDashboardParametersPhases', true, function(series) { var Range = ArrayAbsMax(CalcRanges(PhiData, series, false, false, false, true)); return [-Range, Range]; }, undefined); 
  GraphPhaseButtons(THDVChart,        $('#groupParametersPhases'), 'VisibleDashboardParametersPhases', true, function(series) { var Range = ArrayAbsMax(CalcRanges(THDVData, series, false, true, false, true)); return [-Range, Range]; }, undefined); 
  GraphPhaseButtons(THDIChart,        $('#groupParametersPhases'), 'VisibleDashboardParametersPhases', true, function(series) { var Range = ArrayAbsMax(CalcRanges(THDIData, series, false, true, false, true)); return [-Range, Range]; }, undefined); 
}
function Live_OnSuccess(result) {
  SetCubismNowLabel(ActiveChart, result['active_power']);
  SetCubismNowLabel(ReactiveChart, result['reactive_power']);
  SetCubismNowLabel(PowerFactorChart, result['power_factor']);
  SetCubismNowLabel(PhiChart, result['phi']);
  SetCubismNowLabel(THDVChart, result['thd_v']);
  SetCubismNowLabel(THDIChart, result['thd_i']);
}
$('#groupParametersPhases').on('click', 'button', ParametersSeries);
var MainScrollable = null;
function UpdatePage() {
  MainScrollable = new Scrollable( {Spinners: ['#EnergySpinner'], NowBtn: '#toolbarEnergy button[data-function="Update"]', Series: ['ACTIVE_POWER'], AggregVariable: 'GraphAggregationEnergy', OnSuccess: Scrollable_OnSuccess, Counters: Scrollable_UpdateCounters } );
  new Live( { NowBtn: undefined, RefreshInterval: 5000, Spinners: [], Series: ['ACTIVE_POWER', 'REACTIVE_POWER', 'POWER_FACTOR', 'PHI', 'THD_V', 'THD_I'], OnSuccess: Live_OnSuccess});
  new PeriodView( {Spinners: ['#VoltageSpinner', '#CurrentSpinner', '#PowerSpinner', '#FrequencySpinner'], Series: ['RMS_V', 'FLAG', 'RMS_I', 'ACTIVE_POWER', 'FREQUENCY'], Period: 7200000, Aggreg: '3S', OnSuccess: PeriodView_OnSuccess} );
  new PeriodView( {Spinners: ['#MainParametersSpinner'], Series: ['ACTIVE_POWER', 'REACTIVE_POWER', 'POWER_FACTOR', 'PHI', 'THD_V', 'THD_I'], Period: 86400000, Aggreg: '1M', OnSuccess: PeriodViewParams_OnSuccess} );
}