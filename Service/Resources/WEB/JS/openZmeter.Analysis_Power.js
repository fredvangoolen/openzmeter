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

/* Graficas de energia y potencia*/
var ActivePowerData      = [];
var ReactivePowerData    = [];
var ActivePowerChart     = null;
var ReactivePowerChart   = null;
function Scrollable_OnSuccess(result, clear) {
  if(clear)  {
    ActivePowerData    = [];
    ReactivePowerData  = [];
  }
  ActivePowerData    = GraphDataAppend(result, 'Active_Power', ActivePowerData);
  ReactivePowerData  = GraphDataAppend(result, 'Reactive_Power', ReactivePowerData);
  if(ActivePowerChart == null) {
    ActivePowerChart = new Dygraph(document.getElementById("ActivePowerChart"), ActivePowerData,
      { strokeWidth: 0.5, stackedGraph: true, stepPlot: true, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], xAxisHeight: 26, dateWindow: [result['From'], result['To']],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(Analyzer.GetSeriesNames('Active_Power')),
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: ColorArray($('#ActivePowerPanel').css('border-color'), 3), plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        underlayCallback: function(context, area, dygraph) { GraphHightlightWeekends("#EEE", context, area, dygraph); },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Units = GetBestUnits(x); return Round2(ScaleToUnits(x, Units)) + ' <em class="x-small">' + Units + 'W</em>'; },
               axisLabelFormatter: function (x) { return  Round2(x); }
             },
          x: MainScrollable.XAxis
          }
        }
    );
  } else {
    ActivePowerChart.updateOptions({'file': ActivePowerData,dateWindow: [result['From'], result['To']]});
  }
  if(ReactivePowerChart == null) {
    ReactivePowerChart = new Dygraph(document.getElementById("ReactivePowerChart"), ReactivePowerData,
      { strokeWidth: 0.5, stackedGraph: true, stepPlot: true, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], xAxisHeight: 26, dateWindow: [result['From'], result['To']], 
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(Analyzer.GetSeriesNames('Reactive_Power')),
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: ColorArray($('#ReactivePowerPanel').css('border-color'), 3), plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Units = GetBestUnits(x); return Round2(ScaleToUnits(x, Units)) + ' <em class="x-small">' + Units + 'var</em>'; },
               axisLabelFormatter: function (x) { return  Round2(x); }
             },
          x: MainScrollable.XAxis
          }
        }
    );
    window.dispatchEvent(new Event('resize'));
    Dygraph.synchronize([ActivePowerChart, ReactivePowerChart], { range: false, zoom: false } );
  } else {
    ReactivePowerChart.updateOptions({'file': toShow, valueRange: Range, dateWindow: [result['From'], result['To']]});
  }
}
function ActiveEnergyResizeEvent() {
  var Height = $('#ActiveEnergyCurrentPanel').height() + $('#ActiveEnergyPrevPanel').height() + $('#ActiveEnergySelectedPanel').height();
  if(ActiveEnergyChart !== null) ActiveEnergyChart.resize($('#ActiveEnergyCol').width(), Height + 6);
}
/* Contadores de energia */
var EnergyCountersTimer  = null;
function Scrollable_UpdateCounters(change) {
  return;
  if(EnergyCountersTimer != null) clearInterval(EnergyCountersTimer);
  EnergyCountersTimer = setTimeout(EnergyCountersTimer, 60000);
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
           data: JSON.stringify({ Series: "sum(active_energy)", Aggreg: Aggreg, Period: Period, To: Current + Period }),
           success: EnergyUpdateCountersCurrent
  });
  $.ajax({ url: "getSeriesAggreg",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: "sum(active_energy)", Aggreg: Aggreg, Period: Period, To: Current }),
           success: EnergyUpdateCountersPrevs
  });
}
function EnergyUpdateCountersCurrent(result) {
  var CountersPeriod = localStorage.getItem('CountersPeriod');
  if(CountersPeriod === 'Week') {
    $('#labelCurrentActiveEnergyPeriod').text('This week');
  } else if(CountersPeriod === 'Month') {
    $('#labelCurrentActiveEnergyPeriod').text('This month');
  } else if(CountersPeriod === 'Year') {
    $('#labelCurrentActiveEnergyPeriod').text('This year');
  } else if(CountersPeriod === 'Day') {
    $('#labelCurrentActiveEnergyPeriod').text('Today');
  }
  var Units = 'Wh';
  for(i = 0; i < result['sum_active_energy'].length; i++) {
    if(result['sum_active_energy'][i] > 1000) Units = 'kWh';
  }
  $('#labelCurrentActiveEnergyUnits').text(Units);
  if(Units == 'kWh') {
    for(i = 0; i < result['sum_active_energy'].length; i++) result['sum_active_energy'][i] = result['sum_active_energy'][i] / 1000;
  }
  if(Nodes['Devices'][Node]['Type'] === '1Phase') {  
    $('#labelCurrentActiveEnergy').html(Round2(result['sum_active_energy'][0]));
  } else {
    $('#labelCurrentActiveEnergy').html(Round2(result['sum_active_energy'][0] + result['sum_active_energy'][1] + result['sum_active_energy'][2]));
    $('#labelCurrentActiveEnergyPhases').html('<br/>By phase&nbsp;&nbsp;<span style="font-size:150%">' + Array_Round2(result['sum_active_energy'], '&nbsp;&nbsp;') + '</span>&nbsp;&nbsp;' + Units);
  }
  ActiveEnergyResizeEvent();
}
function EnergyUpdateCountersPrevs(result) {
  var CountersPeriod = localStorage.getItem('CountersPeriod');
  if(CountersPeriod === 'Week') {
    $('#labelPrevPeriod').text('Previous week');
  } else if(CountersPeriod === 'Month') {
    $('#labelPrevPeriod').text('Previous month');
  } else if(CountersPeriod === 'Year') {
    $('#labelPrevPeriod').text('Previous year');
  } else if(CountersPeriod === 'Day') {
    $('#labelPrevPeriod').text('Previous day');
  }
  var Units = 'Wh';
  for(i = 0; i < result['sum_active_energy'].length; i++) {
    if(result['sum_active_energy'][i] > 1000) Units = 'kWh';
  }
  $('#labelPrevActiveEnergyUnits').text(Units);
  if(Units == 'kWh') {
    for(i = 0; i < result['sum_active_energy'].length; i++) result['sum_active_energy'][i] = result['sum_active_energy'][i] / 1000;
  }
  if(Nodes['Devices'][Node]['Type'] === '1Phase') {  
    $('#labelPrevActiveEnergy').html(Round2(result['sum_active_energy'][0]));
  } else {
    $('#labelPrevActiveEnergy').html(Round2(result['sum_active_energy'][0] + result['sum_active_energy'][1] + result['sum_active_energy'][2]));
    $('#labelPrevActiveEnergyPhases').html('<br/>By phase&nbsp;&nbsp;<span style="font-size:150%">' + Array_Round2(result['sum_active_energy'], '&nbsp;&nbsp;') + '</span>&nbsp;&nbsp;' + Units);
  }
  ActiveEnergyResizeEvent();
}

