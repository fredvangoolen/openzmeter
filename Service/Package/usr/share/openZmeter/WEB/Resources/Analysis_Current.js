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

/* Current graph */
var CurrentChart     = null;
var CurrentData      = [];
function Scrollable_OnSuccess(result, clear) {
  if(clear) CurrentData = [Array(SeriesNames.Current().length + 1).fill(0)];
  CurrentData = GraphDataAppend(result, 'rms_i', CurrentData);
  if(CurrentChart == null) {
    CurrentChart = new Dygraph(document.getElementById("CurrentChart"), CurrentData,
      { strokeWidth: 0.5, stepPlot: true, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(SeriesNames.Current()), dateWindow: [result['From'], result['To']],
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: ColorArray($('#CurrentPanel').css('border-color'), 4), plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
        underlayCallback: function(canvas, area, g) {
          var MC = Analyzers[Analyzer].MaxCurrent, Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
          GraphHBackgroundRanges([[MC * 0.7, MC * 1, Yellow], [Infinity, MC, Red]], g, canvas);
        },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">A</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: MainScrollable.XAxis
        }
      }
    );
    $(window).bind('resize', function() { CurrentChart.resize($('#CurrentCol').width(), $('#panelCounters').height() - 4); });
    window.dispatchEvent(new Event('resize'));
  } else {
    CurrentChart.updateOptions({'file': CurrentData, dateWindow: [result['From'], result['To']]});
  }
  CurrentSeries();
}
function CurrentSeries(event) {
  GraphPhaseButtons(CurrentChart, $('#groupCurrentPhases'), 'VisibleCurrentMainPhases', false, function(series) { return CalcRanges(CurrentData, series, false); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupCurrentPhases').on('click', 'button', CurrentSeries);
$('#btnExportCurrent').click(function() {
  exportToCsv('Current.csv', [['Time'].concat(CurerntNames)].concat(CurrentData));
});
$('#btnMaximizeCurrent').click(function() {
  $('#GraphDialogTitle').text('Current');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#CurrentCol');
  DialogElement = $('#CurrentMaximizeDiv').detach();
  DialogPrevResizeHandler = CurrentResizeEvent;
  DialogResizeHandler = function() {
    CurrentChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
  };
  $(window).unbind('resize', CurrentResizeEvent);
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});

/* Current counters */
function Scrollable_UpdateCounters(times, renew) {
  if(renew === true) {
    $('#labelCurrentAvgPrimary').text(times.PrimaryText + ' average');
    $('#labelCurrentMaxPrimary').text(times.PrimaryText + ' maximun');
    $('#labelCurrentMinPrimary').text(times.PrimaryText + ' minimun');
    $('#labelCurrentAvgPrimaryValue, #labelCurrentMaxPrimaryValue, #labelCurrentMinPrimaryValue').text('Calculating...');
    $('#labelCurrentAvgSecondary, #labelCurrentMaxSecondary, #labelCurrentMinSecondary').text(times.SecondaryText);
    $('#labelCurrentAvgSecondaryValue, #labelCurrentMaxSecondaryValue, #labelCurrentMinSecondaryValue').text((times.SecondaryTo !== undefined) ? 'Calculating...' : '');
    if(times.SecondaryTo !== undefined) {
      Ajax.GetSeriesAggreg(times.SecondaryPeriod, times.SecondaryTo, times.PreferedAggreg, 'avg(rms_i), max(rms_i), min(rms_i)',
        function(result) {
          if(result['Samples'] !== 0) {
            $('#labelCurrentAvgSecondaryValue').html(Array_Round2(result['avg_rms_i'], '&nbsp;') + '&nbsp;<em>A</em>');
            $('#labelCurrentMinSecondaryValue').html(Array_Round2(result['min_rms_i'], '&nbsp;') + '&nbsp;<em>A</em>');
            $('#labelCurrentMaxSecondaryValue').html(Array_Round2(result['max_rms_i'], '&nbsp;') + '&nbsp;<em>A</em>');
          } else {
            $('#labelVoltageAvgSecondaryValue, #labelVoltageMaxSecondaryValue, #labelVoltageMinSecondaryValue').text('No data'); 
          }
          window.dispatchEvent(new Event('resize'));
        }
      );
    }
  }
  Ajax.GetSeriesAggreg(times.PrimaryPeriod, times.PrimaryTo, times.PreferedAggreg, 'avg(rms_i), max(rms_i), min(rms_i)', 
    function(result) {
      $('#labelCurrentAvgPrimaryValue').html(Array_Round2(result['avg_rms_i'], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">A</em>');
      $('#labelCurrentMinPrimaryValue').html(Array_Round2(result['min_rms_i'], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">A</em>');
      $('#labelCurrentMaxPrimaryValue').html(Array_Round2(result['max_rms_i'], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">A</em>');
      window.dispatchEvent(new Event('resize'));
    }
  );
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

var WaveformDataset = [], HarmonicsDataset = [], FFTDataset = [];
function Live_OnSuccess(result) {
  $('#DetailTime').text('Last update at ' + new Date(result['Sampletime']).toLocaleTimeString() + '.' + new Date(result['Sampletime']).getMilliseconds());
  WaveformDataset = [];
  HarmonicsDataset = [];
  FFTDataset = [];
  for(i = 0; i < result['wave_i'][0].length; i++) {
    var Row = [i];
    for(n = 0; n < result['wave_i'].length; n++) Row.push(result['wave_i'][n][i]);
    for(n = 0; n < result['wave_v'].length; n++) Row.push(result['wave_v'][n][i]);
    WaveformDataset.push(Row);
  }
  for(i = 0; i < 50; i++) {
    var Row = [];
    for(n = 0; n < result['harmonics_i'].length; n++) Row.push(result['harmonics_i'][n][i]);
    for(n = 0; n < result['harmonics_v'].length; n++) Row.push(result['harmonics_v'][n][i]);
    HarmonicsDataset.push(Row);
  }  
  for(i = 0; i < result['fft_i'][0].length; i++) {
    var Row = [];
    for(n = 0; n < result['harmonics_i'].length; n++) Row.push(result['fft_i'][n][i]);
    for(n = 0; n < result['harmonics_v'].length; n++) Row.push(result['fft_v'][n][i]);
    FFTDataset.push(Row);
  }
  UpdateWaveformChart();
  UpdateHarmonicsChart();
  UpdateFFTChart();
  $('#PowerFactorNow').html(Array_Round2(result['power_factor'], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 3)));
  $('#THDNow').html(Array_Round2(result['thd_i'].map(function(val) {return val * 100.0; }), '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 3)));
  $('#PHINow').html(Array_Round2(result['phi'], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 3), " (CAP)", " (IND)"));
}
function DetailTabChange(index) {
  $('#groupDetailPhasesSingle, #groupDetailPhasesMulti, #btnShowDetail1Harmonic, #btnShowDetailLog, #btnShowDetailVoltage').hide();
  if((index === 0) || (index === 1) || (index === 3)) $('#groupDetailPhasesMulti').show();
  if((index === 0) || (index === 1) || (index === 3)) $('#btnShowDetailVoltage').show();
  if((index === 2) || (index === 4)) $('#groupDetailPhasesSingle').show();
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
  if(HarmonicsCharts.length !== 0) {
    if(index === 2) {
      for(i = 0; i < 50; i++) $(HarmonicsCharts[i].graphDiv).parent().removeAttr('style');
    }
  }
  if(PowerFactorChart !== null) {
    if(index === 4) {
      $(PowerFactorChart.graphDiv).parent().removeAttr('style');
      $(THDChart.graphDiv).parent().removeAttr('style');
      $(PHIChart.graphDiv).parent().removeAttr('style');
    }
  }
}
function DetailsButtonUpdate(event) {
  GroupButtons($('#groupDetailPhasesMulti'), 'DetailPhases', Math.max(SeriesNames.Current().length, SeriesNames.Voltage().length), false, undefined, undefined);
  if(localStorage.getItem('CurrentDetailFirstHarmonic') === 'true') {
    $('#btnShowDetail1Harmonic').addClass('active');
  } else {
    $('#btnShowDetail1Harmonic').removeClass('active');
  }
  if(localStorage.getItem('CurrentDetailVoltage') === 'true') {
    $('#btnShowDetailVoltage').addClass('active');
  } else {
    $('#btnShowDetailVoltage').removeClass('active');
  }
  if(localStorage.getItem('CurrentDetailLog') === 'true') {
    $('#btnShowDetailLog').addClass('active');
  } else {
    $('#btnShowDetailLog').removeClass('active');
  }
}
$('#groupDetailPhasesMulti').on('click', 'button', function(evt) {
  GroupButtons($('#groupDetailPhasesMulti'), 'CurrentDetailVisiblePhasesMulti', Math.max(SeriesNames.Current().length, SeriesNames.Voltage().length), false,
    function() { UpdateWaveformChart(); UpdateHarmonicsChart(); }, evt.target);
});
$('#btnShowDetailVoltage').click(function() {
  localStorage.setItem('CurrentDetailVoltage', (localStorage.getItem('CurrentDetailVoltage') === 'true') ? 'false' : 'true');
  DetailsButtonUpdate();
  UpdateWaveformChart();
  UpdateHarmonicsChart();
  UpdateFFTChart();
  if(WaveformChart !== null) { WaveformChart.updateOptions({ showRangeSelector: true}); WaveformChart.updateOptions({ showRangeSelector: false}); }
  if(HarmonicsChart !== null) { HarmonicsChart.updateOptions({ showRangeSelector: true}); HarmonicsChart.updateOptions({ showRangeSelector: false}); }
  if(FFTChart !== null) { FFTChart.updateOptions({ showRangeSelector: true}); FFTChart.updateOptions({ showRangeSelector: false}); }
});
$('#btnShowDetail1Harmonic').click(function() {
  localStorage.setItem('CurrentDetailFirstHarmonic', (localStorage.getItem('CurrentDetailFirstHarmonic') === 'true') ? 'false' : 'true');
  DetailsButtonUpdate();
  UpdateWaveformChart();
  UpdateHarmonicsChart();
});
$('#btnShowDetailLog').click(function() {
  localStorage.setItem('CurrentDetailLog', (localStorage.getItem('CurrentDetailLog') === 'true') ? 'false' : 'true');
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
  var ShowVoltage = (localStorage.getItem("CurrentDetailVoltage") === 'true');
  GroupButtons($('#groupDetailPhasesMulti'), 'CurrentDetailVisiblePhasesMulti', Math.max(SeriesNames.Current().length, SeriesNames.Voltage().length), false, 
    function(VisibleSeries, Visibles) {
      VoltageVisibility = VisibleSeries.slice(0, SeriesNames.Voltage().length);
      CurrentVisibility = VisibleSeries.slice(0, SeriesNames.Current().length);
      if(ShowVoltage === false) VoltageVisibility.fill(false);
      Y1Range = CalcRanges(WaveformDataset, Visibles);
      Y2Range = CalcRanges(WaveformDataset, Visibles.map(function(val) { return val + SeriesNames.Current().length }));
      SeriesNames.Voltage().map(function(val) {Series[val] = { axis: 'y2' }; });
      var Options = { 
        strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Sample'].concat(SeriesNames.Current()).concat(SeriesNames.Voltage()), series: Series, visibility: CurrentVisibility.concat(VoltageVisibility),
        colors: ColorArray($('#CurrentPanel').css('border-color'), SeriesNames.Current().length).concat(ColorArray('#d9534f', SeriesNames.Current().length)), plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: Y1Range, 
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">A</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
          },
          y2: { pixelsPerLabel: 15, axisLabelWidth: ShowVoltage ? 34 : 0, valueRange: Y2Range, 
                valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">V</em>'; },
                axisLabelFormatter: function (x) { return (ShowVoltage == true) ? Round2(x) : ''; }
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
  var ShowVoltage = (localStorage.getItem("CurrentDetailVoltage") === 'true');
  var ShowFirst   = (localStorage.getItem('CurrentDetailFirstHarmonic') === 'true');
  var Log         = (localStorage.getItem('CurrentDetailLog') === 'true');
  for(i = ShowFirst ? 0 : 1; i < HarmonicsDataset.length; i++) {
    var Row = [i];
    for(n = 0; n < HarmonicsDataset[i].length; n++) Row.push(Log ? 20.0 * Math.log10(Math.max(0.001, HarmonicsDataset[i][n])) : HarmonicsDataset[i][n]);
    ToShow.push(Row);
  }
  var VoltageVisibility = [], CurrentVisibility = [], Y1Range = [], Y2Range = [], Series = {};
  GroupButtons($('#groupDetailPhasesMulti'), 'CurrentDetailVisiblePhasesMulti', Math.max(SeriesNames.Current().length, SeriesNames.Voltage().length), false, 
    function(VisibleSeries, Visibles) {
      VoltageVisibility = VisibleSeries.slice(0, SeriesNames.Voltage().length);
      CurrentVisibility = VisibleSeries.slice(0, SeriesNames.Current().length);
      if(ShowVoltage === false) VoltageVisibility.fill(false);
      Y1Range = CalcRanges(ToShow, Visibles, !Log);
      Y2Range = CalcRanges(ToShow, Visibles.map(function(val) { return val + SeriesNames.Current().length }), !Log);
      HarmonicsCOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
      HarmonicsVOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
      for(i = 0; i < ToShow.length; i++) {
        var Row = [ToShow[i][0]];
        for(n = 0; n < SeriesNames.Current().length; n++) Row.push(ToShow[i][n + 1] + HarmonicsCOffset);
        for(n = 0; n < SeriesNames.Voltage().length; n++) Row.push(ToShow[i][n + 1 + SeriesNames.Current().length] + HarmonicsVOffset);
        ToShow[i] = Row;
      }
      SeriesNames.Voltage().map(function(val) {Series[val] = { axis: 'y2' }; });  
      var Options = { 
        strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { }, plotter: multiColumnBarPlotter, dateWindow: [ToShow[0][0] - 0.5, ToShow[ToShow.length - 1][0] + 0.5],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Harmonic'].concat(SeriesNames.Current()).concat(SeriesNames.Voltage()), series: Series,
        colors: ColorArray($('#CurrentPanel').css('border-color'), SeriesNames.Current().length).concat(ColorArray('#d9534f', SeriesNames.Voltage().length)), visibility: CurrentVisibility.concat(VoltageVisibility),
        axes: { 
          y: { pixelsPerLabel: 15, axisLabelWidth: 20, valueRange: [Y1Range[0] + HarmonicsCOffset, Y1Range[1] + HarmonicsCOffset],
               ticker: Log ? function(min, max) { return GraphDBTicks(min, max, HarmonicsCOffset); } : Dygraph.numericTicks,
               valueFormatter: function(x) { return Round2(x - HarmonicsCOffset) + '<em class="x-small">' + (Log ? 'dB' : 'A') + '</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          y2: { pixelsPerLabel: 15, axisLabelWidth: ShowVoltage ? 15 : 0, valueRange: [Y2Range[0] + HarmonicsVOffset, Y2Range[1] + HarmonicsVOffset],
                valueFormatter: function(x) { return Round2(x - HarmonicsVOffset) + '<em class="x-small">' + (Log ? 'dB' : 'V') + '</em>'; },
                ticker: (Log || ShowVoltage === false) ? function(min, max) { return (ShowVoltage === true) ? GraphDBTicks(min, max, HarmonicsVOffset) : []; } : Dygraph.numericTicks,
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
  var ShowVoltage = (localStorage.getItem("DetailVoltageCurrent") === 'true');
  var Log         = (localStorage.getItem('DetailLog') === 'true');
  for(i = 0; i < FFTDataset.length; i++) {
    var Row = [i];
    for(n = 0; n < FFTDataset[i].length; n++) Row.push(Log ? 20.0 * Math.log10(Math.max(0.001, FFTDataset[i][n])) : FFTDataset[i][n]);
    ToShow.push(Row);
  }
  var VoltageVisibility = [], CurrentVisibility = [], Y1Range = [], Y2Range = [], Series = {};
  GroupButtons($('#groupDetailPhasesMulti'), 'CurrentDetailVisiblePhasesMulti', Math.max(SeriesNames.Current().length, SeriesNames.Voltage().length), false, 
    function(VisibleSeries, Visibles) {
      VoltageVisibility = VisibleSeries.slice(0, SeriesNames.Voltage().length);
      CurrentVisibility = VisibleSeries.slice(0, SeriesNames.Current().length);
      if(ShowVoltage === false) VoltageVisibility.fill(false);
      Y1Range = CalcRanges(ToShow, Visibles, !Log);
      Y2Range = CalcRanges(ToShow, Visibles.map(function(val) { return val + SeriesNames.Current().length }), !Log);
      FFTCOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
      FFTVOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
      for(i = 0; i < ToShow.length; i++) {
        var Row = [i];
        for(n = 0; n < SeriesNames.Current().length; n++) Row.push(ToShow[i][n + 1] + FFTCOffset);
        for(n = 0; n < SeriesNames.Voltage().length; n++) Row.push(ToShow[i][n + 1 + SeriesNames.Voltage().length] + FFTVOffset);
        ToShow[i] = Row;
      }
      SeriesNames.Voltage().map(function(val) {Series[val] = { axis: 'y2' }; });
      var Options = {
        strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], fillGraph: true, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Bin'].concat(SeriesNames.Current()).concat(SeriesNames.Voltage()), series: Series, plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
        colors: ColorArray($('#CurrentPanel').css('border-color'), SeriesNames.Current().length).concat(ColorArray('#d9534f', SeriesNames.Voltage().length)), visibility: CurrentVisibility.concat(VoltageVisibility),
        axes: { 
          y: { pixelsPerLabel: 15, axisLabelWidth: 20, valueRange: [Y1Range[0] + FFTCOffset, Y1Range[1] + FFTCOffset], 
               ticker: Log ? function(min, max) { return GraphDBTicks(min, max, FFTCOffset); } : Dygraph.numericTicks,
               valueFormatter: function(x) { return Round2(x - HarmonicsCOffset) + '<em class="x-small">' + (Log ? 'dB' : 'A') + '</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
              },
          y2: { pixelsPerLabel: 15, axisLabelWidth: ShowVoltage ? 15 : 0, valueRange: [Y2Range[0] + FFTVOffset, Y2Range[1] + FFTVOffset],
                valueFormatter: function(x) { return Round2(x - FFTVOffset) + ' <em class="x-small">' + (Log ? 'dB' : 'V') + '</em>'; },
                ticker: (Log || ShowVoltage === false) ? function(min, max) { return (ShowVoltage === true) ? GraphDBTicks(min, max, FFTVOffset) : []; } : Dygraph.numericTicks,
                axisLabelFormatter: function (x) { return Round2(x); }
              },
          x: { drawGrid: true,
               valueFormatter: function (y) { return Math.round(y / 10 * Analyzers[Analyzer].NominalFreq) + '<em class="small"> Hz</em>'; },
               axisLabelFormatter: function (y) {
              return;
                 y = (y / 10 * Nodes['NominalFreq']);
             if(y > 1000) {
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

/* Evolucion de parametros*/
var HarmonicsCharts = [], HarmonicsData = [], PowerFactorChart = null, PowerFactorData = [], THDChart = null, THDData = [], PHIChart = null, PHIData = [];
function PeriodView_OnSuccess(result) {
  while(HarmonicsData.length !== 50) HarmonicsData.push([]);
  for(k = 0; k < 50; k++) {
    for(i = 0; i < result['harmonics_i'].length; i++) {
      var Row = [new Date(result['Sampletime'][i])];
      for(j = 0; j < result['harmonics_i'][i].length; j++) Row.push(20 * Math.log10(result['harmonics_i'][i][j][k]));
      HarmonicsData[k].push(Row);
    }
    ShiftArray(HarmonicsData[k], result['MaxSamples']);
  }
  PowerFactorData = GraphDataAppend(result, 'power_factor', PowerFactorData);
  THDData = GraphDataAppend(result, 'thd_i', THDData);
  PHIData = GraphDataAppend(result, 'phi', PHIData);
  if(HarmonicsCharts.length === 0) {
    for(var i = 0; i < 50; i++) {
      HarmonicsCharts[i] = AppendCubismGraph($('#EvolutionPanel'), HarmonicsData[i], SeriesNames.Harmonics[i] + ' harmonic', [result['From'], result['To']], function (x) { return Round2(x) + '<em>dB</em>' });
    }
    Dygraph.synchronize(HarmonicsCharts, { range: false, zoom: false });
    PowerFactorChart = AppendCubismGraph($('#PowerFactorChart'), PowerFactorData, "Power Factor", [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return Round2(x); });
    THDChart         = AppendCubismGraph($('#THDChart'), THDData, "Current THD", [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return Round2(x) + '<em>%</em>'; });
    PHIChart         = AppendCubismGraph($('#PHIChart'), PHIData, "Phi", [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return Round2(Math.abs(x)) + '<em>' + ((x < 0) ? 'º CAP' : (x > 0) ? 'º IND' : 'º') + '</em>'; });
    Dygraph.synchronize([PowerFactorChart, THDChart, PHIChart], { range: false, zoom: false });
  } else {
    for(var i = 0; i < 50; i++) {
      HarmonicsCharts[i].updateOptions({ file: HarmonicsData[i], dateWindow: [result['From'], result['To']] });
    }
    PowerFactorChart.updateOptions({ file: PowerFactorData, dateWindow: [result['From'], result['To']] });
    THDChart.updateOptions({ file: THDData, dateWindow: [result['From'], result['To']] });
    PHIChart.updateOptions({ file: PHIData, dateWindow: [result['From'], result['To']] });
  }
  ParametersSeries();
}
function ParametersSeries(event) {
  GraphPhaseButtons(PowerFactorChart, $('#groupDetailPhasesSingle'), 'CurrentDetailVisiblePhasesSingle', true, function(series) { return CalcRanges(PowerFactorData, series, false); }, (event !== undefined) ? event.target : undefined); 
  GraphPhaseButtons(PHIChart,         $('#groupDetailPhasesSingle'), 'CurrentDetailVisiblePhasesSingle', true, function(series) { return CalcRanges(PHIData, series, false); }, undefined); 
  GraphPhaseButtons(THDChart,         $('#groupDetailPhasesSingle'), 'CurrentDetailVisiblePhasesSingle', true, function(series) { return CalcRanges(THDData, series, false); }, undefined); 
  for(var i = 0; i < 50; i++) {
    GraphPhaseButtons(HarmonicsCharts[i], $('#groupDetailPhasesSingle'), 'CurrentDetailVisiblePhasesSingle', true, function(series) { return CalcRanges(HarmonicsData[i], series, false); }, undefined); 
  }
}

/* Exportar y ampliar graficas de detalles */
$('#btnExportDetail').click(function () {
  var Data = [];
  var Filename;
  if($('#WaveformPanel').hasClass('hidden') === false) {
    exportToCsv("Current_Waveform_" + Node + '_' + DetailTimestamp + '.csv', ((Nodes['Devices'][Node]['Type'] === '3Phase') ? [['DateTime', 'Voltage R', 'Voltage S', 'Voltage T', 'Current R', 'Current S', 'Current T']] : [['DateTime', 'Voltage', 'Current']]).concat(WaveformDataset));
  } else if($('#FFTPanel').hasClass('hidden') === false) {
    Data = [['Index'].concat(((Nodes['Devices'][Node]['Type'] === '3Phase') ? ['dB (R)', 'V_rms (R)', 'dB (S)', 'V_rms (S)', 'dB (T)', 'V_rms (T)', 'dB (R)', 'I_rms (R)', 'dB (S)', 'I_rms (S)', 'dB (T)', 'I_rms (T)'] : ['dB', 'V_rms', 'dB', 'I_rms'])).concat('Note: This is the result of our FFT algorithm. The index colum should be multiplied by 5 in order to get the frequencies.')];
    for(i = 0; i < FFTDataset.length; i++) {
      if(Nodes['Devices'][Node]['Type'] === '3Phase') {    
        Data.push([i, 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][0])), FFTDataset[i][0], 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][1])), FFTDataset[i][1],
                      20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][2])), FFTDataset[i][2], 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][3])), FFTDataset[i][3],
                      20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][4])), FFTDataset[i][4], 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][5])), FFTDataset[i][5]]);
      } else {
        Data.push([i, 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][0])), FFTDataset[i][0], 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][1])), FFTDataset[i][1]]);
      }
    }
    exportToCsv("Current_FFT_" + Node + '_' + DetailTimestamp + '.csv', Data);
  } else if($('#HarmonicsPanel').hasClass('hidden') == false) {
    Data = [['Harmonic'].concat(((Nodes['Devices'][Node]['Type'] === '3Phase') ? ['dB (R)', 'V_rms (R)', 'dB (S)', 'V_rms (S)', 'dB (T)', 'V_rms (T)', 'dB (R)', 'I_rms (R)', 'dB (S)', 'I_rms (S)', 'dB (T)', 'I_rms (T)'] : ['dB', 'V_rms', 'dB', 'I_rms'])).concat('Note: This is the result of our FFT algorithm.')];
    for(i = 0; i < HarmonicsDataset.length; i++) {
      if(Nodes['Devices'][Node]['Type'] === '3Phase') {
        Data.push([HarmonicsNames[i], 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][0])), HarmonicsDataset[i][0], 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][1])), HarmonicsDataset[i][1],
                                      20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][2])), HarmonicsDataset[i][2], 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][3])), HarmonicsDataset[i][3],
                                      20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][4])), HarmonicsDataset[i][4], 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][5])), HarmonicsDataset[i][5]]);
      } else {
        Data.push([HarmonicsNames[i], 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][0])), HarmonicsDataset[i][0], 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][1])), HarmonicsDataset[i][1]]);
      }
    }
    exportToCsv("Current_Harmonics_" + Node + '_' + DetailTimestamp + '.csv', Data);
  } else if($('#EvolutionPanel').hasClass('hidden') == false) {
    Filename = 'Current_Harmonics_Evolution' + LastTimestamp + '.csv';
    Data = [['Date/Time']];
    Data[0] = Data[0].concat(HarmonicsNames);
    for(i = 0; i <  EvolutionDatasets[0].length; i++) {
      var Row = [EvolutionDatasets[0][i][0]];
      for(j = 0; j <  EvolutionDatasets.length; j++) {
        Row.push(EvolutionDatasets[j][i][1])
      }
      Data.push(Row);
    }
    exportToCsv("Current_Harmonics_Evolution" + ((Nodes['Devices'][Node]['Type'] === '3Phase') ? '(' + localStorage.getItem('VoltageDetailEvolutionPhase') + ')_' : '_') + Node + '_' + DetailTimestamp + '.csv', Data);  
  }
});
$('#btnMaximizeDetail').click(function() {
  if(localStorage.getItem('CurrentDetailView') === 'Waveform') {
    $('#GraphDialogTitle').text('Waveform');
    PrevDialogContainer = $('#WaveformPanel');
  } else if(localStorage.getItem('CurrentDetailView') === 'Harmonics') {
    $('#GraphDialogTitle').text('Harmonics');
    PrevDialogContainer = $('#HarmonicsPanel');
  } else if(localStorage.getItem('CurrentDetailView') === 'Advanced') {
    $('#GraphDialogTitle').text('Advanced');
    PrevDialogContainer = $('#FFTPanel');
  }
  $('#GraphDialog').modal('show');
  if(localStorage.getItem('CurrentDetailView') === 'Waveform') {
    DialogElement = $('#WaveformMaximize').detach();
    DialogPrevResizeHandler = function() {
      $(WaveformChart.graphDiv).parent().removeAttr('style');
    };
    DialogResizeHandler = function() {
      WaveformChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
    };
  } else if(localStorage.getItem('CurrentDetailView') === 'Harmonics') {
    DialogElement = $('#HarmonicsMaximize').detach();
    DialogPrevResizeHandler = function() {
      $(HarmonicsChart.graphDiv).parent().removeAttr('style');
    };
    DialogResizeHandler = function() {
      HarmonicsChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
    };
  } else if(localStorage.getItem('CurrentDetailView') === 'Advanced') {
    DialogElement = $('#FFTMaximize').detach();
    DialogPrevResizeHandler = function() {
      $(FFTChart.graphDiv).parent().removeAttr('style');
    };
    DialogResizeHandler = function() {
      FFTChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
    };
  }
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

var MainScrollable = null;
function UpdatePage() {
  new TabSelector('CurrentDetailView', '#DetailTabs', '#DetailDivs', DetailTabChange);
  DetailsButtonUpdate();
  MainScrollable = new Scrollable( {Spinners: ['#CurrentSpinner'], Series: 'rms_i', OnSuccess: Scrollable_OnSuccess, Counters: Scrollable_UpdateCounters } );
  new Live( {RefreshInterval: 5000, Spinners: ['#DetailSpinner'], Series: 'wave_i, wave_v, harmonics_i, harmonics_v, fft_i, fft_v, power_factor, thd_i, phi, rms_i, rms_v', OnSuccess: Live_OnSuccess});
  new PeriodView( {Spinners: [], Series: 'power_factor, thd_i, phi, harmonics_i', Period: 86400000, Aggreg: '1m', OnSuccess: PeriodView_OnSuccess} );
}//1485