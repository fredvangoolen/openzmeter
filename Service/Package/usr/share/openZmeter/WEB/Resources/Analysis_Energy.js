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
var ActiveEnergyData     = [];
var ReactiveEnergyData   = [];
var ActiveEnergyChart    = null;
var ReactiveEnergyChart  = null;
function Scrollable_OnSuccess(result, clear) {
  if(clear)  {
    ActiveEnergyData   = [Array(SeriesNames.ActiveEnergy().length + 1).fill(0)];
    ReactiveEnergyData = [Array(SeriesNames.ReactiveEnergy().length + 1).fill(0)];
  }
  ActiveEnergyData   = GraphDataAppend(result, 'active_energy', ActiveEnergyData);
  ReactiveEnergyData = GraphDataAppend(result, 'reactive_energy', ReactiveEnergyData);
  if(ActiveEnergyChart == null) {
    ActiveEnergyChart = new Dygraph(document.getElementById("ActiveEnergyChart"), ActiveEnergyData,
      { strokeWidth: 0.5, stackedGraph: true, stepPlot: true, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], dateWindow: [result['From'], result['To']], plotter: barChartNoSpacedPlotter,
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(SeriesNames.ActiveEnergy()),
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: ColorArray($('#ActiveEnergyPanel').css('border-color'), 3), plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        underlayCallback: function(context, area, dygraph) { GraphHightlightWeekends("#EEE", context, area, dygraph); },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Units = GetBestUnits(x); return Round2(ScaleToUnits(x, Units)) + ' <em class="x-small">' + Units + 'Wh</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: MainScrollable.XAxis
          }
        }
    );
    $(window).bind('resize', function() { ActiveEnergyChart.resize($('#ActiveEnergyCol').width(), $('#panelCounters').height() - 4); });
    window.dispatchEvent(new Event('resize'));
  } else {
    ActiveEnergyChart.updateOptions({file: ActiveEnergyData, dateWindow: [result['From'], result['To']] });
  }
  if(ReactiveEnergyChart == null) {
    ReactiveEnergyChart = new Dygraph(document.getElementById("ReactiveEnergyChart"), ReactiveEnergyData,
      { strokeWidth: 0.5, stackedGraph: true, stepPlot: true, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], dateWindow: [result['From'], result['To']], plotter: barChartNoSpacedPlotter,
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(SeriesNames.ReactiveEnergy()),
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: ColorArray($('#ReactiveEnergyPanel').css('border-color'), 3), plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        underlayCallback: function(context, area, dygraph) { GraphHightlightWeekends("#EEE", context, area, dygraph); },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Units = GetBestUnits(x); return Round2(ScaleToUnits(x, Units)) + ' <em class="x-small">' + Units + 'varh</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: MainScrollable.XAxis
          }
        }
    );
  } else {
    ReactiveEnergyChart.updateOptions({file: ReactiveEnergyData, dateWindow: [result['From'], result['To']]});
  }
  EnergySeries();
}
function ActiveEnergyResizeEvent() {
  var Height = $('#ActiveEnergyCurrentPanel').height() + $('#ActiveEnergyPrevPanel').height() + $('#ActiveEnergySelectedPanel').height();
  if(ActiveEnergyChart !== null) ActiveEnergyChart.resize($('#ActiveEnergyCol').width(), Height + 6);
}
function EnergySeries(event) {
  GraphPhaseButtons(ActiveEnergyChart, $('#groupEnergyPhases'), 'VisibleEnergyPhases', false, function(series) { return CalcRanges(ActiveEnergyData, series, true, false, true); }, (event !== undefined) ? event.target : undefined); 
  GraphPhaseButtons(ReactiveEnergyChart, $('#groupEnergyPhases'), 'VisibleEnergyPhases', false, function(series) { return CalcRanges(ReactiveEnergyData, series, true, false, true); }, undefined); 
}
$('#groupEnergyPhases').on('click', 'button', EnergySeries);

