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
var ActiveEnergyConsumptionData = [], ActiveEnergyCostData = [], ActiveEnergyConsumptionChart = null, ActiveEnergyCostChart = null;
function Scrollable_OnSuccess(result, clear) {
  if(clear)  {
    ActiveEnergyConsumptionData = [];
    ActiveEnergyCostData        = [];
  }
  if(result['Sampletime'] !== null && result['Sampletime'].length > 0) {
    if(ActiveEnergyConsumptionData.length === 0 || ActiveEnergyConsumptionData[ActiveEnergyConsumptionData.length - 1][0] < result['Sampletime'][0]) {
      for(i = 0; i < result['Sampletime'].length; i++) {
        ActiveEnergyConsumptionData.push([new Date(result['Sampletime'][i]), result['active_energy'][i], result['price'][i]]);
        ActiveEnergyCostData.push([new Date(result['Sampletime'][i]), result['active_energy'][i] * result['price'][i] / 1000.0, result['price'][i]]);
      }
      ShiftArray(ActiveEnergyConsumptionData, result['MaxSamples']);
      ShiftArray(ActiveEnergyCostData, result['MaxSamples']);
    }
  }
  if(ActiveEnergyConsumptionChart == null) {
    ActiveEnergyConsumptionChart = new Dygraph(document.getElementById("ActiveEnergyConsumptionChart"), ActiveEnergyConsumptionData,
      { strokeWidth: 0.5, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], dateWindow: [result['From'], result['To']], includeZero: true, plotter: barChartLineNoSpacedPlotter,
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time', 'Consumption', 'Price'],
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: [$('#ActiveEnergyConsumptionPanel').css('border-color'), '#d9534f'], plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        underlayCallback: function(context, area, dygraph) { GraphHightlightWeekends("#EEE", context, area, dygraph); },
        series: {'Price': { axis: 'y2' } },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Units = GetBestUnits(x); return Round2(ScaleToUnits(x, Units)) + ' <em class="x-small">' + Units + 'Wh</em>'; },
               axisLabelFormatter: function (x) {var Units = GetBestUnits(x); return Round2(ScaleToUnits(x, Units)); }
             },
          y2: { pixelsPerLabel: 15, axisLabelWidth: 26,
                valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">€/kWh</em>'; },
                axisLabelFormatter: function (x) { return Round2(x); }
              },
          x: MainScrollable.XAxis
          }
        }
    );
  } else {
    ActiveEnergyConsumptionChart.updateOptions({file: ActiveEnergyConsumptionData, dateWindow: [result['From'], result['To']] });
  }
  if(ActiveEnergyCostChart == null) {
    ActiveEnergyCostChart = new Dygraph(document.getElementById("ActiveEnergyCostChart"), ActiveEnergyCostData,
      { strokeWidth: 0.5, stepPlot: false, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], dateWindow: [result['From'], result['To']], includeZero: true, plotter: barChartLineNoSpacedPlotter,
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time', 'Cost', 'Price'],
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: [$('#ActiveEnergyCostPanel').css('border-color'), '#d9534f'], plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        underlayCallback: function(context, area, dygraph) { GraphHightlightWeekends("#EEE", context, area, dygraph); },
        series: {'Price': { axis: 'y2' } },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">' + '€' + '</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          y2: { pixelsPerLabel: 15, axisLabelWidth: 26,
                valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">€/kWh</em>'; },
                axisLabelFormatter: function (x) { return Round2(x); }
              },
          x: MainScrollable.XAxis
          }
        }
    );
    Dygraph.synchronize([ActiveEnergyConsumptionChart, ActiveEnergyCostChart], { range: false, zoom: false } );
  } else {
    ActiveEnergyCostChart.updateOptions({file: ActiveEnergyCostData, dateWindow: [result['From'], result['To']]});
  }
//  EnergySeries();
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


/* Secuencia de inicio para cargar la pagina */
var MainScrollable = null;
function UpdatePage() {
  MainScrollable = new Scrollable( {Spinners: ['#ActiveEnergyConsumptionSpinner', '#ActiveEnergyCostSpinner'], Series: 'Prices', OnSuccess: Scrollable_OnSuccess} );
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
