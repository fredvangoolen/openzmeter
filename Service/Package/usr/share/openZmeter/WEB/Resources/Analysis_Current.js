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
  SetCurrentSeries();
}
function CurrentSeries(event) {
  GraphPhaseButtons(CurrentChart, $('#groupCurrentPhases'), 'VisibleCurrentMainPhases', false, function(series) { return CalcRanges(CurrentData, series, false); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupCurrentPhases').on('click', 'button', CurrentSeries);
$('#btnExportCurrent').click(function() {
  exportToCsv("Current_" + Node + '.csv', [['Time'].concat(CurerntNames)].concat(CurrentData));
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
var CurrentCountersTimer  = null;
function Scrollable_UpdateCounters(change) {
  if(CurrentCountersTimer !== null) clearInterval(CurrentCountersTimer);
  CurrentCountersTimer = setTimeout(Scrollable_UpdateCounters, 60000);
  var Times = GetCountersTimePeriod();
  if(change === true) {
    $('#labelCurrentAvgPrimary').text(Times.PrimaryText + ' average');
    $('#labelCurrentAvgPrimaryValue').text('Calculating...');
    $('#labelCurrentAvgSecondary').text(Times.SecondaryText);
    $('#labelCurrentMaxPrimary').text(Times.PrimaryText + ' maximun');
    $('#labelCurrentMaxPrimaryValue').text('Calculating...');
    $('#labelCurrentMaxSecondary').text(Times.SecondaryText);
    $('#labelCurrentMinPrimary').text(Times.PrimaryText + ' minimun');
    $('#labelCurrentMinPrimaryValue').text('Calculating...');
    $('#labelCurrentMinSecondary').text(Times.SecondaryText);
  }
    Ajax.GetSeriesAggreg(Times.PrimaryPeriod, Times.PrimaryTo, Times.PreferedAggreg, 'avg(rms_i), max(rms_i), min(rms_i)', 
    function(result) {
      $('#labelCurrentAvgPrimaryValue').html(Array_Round2(result['avg_rms_i'], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">A</em>');
      $('#labelCurrentMinPrimaryValue').html(Array_Round2(result['min_rms_i'], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">A</em>');
      $('#labelCurrentMaxPrimaryValue').html(Array_Round2(result['max_rms_i'], '&nbsp;', ColorArray($('#CurrentPanel').css('border-color'), 3)) + '&nbsp;<em class="huge-small-unit">A</em>');
      window.dispatchEvent(new Event('resize'));
    }
  );
  if(Times.SecondaryTo !== undefined) {
    $('#labelCurrentAvgSecondaryValue').text('Calculating...');
    $('#labelCurrentMaxSecondaryValue').text('Calculating...');
    $('#labelCurrentMinSecondaryValue').text('Calculating...');
    Ajax.GetSeriesAggreg(Times.SecondaryPeriod, Times.SecondaryTo, Times.PreferedAggreg, 'avg(rms_i), max(rms_i), min(rms_i)',
      function(result) {
        $('#labelCurrentAvgSecondaryValue').html(Array_Round2(result['avg_rms_i'], '&nbsp;') + '&nbsp;<em>A</em>');
        $('#labelCurrentMinSecondaryValue').html(Array_Round2(result['min_rms_i'], '&nbsp;') + '&nbsp;<em>A</em>');
        $('#labelCurrentMaxSecondaryValue').html(Array_Round2(result['max_rms_i'], '&nbsp;') + '&nbsp;<em>A</em>');
        window.dispatchEvent(new Event('resize'));
      }
    );
  } else {
    $('#labelCurrentAvgSecondaryValue').text('');
    $('#labelCurrentMaxSecondaryValue').text('');
    $('#labelCurrentMinSecondaryValue').text('');
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
  $('#PowerFactorNow').html(Array_Round2(result['power_factor'], '&nbsp;&nbsp;', ["#d9534f", "#534fd9", "#4fd953"]));
  $('#THDNow').html(Array_Round2(result['thd_v'].map(function(val) {return val * 100.0; }), '&nbsp;&nbsp;', ["#d9534f", "#534fd9", "#4fd953"]));
  $('#PHINow').html(Array_Round2(result['phi'], '&nbsp;&nbsp;', ["#d9534f", "#534fd9", "#4fd953"], " (CAP)", " (IND)"));
}
function DetailTabChange(index) {
  if((index === 0) || (index === 1) || (index === 3)) {
    $('#groupDetailPhasesMulti').show();
    $('#btnShowDetailVoltageAndCurrent').show();
  } else {
    $('#groupDetailPhasesMulti').hide();
    $('#btnShowDetailVoltageAndCurrent').hide();
  }
  if((index === 1) || (index === 3)) {
    $('#btnShowDetailLog').show();
  } else {
    $('#btnShowDetailLog').hide();
  }
  if(index === 1) {
    $('#btnShowDetail1Harmonic').show();
  } else {
    $('#btnShowDetail1Harmonic').hide();
  }
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
    if(index === 5) {
      $(PowerFactorChart.graphDiv).parent().removeAttr('style');
      $(THDChart.graphDiv).parent().removeAttr('style');
      $(PHIChart.graphDiv).parent().removeAttr('style');
    }
  }
//  switch(localStorage.getItem('VoltageDetailView')) {
//    case 'Parameters' :
//      if(PowerFactorChart !== null) UpdatePowerFactorChart(null);
//      if(THDChart !== null) UpdateTHDChart(null);
//      if(PHIChart !== null) UpdatePHIChart(null);
//      break;

//    case 'Events' :
//      if(EventChart !== null) $(EventChart.graphDiv).parent().removeAttr('style');
//      break;
//  }
}
function DetailsButtonUpdate(event) {
  
}

$('#btnShowDetailWaveform').click(DetailsButtonClick);
$('#btnShowDetailHarmonics').click(DetailsButtonClick);
$('#btnShowDetailEvolution').click(DetailsButtonClick);
$('#btnShowDetailParameters').click(DetailsButtonClick);
$('#btnShowDetailAdvanced').click(DetailsButtonClick);
$('#btnShowDetailRPhase').click(DetailsButtonClick);
$('#btnShowDetailSPhase').click(DetailsButtonClick);
$('#btnShowDetailTPhase').click(DetailsButtonClick);
$('#btnShowDetailVoltageAndCurrent').click(DetailsButtonClick);
$('#btnShowDetail1Harmonic').click(DetailsButtonClick);
$('#btnPauseDetail').click(DetailsButtonClick);

/* Grafica de forma de onda */
var WaveformChart = null;
function UpdateWaveformChart() {
  var VoltageVisibility = [], CurrentVisibility = [], Y1Range = [], Y2Range = [], Series = {};
  var ShowVoltage = (localStorage.getItem("DetailShowVoltageCurrent") === 'true');
  return;
  if(result !== null) {
    var date = new Date(result['Sampletime']);
    $('#WaveformTime').text('Last update at ' + date.toLocaleTimeString() + '.' + date.getMilliseconds());
    WaveformDataset = [];
    for(i = 0; i < result['wave_i'][0].length; i++) {
      if(Nodes['Devices'][Node]['Type'] === '3Phase') {
        WaveformDataset.push([i, result['wave_v'][0][i], result['wave_v'][1][i], result['wave_v'][2][i], result['wave_i'][0][i], result['wave_i'][1][i], result['wave_i'][2][i]]);
      } else {
        WaveformDataset.push([i, result['wave_v'][0][i], result['wave_i'][0][i]]);      
      }
    }
  }
  var ShowR = (localStorage.getItem("CurrentDetailPhaseR") === 'true');
  var ShowS = (localStorage.getItem("CurrentDetailPhaseS") === 'true');
  var ShowT = (localStorage.getItem("CurrentDetailPhaseT") === 'true');
  var ShowVoltage= (localStorage.getItem("CurrentDetailShowVoltage") === 'true');
  var VoltageSeries = [];
  var CurrentSeries = [];
  var Visibility    = [];
  if(Nodes['Devices'][Node]['Type'] === '3Phase') {
    if(ShowR && ShowVoltage) VoltageSeries.push(1);
    if(ShowS && ShowVoltage) VoltageSeries.push(2);
    if(ShowT && ShowVoltage) VoltageSeries.push(3);
    if(ShowR) CurrentSeries.push(4);
    if(ShowS) CurrentSeries.push(5);
    if(ShowT) CurrentSeries.push(6);
    Visibility = [ShowR && ShowVoltage, ShowS && ShowVoltage, ShowT && ShowVoltage, ShowR, ShowS, ShowT];
  } else {
    VoltageSeries.push(1);
    CurrentSeries.push(2);
    Visibility = [ShowVoltage, true];
  }
  var Max_Voltage = CalcRanges(WaveformDataset, VoltageSeries);
  var Max_Current = CalcRanges(WaveformDataset, CurrentSeries);
  var Options = { 
    strokeWidth: 1,
    labelsDiv: "WaveformLegend",  
    labels: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['Time', 'Voltage (R)', 'Voltage (S)', 'Voltage (T)', 'Current (R)', 'Current (S)', 'Current (T)'] : ['Time', 'Voltage', 'Current'],
    colors: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['#d9534f', '#534fd9', '#4fd953', '#428bca', '#42ca81', '#ca428b'] : ['#d9534f', '#428bca'],
    visibility: Visibility,
    gridLineColor: "#999",
    gridLinePattern: [4, 4],
    showRangeSelector: true,
    rangeSelectorHeight: 20,
    rangeSelectorPlotFillColor: "#FFFFFF",
    rangeSelectorPlotStrokeColor: "#428bca",
    xAxisHeight: 10,
    interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
    series: (Nodes['Devices'][Node]['Type'] === '3Phase') ? { 'Current (R)': { showInRangeSelector: true }, 'Voltage (R)': { axis: 'y2' }, 'Voltage (S)': { axis: 'y2' }, 'Voltage (T)': { axis: 'y2'  } } : { 'Current': { showInRangeSelector: true }, 'Voltage': { axis: 'y2' } },
    plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
    axes: {
      y: { pixelsPerLabel: 15,
           axisLabelWidth: 34,
           valueRange: Max_Current, 
           valueFormatter: function (x) {
             return Round2(x) + ' <em class="x-small">A</em>';
           },
           axisLabelFormatter: function (x) {
             return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
           }
         },
      y2: { pixelsPerLabel: 15,
            axisLabelWidth: ShowVoltage ? 34 : 0,
            valueRange: Max_Voltage, 
            valueFormatter: function (x) {
              return Round2(x) + ' <em class="x-small">V</em>';
            },
            axisLabelFormatter: function (x) {
              if(ShowVoltage == true) {
                return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
              } else {
                return '';
              }
            }
          },
      x: { ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             for(var i = 1; i <= Math.floor(max / Nodes['Devices'][Node]['SampleFreq'] * Nodes['NominalFreq']); i++) {
               ticks.push({ v: i * Nodes['Devices'][Node]['SampleFreq'] / Nodes['NominalFreq'], label: '<div style="font-size: 10px; margin-top: -2px; text-align: center">' + Round2(i / Nodes['NominalFreq'] * 1000) + '</div>' });
             }
             return ticks;
           },
           valueFormatter: function (x) {
             return Round2((x * 1000) / Nodes['Devices'][Node]['SampleFreq']) + ' <em class="x-small">ms</em>';
           }
         }
    }
  };
  if(WaveformChart === null) {
    WaveformChart = new Dygraph(document.getElementById("WaveformChart"), WaveformDataset, Options);
  } else {
    WaveformChart.updateOptions(Options, true);
    WaveformChart.updateOptions({ file: WaveformDataset });
  }
}

/* Grafica de armonicos */
var HarmonicsChart      = null;
var HarmonicsDataset    = [];
var HarmonicsVOffset    = 0;
var HarmonicsCOffset    = 0;
function UpdateHarmonicsChart(result) {
  if(result !== null) {
    var date = new Date(result['Sampletime']);
    $('#HarmonicsTime').text('Last update at ' + date.toLocaleTimeString() + '.' + date.getMilliseconds());
    HarmonicsDataset = [];
    for(i = 0; i < result['harmonics_v'][0].length; i++) {
      if(Nodes['Devices'][Node]['Type'] === '3Phase') {
        HarmonicsDataset.push([result['harmonics_i'][0][i], result['harmonics_i'][1][i], result['harmonics_i'][2][i], result['harmonics_v'][0][i], result['harmonics_v'][1][i], result['harmonics_v'][2][i]]);
      } else {
        HarmonicsDataset.push([result['harmonics_i'][0][i], result['harmonics_v'][0][i]]);      
      }
    }
  }
  var ShowR = (localStorage.getItem("CurrentDetailPhaseR") === 'true');
  var ShowS = (localStorage.getItem("CurrentDetailPhaseS") === 'true');
  var ShowT = (localStorage.getItem("CurrentDetailPhaseT") === 'true');
  var ShowVoltage = (localStorage.getItem("CurrentDetailShowVoltage") === 'true');
  var VoltageSeries = [];
  var CurrentSeries = [];
  var Visibility    = [];
  if(Nodes['Devices'][Node]['Type'] === '3Phase') {
    if(ShowR) CurrentSeries.push(0);
    if(ShowS) CurrentSeries.push(1);
    if(ShowT) CurrentSeries.push(2);
    if(ShowR && ShowVoltage) VoltageSeries.push(3);
    if(ShowS && ShowVoltage) VoltageSeries.push(4);
    if(ShowT && ShowVoltage) VoltageSeries.push(5);
    Visibility = [ShowR, ShowS, ShowT, ShowR && ShowVoltage, ShowS && ShowVoltage, ShowT && ShowVoltage];
  } else {
    VoltageSeries.push(1);
    CurrentSeries.push(0);
    Visibility = [true, ShowVoltage];
  }
  var ToShow = [];
  for(i = (localStorage.getItem('CurrentDetailFirstHarmonic') === 'true') ? 0 : 1; i < HarmonicsDataset.length; i++) {
    ToShow.push([]);
    var Pos = (localStorage.getItem('CurrentDetailFirstHarmonic') === 'true') ? i : i - 1
    for(n = 0; n < HarmonicsDataset[i].length; n++) ToShow[Pos][n] = 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][n]));
  }
  var VoltageRange = CalcRanges(ToShow, VoltageSeries);
  var CurrentRange = CalcRanges(ToShow, CurrentSeries);
  HarmonicsVOffset = (VoltageRange[0] < 0.0) ? -VoltageRange[0] : 0.0;
  HarmonicsCOffset = (CurrentRange[0] < 0.0) ? -CurrentRange[0] : 0.0;
  for(i = 0; i < ToShow.length; i++) {
    if(Nodes['Devices'][Node]['Type'] === '3Phase') {
      ToShow[i] = [i, ToShow[i][0] + HarmonicsCOffset, ToShow[i][1] + HarmonicsCOffset, ToShow[i][2] + HarmonicsCOffset, ToShow[i][3] + HarmonicsVOffset, ToShow[i][4] + HarmonicsVOffset, ToShow[i][5] + HarmonicsVOffset];
    } else {
      ToShow[i] = [i, ToShow[i][0] + HarmonicsCOffset, ToShow[i][1] + HarmonicsVOffset];
    }
  }
  var Options = { 
    strokeWidth: 1,
    labelsDiv: "HarmonicsLegend",  
    labels: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['Time', 'Current (R)', 'Current (S)', 'Current (T)', 'Voltage (R)', 'Voltage (S)', 'Voltage (T)'] : ['Time', 'Current', 'Voltage'],
    colors: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['#428bca', '#42ca81', '#ca428b', '#d9534f', '#534fd9', '#4fd953'] : ['#428bca', '#d9534f'],
    visibility: Visibility,  
    plotter: multiColumnBarPlotter,
    dateWindow: [-0.5, (localStorage.getItem('CurrentDetailFirstHarmonic') === 'true') ? 49.5 : 48.5],
    gridLineColor: "#999",
    gridLinePattern: [4, 4],
    interactionModel: { },
    series: (Nodes['Devices'][Node]['Type'] === '3Phase') ? { 'Voltage (R)': { axis: 'y2' }, 'Voltage (S)': { axis: 'y2' }, 'Voltage (T)': { axis: 'y2'  } } : { 'Voltage': { axis: 'y2' } },
    axes: { 
      y: { pixelsPerLabel: 15,
           axisLabelWidth: 20,
           valueRange: [CurrentRange[0] + HarmonicsCOffset, CurrentRange[1] + HarmonicsCOffset],
           ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             for(var i = Math.floor(min - HarmonicsCOffset); i < Math.ceil(max - HarmonicsCOffset); i++) {
               if((i % 10) == 0) ticks.push({ v: i + HarmonicsCOffset, label: '<div style="font-size: 10px; margin-top: -2px; text-align: right">' + i + '</div>' });
             }
             return ticks;
           },
           valueFormatter: function (x, a, b, c, row, col) {
             var Row = ((localStorage.getItem('CurrentDetailFirstHarmonic') === 'true') ? 0 : 1) + row;
             return Round2(x - HarmonicsCOffset) + '&nbsp;<em class="small">dB</em>&nbsp;(' + (Round2(HarmonicsDataset[Row][col - 1])) + '<em class="small">&nbsp;A</em>)';
           }
         },
      y2: { pixelsPerLabel: 15,
            axisLabelWidth: ShowVoltage ? 15 : 0,
            valueRange: [VoltageRange[0] + HarmonicsVOffset, VoltageRange[1] + HarmonicsVOffset], 
            valueFormatter: function (x, a, b, c, row, col) {
              var Row = ((localStorage.getItem('CurrentDetailFirstHarmonic') === 'true') ? 0 : 1) + row;
              return Round2(x - HarmonicsVOffset) + '&nbsp;<em class="small">dB</em>&nbsp;(' + (Round2(HarmonicsDataset[Row][col - 1])) + '<em class="small">&nbsp;V</em>)';
            },
            ticker: function(min, max, pixels, opts, dygraph, vals) {
              var ticks = [];
              if(ShowVoltage == true) {
                for(var i = Math.floor(min - HarmonicsVOffset); i < Math.ceil(max - HarmonicsVOffset); i++) {
                  if((i % 10) == 0) ticks.push({ v: i + HarmonicsVOffset, label: '<div style="font-size: 10px; margin-top: -2px; text-align: right">' + i + '</div>' });
                }
              }
              return ticks;
            }
          },
      x: { drawGrid: false, 
           axisLabelFormatter: function (x) {
             if(localStorage.getItem('CurrentDetailFirstHarmonic') === 'true') return '<div style="font-size: 10px; margin-top: -2px">' + HarmonicsNames[x] + '</div>';
             return '<div style="font-size: 10px; margin-top: -2px">' + HarmonicsNames[x + 1] + '</div>';
           },
           valueFormatter: function (y) {
             if(localStorage.getItem('CurrentDetailFirstHarmonic') === 'true') return HarmonicsNames[y] + ' Harmonic (' + (50 * (y + 1)) + '<em class="small"> Hz</em>)';
             return HarmonicsNames[y+1] + ' Harmonic (' + (50 * (y + 1)) + '<em class="small"> Hz</em>)';
           },
           ticker: function(min, max, pixels, opts, dygraph, vals) {
              var ticks = [];
              for(var i = 0; i < HarmonicsNames.length; i++) {
                if(localStorage.getItem('CurrentDetailFirstHarmonic') === 'true') {
                  if((i % 2) == 0) ticks.push({ v: i, label: '<div style="font-size: 10px; margin-top: -2px;">' + HarmonicsNames[i] + '</div>' });
                } else {
                  if((i % 2) == 1) ticks.push({ v: i, label: '<div style="font-size: 10px; margin-top: -2px;">' + HarmonicsNames[i + 1] + '</div>' });
                }
              }
              return ticks;
            }
         }
    }
  };
  if(HarmonicsChart === null) {
    HarmonicsChart = new Dygraph(document.getElementById("HarmonicsChart"), ToShow, Options);
  } else {
    HarmonicsChart.updateOptions(Options, true);
    HarmonicsChart.updateOptions({ file: ToShow });
  }
}

/* Grafica de FFT */
var FFTChart            = null;
var FFTDataset          = [];
var FFTVOffset          = 0;
var FFTCOffset          = 0;
function UpdateFFTChart(result) {
  if(result !== null) {
    var date = new Date(result['Sampletime']);
    $('#FFTTime').text('Last update at ' + date.toLocaleTimeString() + '.' + date.getMilliseconds());
    FFTDataset = [];
    for(i = 0; i < result['fft_v'][0].length; i++) {
      if(Nodes['Devices'][Node]['Type'] === '3Phase') {
        FFTDataset.push([result['fft_v'][0][i], result['fft_v'][1][i], result['fft_v'][2][i], result['fft_i'][0][i], result['fft_i'][1][i], result['fft_i'][2][i]]);
      } else {
        FFTDataset.push([result['fft_v'][0][i], result['fft_i'][0][i]]);
      }
    }
  }
  var ShowR = (localStorage.getItem("VoltageDetailPhaseR") === 'true');
  var ShowS = (localStorage.getItem("VoltageDetailPhaseS") === 'true');
  var ShowT = (localStorage.getItem("VoltageDetailPhaseT") === 'true');
  var ShowVoltage = (localStorage.getItem("CurrentDetailShowVoltage") === 'true');
  var VoltageSeries = [];
  var CurrentSeries = [];
  var Visibility    = [];
  if(Nodes['Devices'][Node]['Type'] === '3Phase') {
    if(ShowR && ShowVoltage) VoltageSeries.push(0);
    if(ShowS && ShowVoltage) VoltageSeries.push(1);
    if(ShowT && ShowVoltage) VoltageSeries.push(2);
    if(ShowR) CurrentSeries.push(3);
    if(ShowS) CurrentSeries.push(4);
    if(ShowT) CurrentSeries.push(5);
    Visibility = [ShowR && ShowVoltage, ShowS && ShowVoltage, ShowT && ShowVoltage, ShowR, ShowS, ShowT];
  } else {
    VoltageSeries.push(0);
    CurrentSeries.push(1);
    Visibility = [ShowVoltage, true];
  }
  var ToShow = [];
  for(i = 0; i < FFTDataset.length; i++) {
    ToShow.push([]);
    for(n = 0; n < FFTDataset[i].length; n++) ToShow[i][n] = 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][n]));
  }
  var VoltageRange = CalcRanges(ToShow, VoltageSeries);
  var CurrentRange = CalcRanges(ToShow, CurrentSeries);
  FFTVOffset = (VoltageRange[0] < 0.0) ? -VoltageRange[0] : 0.0;
  FFTCOffset = (CurrentRange[0] < 0.0) ? -CurrentRange[0] : 0.0;
  for(i = 0; i < ToShow.length; i++) {
    if(Nodes['Devices'][Node]['Type'] === '3Phase') {
      ToShow[i] = [i, ToShow[i][0] + FFTVOffset, ToShow[i][1] + FFTVOffset, ToShow[i][2] + FFTVOffset, ToShow[i][3] + FFTCOffset, ToShow[i][4] + FFTCOffset, ToShow[i][5] + FFTCOffset];
    } else {
      ToShow[i] = [i, ToShow[i][0] + FFTVOffset, ToShow[i][1] + FFTCOffset];
    }
  }
  var Options = { 
    strokeWidth: 1,
    labelsDiv: "FFTLegend",  
    labels: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['Time', 'Voltage (R)', 'Voltage (S)', 'Voltage (T)', 'Current (R)', 'Current (S)', 'Current (T)'] : ['Time', 'Voltage', 'Current'],
    colors: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['#d9534f', '#534fd9', '#4fd953', '#428bca', '#42ca81', '#ca428b'] : ['#d9534f', '#428bca'],
    visibility: Visibility,
    fillGraph: true,
    gridLineColor: "#999",
    gridLinePattern: [4, 4],
    showRangeSelector: true,
    rangeSelectorHeight: 20,
    rangeSelectorPlotFillColor: "#FFFFFF",
    rangeSelectorPlotStrokeColor: "#428bca",
    interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup : mouseUpOZM, dblclick : dblClickOZM_Restore },
    series: (Nodes['Devices'][Node]['Type'] === '3Phase') ? { 'Current (R)': { showInRangeSelector: true }, 'Voltage (R)': { axis: 'y2' }, 'Voltage (S)': { axis: 'y2' }, 'Voltage (T)': { axis: 'y2'  } } : { 'Current': { showInRangeSelector: true }, 'Voltage': { axis: 'y2' } },
    plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
    axes: { 
      y: { pixelsPerLabel: 15,
           axisLabelWidth: 20,
           valueRange: [CurrentRange[0] + FFTCOffset, CurrentRange[1] + FFTCOffset], 
           ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             for(var i = Math.floor(min - FFTCOffset); i < Math.ceil(max - FFTCOffset); i++) {
               if((i % 10) == 0) ticks.push({ v: i + FFTCOffset, label: '<div style="font-size: 10px; margin-top: -2px; text-align: right">' + i + '</div>' });
             }
             return ticks;
           },
           valueFormatter: function (x, a, b, c, row, col) {
             return Round2(x - FFTCOffset) + '&nbsp;<em class="small">dB</em>&nbsp;(' + (Round2(FFTDataset[row][col - 1])) + '<em class="small">&nbsp;A</em>)';
           }
         },
      y2: { pixelsPerLabel: 15,
            axisLabelWidth: ShowVoltage ? 15 : 0,
            valueRange: [VoltageRange[0] + FFTVOffset, VoltageRange[1] + FFTVOffset],
            valueFormatter: function (x, a, b, c, row, col) {
              return Round2(x - FFTVOffset) + '&nbsp;<em class="small">dB</em>&nbsp;(' + (Round2(FFTDataset[row][col - 1])) + '<em class="small">&nbsp;V</em>)';
            },
            ticker: function(min, max, pixels, opts, dygraph, vals) {
              var ticks = [];
              if(ShowVoltage == true) {
                for(var i = Math.floor(min - FFTCOffset); i < Math.ceil(max - FFTCOffset); i++) {
                  if((i % 10) == 0) ticks.push({ v: i + FFTVOffset, label: '<div style="font-size: 10px; margin-top: -2px; text-align: right">' + i + '</div>' });
                }
              }
              return ticks;
            }
          },
      x: { drawGrid: true,
           axisLabelFormatter: function (y) {
             y = (y / 10 * Nodes['NominalFreq']);
             if(y > 1000) {
               y = y / 1000;
               if((y % 1) != 0) return "";
               return y + '<em class="small"> kHz</em>';
             } else {
               if((y % 1) != 0) return "";
               return y + '<em class="small"> Hz</em>';
             }
           },
           valueFormatter: function (y) {
             return Math.round(y / 10 * Nodes['NominalFreq']) + '<em class="small"> Hz</em>';
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
}

/* Evolucion de parametros*/
var HarmonicsCharts     = [];
var LastEvolutionSample = 0;
var HarmonicsDatasets   = [];
var PowerFactorChart    = null;
var PowerFactorDataset  = [];
var THDChart            = null;
var THDDataset          = [];
var PHIChart            = null;
var PHIDataset          = [];
function AjaxGetEvolution() {
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 86400000, To: 0, Aggreg: '1m', LastSample: LastEvolutionSample, Series: 'power_factor, thd_i, phi, harmonics_i', Node: Nodes['Devices'][Node].ID}),
           success: AjaxEvolutions,
           complete: function() { 
             setTimeout(AjaxGetEvolution, 60000);
           }
  });
}
function AjaxEvolutions(result) {  
  UpdateHarmonicsCharts(result);
  UpdatePowerFactorChart(result);
  UpdateTHDChart(result);
  UpdatePHIChart(result);
  LastEvolutionSample = result['Sampletime'][result['Sampletime'].length - 1];
  Dygraph.synchronize([PowerFactorChart, THDChart, PHIChart], { range: false, zoom: false });
}

