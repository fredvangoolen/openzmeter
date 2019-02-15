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

function PeriodView_OnSuccess(result) {
  VoltageData = GraphDataAppend(result, 'Voltage', VoltageData);
  $('#labelVoltage').html(Array_Round2(result['Voltage'][result['Voltage'].length - 1], ColorArray($('#VoltagePanel').css('border-color'), 3)));
  $('#labelVoltageAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  if(VoltageChart === null) {
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"), VoltageData,
      { strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: false, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {},
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(Analyzer.GetSeriesNames('Voltage')), colors: ColorArray($('#VoltagePanel').css('border-color'), 3), dateWindow: [result['From'], result['To']],
        underlayCallback: function(canvas, area, g) {
          var NV = Analyzer.GetSelected().NominalVoltage, Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
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
    new GraphTools('Voltage', VoltageChart, '#toolbarVoltage', undefined, function() { exportToCsv('Voltage.csv', [['Time'].concat(Analyzer.GetSeriesNames('Voltage'))].concat(VoltageData)); });
  } else {
    VoltageChart.updateOptions({ file: VoltageData, dateWindow: [result['From'], result['To']]});
  }
  for(var n = 0; n < Analyzer.GetSeriesNames('Voltage').length; n++) AddEvents(result, Analyzer.GetSeriesNames('Voltage')[n], VoltageChart);
  VoltageSeries();
  CurrentData = GraphDataAppend(result, 'Current', CurrentData);
  $('#labelCurrent').html(Array_Round2(result['Current'][result['Current'].length - 1], ColorArray($('#CurrentPanel').css('border-color'), 4)));
  $('#CurrentAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  if(CurrentChart === null) {
    CurrentChart = new Dygraph(document.getElementById("CurrentChart"), CurrentData,
      { strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], animatedZooms: true, rollPeriod: 0,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], interactionModel: {}, 
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(Analyzer.GetSeriesNames('Current')), colors: ColorArray($('#CurrentPanel').css('border-color'), 4), dateWindow: [result['From'], result['To']],
        underlayCallback: function(canvas, area, g) {
          var MC = Analyzer.GetSelected().MaxCurrent, Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
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
    new GraphTools('Current', CurrentChart, '#toolbarCurrent', undefined, function() { exportToCsv('Current.csv', [['Time'].concat(Analyzer.GetSeriesNames('Current'))].concat(CurrentData)); });
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
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(Analyzer.GetSeriesNames('Active_Power')), colors: ColorArray($('#PowerPanel').css('border-color'), 3), dateWindow: [result['From'], result['To']],
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
    new GraphTools('Active power', PowerChart, '#toolbarPower', undefined, function() { exportToCsv('Active power.csv', [['Time'].concat(Analyzer.GetSeriesNames('Active_Power'))].concat(PowerData)); });
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
function EnergyResizeEvent() {
  if(EnergyChart !== null) EnergyChart.resize('100%', $('#EnergyPanel > .panel-body > .row > div:first-child').height());
}

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
      Analyzer.GetSeriesAggreg(times.SecondaryPeriod, times.SecondaryTo, times.PreferedAggreg, 'sum(active_energy)', 
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
      Analyzer.GetPrices(times.SecondaryPeriod, times.SecondaryTo, 0,
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
  Analyzer.GetSeriesAggreg(times.PrimaryPeriod, times.PrimaryTo, times.PreferedAggreg, 'sum(active_energy)', 
    function(result) {
      var Sum = ArraySum(result['sum_active_energy']);
      var Units = GetBestUnits(Sum);
      $('#labelEnergyPrimaryValue').html(Round2(ScaleToUnits(Sum, Units)));
      $('#labelEnergyPrimaryUnits').text(Units + 'Wh');
      if(result['sum_active_energy'].length > 1) $('#labelEnergyPrimaryPhases').html('By phases:&nbsp;' + Array_Round2(ScaleArrayToUnits(result['sum_active_energy'], Units)) + '&nbsp;' + Units + 'Wh');
      window.dispatchEvent(new Event('resize'));
    }
  );
  Analyzer.GetPrices(times.PrimaryPeriod, times.PrimaryTo, 0,
    function(result) {
      $('#labelBudgetUnits').text('€');
      var Budget = 0.0;
      for(var i = 0; i < result['Sampletime'].length; i++) Budget += result['active_energy'][i] * result['price'][i] / 1000.0;
      $('#labelBudgetPrimaryValue').html(Round2(Budget));
    }
  );
}
/* Parameters */
function PeriodViewParams_OnSuccess(result) {
    ActiveChart      = AppendCubismGraph($('#EvolutionPanel'), ActiveData,      "Active Power",   [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return (x === null) ? '~' : (Round2(ScaleToUnits(x, Units)) + '<em>' + Units + 'W</em>'); });
    ReactiveChart    = AppendCubismGraph($('#EvolutionPanel'), ReactiveData,    "Reactive Power", [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return (x === null) ? '~' : (Round2(ScaleToUnits(x, Units)) + '<em>' + Units + 'VAr</em>'); });
    PowerFactorChart = AppendCubismGraph($('#EvolutionPanel'), PowerFactorData, "Power Factor",   [result['From'], result['To']], function (x) { return (x === null) ? '~' : Round2(x); });
    PhiChart         = AppendCubismGraph($('#EvolutionPanel'), PhiData,         "Angle",          [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(Math.abs(x)) + '<em>' + ((x < 0) ? 'º CAP' : (x > 0) ? 'º IND' : 'º') + '</em>'); });
    THDVChart        = AppendCubismGraph($('#EvolutionPanel'), THDVData,        "Voltage THD",    [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(x * 100.0) + '<em>%</em>'); });
    THDIChart        = AppendCubismGraph($('#EvolutionPanel'), THDIData,        "Current THD",    [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(x * 100.0) + '<em>%</em>'); });
    Dygraph.synchronize([ActiveChart, ReactiveChart, PowerFactorChart, PhiChart, THDVChart, THDIChart], { range: false, zoom: false });
}
function Live_OnSuccess(result) {
  SetCubismNowLabel(ActiveChart, result['Active_Power']);
  SetCubismNowLabel(ReactiveChart, result['Reactive_Power']);
  SetCubismNowLabel(PowerFactorChart, result['Power_Factor']);
  SetCubismNowLabel(PhiChart, result['Phi']);
  SetCubismNowLabel(THDVChart, result['Voltage_THD']);
  SetCubismNowLabel(THDIChart, result['Current_THD']);
}
$('#groupParametersPhases').on('click', 'button', ParametersSeries);
var MainScrollable = null;
function UpdatePage() {
  MainScrollable = new Scrollable( {Spinners: ['#EnergySpinner'], NowBtn: '#toolbarEnergy button[data-function="Update"]', Series: ['ACTIVE_POWER'], AggregVariable: 'GraphAggregationEnergy', OnSuccess: Scrollable_OnSuccess, Counters: Scrollable_UpdateCounters } );
  new Live( { NowBtn: undefined, RefreshInterval: 5000, Spinners: [], Series: ['ACTIVE_POWER', 'REACTIVE_POWER', 'POWER_FACTOR', 'PHI', 'VOLTAGE_THD', 'CURRENT_THD'], OnSuccess: Live_OnSuccess});
  new PeriodView( {Spinners: ['#VoltageSpinner', '#CurrentSpinner', '#PowerSpinner', '#FrequencySpinner'], Series: ['VOLTAGE', 'FLAG', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY'], Period: 7200000, Aggreg: '3S', OnSuccess: PeriodView_OnSuccess} );
  new PeriodView( {Spinners: ['#MainParametersSpinner'], Series: ['ACTIVE_POWER', 'REACTIVE_POWER', 'POWER_FACTOR', 'PHI', 'VOLTAGE_THD', 'CURRENT_THD'], Period: 86400000, Aggreg: '1M', OnSuccess: PeriodViewParams_OnSuccess} );
}