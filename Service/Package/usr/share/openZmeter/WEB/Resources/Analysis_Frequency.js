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

/* Grafica de frecuencia */
var FrequencyChart   = null;
var FrequencyData    = [];
var ScrollableSeries = "freq";
function Scrollable_ShowSpinner() {
  $('#FrequencySpinner').show();
}
function Scrollable_StopSpinner() {
  $('#FrequencySpinner').hide();
}
function Scrollable_OnSuccess(result) {
  if(ScrollableLastSample === 0) FrequencyData = [[0, 0]];
  FrequencyData = GraphDataAppend(result, 'freq', FrequencyData);
  if(FrequencyChart == null) {
    FrequencyChart = new Dygraph(document.getElementById("FrequencyChart"), FrequencyData,
      { strokeWidth: 0.5, stepPlot: true, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ["Time", "Frequency"], dateWindow: [result['From'], result['To']],
        colors: $('#FreqeuncyPanel').css('border-color'), plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        zoomCallback: Scrollable_Zoom, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: Scrollable_MouseUp, dblclick: Scrollable_DoubleClick },
        underlayCallback: function(canvas, area, g) {
//          var NominalFreq = Nodes['NominalFreq'][Node];
//          var Top    = g.toDomYCoord(NominalFreq + NominalFreq * 0.1);
//          var Bottom = g.toDomYCoord(NominalFreq + NominalFreq * 0.01);          
//          canvas.fillStyle = "rgba(255, 255, 0, 0.15)";
//          canvas.fillRect(0, Top, area.w, Top - Bottom);

//          var YellowLowA = ;
//          var YellowHeightA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.1) - YellowLowA;
//          var YellowLowB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.01);
//          var YellowHeightB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.10) - YellowLowB;

          
//          var RedLowA = g.toDomYCoord(1000);
//          var RedHeightA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.10) - RedLowA;
//          var RedLowB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.10);
//          var RedHeightB = g.toDomYCoord(0) - RedLowB;
//          canvas.fillRect(0, YellowLowB, 10000, YellowHeightB);
//          canvas.fillStyle = "rgba(180, 0, 0, 0.15)";
//          canvas.fillRect(0, RedLowA, 10000, RedHeightA);
//          canvas.fillRect(0, RedLowB, 10000, RedHeightB);
        },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">Hz</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleDateString() + ' - ' + new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return new Date(x).toLocaleTimeString() + '<br>' + new Date(x).toLocaleDateString(); }
             }
          }
        }  
    );
    $(window).bind('resize', FrequencyResizeEvent);
    window.dispatchEvent(new Event('resize'));
  } else {
    FrequencyChart.updateOptions( {'file': FrequencyData, valueRange: CalcRanges(FrequencyData, [1], false), dateWindow: [result['From'], result['To']]});
  }
  FreqeuncySeries();
}
function FrequencyResizeEvent() {
  var Height = $('#FreqAvgPanel').height() + $('#FreqMaxPanel').height() + $('#FreqMinPanel').height();
  FrequencyChart.resize($('#FrequencyCol').width(), Height - 4);
}
function FreqeuncySeries(event) {
//  GraphPhaseButtons(FrequencyChart, $('#groupEnergyPhases'), 'VisibleDashboardEnergyPhases', false, function(series) { return CalcRanges(EnergyData, series, true, false, true); }, (event !== undefined) ? event.target : undefined); 
}
$('#btnMaximizeFrequency').click(function() {
  $('#GraphDialogTitle').text('Frequency');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#FrequencyCol');
  DialogElement = $('#FrequencyMaximizeDiv').detach();
  DialogPrevResizeHandler = FrequencyResizeEvent;
  DialogResizeHandler = function() {
    FrequencyChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
  };
  $(window).unbind('resize', FrequencyResizeEvent);
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#btnExportFrequency').click(function() {
  var Data = [];
  var Filename = "Frequency_" + LastFrequencyTime + '.csv';
  Data.push([['Index', 'Frequency']]);
  Data = Data.concat(FrequencyData);
  exportToCsv(Filename, Data);
  $(this).blur();
});