/* Secuencia de inicio para cargar la pagina */
var MainScrollable = null;
function UpdatePage() {
  MainScrollable = new Scrollable( {Spinners: ['#ActivePowerSpinner'], Series: ['ACTIVE_POWER', 'REACTIVE_POWER'], OnSuccess: Scrollable_OnSuccess} );
  Scrollable_UpdateCounters(true);
}

/* Exportar datos */
$('#btnExportActivePower').click(function() {
  exportToCsv("ActivePower_" + Node + '_' + LastEnergyPowerSample + '.csv', ((Nodes['Devices'][Node]['Type'] == '3Phase') ? [['DateTime', 'Active Power R', 'Active Power S', 'Active Power T']] : [['DateTime', 'Active Power']]).concat(ActivePowerData));
});
$('#btnExportReactivePower').click(function() {
  exportToCsv("ReactivePower_" + Node + '_' + LastEnergyPowerSample + '.csv', ((Nodes['Devices'][Node]['Type'] == '3Phase') ? [['DateTime', 'Reactive Power R', 'Reactive Power S', 'Reactive Power T']] : [['DateTime', 'Reactive Power']]).concat(ReactivePowerData));
});
$('#btnExportActiveEnergy').click(function() {
  exportToCsv("ActiveEnergy_" + Node + '_' + LastEnergyPowerSample + '.csv', ((Nodes['Devices'][Node]['Type'] == '3Phase') ? [['DateTime', 'Active Energy R', 'Active Energy S', 'Active Energy T']] : [['DateTime', 'Active Energy']]).concat(ActiveEnergyData));
});
$('#btnExportReactiveEnergy').click(function() {
  exportToCsv("ReactiveEnergy_" + Node + '_' + LastEnergyPowerSample + '.csv', ((Nodes['Devices'][Node]['Type'] == '3Phase') ? [['DateTime', 'Reactive Energy R', 'Reactive Energy S', 'Reactive Energy T']] : [['DateTime', 'Reactive Energy']]).concat(ReactiveEnergyData));
});//792