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
var ActivePowerData      = [];
var ReactivePowerData    = [];
var ActiveEnergyChart    = null;
var ReactiveEnergyChart  = null;
var ActivePowerChart     = null;
var ReactivePowerChart   = null;
var ActiveEnergyUnits    = '';
var ReactiveEnergyUnits  = '';
var ActivePowerUnits     = '';
var ReactivePowerUnits   = '';
var ScrollableSeries     = 'active_energy, active_power, reactive_energy, reactive_power';
function Scrollable_ShowSpinner() {
  $('#EnergyPowerSpinner').show();
}
function Scrollable_StopSpinner() {
  $('#EnergyPowerSpinner').hide();
}
function Scrollable_OnSuccess(result, clear) {
  if(clear)  {
    ActiveEnergyData   = [Array(SeriesNames.ActiveEnergy().length + 1).fill(0)];
    ReactiveEnergyData = [Array(SeriesNames.ReactiveEnergy().length + 1).fill(0)];
  }
  ActiveEnergyData   = GraphDataAppend(result, 'active_energy', ActiveEnergyData);
  ReactiveEnergyData = GraphDataAppend(result, 'reactive_energy', ReactiveEnergyData);

//  var Range = CalcRanges(ActiveEnergyData, [1], true); 
//  var toShow = [];
//  if((Math.abs(Range[0]) > 1000) || (Math.abs(Range[1]) > 1000)) {
//    ActiveEnergyChartUnits = 'kWh';
//    Range[0] = Range[0] / 1000;
//    Range[1] = Range[1] / 1000;
//    toShow = Div1000(ActiveEnergyData);
//  } else {
//    ActiveEnergyChartUnits = 'Wh';
//    toShow = ActiveEnergyData;
//  }
//  var Units = 'Wh';
//  var Sum = [0, 0, 0];
//  for(i = 0; i < ActiveEnergyData.length; i++) {
//    for(n = 1; n < ActiveEnergyData[i].length; n++) Sum[n - 1] += ActiveEnergyData[i][n];
//  }  
//  for(i = 0; i < Sum.length; i++) {
//    if(Sum[i] > 1000) Units = 'kWh';
//  }
//  $('#labelSelectedActiveEnergyUnits').text(Units);
//  if(Units == 'kWh') {
//    for(i = 0; i < Sum.length; i++) Sum[i] = Sum[i] / 1000;
//  }
//  if(Nodes['Devices'][Node]['Type'] === '1Phase') {  
//    $('#labelSelectedActiveEnergy').html(Round2(Sum[0]));
//  } else {
//    $('#labelSelectedActiveEnergy').html(Round2(Sum[0] + Sum[1] + Sum[2]));
//    $('#labelSelectedActiveEnergyPhases').html('<br/>By phase&nbsp;&nbsp;<span style="font-size:150%">' + Array_Round2(Sum, '&nbsp;&nbsp;') + '</span>&nbsp;&nbsp;' + Units);
//  }
//  
//  var StartDate = new Date(result['From']);  
//  var EndDate = new Date(result['To']);  
//  $('#labelSelectedPeriodRange').html('From ' + StartDate.toLocaleDateString() + '-' + StartDate.toLocaleTimeString() + ' to ' + EndDate.toLocaleDateString() + '-' + EndDate.toLocaleTimeString());
//  ActiveEnergyResizeEvent();
// 
  if(ActiveEnergyChart == null) {
    ActiveEnergyChart = new Dygraph(document.getElementById("ActiveEnergyChart"), ActiveEnergyData,
      { strokeWidth: 0.5, stackedGraph: true, stepPlot: true, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], xAxisHeight: 26, dateWindow: [result['From'], result['To']], plotter: barChartNoSpacedPlotter,
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(SeriesNames.ActiveEnergy()),
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: ["#31708f", "#708f31", "#8f3170"],
       
        valueRange: Range,
        
        
        
        underlayCallback: function(context, area, dygraph) {
         // HightlightWeekends("#EEE", context, area, dygraph);
        },
        
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">' + ActiveEnergyUnits + '</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: MainScrollable.XAxis
          }
        }
    );
  } else {
    ActiveEnergyChart.updateOptions({'file': toShow, valueRange: Range, dateWindow: [result['From'], result['To']] });
  }
  return;
  Range = CalcRanges(ReactiveEnergyData, [1], true);
  var toShow = [];
  if((Math.abs(Range[0]) > 1000) || (Math.abs(Range[1]) > 1000)) {
    ReactiveEnergyChartUnits = 'kvarh';
    Range[0] = Range[0] / 1000;
    Range[1] = Range[1] / 1000;
    toShow = Div1000(ReactiveEnergyData);
  } else {
    ReactiveEnergyChartUnits = 'varh';
    toShow = ReactiveEnergyData;
  }
  if(ReactiveEnergyChart == null) {
    ReactiveEnergyChart = new Dygraph(document.getElementById("ReactiveEnergyChart"),
      toShow,
      { strokeWidth: 0.5, stackedGraph: true, stepPlot: true, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], xAxisHeight: 26, dateWindow: [result['From'], result['To']], plotter: barChartNoSpacedPlotter,
        labelsDiv: "ReactiveEnergyLegend",
        labels: ["Time"].concat(ReactiveEnergyNames),

plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        zoomCallback: Scrollable_Zoom, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: Scrollable_MouseUp, dblclick: Scrollable_DoubleClick },

      colors: ["#314190"],
        valueRange: Range,
        
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">' + ReactiveEnergyChartUnits + '</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleDateString() + ' - ' + new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return new Date(x).toLocaleTimeString() + '<br>' + new Date(x).toLocaleDateString(); }
             }
          }
        }
    );
  } else {
    ReactiveEnergyChart.updateOptions({'file': toShow, valueRange: Range, dateWindow: [result['From'], result['To']]});
  }

  Range = CalcRanges(ActivePowerData, [1], true);
  var toShow = [];
  if((Math.abs(Range[0]) > 1000) || (Math.abs(Range[1]) > 1000)) {
    ActivePowerChartUnits = 'kW';
    Range[0] = Range[0] / 1000;
    Range[1] = Range[1] / 1000;
    toShow = Div1000(ActivePowerData);
  } else {
    ActivePowerChartUnits = 'W';
    toShow = ActivePowerData;
  }
  if(ActivePowerChart == null) {
    ActivePowerChart = new Dygraph(document.getElementById("ActivePowerChart"),
      toShow,
      { strokeWidth: 0.5, stackedGraph: true, stepPlot: true, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], xAxisHeight: 26, dateWindow: [result['From'], result['To']],
        labelsDiv: "ActivePowerLegend",
        labels: ["Time"].concat(ActivePowerNames),
        colors: ["#f0ad4e"],
        valueRange: Range,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        zoomCallback: Scrollable_Zoom, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: Scrollable_MouseUp, dblclick: Scrollable_DoubleClick },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">' + ActivePowerChartUnits + '</em>';
               },
               axisLabelFormatter: function (x) {
                 return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleDateString() + ' - ' + new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return new Date(x).toLocaleTimeString() + '<br>' + new Date(x).toLocaleDateString(); }
             }
          }
        }
    );
  } else {
    ActivePowerChart.updateOptions({'file': toShow, valueRange: Range, dateWindow: [result['From'], result['To']]});
  }

  Range = CalcRanges(ReactivePowerData, [1], true);
  var toSHow = [];
  if((Math.abs(Range[0]) > 1000) || (Math.abs(Range[1]) > 1000)) {
    ReactivePowerChartUnits = 'kvar';
    Range[0] = Range[0] / 1000;
    Range[1] = Range[1] / 1000;
    toShow = Div1000(ReactivePowerData);
  } else {
    ReactivePowerChartUnits = 'var';
    toShow = ReactivePowerData;
  }
  if(ReactivePowerChart == null) {
    ReactivePowerChart = new Dygraph(document.getElementById("ReactivePowerChart"),
      toShow,
      { strokeWidth: 0.5, stackedGraph: true, stepPlot: true, fillGraph: true, gridLineColor: "#BBB", gridLinePattern: [4, 4], xAxisHeight: 26, dateWindow: [result['From'], result['To']], 
        labelsDiv: "ReactivePowerLegend",
        labels: ["Time"].concat(ReactivePowerNames),
        colors: ["#314190"],
        valueRange: Range,
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        zoomCallback: Scrollable_Zoom, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: Scrollable_MouseUp, dblclick: Scrollable_DoubleClick },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">' + ReactivePowerChartUnits + '</em>';
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
    window.dispatchEvent(new Event('resize'));
    Dygraph.synchronize([ActiveEnergyChart, ReactiveEnergyChart, ActivePowerChart, ReactivePowerChart], { range: false, zoom: false } );
  } else {
    ReactivePowerChart.updateOptions({'file': toShow, valueRange: Range, dateWindow: [result['From'], result['To']]});
  }
  $('#btnShowEnergyPowerYear').removeClass('active');
  $('#btnShowEnergyPowerMonth').removeClass('active');
  $('#btnShowEnergyPowerWeek').removeClass('active');
  $('#btnShowEnergyPowerDay').removeClass('active');
  if((result['To'] - result['From']) === 86400000) {
    $('#btnShowEnergyPowerDay').addClass('active');
  } else if((result['To'] - result['From']) === 604800000) {
    $('#btnShowEnergyPowerWeek').addClass('active');
  } else if((result['To'] - result['From']) === 2592000000) {
    $('#btnShowEnergyPowerMonth').addClass('active');
  } else if((result['To'] - result['From']) === 31536000000) {
    $('#btnShowEnergyPowerYear').addClass('active');
  }
  var GraphAggregation = localStorage.getItem('GraphAggregation');
  if(GraphAggregation !== result['Aggregation_Source']) {
    $('#aggreg_modal').modal('show');
    GraphAggregation = result['Aggregation_Source'];
  }
  $('#btnEnergyPower3S').removeClass('active');
  $('#btnEnergyPower1M').removeClass('active');
  $('#btnEnergyPower10M').removeClass('active');
  $('#btnEnergyPower1H').removeClass('active');
  if(GraphAggregation == '3s') {
    $('#btnEnergyPower3S').addClass('active');
  } else if(GraphAggregation == '1m') {
    $('#btnEnergyPower1M').addClass('active');
  } else if(GraphAggregation == '10m') {
    $('#btnEnergyPower10M').addClass('active');
  } else if(GraphAggregation == '1h') {
    $('#btnEnergyPower1H').addClass('active');
  }
  var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
  if(GraphEnd === 0) {
    $('#btnEnergyPowerNow').addClass('active');
  } else {
    $('#btnEnergyPowerNow').removeClass('active');
  }
  localStorage.setItem('GraphAggregation', GraphAggregation);
}
function ActiveEnergyResizeEvent() {
  var Height = $('#ActiveEnergyCurrentPanel').height() + $('#ActiveEnergyPrevPanel').height() + $('#ActiveEnergySelectedPanel').height();
  if(ActiveEnergyChart !== null) ActiveEnergyChart.resize($('#ActiveEnergyCol').width(), Height + 6);
}
$('#btnEnergyPower3S').click(function() {
  localStorage.setItem('GraphAggregation', '3s');
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
});
$('#btnEnergyPower1M').click(function() {
  localStorage.setItem('GraphAggregation', '1m');
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
});
$('#btnEnergyPower10M').click(function() {
  localStorage.setItem('GraphAggregation', '10m');
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
});
$('#btnEnergyPower1H').click(function() {
  localStorage.setItem('GraphAggregation', '1h');
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
});
$('#btnShowEnergyPowerYear').click(function() {
  localStorage.setItem('GraphPeriod', 31536000000);
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
  localStorage.setItem('CountersPeriod', 'Year');
  AjaxGetEnergyCounters();
});
$('#btnShowEnergyPowerMonth').click(function() {
  localStorage.setItem('GraphPeriod', 2592000000);
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
  localStorage.setItem('CountersPeriod', 'Month');
  AjaxGetEnergyCounters();
});
$('#btnShowEnergyPowerWeek').click(function() {
  localStorage.setItem('GraphPeriod', 604800000);
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
  localStorage.setItem('CountersPeriod', 'Week');
  AjaxGetEnergyCounters();
});
$('#btnShowEnergyPowerDay').click(function() {
  localStorage.setItem('GraphPeriod', 86400000);
  LastEnergyPowerSample = 0;
  AjaxGetEnergyPower();
  localStorage.setItem('CountersPeriod', 'Day');
  AjaxGetEnergyCounters();
  
});
$('#btnEnergyPowerNow').click(function() {
  var End = parseInt(localStorage.getItem('GraphEnd'));
  if(End === 0) {
    $('#btnEnergyPowerNow').removeClass('active');
    localStorage.setItem('GraphEnd', ActiveEnergyChart.getOption('dateWindow')[1]);
    if(EnergyPowerTimer !== null) clearInterval(EnergyPowerTimer);
  } else {
    $('#btnEnergyPowerNow').addClass('active');
    localStorage.setItem('GraphEnd', 0);
    LastEnergyPowerSample = 0;
    AjaxGetEnergyPower();
  }
});
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