$('#btnShowEnergy').click(function() {
  $('#EnergyPowerTitle').text('Energy');
  $('#EnergySubPanel').removeClass('hidden');
  $('#PowerSubPanel').addClass('hidden');
  if(ActiveEnergyChart) $(ActiveEnergyChart.graphDiv).parent().removeAttr('style');
  if(ReactiveEnergyChart) $(ReactiveEnergyChart.graphDiv).parent().removeAttr('style');
  window.dispatchEvent(new Event('resize'));
  $('#btnShowEnergy').addClass('active');
  $('#btnShowPower').removeClass('active');
  localStorage.setItem('EnergyPowerTab', 'Energy');
});
$('#btnShowPower').click(function() {
  $('#EnergyPowerTitle').text('Power');
  $('#EnergySubPanel').addClass('hidden');
  $('#PowerSubPanel').removeClass('hidden');
  if(ActivePowerChart) $(ActivePowerChart.graphDiv).parent().removeAttr('style');
  if(ReactivePowerChart) $(ReactivePowerChart.graphDiv).parent().removeAttr('style');
  window.dispatchEvent(new Event('resize'));
  $('#btnShowEnergy').removeClass('active');
  $('#btnShowPower').addClass('active');
  localStorage.setItem('EnergyPowerTab', 'Power');
});

/* Maximizar y minimizar graficos */
var DialogElement           = null;
var PrevDialogContainer     = null;
var DialogResizeHandler     = null;
var DialogPrevResizeHandler = null;
$('#aggreg_modal_close').click(function() {
  $('#aggreg_modal').modal('hide');
});
$('#btnMaximizeActiveEnergy').click(function() {
  $('#GraphDialogTitle').text('Active Energy');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#ActiveEnergyCol');
  DialogElement = $('#ActiveEnergyMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(ActiveEnergyChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    ActiveEnergyChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#btnMaximizeReactiveEnergy').click(function() {
  $('#GraphDialogTitle').text('Reactive Energy');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#ReactiveEnergyCol');
  DialogElement = $('#ReactiveEnergyMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(ReactiveEnergyChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    ReactiveEnergyChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#btnMaximizeActivePower').click(function() {
  $('#GraphDialogTitle').text('Active Power');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#ActivePowerCol');
  DialogElement = $('#ActivePowerMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(ActivePowerChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    ActivePowerChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
  };
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#btnMaximizeReactivePower').click(function() {
  $('#GraphDialogTitle').text('Reactive Power');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#ReactivePowerCol');
  DialogElement = $('#ReactivePowerMaximizeDiv').detach();
  DialogPrevResizeHandler = function() {
    $(ReactivePowerChart.graphDiv).parent().removeAttr('style');
  };
  DialogResizeHandler = function() {
    ReactivePowerChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 100);
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

/* Contadores de energia */
var EnergyCountersTimer  = null;
function Scrollable_UpdateCounters(change) {
  if(EnergyCountersTimer != null) clearInterval(EnergyCountersTimer);
  EnergyCountersTimer = setTimeout(EnergyCountersTimer, 60000);
  var Times = GetCountersTimePeriod();
  if(change === true) {

  }
  Ajax.GetSeriesAggreg(Times.PrimaryPeriod, Times.PrimaryTo, Times.PreferedAggreg, 'sum(active_energy)',
    function(result) {
    }
  );
  if(Times.SecondaryTo !== undefined) {
    Ajax.GetSeriesAggreg(Times.SecondaryPeriod, Times.SecondaryTo, Times.PreferedAggreg, 'avg(rms_v), max(rms_v), min(rms_v)',
      function(result) {
      }
    );
  } else {
  }
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
  MainScrollable = new Scrollable( {Spinners: ['#EnergySpinner'], Series: 'active_energy, reactive_energy', OnSuccess: Scrollable_OnSuccess} );
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