/* Graficos de evolucion de parametros */
function UpdatePowerFactorChart(result) {
  if(result !== null) {
    for(i = 0; i < result['power_factor'].length; i++) PowerFactorDataset.push([new Date(result['Sampletime'][i])].concat(result['power_factor'][i]));
    while(PowerFactorDataset.length > result['MaxSamples']) PowerFactorDataset.shift();
  }
  var Visibility = (Nodes['Devices'][Node]['Type'] === '3Phase') ? [localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseR', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseS', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseT'] : [true];
  if(PowerFactorChart === null) {
    PowerFactorChart = new Dygraph(document.getElementById("PowerFactorChart"),
      PowerFactorDataset,
      { strokeWidth: 0.5,
        labels: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['Time', 'Power Factor (R)', 'Power Factor (S)', 'Power Factor (T)'] : ['Time', 'Power Factor'],
        colors: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['#d9534f', '#d9534f', '#d9534f'] : ['#d9534f'],
        visibility: Visibility,
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        highlightCircleSize: 0,
        interactionModel: { },
        labelsDiv: 'PowerFactorChartLegend',
        legend: 'always',
        legendFormatter: function(data) {
          if(data.x == null) {
            for(i = 0; i < data.series.length; i++) 
              if(data.series[i].isVisible) return '<span style="font-size: 18px; font-weight: bold">' + data.series[i].label + '</span>';
          } else {
            for(i = 0; i < data.series.length; i++) 
              if(data.series[i].isVisible) return '<span style="font-size: 15px; font-weight: bold">' + data.series[i].label + '</span><br/>' + data.xHTML + data.series[i].yHTML;
          }
        },
        xAxisHeight: 0.1,
        xLabelHeight: 1,
        plotter: barChartPlotterCubism,
        dateWindow: [result['From'], result['To']],
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        axes: {
          y: { drawAxis: false,
               drawGrid:false,
               valueFormatter: function (x) {
                 return '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + Round2(x) + '</div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return '<div style="font-size: 10px; line-height: 10px">' + date.toLocaleTimeString() + '</div>';
               },
               axisLabelFormatter: function(x, i) {
                 if((x.getHours() == 0) && (x.getMinutes() == 0) && (x.getSeconds() == 0)) {
                   return '<div style="font-size: 10px; margin-top: -73px; line-height: 10px; font-weight: bold">' + x.toLocaleDateString() + '</div>';
                 } else {
                   return '<div style="font-size: 10px; margin-top: -73px; line-height: 10px">' + x.toLocaleTimeString() + '</div>';                       
                 }
               }
          }
        }
      }
    );
  } else {
    if(result !== null) PowerFactorChart.updateOptions({ dateWindow: [result['From'], result['To']] }, true);
    PowerFactorChart.updateOptions({ file: PowerFactorDataset, visibility: Visibility });
  }
}
function UpdateTHDChart(result) {
  if(result !== null) {
    for(i = 0; i < result['thd_i'].length; i++) {
      THDDataset.push([new Date(result['Sampletime'][i])].concat(result['thd_i'][i]));
    }
    while(THDDataset.length > result['MaxSamples']) THDDataset.shift();
  }
  var Visibility = (Nodes['Devices'][Node]['Type'] === '3Phase') ? [localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseR', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseS', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseT'] : [true];
  if(THDChart === null) {
    THDChart = new Dygraph(document.getElementById("THDChart"),
      THDDataset,
      { strokeWidth: 0.5,
        labels: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['Time', 'THD (R)', 'THD (S)', 'THD (T)'] : ['Time', 'THD'],
        colors: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['#d9534f', '#d9534f', '#d9534f'] : ['#d9534f'],
        visibility: Visibility,
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        highlightCircleSize: 0,
        interactionModel: { },
        labelsDiv: 'THDChartLegend',
        legend: 'always',
        legendFormatter: function(data) {
          if(data.x == null) {
            for(i = 0; i < data.series.length; i++) 
              if(data.series[i].isVisible) return '<span style="font-size: 18px; font-weight: bold">' + data.series[i].label + '</span>';
          } else {
            for(i = 0; i < data.series.length; i++) 
              if(data.series[i].isVisible) return '<span style="font-size: 15px; font-weight: bold">' + data.series[i].label + '</span><br/>' + data.xHTML + data.series[i].yHTML;
          }
        },
        xAxisHeight: 0.1,
        xLabelHeight: 1,
        plotter: barChartPlotterCubism,
        dateWindow: [result['From'], result['To']],
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        axes: {
          y: { drawAxis: false,
               drawGrid:false,
               valueFormatter: function (x) {
                 return '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + Round2(x * 100) + '<em>%</em></div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return '<div style="font-size: 10px; line-height: 10px">' + date.toLocaleTimeString() + '</div>';
               },
               axisLabelFormatter: function(x, i) {
                 if((x.getHours() == 0) && (x.getMinutes() == 0) && (x.getSeconds() == 0)) {
                   return '<div style="font-size: 10px; margin-top: -73px; line-height: 10px; font-weight: bold">' + x.toLocaleDateString() + '</div>';
                 } else {
                   return '<div style="font-size: 10px; margin-top: -73px; line-height: 10px">' + x.toLocaleTimeString() + '</div>';                       
                 }
               }
          }
        }
      }
    ); 
  } else {
    if(result !== null) THDChart.updateOptions({ dateWindow: [result['From'], result['To']] }, true);
    THDChart.updateOptions({ file: THDDataset, visibility: Visibility });
  }
}
function UpdatePHIChart(result) {
  if(result !== null) {
    for(i = 0; i < result['phi'].length; i++) PHIDataset.push([new Date(result['Sampletime'][i])].concat(result['phi'][i]));
    while(PHIDataset.length > result['MaxSamples']) PHIDataset.shift();
  }
  var Visibility = (Nodes['Devices'][Node]['Type'] === '3Phase') ? [localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseR', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseS', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseT'] : [true];
  if(PHIChart === null) {
    PHIChart = new Dygraph(document.getElementById("PHIChart"),
      PHIDataset,
      { strokeWidth: 0.5,
        labels: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['Time', 'PHI (R)', 'PHI (S)', 'PHI (T)'] : ['Time', 'PHI'],
        colors: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['#d9534f', '#d9534f', '#d9534f'] : ['#d9534f'],
        visibility: Visibility,
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        highlightCircleSize: 0,
        interactionModel: { },
        labelsDiv: 'PHIChartLegend',
        legend: 'always',
        legendFormatter: function(data) {
          if(data.x == null) {
            for(i = 0; i < data.series.length; i++) 
              if(data.series[i].isVisible) return '<span style="font-size: 18px; font-weight: bold">' + data.series[i].label + '</span>';
          } else {
            for(i = 0; i < data.series.length; i++) 
              if(data.series[i].isVisible) return '<span style="font-size: 15px; font-weight: bold">' + data.series[i].label + '</span><br/>' + data.xHTML + data.series[i].yHTML;
          }
        },
        xAxisHeight: 0.1,
        xLabelHeight: 1,
        plotter: barChartPlotterCubism,
        dateWindow: [result['From'], result['To']],
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        axes: {
          y: { drawAxis: false,
               drawGrid:false,
               valueFormatter: function (x) {
                 return '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + Round2(Math.abs(x)) + '<em>º' + ((x > 0) ? '(IND)' : (x < 0) ? '(CAP)' : '') + '</em></div>';
               }
             },
          x: { valueFormatter: function (x) {
                 var date = new Date(x);
                 return '<div style="font-size: 10px; line-height: 10px">' + date.toLocaleTimeString() + '</div>';
               },
               axisLabelFormatter: function(x, i) {
                 if((x.getHours() == 0) && (x.getMinutes() == 0) && (x.getSeconds() == 0)) {
                   return '<div style="font-size: 10px; margin-top: -73px; line-height: 10px; font-weight: bold">' + x.toLocaleDateString() + '</div>';
                 } else {
                   return '<div style="font-size: 10px; margin-top: -73px; line-height: 10px">' + x.toLocaleTimeString() + '</div>';                       
                 }
               }
          }
        }
      }
    );
  } else {
    if(result !== null) PHIChart.updateOptions({ dateWindow: [result['From'], result['To']] }, true);
    PHIChart.updateOptions({ file: PHIDataset, visibility: Visibility });
  }
}
/* Graficos de evolucion de harmonicos */
function UpdateHarmonicsCharts(result) {
  if(result !== null) {
    while(HarmonicsDatasets.length !== 50) HarmonicsDatasets.push([]);
    for(k = 0; k < 50; k++) {
      for(i = 0; i < result['harmonics_i'].length; i++) {
        var Row = [new Date(result['Sampletime'][i])];
        for(j = 0; j < result['harmonics_i'][i].length; j++) Row.push(20 * Math.log10(result['harmonics_i'][i][j][k]));
        HarmonicsDatasets[k].push(Row);
      }
      while(HarmonicsDatasets[k].length > result['MaxSamples']) HarmonicsDatasets[k].shift();
    }
  }
  var Visibility = (Nodes['Devices'][Node]['Type'] === '3Phase') ? [localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseR', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseS', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseT'] : [true];
  if(HarmonicsCharts.length == 0) {
    for(i = 0; i < 50; i++) {
      HarmonicsCharts[i] = new Dygraph(document.getElementById("HarmonicsEvolutionChart_" + i),
        HarmonicsDatasets[i],
        { strokeWidth: 0.5,
          labels: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['Time', HarmonicsNames[i] + ' (R)', HarmonicsNames[i] + ' (S)', HarmonicsNames[i] + ' (T)'] : ['Time', HarmonicsNames[i]],
          colors: (Nodes['Devices'][Node]['Type'] === '3Phase') ? ['#d9534f', '#d9534f', '#d9534f'] : ['#d9534f'],
          visibility: Visibility,
          gridLineColor: "#999",
          gridLinePattern: [4, 4],
          highlightCircleSize: 0,
          interactionModel: { },
          labelsDiv: 'HarmonicsEvolutionChartLegend_' + i,
          legend: 'always',
          legendFormatter: function(data) {
            var Index = (Visibility[0] === true) ? 0 : (Visibility[1] === true) ? 1 : 2;
            if(data.x == null) {
              return '<span style="font-size: 18px; font-weight: bold">' + data.series[Index].label + '</span>';
            } else {
              return '<span style="font-size: 15px; font-weight: bold">' + data.series[Index].label + '</span><br/>' + data.xHTML + data.series[Index].yHTML;
            }
          },
          xAxisHeight: 0.1,
          xLabelHeight: 1,
          plotter: barChartPlotterCubism,
          dateWindow: [result['From'], result['To']],
          plugins: [
            new Dygraph.Plugins.Crosshair({
              direction: "vertical"
            })
          ],
          axes: {
            y: { drawAxis: false,
                 drawGrid:false,
                 valueFormatter: function (x) {
                   return '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + Round2(x) + '<em>dB</em></div>';
                 }
               },
            x: { valueFormatter: function (x) {
                   var date = new Date(x);
                   return '<div style="font-size: 10px; line-height: 10px">' + date.toLocaleTimeString() + '</div>';
                 },
                 axisLabelFormatter: (i === 0) ? function(x, i) {
                   var date = new Date(x);
                   if((date.getHours() == 0) && (date.getMinutes() == 0) && (date.getSeconds() == 0)) {
                     return '<div style="font-size: 10px; margin-top: -63px; line-height: 10px; font-weight: bold">' + date.toLocaleDateString() + '</div>';
                   } else {
                     return '<div style="font-size: 10px; margin-top: -63px; line-height: 10px">' + date.toLocaleTimeString() + '</div>';                       
                   }
                 } : function() { return ''; }
            }
          }
        }
      );
    }
    Dygraph.synchronize(HarmonicsCharts, { range: false, zoom: false });
  } else {
    for(i = 0; i < 50; i++) {
      if(result !== null) HarmonicsCharts[i].updateOptions({ dateWindow: [result['From'], result['To']] }, true);
      HarmonicsCharts[i].updateOptions({ file: HarmonicsDatasets[i], visibility: Visibility });
    }
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
  MainScrollable = new Scrollable( {Spinners: ['#CurrentSpinner'], Series: 'rms_i', OnSuccess: Scrollable_OnSuccess} );
  new Live( {RefreshInterval: 5000, Spinners: ['#DetailSpinner'], Series: 'wave_i, wave_v, harmonics_i, harmonics_v, fft_i, fft_v, power_factor, thd_i, phi, rms_i, rms_v', OnSuccess: Live_OnSuccess});
  new PeriodView( {Spinners: [], Series: 'power_factor, thd_i, phi, harmonics_i', Period: 86400000, Aggreg: '1m', OnSuccess: PeriodView_OnSuccess} );
  Scrollable_UpdateCounters(true);
}//1485