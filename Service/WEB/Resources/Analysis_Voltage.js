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

/* Voltage graph */
var VoltageChart     = null;
var VoltageData      = [];
function Scrollable_OnSuccess(result, clear) {
  if(clear) VoltageData = [];
  AjaxGetEvents(result['From'], result['To'], clear);
  VoltageData = GraphDataAppend(result, 'rms_v', VoltageData);
  if(VoltageChart === null) {
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"), VoltageData,
      { strokeWidth: 0.5, stepPlot: true, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(SeriesNames.Voltage()), dateWindow: [result['From'], result['To']],
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: ColorArray($('#VoltagePanel').css('border-color'), 3), plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ], xAxisHeight: 24,
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
    $(window).bind('resize', VoltageResizeEvent);
    window.dispatchEvent(new Event('resize'));
    new GraphTools('Voltage', VoltageChart, '#toolbarVoltage', VoltageResizeEvent, function() { exportToCsv('Voltage.csv', [['Time'].concat(SeriesNames.Voltage())].concat(VoltageData)); });
  } else {
    VoltageChart.updateOptions({file: VoltageData, dateWindow: [result['From'], result['To']]});
  }
  VoltageSeries();
}
function VoltageResizeEvent() { 
  if(VoltageChart !== null) VoltageChart.resize('100%', $('#VoltagePanel > .panel-body > .row > div:first-child').height());
}
function VoltageSeries(event) {
  GraphPhaseButtons(VoltageChart, $('#groupVoltagePhases'), 'VisibleVoltageMainPhases', false, function(series) { return CalcRanges(VoltageData, series, false); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupVoltagePhases').on('click', 'button', VoltageSeries);
/* Voltage counters */
function Scrollable_UpdateCounters(times, renew) {
  if(renew === true) {
    $('#labelVoltageAvgPrimary').text(times.PrimaryText + ' average');
    $('#labelVoltageMaxPrimary').text(times.PrimaryText + ' maximun');
    $('#labelVoltageMinPrimary').text(times.PrimaryText + ' minimun');
    $('#labelVoltageAvgPrimaryValue, #labelVoltageMaxPrimaryValue, #labelVoltageMinPrimaryValue').text('Calculating...');
    $('#labelVoltageAvgSecondary, #labelVoltageMaxSecondary, #labelVoltageMinSecondary').text(times.SecondaryText);
    $('#labelVoltageAvgSecondaryValue, #labelVoltageMaxSecondaryValue, #labelVoltageMinSecondaryValue').text((times.SecondaryTo !== undefined) ? 'Calculating...' : '');
    if(times.SecondaryTo !== undefined) {
      Ajax.GetSeriesAggreg(times.SecondaryPeriod, times.SecondaryTo, times.PreferedAggreg, 'avg(rms_v), max(rms_v), min(rms_v)',
        function(result) {
          if(result['Samples'] !== 0) {
            $('#labelVoltageAvgSecondaryValue').html(Array_Round2(result['avg_rms_v'], '&nbsp;') + '&nbsp;<em>V</em>');
            $('#labelVoltageMinSecondaryValue').html(Array_Round2(result['min_rms_v'], '&nbsp;') + '&nbsp;<em>V</em>');
            $('#labelVoltageMaxSecondaryValue').html(Array_Round2(result['max_rms_v'], '&nbsp;') + '&nbsp;<em>V</em>');
          } else {
            $('#labelVoltageAvgSecondaryValue, #labelVoltageMaxSecondaryValue, #labelVoltageMinSecondaryValue').text('No data'); 
          }
          window.dispatchEvent(new Event('resize'));
        }
      );
    }
  }
  Ajax.GetSeriesAggreg(times.PrimaryPeriod, times.PrimaryTo, times.PreferedAggreg, 'avg(rms_v), max(rms_v), min(rms_v)', 
    function(result) {
      $('#labelVoltageAvgPrimaryValue').html(Array_Round2(result['avg_rms_v'], '&nbsp;', ColorArray($('#VoltagePanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">V</em>');
      $('#labelVoltageMinPrimaryValue').html(Array_Round2(result['min_rms_v'], '&nbsp;', ColorArray($('#VoltagePanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">V</em>');
      $('#labelVoltageMaxPrimaryValue').html(Array_Round2(result['max_rms_v'], '&nbsp;', ColorArray($('#VoltagePanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">V</em>');
      window.dispatchEvent(new Event('resize'));
    }
  );
}
var WaveformDataset = [], HarmonicsDataset = [], FFTDataset = [];
function Live_OnSuccess(result) {
  $('#DetailTime').text('Last update at ' + new Date(result['Sampletime']).toLocaleTimeString() + '.' + new Date(result['Sampletime']).getMilliseconds());
  WaveformDataset = [];
  HarmonicsDataset = [];
  FFTDataset = [];
  for(var i = 0; i < result['wave_v'][0].length; i++) {
    var Row = [i];
    for(n = 0; n < result['wave_v'].length; n++) Row.push(result['wave_v'][n][i]);
    for(n = 0; n < result['wave_i'].length; n++) Row.push(result['wave_i'][n][i]);
    WaveformDataset.push(Row);
  }
  for(var i = 0; i < 50; i++) {
    var Row = [];
    for(n = 0; n < result['harmonics_v'].length; n++) Row.push(result['harmonics_v'][n][i]);
    for(n = 0; n < result['harmonics_i'].length; n++) Row.push(result['harmonics_i'][n][i]);
    HarmonicsDataset.push(Row);
  }  
  for(var i = 0; i < result['fft_v'][0].length; i++) {
    var Row = [];
    for(n = 0; n < result['harmonics_v'].length; n++) Row.push(result['fft_v'][n][i]);
    for(n = 0; n < result['harmonics_i'].length; n++) Row.push(result['fft_i'][n][i]);
    FFTDataset.push(Row);
  }  
  UpdateWaveformChart();
  UpdateHarmonicsChart();
  UpdateFFTChart();
  UpdatePhasorChart(result);
  SetCubismNowLabel(PowerFactorChart, result['power_factor']);
  SetCubismNowLabel(THDChart, result['thd_v']); 
  SetCubismNowLabel(PHIChart, result['phi']); 
}
function DetailTabChange(index) {
  $('#groupDetailPhasesSingle, #groupDetailPhasesMulti, #btnShowDetail1Harmonic, #btnShowDetailLog, #btnShowDetailCurrent, #toolbarDetails button[data-function="Maximize"]').hide();
  if((index === 0) || (index === 1) || (index === 3)) $('#groupDetailPhasesMulti').show();
  if((index === 0) || (index === 1) || (index === 3)) $('#btnShowDetailCurrent').show();
  if((index === 0) || (index === 1) || (index === 3)) $('#toolbarDetails button[data-function="Maximize"]').show();
  if((index === 2) || (index === 5)) $('#groupDetailPhasesSingle').show();
  if((index === 1) || (index === 3)) $('#btnShowDetailLog').show();
  if(index === 1) $('#btnShowDetail1Harmonic').show();
  if(WaveformChart !== null) {
    if(index === 0) $(WaveformChart.graphDiv).parent().removeAttr('style');
    UpdateWaveformChart();
  }
  if(HarmonicsChart !== null) {
    if(index === 1) $(HarmonicsChart.graphDiv).parent().removeAttr('style');
    UpdateHarmonicsChart();
  }
  if(FFTChart !== null) {
    if(index === 3) $(FFTChart.graphDiv).parent().removeAttr('style');
    UpdateFFTChart();
  }
  if(PhasorChart !== null) {
    if(index === 4) PhasorResizeEvent();
  }
  if(HarmonicsCharts.length !== 0) {
    if(index === 2) {
      for(i = 0; i < 50; i++) $(HarmonicsCharts[i].graphDiv).parent().removeAttr('style');
    }
  }
  if(PowerFactorChart !== null) {
    if(index === 5) {
      $(PowerFactorChart.graphDiv).parent().removeAttr('style');
      $(THDChart.graphDiv).parent().removeAttr('style');
      $(PHIChart.graphDiv).parent().removeAttr('style');
    }
  }
  if(EventChart !== null) {
    if(index === 6) {
      $(EventChart.graphDiv).parent().removeAttr('style');
      EventsTable.columns.adjust();
    }
  }
}
function PhasorResizeEvent() {
  var Width = $('#PhasorChart').parent().width();
  if(PhasorChart !== null) PhasorChart.resize(Width, Width);
}
function DetailsButtonUpdate(event) {
  GroupButtons($('#groupDetailPhasesMulti'), 'DetailPhases', Math.max(SeriesNames.Current().length, SeriesNames.Voltage().length), false, undefined, undefined);
  if(localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') {
    $('#btnShowDetail1Harmonic').addClass('active');
  } else {
    $('#btnShowDetail1Harmonic').removeClass('active');
  }
  if(localStorage.getItem('VoltageDetailCurrent') === 'true') {
    $('#btnShowDetailCurrent').addClass('active');
  } else {
    $('#btnShowDetailCurrent').removeClass('active');
  }
  if(localStorage.getItem('VoltageDetailLog') === 'true') {
    $('#btnShowDetailLog').addClass('active');
  } else {
    $('#btnShowDetailLog').removeClass('active');
  }
}
$('#groupDetailPhasesMulti').on('click', 'button', function(evt) {
  GroupButtons($('#groupDetailPhasesMulti'), 'VoltageDetailVisiblePhasesMulti', Math.max(SeriesNames.Current().length, SeriesNames.Voltage().length), false,
    function() { UpdateWaveformChart(); UpdateHarmonicsChart(); }, evt.target);
});
$('#btnShowDetailCurrent').click(function() {
  localStorage.setItem('VoltageDetailCurrent', (localStorage.getItem('VoltageDetailCurrent') === 'true') ? 'false' : 'true');
  DetailsButtonUpdate();
  UpdateWaveformChart();
  UpdateHarmonicsChart();
  UpdateFFTChart();
  if(WaveformChart !== null) { WaveformChart.updateOptions({ showRangeSelector: true}); WaveformChart.updateOptions({ showRangeSelector: false}); }
  if(HarmonicsChart !== null) { HarmonicsChart.updateOptions({ showRangeSelector: true}); HarmonicsChart.updateOptions({ showRangeSelector: false}); }
  if(FFTChart !== null) { FFTChart.updateOptions({ showRangeSelector: true}); FFTChart.updateOptions({ showRangeSelector: false}); }
});
$('#btnShowDetail1Harmonic').click(function() {
  localStorage.setItem('VoltageDetailFirstHarmonic', (localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') ? 'false' : 'true');
  DetailsButtonUpdate();
  UpdateWaveformChart();
  UpdateHarmonicsChart();
});
$('#btnShowDetailLog').click(function() {
  localStorage.setItem('VoltageDetailLog', (localStorage.getItem('VoltageDetailLog') === 'true') ? 'false' : 'true');
  DetailsButtonUpdate();
  UpdateWaveformChart();
  UpdateHarmonicsChart();
  UpdateFFTChart();
});
$('#groupDetailPhasesSingle').on('click', 'button', ParametersSeries);
/* Waveform view */
var WaveformChart = null;
function UpdateWaveformChart() {
  var VoltageVisibility = [], CurrentVisibility = [], Y1Range = [], Y2Range = [], Series = {};
  var ShowCurrent = (localStorage.getItem("VoltageDetailCurrent") === 'true');
  GroupButtons($('#groupDetailPhasesMulti'), 'VoltageDetailVisiblePhasesMulti', Math.max(SeriesNames.Current().length, SeriesNames.Voltage().length), false, 
    function(VisibleSeries, Visibles) {
      VoltageVisibility = VisibleSeries.slice(0, SeriesNames.Voltage().length);
      CurrentVisibility = VisibleSeries.slice(0, SeriesNames.Current().length);
      if(ShowCurrent === false) CurrentVisibility.fill(false);
      Y1Range = CalcRanges(WaveformDataset, Visibles);
      Y2Range = CalcRanges(WaveformDataset, Visibles.map(function(val) { return val + SeriesNames.Voltage().length }));
      SeriesNames.Current().map(function(val) {Series[val] = { axis: 'y2' }; });
      var Options = { 
        strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Sample'].concat(SeriesNames.Voltage()).concat(SeriesNames.Current()), series: Series, visibility: VoltageVisibility.concat(CurrentVisibility),
        colors: ColorArray($('#VoltagePanel').css('border-color'), SeriesNames.Voltage().length).concat(ColorArray('#428bca', SeriesNames.Current().length)), plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: Y1Range, 
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">V</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
          },
          y2: { pixelsPerLabel: 15, axisLabelWidth: ShowCurrent ? 34 : 0, valueRange: Y2Range, 
                valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">A</em>'; },
                axisLabelFormatter: function (x) { return (ShowCurrent == true) ? Round2(x) : ''; }
          },
          x: { ticker: function(min, max) { return GraphWaveTimeTicks(max); },
               valueFormatter: function (x) { return Round2((x * 1000) / Analyzers[Analyzer]['SampleFreq']) + ' <em class="x-small">ms</em>'; }
             }
        }
      };
      if(WaveformChart === null) {
        WaveformChart = new Dygraph(document.getElementById("WaveformChart"), WaveformDataset, Options);
      } else {
        WaveformChart.updateOptions(Options, true);
        WaveformChart.updateOptions({ file: WaveformDataset });
      }
    }, undefined);
}
/* Grafica de armonicos */
var HarmonicsChart      = null;
var HarmonicsVOffset    = 0;
var HarmonicsCOffset    = 0;
function UpdateHarmonicsChart() {
  var ToShow      = [];
  var ShowCurrent = (localStorage.getItem("VoltageDetailCurrent") === 'true');
  var ShowFirst   = (localStorage.getItem('VoltageDetailFirstHarmonic') === 'true');
  var Log         = (localStorage.getItem('VoltageDetailLog') === 'true');
  for(i = ShowFirst ? 0 : 1; i < HarmonicsDataset.length; i++) {
    var Row = [i];
    for(n = 0; n < HarmonicsDataset[i].length; n++) Row.push(Log ? 20.0 * Math.log10(Math.max(0.001, HarmonicsDataset[i][n])) : HarmonicsDataset[i][n]);
    ToShow.push(Row);
  }
  var VoltageVisibility = [], CurrentVisibility = [], Y1Range = [], Y2Range = [], Series = {};
  GroupButtons($('#groupDetailPhasesMulti'), 'VoltageDetailVisiblePhasesMulti', Math.max(SeriesNames.Current().length, SeriesNames.Voltage().length), false, 
    function(VisibleSeries, Visibles) {
      VoltageVisibility = VisibleSeries.slice(0, SeriesNames.Voltage().length);
      CurrentVisibility = VisibleSeries.slice(0, SeriesNames.Current().length);
      if(ShowCurrent === false) CurrentVisibility.fill(false);
      Y1Range = CalcRanges(ToShow, Visibles, !Log);
      Y2Range = CalcRanges(ToShow, Visibles.map(function(val) { return val + SeriesNames.Voltage().length }), !Log);
      HarmonicsVOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
      HarmonicsCOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
      for(i = 0; i < ToShow.length; i++) {
        var Row = [ToShow[i][0]];
        for(n = 0; n < SeriesNames.Voltage().length; n++) Row.push(ToShow[i][n + 1] + HarmonicsVOffset);
        for(n = 0; n < SeriesNames.Current().length; n++) Row.push(ToShow[i][n + 1 + SeriesNames.Voltage().length] + HarmonicsCOffset);
        ToShow[i] = Row;
      }
      SeriesNames.Current().map(function(val) {Series[val] = { axis: 'y2' }; });
      var Options = { 
        strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { }, plotter: multiColumnBarPlotter, dateWindow: [ToShow[0][0] - 0.5, ToShow[ToShow.length - 1][0] + 0.5],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Harmonic'].concat(SeriesNames.Voltage()).concat(SeriesNames.Current()), series: Series,
        colors: ColorArray($('#VoltagePanel').css('border-color'), SeriesNames.Voltage().length).concat(ColorArray('#428bca', SeriesNames.Current().length)), visibility: VoltageVisibility.concat(CurrentVisibility),
        axes: { 
          y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: [Y1Range[0] + HarmonicsVOffset, Y1Range[1] + HarmonicsVOffset],
               ticker: Log ? function(min, max) { return GraphDBTicks(min, max, HarmonicsVOffset); } : Dygraph.numericTicks,
               valueFormatter: function(x) { return Round2(x - HarmonicsVOffset) + '<em class="x-small">' + (Log ? 'dB' : 'V') + '</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          y2: { pixelsPerLabel: 15, axisLabelWidth: ShowCurrent ? 34 : 0, valueRange: [Y2Range[0] + HarmonicsCOffset, Y2Range[1] + HarmonicsCOffset],
                valueFormatter: function(x) { return Round2(x - HarmonicsCOffset) + ' <em class="x-small">' + (Log ? 'dB' : 'A') + '</em>'; },
                ticker: (Log || ShowCurrent === false) ? function(min, max) { return (ShowCurrent === true) ? GraphDBTicks(min, max, HarmonicsCOffset) : []; } : Dygraph.numericTicks,
                axisLabelFormatter: function (x) { return Round2(x); }
              },
          x: { drawGrid: false, ticker: GraphOddHarmonics, axisLabelFormatter: function (x) { return SeriesNames.Harmonics[x]; },
               valueFormatter: function(y) { return SeriesNames.Harmonics[y] + ' Harmonic (' + (Analyzers[Analyzer].NominalFreq * (y + 1)) + '<em class="small"> Hz</em>)'; }           
             }
        }
      };
      if(HarmonicsChart === null) {
        HarmonicsChart = new Dygraph(document.getElementById("HarmonicsChart"), ToShow, Options);
      } else {
        HarmonicsChart.updateOptions(Options, true);
        HarmonicsChart.updateOptions({ file: ToShow });
      }
  }, undefined);
}
/* Grafica de FFT */
var FFTChart            = null;
var FFTVOffset          = 0;
var FFTCOffset          = 0;
function UpdateFFTChart() {
  var ToShow      = [];
  var ShowCurrent = (localStorage.getItem("VoltageDetailCurrent") === 'true');
  var Log         = (localStorage.getItem('VoltageDetailLog') === 'true');
  for(i = 0; i < FFTDataset.length; i++) {
    var Row = [i];
    for(n = 0; n < FFTDataset[i].length; n++) Row.push(Log ? 20.0 * Math.log10(Math.max(0.001, FFTDataset[i][n])) : FFTDataset[i][n]);
    ToShow.push(Row);
  }
  var VoltageVisibility = [], CurrentVisibility = [], Y1Range = [], Y2Range = [], Series = {};
  GroupButtons($('#groupDetailPhasesMulti'), 'VoltageDetailVisiblePhasesMulti', Math.max(SeriesNames.Current().length, SeriesNames.Voltage().length), false, 
    function(VisibleSeries, Visibles) {
      VoltageVisibility = VisibleSeries.slice(0, SeriesNames.Voltage().length);
      CurrentVisibility = VisibleSeries.slice(0, SeriesNames.Current().length);
      if(ShowCurrent === false) CurrentVisibility.fill(false);
      Y1Range = CalcRanges(ToShow, Visibles, !Log);
      Y2Range = CalcRanges(ToShow, Visibles.map(function(val) { return val + SeriesNames.Voltage().length }), !Log);
      FFTVOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
      FFTCOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
      for(i = 0; i < ToShow.length; i++) {
        var Row = [i];
        for(n = 0; n < SeriesNames.Voltage().length; n++) Row.push(ToShow[i][n + 1] + FFTVOffset);
        for(n = 0; n < SeriesNames.Current().length; n++) Row.push(ToShow[i][n + 1 + SeriesNames.Voltage().length] + FFTCOffset);
        ToShow[i] = Row;
      }
      SeriesNames.Current().map(function(val) {Series[val] = { axis: 'y2' }; });
      var Options = { 
        strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], fillGraph: true, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Bin'].concat(SeriesNames.Voltage()).concat(SeriesNames.Current()), series: Series, plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
        colors: ColorArray($('#VoltagePanel').css('border-color'), SeriesNames.Voltage().length).concat(ColorArray('#428bca', SeriesNames.Current().length)), visibility: VoltageVisibility.concat(CurrentVisibility),
        axes: { 
          y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: [Y1Range[0] + FFTVOffset, Y1Range[1] + FFTVOffset], 
               ticker: Log ? function(min, max) { return GraphDBTicks(min, max, FFTVOffset); } : Dygraph.numericTicks,
               valueFormatter: function(x) { return Round2(x - HarmonicsVOffset) + '<em class="x-small">' + (Log ? 'dB' : 'V') + '</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          y2: { pixelsPerLabel: 15, axisLabelWidth: ShowCurrent ? 34 : 0, valueRange: [Y2Range[0] + FFTCOffset, Y2Range[1] + FFTCOffset],
                valueFormatter: function(x) { return Round2(x - FFTCOffset) + ' <em class="x-small">' + (Log ? 'dB' : 'A') + '</em>'; },
                ticker: (Log || ShowCurrent === false) ? function(min, max) { return (ShowCurrent === true) ? GraphDBTicks(min, max, FFTCOffset) : []; } : Dygraph.numericTicks,
                axisLabelFormatter: function (x) { return Round2(x); }
              },
          x: { drawGrid: true,  
               valueFormatter: function (y) { return Math.round(y / 10 * Analyzers[Analyzer].NominalFreq) + '<em class="small"> Hz</em>'; },
               axisLabelFormatter: function (y) {
                 return;
             y = (y / 10 * Nodes['NominalFreq']);
             if(y >= 1000) {
               y = y / 1000;
               if((y % 1) != 0) return "";
               return y + '<em class="small"> kHz</em>';
             } else {
               if((y % 1) != 0) return "";
               return y + '<em class="small"> Hz</em>';
             }
           }
           
         }
    }
  };
    if(FFTChart === null) {
      FFTChart = new Dygraph(document.getElementById("FFTChart"), ToShow, Options);
    } else {
      FFTChart.updateOptions(Options, true);
      FFTChart.updateOptions({ file: ToShow });
    }
  }, undefined);
}
/* Evolucion de parametros */
var HarmonicsCharts = [], HarmonicsData = [], PowerFactorChart = null, PowerFactorData = [], THDChart = null, THDData = [], PHIChart = null, PHIData = [];
function PeriodView_OnSuccess(result) {
  while(HarmonicsData.length !== 50) HarmonicsData.push([]);
  for(k = 0; k < 50; k++) {
    for(i = 0; i < result['harmonics_v'].length; i++) {
      var Row = [new Date(result['Sampletime'][i])];
      for(j = 0; j < result['harmonics_v'][i].length; j++) Row.push(20 * Math.log10(Math.max(0.000000001, result['harmonics_v'][i][j][k])));
      HarmonicsData[k].push(Row);
    }
    ShiftArray(HarmonicsData[k], result['MaxSamples']);
  }
  PowerFactorData = GraphDataAppend(result, 'power_factor', PowerFactorData);
  THDData = GraphDataAppend(result, 'thd_v', THDData);
  PHIData = GraphDataAppend(result, 'phi', PHIData);
  if(HarmonicsCharts.length === 0) {
    for(i = 0; i < 50; i++) {
      HarmonicsCharts[i] = AppendCubismGraph($('#EvolutionPanel'), HarmonicsData[i], SeriesNames.Harmonics[i] + ' harmonic', [result['From'], result['To']], function (x) { return (x === null) ? '~' : Round2(x) + '<em>dB</em>' });
    }
    Dygraph.synchronize(HarmonicsCharts, { range: false, zoom: false });
    PowerFactorChart = AppendCubismGraph($('#ParametersPanel'), PowerFactorData, "Power Factor", [result['From'], result['To']], function (x) { return (x === null) ? '~' : Round2(x); });
    THDChart         = AppendCubismGraph($('#ParametersPanel'), THDData, "Voltage THD", [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(x * 100.0) + '<em>%</em>'); });
    PHIChart         = AppendCubismGraph($('#ParametersPanel'), PHIData, "Angle", [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(Math.abs(x)) + '<em>' + ((x < 0) ? 'º CAP' : (x > 0) ? 'º IND' : 'º') + '</em>'); });
    Dygraph.synchronize([PowerFactorChart, THDChart, PHIChart], { range: false, zoom: false });
  } else {
    for(var i = 0; i < 50; i++) HarmonicsCharts[i].updateOptions({ file: HarmonicsData[i], dateWindow: [result['From'], result['To']] });
    PowerFactorChart.updateOptions({ file: PowerFactorData, dateWindow: [result['From'], result['To']] });
    THDChart.updateOptions({ file: THDData, dateWindow: [result['From'], result['To']] });
    PHIChart.updateOptions({ file: PHIData, dateWindow: [result['From'], result['To']] });
  }
  ParametersSeries();
}
function ParametersSeries(event) {
  GraphPhaseButtons(PowerFactorChart, $('#groupDetailPhasesSingle'), 'VoltageDetailVisiblePhasesSingle', true, function(series) { var Range = ArrayAbsMax(CalcRanges(PowerFactorData, series, false)); return [-Range, Range]; }, (event !== undefined) ? event.target : undefined); 
  GraphPhaseButtons(PHIChart,         $('#groupDetailPhasesSingle'), 'VoltageDetailVisiblePhasesSingle', true, function(series) { var Range = ArrayAbsMax(CalcRanges(PHIData, series, false, false, false, true)); return [-Range, Range]; }, undefined); 
  GraphPhaseButtons(THDChart,         $('#groupDetailPhasesSingle'), 'VoltageDetailVisiblePhasesSingle', true, function(series) { var Range = ArrayAbsMax(CalcRanges(THDData, series, false, false, false, true)); return [-Range, Range];}, undefined); 
  for(var g = 0; g < 50; g++) GraphPhaseButtons(HarmonicsCharts[g], $('#groupDetailPhasesSingle'), 'VoltageDetailVisiblePhasesSingle', true, function(series) { return CalcRanges(HarmonicsData[g], series, false); }, undefined); 
}
/* Grafico de fasores */
var PhasorChart = null;
var PhasorTable = null;
function UpdatePhasorChart(result) {
  var Vectors = [];
  var MaxCurrent = 0;
  var MaxVoltage = 0;
  for(i = 0; i < result['rms_v'].length; i++) {
    Vectors.push([2 * i, 120 * i, result['rms_v'][i]]);
    Vectors.push([2 * i + 1, 120 * i - result['phi'][i], result['rms_i'][i]]);
    MaxVoltage = Math.max(MaxVoltage, result['rms_v'][i]);
    MaxCurrent = Math.max(MaxCurrent, result['rms_i'][i]);
  }
  for(i = 0; i < Vectors.length; i += 2) {
    Vectors[i][2] = Vectors[i][2] / MaxVoltage;
    Vectors[i + 1][2] = Vectors[i + 1][2] / MaxCurrent;
  }
  var Options = { 
    strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], highlightCircleSize: 0,  interactionModel: { }, legend: 'never', valueRange: [-115, 115], dateWindow: [-115, 115], plotter: polarLinePlotter,
    labels: ['Index', 'Phase', 'Magnitude'], colors: ['#d9534f', '#428bca'], xAxisHeight: 0.1, xLabelHeight: 0.1, plugins: [ new Dygraph.Plugins.PolarCrosshair( { } ) ],
    axes: {
      y: { drawAxis: true, drawGrid: false, axisLabelWidth: 0.0, axisLineColor: "#FFF",
           ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             var Width = document.getElementById("PhasorChart").clientWidth / 2 - 1;
             for(var i = 1; i <= 4; i++) {
               ticks.push({ v: i * 25, label: '<div style="font-size: 11px; background: rgba(255, 255, 255, 0.8); margin-left: ' + Width + 'px; text-align: left; width: 44px; line-height: 12px">' + Round2(i / 4 * MaxVoltage) + '<em>V</em></div>' });
             }
             return ticks;
           },            
         },
      x: { drawAxis: true, drawGrid: false, axisLabelWidth: 0, axisLineColor: "#FFF",
           ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             var Height = document.getElementById("PhasorChart").clientHeight / 2 + 2;
             for(var i = 1; i <= 4; i++) {
               ticks.push({ v: -i * 25, label: '<div style="font-size: 11px; background: rgba(255, 255, 255, 0.8); margin-left: -20px; margin-top: ' + -Height + 'px; text-align: center; width: 40px; line-height: 12px">' + Round2(i / 4 * MaxCurrent) + '<em>A</em></div>' });
             }
             return ticks;
           }
         }
      }
    };
  if(PhasorChart === null) {
    PhasorChart = new Dygraph(document.getElementById("PhasorChart"), Vectors, Options);
    $(window).bind('resize', PhasorResizeEvent);
  } else {
    PhasorChart.updateOptions({ file: Vectors });
    PhasorChart.updateOptions(Options);
  }
  if(PhasorTable === null) {
    PhasorTable = $('#PhasorsTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                                 columnDefs: [ { targets: [0, 1, 2, 3], searchable: false } ],                            
    });
  }
  PhasorTable.clear();
  for(i = 0; i < result['rms_v'].length; i++) {
    var PhaseName = "L";
    if(result['rms_v'].length > 1) {
      if(i === 0) PhaseName = "R";
      if(i === 1) PhaseName = "S";
      if(i === 2) PhaseName = "T";
    }
    PhasorTable.row.add( [PhaseName, "Voltage", Round2(i * 120) + 'º', Round2(result['rms_v'][i]) + '<em> V</em>'] ).draw(false);
    PhasorTable.row.add( [PhaseName, "Current", Round2(i * 120 + -result['phi'][i]) + 'º', Round2(result['rms_i'][i]) + '<em> A</em>'] ).draw();
  }
  PhasorResizeEvent();
}
/* Tabla de eventos */
var LastEventStartTime = 0;
var EventsDatasheet    = [];
var EventsTable        = null;
//var EventChart         = null;
//var EventSpinner       = new Spinner(SpinnerOpts);
function AjaxGetEvents(from, to, clear) {
  if(clear) LastEventStartTime = 0;
  Ajax.GetEvents(to - from, to, LastEventStartTime,
    function(result) {
      if(EventsTable === null) EventsTable = $('#EventsTable').DataTable({ responsive: false, paging: false, scrollCollapse: true, scrollY: '224px', dom: '<"top float-left"i><"top float-right"f><"clear">t<"clear">', language: { info: "_MAX_ events in selected period", infoEmpty: "No events in selection"}, columnDefs: [ { targets: [5], orderable: false }, { targets: [0, 1, 2, 3, 4], searchable: true } ], order: [0, 'desc']});
      if(LastEventStartTime === 0) {
        EventsTable.clear().draw();
        EventsDatasheet = [];
      }
      if(result['Starttime'] !== null) {
        for(i = 0; i < result['Starttime'].length; i++) {
          var Time = new Date(result['Starttime'][i]);
          var RowText = '<tr>' +
            '<td data-search="' + result['Starttime'][i] + '">' + Time.toLocaleString() + '.' + Time.getMilliseconds() + '</td>' +
            '<td>' + result['Type'][i] + '</td>' +
            '<td>' + Round2(result['Duration'][i] / Analyzers[Analyzer]['SampleFreq']) + '</td>' +
            '<td>' + Round2(result['Voltage'][i]) + '</td>' +
            '<td>' + Round2(result['Percentage'][i]) + '</td>' +
            '<td><button class="btn btn-xs btn-primary get_samples"><i class="fa fa-line-chart fa-fw"></i> Get samples</button></td>' +
            '</tr>';
          EventsTable.row.add($(RowText)).draw();
          LastEventStartTime = result['Starttime'][i];
          EventsDatasheet.push(result['Starttime'][i]);
        }
      }
      if(EventChart === null) {
        EventChart = new Dygraph(document.getElementById("EventChart"), [Array(SeriesNames.Voltage().length + 1).fill(0)], {
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], labels: ['Time'].concat(SeriesNames.Voltage()), legend: 'follow', legendFormatter: LegendFormatter_Follow,
          colors: ColorArray($('#VoltagePanel').css('border-color'), 3), plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
          interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
        });
        $('#EventChart').append('<div class="no-data"><div class="alert alert-info float-center">Press <strong><em>Get samples</em></strong> to view the waveform of an event.</div></div>');
        $('#EventSpinner').hide();
      }
    }
  );
}
$("table#EventsTable").on('click', 'button.get_samples', function () {
  var Row = EventsTable.row($(this).parents('tr'));
  $('#EventChart div.no-data').addClass('hidden');
  EventsTable.rows().nodes().to$().removeClass('info');
  $(Row.node()).addClass('info');
  ShowSpinner(EventSpinner, 'EventSpinner');
  $.ajax({
    url: "getEventSamples",
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data: JSON.stringify({ Node: Nodes[Node].ID, Starttime: EventsDatasheet[Row.index()] }),
    success: AjaxSamples
  });
});
function AjaxSamples(result) {
  StopSpinner(EventSpinner, 'EventSpinner');
  var ToShow = [];
  for(i = 0; i < result['Samples'].length; i++) {
    if((result['Duration'] > result['Envelope'] * 4) && (i > result['Envelope'] * 2)) {
      ToShow.push([i + 100].concat(result['Samples'][i]));
    } else {
      ToShow.push([i].concat(result['Samples'][i]));
    }
    if((result['Duration'] > result['Envelope'] * 4) && (i === result['Envelope'] * 2)) {
      for(n = 0; n < 100; n++) ToShow.push([i + n + 1].concat(Nodes[Node]['Type'] == '3Phase') ? ['NaN', 'NaN', 'NaN'] : ['NaN']);
    }
  }
  var Options = {
    panEdgeFraction: 0.000000001,
    file: ToShow,
    dateWindow: [0, result['Samples'].length + ((result['Duration'] > result['Envelope'] * 4) ? 100 : 0)],
    valueRange: CalcRanges(ToShow, (Nodes[Node]['Type'] == '3Phase') ? [1, 2, 3] : [1]),
    underlayCallback: function(canvas, area, g) {
      canvas.fillStyle = "#EEE";
      canvas.fillRect(g.toDomXCoord(0), 0, g.toDomXCoord(result['Envelope']) - g.toDomXCoord(0), g.height_);
      canvas.fillRect(g.toDomXCoord(ToShow.length - result['Envelope']), 0, g.toDomXCoord(ToShow.length), g.height_);
      var NominalVoltage = Nodes['NominalVoltage'] * Math.SQRT2;
      canvas.beginPath();
      canvas.moveTo(g.toDomXCoord(0), g.toDomYCoord(NominalVoltage));
      canvas.lineTo(g.width_, g.toDomYCoord(NominalVoltage));
      canvas.moveTo(g.toDomXCoord(0), g.toDomYCoord(-NominalVoltage));
      canvas.lineTo(g.width_, g.toDomYCoord(-NominalVoltage));
      canvas.strokeStyle = '#f00';
      canvas.lineWidth = 0.5;
      canvas.stroke();
      if(result['Duration'] > result['Envelope'] * 4) {
        canvas.fillStyle = "#fff";
        canvas.fillRect(g.toDomXCoord(result['Envelope'] * 2), 0, g.toDomXCoord(result['Envelope'] * 2 + 100) - g.toDomXCoord(result['Envelope'] * 2), g.height_);
        canvas.beginPath();
        canvas.moveTo(g.toDomXCoord(result['Envelope'] * 2), 0);
        canvas.lineTo(g.toDomXCoord(result['Envelope'] * 2), g.height_);
        canvas.moveTo(g.toDomXCoord(result['Envelope'] * 2 + 100), 0);
        canvas.lineTo(g.toDomXCoord(result['Envelope'] * 2 + 100), g.height_);
        canvas.strokeStyle = '#000';
        canvas.lineWidth = 0.5;
        canvas.stroke();
        canvas.save();
        canvas.translate(g.toDomXCoord(result['Envelope'] * 2 + 50), g.height_ / 2);
        canvas.rotate(-Math.PI / 2);
        canvas.textAlign = 'center';
        canvas.textBaseline = 'middle';
        canvas.font = "12px sans-serif";
        canvas.fillStyle = "#555";
        canvas.fillText(Round2Plain((result['Duration'] - result['Envelope'] * 2) / Nodes[Node]['SampleFreq']) + " seconds gap", 0 , 0, g.height_);
        canvas.restore();
      }
      canvas.beginPath();
      canvas.moveTo(g.toDomXCoord(result['Envelope']), 0);
      canvas.lineTo(g.toDomXCoord(result['Envelope']), g.height_);
      canvas.moveTo(g.toDomXCoord(ToShow.length - result['Envelope']), 0);
      canvas.lineTo(g.toDomXCoord(ToShow.length - result['Envelope']), g.height_);
      canvas.strokeStyle = '#000';
      canvas.lineWidth = 0.5;
      canvas.stroke();
    },
    axes: {
      y: {
        valueFormatter: function (x) {
          return Round2(x) + ' <em class="small">V</em>';
        },
        axisLabelFormatter: function (x) {
          return '<small>' + Round2(x) + ' <em class="small">V</em></small>';
        }
      },
      x: {
        valueFormatter: function (y) {
          var Label = '';
          if((result['Duration'] > result['Envelope'] * 4) && (y <= result['Envelope'] * 2)) {
            Label = Round2((y - result['Envelope']) / Nodes[Node]['SampleFreq'] * 1000);
          } else if((result['Duration'] > result['Envelope'] * 4) && (y > result['Envelope'] * 3 + 100)) {
            Label = 'd+' + Round2((y - result['Envelope'] * 3  - 100) / Nodes[Node]['SampleFreq'] * 1000);
          } else if((result['Duration'] > result['Envelope'] * 4) && (y > result['Envelope'] * 2 + 100)) {
            Label = 'd-' + Round2((y - result['Envelope'] * 3  - 100) / Nodes[Node]['SampleFreq'] * -1000);
          } else if(result['Duration'] > result['Envelope'] * 4) {
            Label = "";
          } else {
            Label = Round2((y - result['Envelope']) / Nodes[Node]['SampleFreq'] * 1000);
          }
          return Label + ' ms';
        },
        axisLabelFormatter: function (y) {
          var Label = '';
          if((result['Duration'] > result['Envelope'] * 4) && (y <= result['Envelope'] * 2)) {
            Label = Round2((y - result['Envelope']) / Nodes[Node]['SampleFreq'] * 1000);
          } else if((result['Duration'] > result['Envelope'] * 4) && (y > result['Envelope'] * 3 + 100)) {
            Label = 'd+' + Round2((y - result['Envelope'] * 3  - 100) / Nodes[Node]['SampleFreq'] * 1000);
          } else if((result['Duration'] > result['Envelope'] * 4) && (y > result['Envelope'] * 2 + 100)) {
            Label = 'd-' + Round2((y - result['Envelope'] * 3  - 100) / Nodes[Node]['SampleFreq'] * -1000);
          } else if(result['Duration'] > result['Envelope'] * 4) {
            Label = "";
          } else {
            Label = Round2((y - result['Envelope']) / Nodes[Node]['SampleFreq'] * 1000);
          }
          return '<div style="font-size: 10px; margin-top: -4px">' + Label + ' <em class="small">ms</em></div>';
        }
      }
    }
  };
  EventChart.updateOptions(Options);
  EventLoaded = true;
}
function ExportDetail() {
  var Data = [];
  var Filename;
  if(DetailTabSelector.GetSaved() === 0) {
    exportToCsv('Waveform.csv', [['Time'].concat(SeriesNames.Voltage()).concat(SeriesNames.Current())].concat(WaveformDataset));
  } else if(DetailTabSelector.GetSaved() === 3) {
    var Header = ['Index'];
    for(var i = 0; i < SeriesNames.Voltage().length; i++) { Header.push(SeriesNames.Voltage()[i]); Header.push(SeriesNames.Voltage()[i] + ' (dB)'); }
    for(var i = 0; i < SeriesNames.Current().length; i++) { Header.push(SeriesNames.Current()[i]); Header.push(SeriesNames.Current()[i] + ' (dB)'); }
    Data.push(Header);
    for(var i = 0; i < FFTDataset.length; i++) {
      var Row = [i];
      for(var n = 0; n < SeriesNames.Voltage().length + SeriesNames.Current().length; n++) { Row.push(FFTDataset[i][n]); Row.push(20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][n]))); }
      Data.push(Row);
    }
    exportToCsv('FFT.csv', Data);
  } else if(DetailTabSelector.GetSaved() === 1) {
    var Header = ['Harmonic'];
    for(var i = 0; i < SeriesNames.Voltage().length; i++) { Header.push(SeriesNames.Voltage()[i]); Header.push(SeriesNames.Voltage()[i] + ' (dB)'); }
    for(var i = 0; i < SeriesNames.Current().length; i++) { Header.push(SeriesNames.Current()[i]); Header.push(SeriesNames.Current()[i] + ' (dB)'); }
    Data.push(Header);
    for(i = 0; i < HarmonicsDataset.length; i++) {
      var Row = [SeriesNames.Harmonics[i]];
      for(var n = 0; n < SeriesNames.Voltage().length + SeriesNames.Current().length; n++) { Row.push(HarmonicsDataset[i][n]); Row.push(20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][n]))); }
      Data.push(Row);
    }
    exportToCsv('Harmonics.csv', Data);
  } else if(DetailTabSelector.GetSaved() === 2) {
    var Header = ['Date/Time'];
    for(var i = 0; i < SeriesNames.Harmonics.length; i++) {
      for(var n = 0; n < SeriesNames.Voltage().length; n++) Header.push( SeriesNames.Harmonics[i] + '(' + SeriesNames.Voltage()[n] + ')');
    }
    Data.push(Header);
    for(var i = 0; i < HarmonicsData[0].length; i++) {
      var Row = [new Date(HarmonicsData[0][i][0]).toLocaleTimeString()];
      for(var j = 0; j < SeriesNames.Harmonics.length; j++) {
        for(var k = 0; k < SeriesNames.Voltage().length; k++) Row.push(HarmonicsData[j][i][k + 1]);
      }
      Data.push(Row);
    }
    exportToCsv('Voltage_Harmonics_Evolution.csv', Data);
  } else if(DetailTabSelector.GetSaved() === 5) {
    var Header = ['Date/Time'].concat(SeriesNames.PowerFactor()).concat(SeriesNames.THDv()).concat(SeriesNames.PHI());
    Data.push(Header);
    for(var i = 0; i < PowerFactorData.length; i++) {
      var Row = [new Date(PowerFactorData[i][0]).toLocaleTimeString()];
      for(var k = 0; k < SeriesNames.PowerFactor().length; k++) Row.push(PowerFactorData[i][k + 1]);
      for(var k = 0; k < SeriesNames.THDv().length; k++) Row.push(THDData[i][k + 1]);
      for(var k = 0; k < SeriesNames.PHI().length; k++) Row.push(PHIData[i][k + 1]);
      Data.push(Row);
    }
    exportToCsv('PF_THDv_PHI.csv', Data);
  }
}
var MainScrollable = null;
var DetailTabSelector = null;
function UpdatePage() {
  DetailTabSelector = new TabSelector('VoltageDetailView', '#DetailTabs', '#DetailDivs', DetailTabChange);
  DetailsButtonUpdate();
  MainScrollable = new Scrollable( {Spinners: ['#VoltageSpinner'], NowBtn: '#toolbarVoltage button[data-function="Update"]', Series: 'rms_v', OnSuccess: Scrollable_OnSuccess, Counters: Scrollable_UpdateCounters } );
  new Live( {NowBtn: '#toolbarDetails button[data-function="Update"]', RefreshInterval: 5000, Spinners: ['#DetailSpinner'], Series: 'wave_v, wave_i, harmonics_v, harmonics_i, fft_v, fft_i, power_factor, thd_v, phi, rms_v, rms_i', OnSuccess: Live_OnSuccess});
  new PeriodView( {Spinners: [], Series: 'power_factor, thd_v, phi, harmonics_v', Period: 86400000, Aggreg: '1m', OnSuccess: PeriodView_OnSuccess} );
  new GraphTools(function() { return ['Waveform', 'Harmonics', '', 'Advanced'][DetailTabSelector.GetSaved()]; }, function() { return [WaveformChart, HarmonicsChart, null, FFTChart][DetailTabSelector.GetSaved()]; }, '#toolbarDetails', undefined, ExportDetail);
}
//1752
