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
  if(clear) FrequencyData = [];
  FrequencyData = GraphDataAppend(result, 'freq', FrequencyData);
  if(FrequencyChart == null) {
    FrequencyChart = new Dygraph(document.getElementById("FrequencyChart"), FrequencyData,
      { strokeWidth: 0.5, stepPlot: true, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ["Time", "Frequency"], dateWindow: [result['From'], result['To']],
        colors: [$('#FrequencyPanel').css('border-color')], plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], xAxisHeight: 24,
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
    new GraphTools('Frequency', FrequencyChart, '#toolbarFrequency', FrequencyResizeEvent, function() { exportToCsv('Frequency.csv', [['Index', 'Frequency']].concat(FrequencyData)); });
  } else {
    FrequencyChart.updateOptions( {'file': FrequencyData, valueRange: CalcRanges(FrequencyData, [1], false), dateWindow: [result['From'], result['To']]});
  }
  FrequencyChart.updateOptions({ valueRange: CalcRanges(FrequencyData, [1]) });
}
function FrequencyResizeEvent() {
  if(FrequencyChart != null) FrequencyChart.resize('100%', $('#FrequencyPanel > .panel-body > .row > div:first-child').height());
}
/* Contadores de frequencia */
var FrequencyCountersTimer  = null;
function Scrollable_UpdateCounters(times, renew) {
  if(renew === true) {
    $('#labelFrequencyAvgPrimary').text(times.PrimaryText + ' average');
    $('#labelFrequencyMaxPrimary').text(times.PrimaryText + ' maximum');
    $('#labelFrequencyMinPrimary').text(times.PrimaryText + ' minimum');
    $('#labelFrequencyAvgPrimaryValue, #labelFrequencyMaxPrimaryValue, #labelFrequencyMinPrimaryValue').text('Calculating...');
    $('#labelFrequencyAvgSecondary, #labelFrequencyMaxSecondary, #labelFrequencyMinSecondary').text(times.SecondaryText);
    $('#labelFrequencyAvgSecondaryValue, #labelFrequencyMaxSecondaryValue, #labelFrequencyMinSecondaryValue').text((times.SecondaryTo !== undefined) ? 'Calculating...' : '');
    if(times.SecondaryTo !== undefined) {
      Ajax.GetSeriesAggreg(times.SecondaryPeriod, times.SecondaryTo, times.PreferedAggreg, 'avg(freq), max(freq), min(freq)', 
        function(result) {
          if(result['Samples'] !== 0) {
            $('#labelFrequencyAvgSecondaryValue').html(Round2(result['avg_freq']) + '&nbsp;<em class="huge-small-unit">Hz</em>');
            $('#labelFrequencyMaxSecondaryValue').html(Round2(result['max_freq']) + '&nbsp;<em class="huge-small-unit">Hz</em>');
            $('#labelFrequencyMinSecondaryValue').html(Round2(result['min_freq']) + '&nbsp;<em class="huge-small-unit">Hz</em>');
          } else {
            $('#labelFrequencyAvgSecondaryValue, #labelFrequencyMaxSecondaryValue, #labelFrequencyMinSecondaryValue').text('No data'); 
          }
        }
      );      
    }
  }
  Ajax.GetSeriesAggreg(times.PrimaryPeriod, times.PrimaryTo, times.PreferedAggreg, 'avg(freq), max(freq), min(freq)', 
    function FrequencyUpdateCountersCurrent(result) {
      $('#labelFrequencyAvgPrimaryValue').html(Array_Round2([result['avg_freq']], ColorArray($('#FrequencyPanel').css('border-color'), 1)) + '<em class="huge-small-unit">Hz</em>');
      $('#labelFrequencyMaxPrimaryValue').html(Array_Round2([result['max_freq']], ColorArray($('#FrequencyPanel').css('border-color'), 1)) + '<em class="huge-small-unit">Hz</em>');
      $('#labelFrequencyMinPrimaryValue').html(Array_Round2([result['min_freq']], ColorArray($('#FrequencyPanel').css('border-color'), 1)) + '<em class="huge-small-unit">Hz</em>');
      window.dispatchEvent(new Event('resize'));
    }
  );
}
var MainScrollable = null;
function UpdatePage() {
  MainScrollable = new Scrollable( {Spinners: ['#FrequencySpinner'], NowBtn: '#toolbarFrequency button[data-function="Update"]', Series: ['freq'], OnSuccess: Scrollable_OnSuccess, Counters: Scrollable_UpdateCounters} );
}//426