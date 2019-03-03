/*
 * Copyright (C) 2015 Pedro Sánchez Alguacil
 *               2018 Eduardo Viciana Gamez
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
var CurrentChart     = null;
var CurrentData      = [];
function Scrollable_OnSuccess(result, clear) {
  if(clear) CurrentData = [Array(Analyzer.GetSeriesNames('Current').length + 1).fill(0)];
  CurrentData = GraphDataAppend(result, 'Current', CurrentData);
  if(CurrentChart == null) {
    CurrentChart = new Dygraph(document.getElementById("CurrentChart"), CurrentData,
      { strokeWidth: 0.5, stepPlot: true, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(Analyzer.GetSeriesNames('Current')), dateWindow: [result['From'], result['To']],
        zoomCallback: MainScrollable.ZoomEvt, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: MainScrollable.MouseUpEvt, dblclick: MainScrollable.DoubleClickEvt },  
        colors: ColorArray($('#CurrentPanel').css('border-color'), 4), plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
        underlayCallback: function(canvas, area, g) {
          var MC = Analyzer.GetSelected().MaxCurrent, Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
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
    $(window).bind('resize', CurrentResizeEvent);
    window.dispatchEvent(new Event('resize'));
    new GraphTools('Current', CurrentChart, '#toolbarCurrent', CurrentResizeEvent, function() { exportToCsv('Current.csv', [['Time'].concat(Analyzer.GetSeriesNames('Current'))].concat(CurrentData)); });
  } else {
    CurrentChart.updateOptions({'file': CurrentData, dateWindow: [result['From'], result['To']]});
  }
  CurrentSeries();
}
function CurrentResizeEvent() { 
  if(CurrentChart !== null) CurrentChart.resize('100%', $('#CurrentPanel > .panel-body > .row > div:first-child').height());
}
function CurrentSeries(event) {
  GraphPhaseButtons(CurrentChart, $('#groupCurrentPhases'), 'VisibleCurrentMainPhases', false, function(series) { return CalcRanges(CurrentData, series, false); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupCurrentPhases').on('click', 'button', CurrentSeries);
function Scrollable_UpdateCounters(times, renew) {
  if(renew === true) {
    $('#labelCurrentAvgPrimary').text(times.PrimaryText + ' average');
    $('#labelCurrentMaxPrimary').text(times.PrimaryText + ' maximun');
    $('#labelCurrentMinPrimary').text(times.PrimaryText + ' minimun');
    $('#labelCurrentAvgPrimaryValue, #labelCurrentMaxPrimaryValue, #labelCurrentMinPrimaryValue').text('Calculating...');
    $('#labelCurrentAvgSecondary, #labelCurrentMaxSecondary, #labelCurrentMinSecondary').text(times.SecondaryText);
    $('#labelCurrentAvgSecondaryValue, #labelCurrentMaxSecondaryValue, #labelCurrentMinSecondaryValue').text((times.SecondaryTo !== undefined) ? 'Calculating...' : '');
    if(times.SecondaryTo !== undefined) {
      Analyzer.GetSeriesAggreg(times.SecondaryPeriod, times.SecondaryTo, times.PreferedAggreg, 'avg(rms_i), max(rms_i), min(rms_i)',
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
  Analyzer.GetSeriesAggreg(times.PrimaryPeriod, times.PrimaryTo, times.PreferedAggreg, 'avg(rms_i), max(rms_i), min(rms_i)', 
    function(result) {
      $('#labelCurrentAvgPrimaryValue').html(Array_Round2(result['avg_rms_i'], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">A</em>');
      $('#labelCurrentMinPrimaryValue').html(Array_Round2(result['min_rms_i'], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">A</em>');
      $('#labelCurrentMaxPrimaryValue').html(Array_Round2(result['max_rms_i'], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">A</em>');
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
  for(i = 0; i < result.Current_Samples[0].length; i++) {
    var Row = [i];
    for(n = 0; n < result.Current_Samples.length; n++) Row.push(result.Current_Samples[n][i]);
    for(n = 0; n < result.Voltage_Samples.length; n++) Row.push(result.Voltage_Samples[n][i]);
    WaveformDataset.push(Row);
  }
  for(i = 0; i < 50; i++) {
    var Row = [];
    for(n = 0; n < result.Current_Harmonics.length; n++) Row.push(result.Current_Harmonics[n][i]);
    for(n = 0; n < result.Voltage_Harmonics.length; n++) Row.push(result.Voltage_Harmonics[n][i]);
    HarmonicsDataset.push(Row);
  }  
  for(i = 0; i < result.Current_FFT[0].length; i++) {
    var Row = [];
    for(n = 0; n < result.Current_FFT.length; n++) Row.push(result.Current_FFT[n][i]);
    for(n = 0; n < result.Voltage_FFT.length; n++) Row.push(result.Voltage_FFT[n][i]);
    FFTDataset.push(Row);
  }
  UpdateWaveformChart();
  UpdateHarmonicsChart();
  UpdateFFTChart();
  SetCubismNowLabel(PowerFactorChart, result.Power_Factor);
  SetCubismNowLabel(THDChart, result.Current_THD);
  SetCubismNowLabel(PHIChart, result.Phi);
}
function DetailTabChange(index) {
  $('#groupDetailPhasesSingle, #groupDetailPhasesMulti, #btnShowDetail1Harmonic, #btnShowDetailLog, #btnShowDetailVoltage, #toolbarDetails button[data-function="Maximize"]').hide();
  if((index === 0) || (index === 1) || (index === 3)) $('#groupDetailPhasesMulti').show();
  if((index === 0) || (index === 1) || (index === 3)) $('#btnShowDetailVoltage').show();
  if((index === 0) || (index === 1) || (index === 3)) $('#toolbarDetails button[data-function="Maximize"]').show();
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
  GroupButtons($('#groupDetailPhasesMulti'), 'DetailPhases', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false, undefined, undefined);
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
  GroupButtons($('#groupDetailPhasesMulti'), 'CurrentDetailVisiblePhasesMulti', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false,
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
  GroupButtons($('#groupDetailPhasesMulti'), 'CurrentDetailVisiblePhasesMulti', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false, 
    function(VisibleSeries, Visibles) {
      VoltageVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Voltage').length);
      CurrentVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Current').length);
      if(ShowVoltage === false) VoltageVisibility.fill(false);
      Y1Range = CalcRanges(WaveformDataset, Visibles);
      Y2Range = CalcRanges(WaveformDataset, Visibles.map(function(val) { return val + Analyzer.GetSeriesNames('Current').length }));
      Analyzer.GetSeriesNames('Voltage').map(function(val) {Series[val] = { axis: 'y2' }; });
      var Options = { 
        strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Sample'].concat(Analyzer.GetSeriesNames('Current')).concat(Analyzer.GetSeriesNames('Voltage')), series: Series, visibility: CurrentVisibility.concat(VoltageVisibility),
        colors: ColorArray($('#CurrentPanel').css('border-color'), Analyzer.GetSeriesNames('Current').length).concat(ColorArray('#d9534f', Analyzer.GetSeriesNames('Current').length)), plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
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
  GroupButtons($('#groupDetailPhasesMulti'), 'CurrentDetailVisiblePhasesMulti', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false, 
    function(VisibleSeries, Visibles) {
      VoltageVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Voltage').length);
      CurrentVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Current').length);
      if(ShowVoltage === false) VoltageVisibility.fill(false);
      Y1Range = CalcRanges(ToShow, Visibles, !Log);
      Y2Range = CalcRanges(ToShow, Visibles.map(function(val) { return val + Analyzer.GetSeriesNames('Current').length }), !Log);
      HarmonicsCOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
      HarmonicsVOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
      for(i = 0; i < ToShow.length; i++) {
        var Row = [ToShow[i][0]];
        for(n = 0; n < Analyzer.GetSeriesNames('Current').length; n++) Row.push(ToShow[i][n + 1] + HarmonicsCOffset);
        for(n = 0; n < Analyzer.GetSeriesNames('Voltage').length; n++) Row.push(ToShow[i][n + 1 + Analyzer.GetSeriesNames('Current').length] + HarmonicsVOffset);
        ToShow[i] = Row;
      }
      Analyzer.GetSeriesNames('Voltage').map(function(val) {Series[val] = { axis: 'y2' }; });  
      var Options = { 
        strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { }, plotter: multiColumnBarPlotter, dateWindow: [ToShow[0][0] - 0.5, ToShow[ToShow.length - 1][0] + 0.5],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Harmonic'].concat(Analyzer.GetSeriesNames('Current')).concat(Analyzer.GetSeriesNames('Voltage')), series: Series,
        colors: ColorArray($('#CurrentPanel').css('border-color'), Analyzer.GetSeriesNames('Current').length).concat(ColorArray('#d9534f', Analyzer.GetSeriesNames('Voltage').length)), visibility: CurrentVisibility.concat(VoltageVisibility),
        axes: { 
          y: { pixelsPerLabel: 15, axisLabelWidth: 20, valueRange: [Y1Range[0] + HarmonicsCOffset, Y1Range[1] + HarmonicsCOffset],
               ticker: Log ? function(min, max) { return GraphDBTicks(min, max, HarmonicsCOffset); } : Dygraph.numericTicks,
               valueFormatter: function(x) { return Round2(x - HarmonicsCOffset) + '<em class="x-small">' + (Log ? 'dB' : 'A') + '</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          y2: { pixelsPerLabel: 15, axisLabelWidth: ShowVoltage ? 34 : 0, valueRange: [Y2Range[0] + HarmonicsVOffset, Y2Range[1] + HarmonicsVOffset],
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
  var ShowVoltage = (localStorage.getItem("CurrentDetailVoltage") === 'true');
  var Log         = (localStorage.getItem('CurrentDetailLog') === 'true');
  for(i = 0; i < FFTDataset.length; i++) {
    var Row = [i];
    for(n = 0; n < FFTDataset[i].length; n++) Row.push(Log ? 20.0 * Math.log10(Math.max(0.001, FFTDataset[i][n])) : FFTDataset[i][n]);
    ToShow.push(Row);
  }
  var VoltageVisibility = [], CurrentVisibility = [], Y1Range = [], Y2Range = [], Series = {};
  GroupButtons($('#groupDetailPhasesMulti'), 'CurrentDetailVisiblePhasesMulti', Math.max(Analyzer.GetSeriesNames('Current').length, Analyzer.GetSeriesNames('Voltage').length), false, 
    function(VisibleSeries, Visibles) {
      VoltageVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Voltage').length);
      CurrentVisibility = VisibleSeries.slice(0, Analyzer.GetSeriesNames('Current').length);
      if(ShowVoltage === false) VoltageVisibility.fill(false);
      Y1Range = CalcRanges(ToShow, Visibles, !Log);
      Y2Range = CalcRanges(ToShow, Visibles.map(function(val) { return val + Analyzer.GetSeriesNames('Current').length }), !Log);
      FFTCOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
      FFTVOffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
      for(i = 0; i < ToShow.length; i++) {
        var Row = [i];
        for(n = 0; n < Analyzer.GetSeriesNames('Current').length; n++) Row.push(ToShow[i][n + 1] + FFTCOffset);
        for(n = 0; n < Analyzer.GetSeriesNames('Voltage').length; n++) Row.push(ToShow[i][n + 1 + Analyzer.GetSeriesNames('Voltage').length] + FFTVOffset);
        ToShow[i] = Row;
      }
      Analyzer.GetSeriesNames('Voltage').map(function(val) {Series[val] = { axis: 'y2' }; });
      var Options = {
        strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], fillGraph: true, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Bin'].concat(Analyzer.GetSeriesNames('Current')).concat(Analyzer.GetSeriesNames('Voltage')), series: Series, plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
        colors: ColorArray($('#CurrentPanel').css('border-color'), Analyzer.GetSeriesNames('Current').length).concat(ColorArray('#d9534f', Analyzer.GetSeriesNames('Voltage').length)), visibility: CurrentVisibility.concat(VoltageVisibility),
        axes: { 
          y: { pixelsPerLabel: 15, axisLabelWidth: 20, valueRange: [Y1Range[0] + FFTCOffset, Y1Range[1] + FFTCOffset], 
               ticker: Log ? function(min, max) { return GraphDBTicks(min, max, FFTCOffset); } : Dygraph.numericTicks,
               valueFormatter: function(x) { return Round2(x - HarmonicsCOffset) + '<em class="x-small">' + (Log ? 'dB' : 'A') + '</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
              },
          y2: { pixelsPerLabel: 15, axisLabelWidth: ShowVoltage ? 34 : 0, valueRange: [Y2Range[0] + FFTVOffset, Y2Range[1] + FFTVOffset],
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
    for(i = 0; i < result.Current_Harmonics.length; i++) {
      var Row = [new Date(result.Sampletime[i])];
      for(j = 0; j < result.Current_Harmonics[i].length; j++) Row.push(20 * Math.log10(result.Current_Harmonics[i][j][k]));
      HarmonicsData[k].push(Row);
    }
    ShiftArray(HarmonicsData[k], result.MaxSamples);
  }
  PowerFactorData = GraphDataAppend(result, 'Power_Factor', PowerFactorData);
  THDData = GraphDataAppend(result, 'Current_THD', THDData);
  PHIData = GraphDataAppend(result, 'Phi', PHIData);
  if(HarmonicsCharts.length === 0) {
    for(var i = 0; i < 50; i++) {
      HarmonicsCharts[i] = AppendCubismGraph($('#EvolutionPanel'), HarmonicsData[i], Analyzer.GetSeriesNames('Harmonics')[i] + ' harmonic', [result['From'], result['To']], function (x) { return Round2(x) + '<em>dB</em>' });
    }
    Dygraph.synchronize(HarmonicsCharts, { range: false, zoom: false });
    PowerFactorChart = AppendCubismGraph($('#ParametersPanel'), PowerFactorData, "Power Factor", [result['From'], result['To']], function (x) { return (x === null) ? '~' : Round2(x); });
    THDChart         = AppendCubismGraph($('#ParametersPanel'), THDData, "Current THD", [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(x * 100.0) + '<em>%</em>'); });
    PHIChart         = AppendCubismGraph($('#ParametersPanel'), PHIData, "Phi", [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(Math.abs(x)) + '<em>' + ((x < 0) ? 'º CAP' : (x > 0) ? 'º IND' : 'º') + '</em>'); });
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
  GraphPhaseButtons(PowerFactorChart, $('#groupDetailPhasesSingle'), 'CurrentDetailVisiblePhasesSingle', true, function(series) { var Range = ArrayAbsMax(CalcRanges(PowerFactorData, series, false)); return [-Range, Range]; }, (event !== undefined) ? event.target : undefined); 
  GraphPhaseButtons(PHIChart,         $('#groupDetailPhasesSingle'), 'CurrentDetailVisiblePhasesSingle', true, function(series) { var Range = ArrayAbsMax(CalcRanges(PHIData, series, false, false, false, true)); return [-Range, Range]; }, undefined); 
  GraphPhaseButtons(THDChart,         $('#groupDetailPhasesSingle'), 'CurrentDetailVisiblePhasesSingle', true, function(series) { var Range = ArrayAbsMax(CalcRanges(THDData, series, false, false, false, true)); return [-Range, Range]; }, undefined); 
  for(var i = 0; i < 50; i++) {
    GraphPhaseButtons(HarmonicsCharts[i], $('#groupDetailPhasesSingle'), 'CurrentDetailVisiblePhasesSingle', true, function(series) { return CalcRanges(HarmonicsData[i], series, false); }, undefined); 
  }
}
function ExportDetail() {
  var Data = [];
  var Filename;
  if(DetailTabSelector.GetSaved() === 0) {
    exportToCsv('Waveform.csv', [['Time'].concat(Analyzer.GetSeriesNames('Current')).concat(Analyzer.GetSeriesNames('Voltage'))].concat(WaveformDataset));
  } else if(DetailTabSelector.GetSaved() === 3) {
    var Header = ['Index'];
    for(var i = 0; i < Analyzer.GetSeriesNames('Current').length; i++) { Header.push(Analyzer.GetSeriesNames('Current')[i]); Header.push(Analyzer.GetSeriesNames('Current')[i] + ' (dB)'); }
    for(var i = 0; i < Analyzer.GetSeriesNames('Voltage').length; i++) { Header.push(Analyzer.GetSeriesNames('Voltage')[i]); Header.push(Analyzer.GetSeriesNames('Voltage')[i] + ' (dB)'); }
    Data.push(Header);
    for(var i = 0; i < FFTDataset.length; i++) {
      var Row = [i];
      for(var n = 0; n < Analyzer.GetSeriesNames('Voltage').length + Analyzer.GetSeriesNames('Current').length; n++) { Row.push(FFTDataset[i][n]); Row.push(20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][n]))); }
      Data.push(Row);
    }
    exportToCsv('FFT.csv', Data);
  } else if(DetailTabSelector.GetSaved() === 1) {
    var Header = ['Harmonic'];
    for(var i = 0; i < Analyzer.GetSeriesNames('Current').length; i++) { Header.push(Analyzer.GetSeriesNames('Current')[i]); Header.push(Analyzer.GetSeriesNames('Current')[i] + ' (dB)'); }
    for(var i = 0; i < Analyzer.GetSeriesNames('Voltage').length; i++) { Header.push(Analyzer.GetSeriesNames('Voltage')[i]); Header.push(Analyzer.GetSeriesNames('Voltage')[i] + ' (dB)'); }
    Data.push(Header);
    for(i = 0; i < HarmonicsDataset.length; i++) {
      var Row = [SeriesNames.Harmonics[i]];
      for(var n = 0; n < Analyzer.GetSeriesNames('Voltage').length + Analyzer.GetSeriesNames('Current').length; n++) { Row.push(HarmonicsDataset[i][n]); Row.push(20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][n]))); }
      Data.push(Row);
    }
    exportToCsv('Harmonics.csv', Data);
  } else if(DetailTabSelector.GetSaved() === 2) {
    var Header = ['Date/Time'];
    for(var i = 0; i < SeriesNames.Harmonics.length; i++) {
      for(var n = 0; n < Analyzer.GetSeriesNames('Voltage').length; n++) Header.push( SeriesNames.Harmonics[i] + '(' + Analyzer.GetSeriesNames('Current')[n] + ')');
    }
    Data.push(Header);
    for(var i = 0; i < HarmonicsData[0].length; i++) {
      var Row = [new Date(HarmonicsData[0][i][0]).toLocaleTimeString()];
      for(var j = 0; j < SeriesNames.Harmonics.length; j++) {
        for(var k = 0; k < Analyzer.GetSeriesNames('Voltage').length; k++) Row.push(HarmonicsData[j][i][k + 1]);
      }
      Data.push(Row);
    }
    exportToCsv('Current_Harmonics_Evolution.csv', Data);
  } else if(DetailTabSelector.GetSaved() === 4) {
    var Header = ['Date/Time'].concat(SeriesNames.PowerFactor()).concat(SeriesNames.THDi()).concat(SeriesNames.PHI());
    Data.push(Header);
    for(var i = 0; i < PowerFactorData.length; i++) {
      var Row = [new Date(PowerFactorData[i][0]).toLocaleTimeString()];
      for(var k = 0; k < SeriesNames.PowerFactor().length; k++) Row.push(PowerFactorData[i][k + 1]);
      for(var k = 0; k < SeriesNames.THDv().length; k++) Row.push(THDData[i][k + 1]);
      for(var k = 0; k < SeriesNames.PHI().length; k++) Row.push(PHIData[i][k + 1]);
      Data.push(Row);
    }
    exportToCsv('PF_THDi_PHI.csv', Data);
  }
}
var MainScrollable = null;
var DetailTabSelector = null;
function UpdatePage() {
  DetailTabSelector = new TabSelector('CurrentDetailView', '#DetailTabs', '#DetailDivs', DetailTabChange);
  DetailsButtonUpdate();
  MainScrollable = new Scrollable( {Spinners: ['#CurrentSpinner'], NowBtn: '#toolbarCurrent button[data-function="Update"]', Series: ['CURRENT'], OnSuccess: Scrollable_OnSuccess, Counters: Scrollable_UpdateCounters } );
  new Live( {NowBtn: '#toolbarDetails button[data-function="Update"]', RefreshInterval: 5000, Spinners: ['#DetailSpinner'], Series: ['CURRENT_SAMPLES', 'VOLTAGE_SAMPLES', 'CURRENT_HARMONICS', 'VOLTAGE_HARMONICS', 'CURRENT_FFT', 'VOLTAGE_FFT', 'POWER_FACTOR', 'CURRENT_THD', 'PHI', 'CURRENT', 'VOLTAGE'], OnSuccess: Live_OnSuccess});
  new PeriodView( {Spinners: [], Series: ['POWER_FACTOR', 'CURRENT_THD', 'PHI', 'CURRENT_HARMONICS'], Period: 86400000, Aggreg: '1M', OnSuccess: PeriodView_OnSuccess} );
  new GraphTools(function() { return ['Waveform', 'Harmonics', '', 'Advanced'][DetailTabSelector.GetSaved()]; }, function() { return [WaveformChart, HarmonicsChart, null, FFTChart][DetailTabSelector.GetSaved()]; }, '#toolbarDetails', undefined, ExportDetail);
}//1485