/* Contadores de frequencia */
var FrequencyCountersTimer  = null;
function Scrollable_UpdateCounters(change) {
  if(FrequencyCountersTimer != null) clearInterval(FrequencyCountersTimer);
  FrequencyCountersTimer = setTimeout(AjaxGetFrequencyCounters, 60000);
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
           data: JSON.stringify({ Series: "avg(freq), max(freq), min(freq)", Aggreg: Aggreg, Period: Period, To: Current + Period, Node: Node }),
           success: FrequencyUpdateCountersCurrent
  });
  $.ajax({ url: "getSeriesAggreg",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: "avg(freq), max(freq), min(freq)", Aggreg: Aggreg, Period: Period, To: Current, Node: Node }),
           success: FrequencyUpdateCountersPrevs
  });
}
function FrequencyUpdateCountersCurrent(result) {
  var CountersPeriod = localStorage.getItem('CountersPeriod');
  if(CountersPeriod === 'Week') {
    $('#labelFrequencyPeriodAVG').text('This week average');
    $('#labelFrequencyPeriodMAX').text('This week maximum');
    $('#labelFrequencyPeriodMIN').text('This week minimum');
  } else if(CountersPeriod === 'Month') {
    $('#labelFrequencyPeriodAVG').text('This month average');
    $('#labelFrequencyPeriodMAX').text('This month maximum');
    $('#labelFrequencyPeriodMIN').text('This month minimum');
  } else if(CountersPeriod === 'Year') {
    $('#labelFrequencyPeriodAVG').text('This year average');
    $('#labelFrequencyPeriodMAX').text('This year maximum');
    $('#labelFrequencyPeriodMIN').text('This year minimum');
  } else if(CountersPeriod === 'Day') {
    $('#labelFrequencyPeriodAVG').text('Today average');
    $('#labelFrequencyPeriodMAX').text('Today maximum');
    $('#labelFrequencyPeriodMIN').text('Today minimum');
  }
  if(result['avg_freq'] === null) {
    $('#labelFrequencyCurrentAVG').text('?');
    $('#labelFrequencyCurrentMAX').text('?');
    $('#labelFrequencyCurrentMIN').text('?');
  } else {
    $('#labelFrequencyCurrentAVG').html(Round2(result['avg_freq']));
    $('#labelFrequencyCurrentMAX').html(Round2(result['max_freq']));
    $('#labelFrequencyCurrentMIN').html(Round2(result['min_freq']));
  }
}
function FrequencyUpdateCountersPrevs(result) {
  var CountersPeriod = localStorage.getItem('CountersPeriod');
  if(CountersPeriod === 'Week') {
    $('#labelFrequencyPrevPeriodAVG').text('Previous week average');
    $('#labelFrequencyPrevPeriodMAX').text('Previous week maximum');
    $('#labelFrequencyPrevPeriodMIN').text('Previous week minimum');
  } else if(CountersPeriod === 'Month') {
    $('#labelFrequencyPrevPeriodAVG').text('Previous month average');
    $('#labelFrequencyPrevPeriodMAX').text('Previous month maximum');
    $('#labelFrequencyPrevPeriodMIN').text('Previous month minimum');
  } else if(CountersPeriod === 'Year') {
    $('#labelFrequencyPrevPeriodAVG').text('Previous year average');
    $('#labelFrequencyPrevPeriodMAX').text('Previous year maximum');
    $('#labelFrequencyPrevPeriodMIN').text('Previous year minimum');
  } else if(CountersPeriod === 'Day') {
    $('#labelFrequencyPrevPeriodAVG').text('Previous day average');
    $('#labelFrequencyPrevPeriodMAX').text('Previous day maximum');
    $('#labelFrequencyPrevPeriodMIN').text('Previous day minimum');
  }
  if(result['avg_freq'] === null) {
    $('#labelFrequencyPrevAVG').text('?');
    $('#labelFrequencyPrevMAX').text('?');
    $('#labelFrequencyPrevMIN').text('?');
  } else {
    $('#labelFrequencyPrevAVG').html(Round2(result['avg_freq']));
    $('#labelFrequencyPrevMAX').html(Round2(result['max_freq']));
    $('#labelFrequencyPrevMIN').html(Round2(result['min_freq']));
  }
}

/* Ventana de maximizar y de error de agregaciones */
var DialogElement           = null;
var PrevDialogContainer     = null;
var DialogResizeHandler     = null;
var DialogPrevResizeHandler = null;
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

/* Secuencia de inicio para cargar la pagina */
function UpdatePage() {
  GetSeriesScrollable();
  Scrollable_UpdateCounters(true);
}//426