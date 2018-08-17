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

/* Frequency graph */
var FrequencyChart   = null;
var FrequencyData    = [];
function Scrollable_OnSuccess(result, clear) {
  if(clear) FrequencyData = [[0, 0]];
  FrequencyData = GraphDataAppend(result, 'freq', FrequencyData);
  if(FrequencyChart == null) {
    FrequencyChart = new Dygraph(document.getElementById("FrequencyChart"), FrequencyData,
      { strokeWidth: 0.5, stepPlot: true, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ["Time", "Frequency"], dateWindow: [result['From'], result['To']],
        colors: $('#FreqeuncyPanel').css('border-color'), plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },
        underlayCallback: function(canvas, area, g) {
          var NF = Analyzers[Analyzer].NominalFreq, Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
          GraphHBackgroundRanges([[NF * 1.01, NF * 1.005, Yellow], [NF * 0.995, NF * 0.99, Yellow], [Infinity, NF * 1.01, Red], [NF * 0.99, -Infinity, Red]], g, canvas);
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
  FrequencyChart.updateOptions({ valueRange: CalcRanges(FrequencyData, [1]) });
}
function FrequencyResizeEvent() {
  var Height = $('#FreqAvgPanel').height() + $('#FreqMaxPanel').height() + $('#FreqMinPanel').height();
  FrequencyChart.resize($('#FrequencyCol').width(), Height - 4);
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
});

/* Contadores de frequencia */
var FrequencyCountersTimer  = null;
function Scrollable_UpdateCounters(change) {
  if(FrequencyCountersTimer != null) clearInterval(FrequencyCountersTimer);
  FrequencyCountersTimer = setTimeout(Scrollable_UpdateCounters, 60000);
  var Times = GetCountersTimePeriod();
  if(change === true) {
    $('#labelFrequencyAvgPrimary').text(Times.PrimaryText + ' average');
    $('#labelFrequencyMaxPrimary').text(Times.PrimaryText + ' maximum');
    $('#labelFrequencyMinPrimary').text(Times.PrimaryText + ' minimum');
    $('#labelFrequencyAvgSecondary').text(Times.SecondaryText + ' average');
    $('#labelFrequencyMaxSecondary').text(Times.SecondaryText + ' maximum');
    $('#labelFrequencyMinSecondary').text(Times.SecondaryText + ' minimum');
    $('#labelFrequencyAvgPrimaryValue').text('Calculating...');
    $('#labelFrequencyMaxPrimaryValue').text('Calculating...');
    $('#labelFrequencyMinPrimaryValue').text('Calculating...');
  }
  Ajax.GetSeriesAggreg(Times.PrimaryPeriod, Times.PrimaryTo, Times.PreferedAggreg, 'avg(freq), max(freq), min(freq)', 
    function FrequencyUpdateCountersCurrent(result) {
      $('#labelFrequencyAvgPrimaryValue').html(Round2(result['avg_freq']) + '&nbsp;<em class="huge-small-unit">Hz</em>');
      $('#labelFrequencyMaxPrimaryValue').html(Round2(result['max_freq']) + '&nbsp;<em class="huge-small-unit">Hz</em>');
      $('#labelFrequencyMinPrimaryValue').html(Round2(result['min_freq']) + '&nbsp;<em class="huge-small-unit">Hz</em>');
    }
  );
  if(Times.SecondaryTo !== undefined) {
    $('#labelFrequencyAvgSecondaryValue').text('Calculating...');
    $('#labelFrequencyMaxSecondaryValue').text('Calculating...');
    $('#labelFrequencyMinSecondaryValue').text('Calculating...');  
    Ajax.GetSeriesAggreg(Times.SecondaryPeriod, Times.SecondaryTo, Times.PreferedAggreg, 'avg(freq), max(freq), min(freq)', 
      function FrequencyUpdateCountersCurrent(result) {
        $('#labelFrequencyAvgSecondaryValue').html(Round2(result['avg_freq']) + '&nbsp;<em class="huge-small-unit">Hz</em>');
        $('#labelFrequencyMaxSecondaryValue').html(Round2(result['max_freq']) + '&nbsp;<em class="huge-small-unit">Hz</em>');
        $('#labelFrequencyMinSecondaryValue').html(Round2(result['min_freq']) + '&nbsp;<em class="huge-small-unit">Hz</em>');
      }
    );      
  }
}
/* Ventana de maximizar y de error de agregaciones */
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

var MainScrollable = null;
function UpdatePage() {
  MainScrollable = new Scrollable( {Spinners: ['#FrequencySpinner'], Series: 'freq', OnSuccess: Scrollable_OnSuccess} );
  Scrollable_UpdateCounters(true);
}//426