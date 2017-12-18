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

/* Grafica de voltage */
var LastVoltageSample   = 0;
var VoltageChart        = null;
var VoltageData         = [];
var VoltageSpinner      = new Spinner(SpinnerOpts);
var VoltageTimer        = null;
function AjaxGetVoltage() {
  if(VoltageTimer !== null) clearInterval(VoltageTimer);
  $('#btnVoltageNow').addClass('disabled');
  if(LastVoltageSample === 0) ShowSpinner(VoltageSpinner, 'VoltageSpinner');
  var Period = parseInt(Cookies.get('GraphPeriod'));
  if(!Period) {
    Period = 86400000;
    Cookies.set('GraphPeriod', Period);
  }
  var GraphEnd = parseInt(Cookies.get('GraphEnd'));
  if(!GraphEnd) {
    GraphEnd = 0;
    Cookies.set('GraphEnd', 0);
  } 
  var GraphAggregation = Cookies.get('GraphAggregation');
  if(!GraphAggregation || (GetAggregText(GraphAggregation) === false)) {
    GraphAggregation = '10m';
    Cookies.set('GraphAggregation', GraphAggregation);
  }
  $.ajax({ url: "getSeries",
    timeout: 20000,
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data: JSON.stringify({ Period: Period, To: GraphEnd, Aggreg: GraphAggregation, LastSample: LastVoltageSample, Series: 'rms_v', Node: Node}),
    success: AjaxVoltage,
    complete: function() { 
      $('#btnVoltageNow').removeClass('disabled');
      if(parseInt(Cookies.get('GraphEnd')) === 0) VoltageTimer = setTimeout(AjaxGetVoltage, 3000);
    }
 });
}
function AjaxVoltage(result) {
  if(LastVoltageSample === 0) {
    StopSpinner(VoltageSpinner, 'VoltageSpinner');
    VoltageData = [];
  }
  if(result['Sampletime'] !== null) {
    for(i = 0; i < result['Sampletime'].length; i++) VoltageData.push([new Date(result['Sampletime'][i])].concat(result['rms_v'][i]));
    while(VoltageData.length > result['MaxSamples']) VoltageData.shift();
    LastVoltageSample = result['Sampletime'][result['Sampletime'].length - 1];
  }
  $('#VoltageAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');  
  if(VoltageChart == null) {
    var VisibleNames = ["Time"];
    if(Nodes['Devices'][Node]['Type'] == '3Phase') {
      VisibleNames = VisibleNames.concat(['Voltage (R)', 'Voltage (S)', 'Voltage (T)']);
    } else {
      VisibleNames.push('Voltage');
    }
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"),
      VoltageData,
      { strokeWidth: 1,
        labelsDiv: "VoltageLegend",
        labels: VisibleNames,
        stepPlot: true,
        fillGraph: false,
        colors: ["#d9534f", "#534fd9", "#4fd953"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        dateWindow: [result['From'], result['To']],
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        underlayCallback: function(canvas, area, g) {
          var NominalVoltage = Nodes['NominalVoltage'];
          var YellowLowA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.07);
          var YellowHeightA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.10) - YellowLowA;
          var YellowLowB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.07);
          var YellowHeightB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.10) - YellowLowB;
          var RedLowA = g.toDomYCoord(1000);
          var RedHeightA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.10) - RedLowA;
          var RedLowB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.10);
          var RedHeightB = g.toDomYCoord(0) - RedLowB;
          canvas.fillStyle = "rgba(255, 255, 0, 0.15)";
          canvas.fillRect(0, YellowLowA, 10000, YellowHeightA);
          canvas.fillRect(0, YellowLowB, 10000, YellowHeightB);
          canvas.fillStyle = "rgba(180, 0, 0, 0.15)";
          canvas.fillRect(0, RedLowA, 10000, RedHeightA);
          canvas.fillRect(0, RedLowB, 10000, RedHeightB);
        },
        zoomCallback: function(minDate, maxDate, yRanges) {
          clearInterval(VoltageTimer);
          $('#btnVoltageNow').removeClass('active');
          Cookies.set('GraphEnd', maxDate);
          Cookies.set('GraphPeriod', maxDate - minDate);
          LastVoltageSample = 0;
          AjaxGetVoltage();
        },
        interactionModel: {
          mousedown: mouseDownOZM,
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              $('#btnVoltageNow').removeClass('active');
              Cookies.set('GraphEnd', g.dateWindow_[1]);
              Cookies.set('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastVoltageSample = 0;
              AjaxGetVoltage();
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            $('#btnVoltageNow').removeClass('active');
            Cookies.set('GraphEnd', g.dateWindow_[1]);
            Cookies.set('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
            LastVoltageSample = 0;
            AjaxGetVoltage();
          }
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">V</em>';
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
    $(window).bind('resize', VoltageResizeEvent);
    window.dispatchEvent(new Event('resize'));
  } else {
    VoltageChart.updateOptions({file: VoltageData, dateWindow: [result['From'], result['To']]});
  }
  SetVoltageVisibleSeries();
  $('#btnShowVoltageYear').removeClass('active');
  $('#btnShowVoltageMonth').removeClass('active');
  $('#btnShowVoltageWeek').removeClass('active');
  $('#btnShowVoltageDay').removeClass('active');
  if((result['To'] - result['From']) === 86400000) {
    $('#btnShowVoltageDay').addClass('active');
  } else if((result['To'] - result['From']) === 604800000) {
    $('#btnShowVoltageWeek').addClass('active');
  } else if((result['To'] - result['From']) === 2592000000) {
    $('#btnShowVoltageMonth').addClass('active');
  } else if((result['To'] - result['From']) === 31536000000) {
    $('#btnShowVoltageYear').addClass('active');
  }
  var GraphAggregation = Cookies.get('GraphAggregation');
  if(GraphAggregation !== result['Aggregation_Source']) {
    $('#aggreg_modal').modal('show');
    GraphAggregation = result['Aggregation_Source'];
  }
  $('#btnVoltage3S').removeClass('active');
  $('#btnVoltage1M').removeClass('active');
  $('#btnVoltage10M').removeClass('active');
  $('#btnVoltage1H').removeClass('active');
  if(GraphAggregation == '3s') {
    $('#btnVoltage3S').addClass('active');
  } else if(GraphAggregation == '1m') {
    $('#btnVoltage1M').addClass('active');
  } else if(GraphAggregation == '10m') {
    $('#btnVoltage10M').addClass('active');
  } else if(GraphAggregation == '1h') {
    $('#btnVoltage1H').addClass('active');
  }
  var GraphEnd = parseInt(Cookies.get('GraphEnd'));
  if(GraphEnd === 0) {
    $('#btnVoltageNow').addClass('active');
  } else {
    $('#btnVoltageNow').removeClass('active');
  }
  Cookies.set('GraphAggregation', GraphAggregation);
}
function VoltageResizeEvent() {
  var Height = $('#VoltageAvgPanel').height() + $('#VoltageMaxPanel').height() + $('#VoltageMinPanel').height();
  VoltageChart.resize($('#VoltageCol').width(), Height - 4);
}
function SetVoltageVisibleSeries() {
  if(Nodes['Devices'][Node]['Type'] == '3Phase') {
    var ShowR = (Cookies.get('ShowR') === 'true');
    var ShowS = (Cookies.get('ShowS') === 'true');
    var ShowT = (Cookies.get('ShowT') === 'true');
    var VisibleSeries = [ShowR, ShowS, ShowT];
    var Visibles = [];
    if(ShowR) Visibles.push(1);
    if(ShowS) Visibles.push(2);
    if(ShowT) Visibles.push(3);
    VoltageChart.updateOptions({ visibility: VisibleSeries, valueRange: CalcRanges(VoltageData, Visibles, false) });
    if(ShowR === true)
      $('#btnShowVoltageRPhase').addClass('active');
    else    
      $('#btnShowVoltageRPhase').removeClass('active');
    if(ShowS === true)
      $('#btnShowVoltageSPhase').addClass('active');
    else    
      $('#btnShowVoltageSPhase').removeClass('active');
    if(ShowT === true)
      $('#btnShowVoltageTPhase').addClass('active');
    else    
      $('#btnShowVoltageTPhase').removeClass('active');
  } else {
    VoltageChart.updateOptions({ valueRange: CalcRanges(VoltageData, [1], false) });
  }
}
function ChooseVoltageChannels() {
  var ShowR = (Cookies.get('ShowR') === 'true');
  var ShowS = (Cookies.get('ShowS') === 'true');
  var ShowT = (Cookies.get('ShowT') === 'true');
  if(this.innerText === 'R') ShowR = !ShowR;
  if(this.innerText === 'S') ShowS = !ShowS;
  if(this.innerText === 'T') ShowT = !ShowT;
  if((this.innerText === 'R') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowR = true;
  if((this.innerText === 'S') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowS = true;
  if((this.innerText === 'T') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowT = true;
  Cookies.set('ShowR', ShowR);
  Cookies.set('ShowS', ShowS);
  Cookies.set('ShowT', ShowT);
  SetVoltageVisibleSeries();
}
$('#btnShowVoltageRPhase').click(ChooseVoltageChannels);
$('#btnShowVoltageSPhase').click(ChooseVoltageChannels);
$('#btnShowVoltageTPhase').click(ChooseVoltageChannels);
$('#btnVoltage3S').click(function() {
  Cookies.set('GraphAggregation', '3s');
  LastVoltageSample = 0;
  AjaxGetVoltage();
});
$('#btnVoltage1M').click(function() {
  Cookies.set('GraphAggregation', '1m');
  LastVoltageSample = 0;
  AjaxGetVoltage();
});
$('#btnVoltage10M').click(function() {
  Cookies.set('GraphAggregation', '10m');
  LastVoltageSample = 0;
  AjaxGetVoltage();
});
$('#btnVoltage1H').click(function() {
  Cookies.set('GraphAggregation', '1h');
  LastVoltageSample = 0;
  AjaxGetVoltage();
});
$('#btnShowVoltageYear').click(function() {
  Cookies.set('GraphPeriod', 31536000000);
  LastVoltageSample = 0;
  AjaxGetVoltage();
  Cookies.set('CountersPeriod', 'Year');
  AjaxGetVoltageCounters();
});
$('#btnShowVoltageMonth').click(function() {
  Cookies.set('GraphPeriod', 2592000000);
  LastVoltageSample = 0;
  AjaxGetVoltage();
  Cookies.set('CountersPeriod', 'Month');
  AjaxGetVoltageCounters();
});
$('#btnShowVoltageWeek').click(function() {
  Cookies.set('GraphPeriod', 604800000);
  LastVoltageSample = 0;
  AjaxGetVoltage();
  Cookies.set('CountersPeriod', 'Week');
  AjaxGetVoltageCounters();
});
$('#btnShowVoltageDay').click(function() {
  Cookies.set('GraphPeriod', 86400000);
  LastVoltageSample = 0;
  AjaxGetVoltage();
  Cookies.set('CountersPeriod', 'Day');
  AjaxGetVoltageCounters();
});
$('#btnVoltageNow').click(function() {
  var End = parseInt(Cookies.get('GraphEnd'));
  if(End === 0) {
    $('#btnVoltageNow').removeClass('active');
    Cookies.set('GraphEnd', VoltageChart.getOption('dateWindow')[1]);
    if(VoltageTimer !== null) clearInterval(VoltageTimer);
  } else {
    $('#btnVoltageNow').addClass('active');
    Cookies.set('GraphEnd', 0);
    LastVoltageSample = 0;
    AjaxGetVoltage();
  }
});
$('#btnMaximizeVoltage').click(function() {
  $('#GraphDialogTitle').text('Voltage');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#VoltageCol');
  DialogElement = $('#VoltageMaximizeDiv').detach();
  DialogPrevResizeHandler = VoltageResizeEvent;
  DialogResizeHandler = function() {
    VoltageChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
  };
  $(window).unbind('resize', VoltageResizeEvent);
  $('#GraphDialogBody').append(DialogElement);
  $(window).bind('resize', DialogResizeHandler);
  window.dispatchEvent(new Event('resize'));  
});
$('#btnExportVoltage').click(function() {
  var Data = [];
  var Filename = "Voltage_" + LastVoltageTime + '.csv';
  Data.push([['Index', 'Voltage']]);
  Data = Data.concat(VoltageData);
  exportToCsv(Filename, Data);
  $(this).blur();
});

/* Contadores de voltage */
var VoltageCountersTimer  = null;
function AjaxGetVoltageCounters() {
  if(VoltageCountersTimer !== null) clearInterval(VoltageCountersTimer);
  VoltageCountersTimer = setTimeout(AjaxGetVoltageCounters, 60000);
  var CountersPeriod = Cookies.get('CountersPeriod');
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
    Cookies.set('CountersPeriod', CountersPeriod);
    Period = 86400000;
    Current = GetDayTimestamp();
    Aggreg = '1m';
  }
  $.ajax({ url: "getSeriesAggreg",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: "avg(rms_v), max(rms_v), min(rms_v)", Aggreg: Aggreg, Period: Period, To: Current + Period, Node: Node }),
           success: VoltageUpdateCountersCurrent
  });
  $.ajax({ url: "getSeriesAggreg",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: "avg(rms_v), max(rms_v), min(rms_v)", Aggreg: Aggreg, Period: Period, To: Current, Node: Node }),
           success: VoltageUpdateCountersPrevs
  });
}
function VoltageUpdateCountersCurrent(result) {
  var CountersPeriod = Cookies.get('CountersPeriod');
  if(CountersPeriod === 'Week') {
    $('#labelVoltagePeriodAVG').text('This week average');
    $('#labelVoltagePeriodMAX').text('This week maximum');
    $('#labelVoltagePeriodMIN').text('This week minimum');
  } else if(CountersPeriod === 'Month') {
    $('#labelVoltagePeriodAVG').text('This month average');
    $('#labelVoltagePeriodMAX').text('This month maximum');
    $('#labelVoltagePeriodMIN').text('This month minimum');
  } else if(CountersPeriod === 'Year') {
    $('#labelVoltagePeriodAVG').text('This year average');
    $('#labelVoltagePeriodMAX').text('This year maximum');
    $('#labelVoltagePeriodMIN').text('This year minimum');
  } else if(CountersPeriod === 'Day') {
    $('#labelVoltagePeriodAVG').text('Today average');
    $('#labelVoltagePeriodMAX').text('Today maximum');
    $('#labelVoltagePeriodMIN').text('Today minimum');
  }
  if(result['avg_rms_v'] === null) {
    $('#labelVoltageCurrentAVG').text('?');
    $('#labelVoltageCurrentMAX').text('?');
    $('#labelVoltageCurrentMIN').text('?');
  } else {
    $('#labelVoltageCurrentAVG').html(Array_Round2(result['avg_rms_v'], '&nbsp;&nbsp;', '', ["#d9534f", "#534fd9", "#4fd953"]));
    $('#labelVoltageCurrentMAX').html(Array_Round2(result['max_rms_v'], '&nbsp;&nbsp;', '', ["#d9534f", "#534fd9", "#4fd953"]));
    $('#labelVoltageCurrentMIN').html(Array_Round2(result['min_rms_v'], '&nbsp;&nbsp;', '', ["#d9534f", "#534fd9", "#4fd953"]));
  }
}
function VoltageUpdateCountersPrevs(result) {
  var CountersPeriod = Cookies.get('CountersPeriod');
  if(CountersPeriod === 'Week') {
    $('#labelVoltagePrevPeriodAVG').text('Previous week average');
    $('#labelVoltagePrevPeriodMAX').text('Previous week maximum');
    $('#labelVoltagePrevPeriodMIN').text('Previous week minimum');
  } else if(CountersPeriod === 'Month') {
    $('#labelVoltagePrevPeriodAVG').text('Previous month average');
    $('#labelVoltagePrevPeriodMAX').text('Previous month maximum');
    $('#labelVoltagePrevPeriodMIN').text('Previous month minimum');
  } else if(CountersPeriod === 'Year') {
    $('#labelVoltagePrevPeriodAVG').text('Previous year average');
    $('#labelVoltagePrevPeriodMAX').text('Previous year maximum');
    $('#labelVoltagePrevPeriodMIN').text('Previous year minimum');
  } else if(CountersPeriod === 'Day') {
    $('#labelVoltagePrevPeriodAVG').text('Yesterday average');
    $('#labelVoltagePrevPeriodMAX').text('Yesterday maximum');
    $('#labelVoltagePrevPeriodMIN').text('Yesterday minimum');
  }
  if(result['rms_rms_v'] === null) {
    $('#labelVoltagePrevAVG').text('?');
    $('#labelVoltagePrevMAX').text('?');
    $('#labelVoltagePrevMIN').text('?');
  } else {
    $('#labelVoltagePrevAVG').html(Array_Round2(result['avg_rms_v'], '&nbsp;&nbsp;', '', ["#d9534f", "#534fd9", "#4fd953"]));
    $('#labelVoltagePrevMAX').html(Array_Round2(result['max_rms_v'], '&nbsp;&nbsp;', '', ["#d9534f", "#534fd9", "#4fd953"]));
    $('#labelVoltagePrevMIN').html(Array_Round2(result['min_rms_v'], '&nbsp;&nbsp;', '', ["#d9534f", "#534fd9", "#4fd953"]));
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

/* Vista de detalles */
function AjaxGetDetails() {
  if(WaveformChart === null) ShowSpinner(DetailSpinner, 'DetailSpinner');
  $.ajax({ url: "getSeriesNow",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: 'wave_v, wave_i, harmonics_v, harmonics_i, fft_v, fft_i', Node: Node}),
           success: function(result) {
             if(WaveformChart === null) StopSpinner(DetailSpinner, 'DetailSpinner');
             UpdateWaveformChart(result);
             UpdateHarmonicsChart(result);
             UpdateFFTChart(result);
           },
           error: function(bliu, blau, blu) {
             $('#WaveformTime').text('No stored data');
             $('#HarmonicsTime').text('No stored data');
             $('#FFTTime').text('No stored data');
           },
           complete: function (x) {
             setTimeout(AjaxGetDetails, 500);
           }
        });
}
$('#btnShowDetailVoltage').click(function() {
//  $('#btnShowDetailVoltage').addClass('active');
  $('#btnShowDetailHarmonics').removeClass('active');
  $('#btnShowDetailEvolution').removeClass('active');
  $('#btnShowDetailAdvanced').removeClass('active');
  $('#btnShowDetailEvents').removeClass('active');
  
  $('#btnShowDetail1Harmonic').addClass('disabled');
  $('#btnShowDetailVoltageAndCurrent').removeClass('disabled');
  $('#btnPauseDetail').removeClass('disabled');

  
  Cookies.set('VoltageDetailview', 'Waveform');
  $('#Tabs > div').addClass('hidden');
  $('#WaveformPanel').removeClass('hidden');
  $(WaveformChart.graphDiv).parent().removeAttr('style');
  window.dispatchEvent(new Event('resize'));
});
$('#btnShowDetailHarmonics').click(function() {
  $('#btnShowDetailVoltage').removeClass('active');
  $('#btnShowDetailEvolution').removeClass('active');
  $('#btnShowDetailAdvanced').removeClass('active');
  $('#btnShowDetailEvents').removeClass('active');
  
  Cookies.set('VoltageDetailview', 'Harmonics');
  $('#Tabs > div').addClass('hidden');
  $('#HarmonicsPanel').removeClass('hidden');
  $(HarmonicsChart.graphDiv).parent().removeAttr('style');
  $('#btnShowDetail1Harmonic').removeClass('disabled');
  $('#btnShowDetailVoltageAndCurrent').removeClass('disabled');
  window.dispatchEvent(new Event('resize'));
});
$('#btnShowDetailEvolution').click(function() {
  $('#btnShowDetailVoltage').removeClass('active');
  $('#btnShowDetailHarmonics').removeClass('active');
  $('#btnShowDetailAdvanced').removeClass('active');
  $('#btnShowDetailEvents').removeClass('active');
  
  Cookies.set('VoltageDetailview', 'Evolution');
  $('#Tabs > div').addClass('hidden');
  $('#EvolutionPanel').removeClass('hidden');
  $(HarmonicsChart.graphDiv).parent().removeAttr('style');
  $('#btnShowDetail1Harmonic').removeClass('disabled');
  $('#btnShowDetailVoltageAndCurrent').removeClass('disabled');
  window.dispatchEvent(new Event('resize'));
});
$('#btnShowDetailAdvanced').click(function() {
  $('#btnShowDetailVoltage').removeClass('active');
  $('#btnShowDetailEvolution').removeClass('active');
  $('#btnShowDetailAdvanced').removeClass('active');
  $('#btnShowDetailEvents').removeClass('active');
  
  Cookies.set('VoltageDetailview', 'Evolution');
  $('#Tabs > div').addClass('hidden');
  $('#FFTPanel').removeClass('hidden');
  $(FFTChart.graphDiv).parent().removeAttr('style');
  $('#btnShowDetail1Harmonic').removeClass('disabled');
  $('#btnShowDetailVoltageAndCurrent').removeClass('disabled');
  window.dispatchEvent(new Event('resize'));
});
/* Grafica de forma de onda */
var WaveformChart       = null;
var WaveformDataset     = [];
function UpdateWaveformChart(result) {
  if(result['wave_v']) {
    var date = new Date(result['Sampletime']);
    $('#WaveformTime').text('Last update at ' + date.toLocaleTimeString() + '.' + date.getMilliseconds());
    WaveformDataset = [];
    for(i = 0; i < result['wave_v'][0].length; i++) {
      WaveformDataset.push([i, result['wave_v'][0][i], 0]);
    }
  }
  var Max_Voltage = 1.0;
  var Max_Current = 0.1;
  for(i = 0; i < WaveformDataset.length; i++) {
    if(Math.abs(WaveformDataset[i][1]) > Max_Voltage) Max_Voltage = Math.abs(WaveformDataset[i][1]);
    if(Math.abs(WaveformDataset[i][2]) > Max_Current) Max_Current = Math.abs(WaveformDataset[i][2]);
  }
  var Options = { 
    strokeWidth: 1,
    labelsDiv: "WaveformLegend",  
    labels: ["Time", "Voltage", "Current"],
    colors: ["#d9534f", "#31708f"],
    gridLineColor: "#999",
    gridLinePattern: [4, 4],
    visibility: [true, ShowCurrent],
    showRangeSelector: true,
    rangeSelectorHeight: 20,
    rangeSelectorPlotFillColor: "#FFFFFF",
    rangeSelectorPlotStrokeColor: "#d9534f",
    xAxisHeight: 10,
    interactionModel: { 
      mousedown: mouseDownOZM,
      mousemove: mouseMoveOZM,
      mouseup  : mouseUpOZM,
      dblclick : dblClickOZM_Restore
    },
    series: {
      Current: {
        axis: 'y2'
      }
    },
    plugins: [
      new Dygraph.Plugins.Crosshair({
        direction: "vertical"
      })
    ],
    axes: {
      y: { pixelsPerLabel: 15,
           axisLabelWidth: 34,
           valueRange: [-Max_Voltage * 1.1, Max_Voltage * 1.1], 
           valueFormatter: function (x) {
             return Round2(x) + ' <em class="x-small">V</em>';
           },
           axisLabelFormatter: function (x) {
             return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
           }
         },
      y2: { pixelsPerLabel: 15,
            axisLabelWidth: ShowCurrent ? 34 : 0,
            valueRange: [-Max_Current * 1.1, Max_Current * 1.1], 
            valueFormatter: function (x) {
              return Round2(x) + ' <em class="x-small">A</em>';
            },
            axisLabelFormatter: function (x) {
              if(ShowCurrent == true) {
                return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
              } else {
                return '';
              }
            }
          },
      x: { ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             for(var i = 1; i <= Math.floor(max / 312.5); i++) {
               ticks.push({ v: i * 312.5, label: '<div style="font-size: 10px; margin-top: -2px; text-align: center">' + Round2((i * 312.5 * 1000) / Nodes['Devices'][Node]['SampleFreq']) + '</div>' });
             }
             return ticks;
           },
           valueFormatter: function (x) {
             return Round2((x * 1000) / Nodes['Devices'][Node]['SampleFreq']) + ' <em class="x-small">ms</em>';
           },
           axisLabelFormatter: function (x) {
             return '<div style="font-size: 10px; margin-top: -2px; line-height: 10px">' + Round2((x * 1000) / SampleRate) + '</div>';
           }
         }
      }
  };
  if(WaveformChart == null) {
    WaveformChart = new Dygraph(document.getElementById("WaveformChart"), WaveformDataset, Options);
  } else {
    WaveformChart.updateOptions(Options);
    WaveformChart.updateOptions({ file: WaveformDataset });
  }
  WaveformChart.updateOptions({ showRangeSelector: false});
  WaveformChart.updateOptions({ showRangeSelector: true});
}

/* Grafica de armonicos */
var HarmonicsChart      = null;
var HarmonicsDataset    = [];
var HarmonicsOffset     = 0;
function UpdateHarmonicsChart(result) {
  if(result['harmonics_v']) {
    var date = new Date(result['Sampletime']);
    $('#HarmonicsTime').text('Last update at ' + date.toLocaleTimeString() + '.' + date.getMilliseconds());
    HarmonicsDataset = [];
    for(i = 0; i < result['harmonics_v'][0].length; i++) {
    //  if(result.harmonics_v[i] < 0.000000001) result.harmonics_v[i] = 0.000000001;
      HarmonicsDataset.push([20.0 * Math.log10(result.harmonics_v[0][i]), result.harmonics_v[0][i]]);
    }
//    HarmonicsOffset = 0;
    for(i = 0; i < 50; i++) {
      if(-HarmonicsDataset[i][0] > 0) HarmonicsOffset = Math.max(HarmonicsOffset, -HarmonicsDataset[i][0]);
    }
    for(i = 0; i < 50; i++) HarmonicsDataset[i][0] = HarmonicsDataset[i][0] + HarmonicsOffset;
  }
  var toShow = [];
  for(i = (HideFundamentals ? 1 : 0); i < 50; i++) toShow.push([i, HarmonicsDataset[i][0]]);
  if(HarmonicsChart === null) {
    HarmonicsChart = new Dygraph(document.getElementById("HarmonicsChart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "HarmonicsLegend",  
        labels: ["Harmonic", "dB"],
        colors: ["#d9534f"],
        plotter: barChartPlotter,
        dateWindow: [(HideFundamentals ? 0 : -1), 50],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        interactionModel: { },
        axes: { 
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 20,
               ticker: function(min, max, pixels, opts, dygraph, vals) {
                         var ticks = [];
                         for(var i = Math.floor(min - HarmonicsOffset); i < Math.ceil(max - HarmonicsOffset); i++) {
                           if((i % 10) == 0) {
                             ticks.push({ v: i + HarmonicsOffset, label: '<div style="font-size: 10px; margin-top: -2px; text-align: right">' + i + '</div>' });
                           }
                         }
                         return ticks;
                       },
               valueFormatter: function (x, a, b, c, row, col) {
                 return Round2(x - HarmonicsOffset) + '<strong style="color: #d9534f">&nbsp;&nbsp;Voltage</strong>: ' + (Round2(HarmonicsDataset[row + (HideFundamentals ? 1 : 0)][1])) + '<em class="small"> V</em>';
               }
          },
          x: { drawGrid: false, 
               ticker: function (min, max, pixels, opts, dygraph, vals) {
                         var ticks = [];
                         for(var i = (HideFundamentals ? 1 : 0); i < 50; i++) {
                           if(i%2 != 0) continue;
                           ticks.push({v: i, label: '<div style="font-size: 10px; margin-top: -2px; text-align: center">' + HarmonicsNames[i] + '</div>' });
                         }
                         return ticks;
                       },
               valueFormatter: function (y) {
                                 return HarmonicsNames[y] + ' Harmonic (' + (50 * (y + 1)) + '<em class="small"> Hz</em>)';
                               }
          }
        }
      }
    );
  } else {
    HarmonicsChart.updateOptions({ file: toShow, dateWindow: [(HideFundamentals ? 0 : -1), 50] });
  }
  if(HarmonicsDataset.length > 0) {
    
    
  } else {
    $('#HarmonicsTime').text('No stored data');
    HarmonicsChart.updateOptions({ file: []});
  }
}

/* Grafica de FFT */
var FFTChart            = null;
var FFTDataset          = [];
var FFTOffset           = 0;
function UpdateFFTChart(result) {
  FFTDataset = [];
  if(result['fft_v']) {
    for(i = 0; i < result['fft_v'][0].length; i++) {
      if(result['fft_v'][0][i] < 0.000000001) result['fft_v'][0][i] = 0.000000001;
      FFTDataset.push([20.0 * Math.log10(result['fft_v'][0][i]), result['fft_v'][0][i]]);
    }
    FFTOffset = 0;
    for(i = 0; i < result['fft_v'][0].length; i++) {
      if(-FFTDataset[i][0] > FFTOffset) FFTOffset = -FFTDataset[i][0];
    }
    for(i = 0; i < result['fft_v'][0].length; i++) FFTDataset[i][0] = FFTDataset[i][0] + FFTOffset;
  }
  var toShow = [];
  for(i = 0; i < FFTDataset.length; i++) toShow.push([i, FFTDataset[i][0]]);
  if(FFTChart == null) {
    FFTChart = new Dygraph(document.getElementById("fft_chart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "FFTLegend",  
        labels: ["Frequency", "dB"],
        colors: ["#d9534f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        showRangeSelector: true,
        rangeSelectorHeight: 20,
        rangeSelectorPlotFillColor: "#d9534f",
        rangeSelectorPlotStrokeColor: "#d9534f",
        xAxisHeight: 10,
        fillGraph: true,
        interactionModel: { 
          mousedown: mouseDownOZM,
          mousemove: mouseMoveOZM,
          mouseup  : mouseUpOZM,
          dblclick : dblClickOZM_Restore  
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 20,
               ticker: function(min, max, pixels, opts, dygraph, vals) {
                         var ticks = [];
                         for(var i = Math.floor(min - FFTOffset); i < Math.ceil(max - FFTOffset); i++) {
                           if((i % 10) == 0) {
                             ticks.push({ v: i + FFTOffset, label: '<div style="font-size: 10px; margin-top: -2px; text-align: right">' + i + '</div>' })
                           }
                         }
                         return ticks;
                       },
               valueFormatter: function (x, a, b, c, row, col) {
                 return Round2(x - FFTOffset) + '<strong style="color: #d9534f">&nbsp;&nbsp;Voltage</strong>: ' + (Round2(FFTDataset[row][1])) + '<em class="small"> V</em>';
               },
             },
          x: { ticker: function(min, max, pixels, opts, dygraph, vals) {
                         var ticks = [];
                         for(var i = 1; i < max; i++) {
                           if((i % 10) == 0) {
                             ticks.push({ v: i, label: '' });
                           }
                           if((i % 50) == 0) {
                             ticks.push({ v: i, label: '<div style="font-size: 10px; margin-top: -2px; text-align: center">' + (i * 5) + '</div>' });
                           }
                         }
                         return ticks;
                       },
               valueFormatter: function (y) {
                                 return (5 * y) + ' <em class="small">Hz</em>';
                               },
             }
        }
      }
    );
  } else {
    FFTChart.updateOptions({ file: toShow });
  }
  if(FFTDataset.length > 0) {
    var date = new Date(result.fft_v.time);
    $('#FFTTime').text('Last update at ' + date.toLocaleTimeString() + '.' + date.getMilliseconds());
  } else {
    $('#FFTTime').text('No stored data');
    FFTChart.updateOptions({ file: []});
  }
}

var ShowCurrent         = false;
var Paused              = false;
var HideFundamentals    = false;




var EvolutionCharts     = [];
var LastEvolutionSample = 0;
var EvolutionDatasets   = [];
var DetailSpinner       = new Spinner(SpinnerOpts);
var HarmonicsNames      = [ '1st',  '2nd',  '3rd',  '4th',  '5th',  '6th',  '7th',  '8th',  '9th', '10th',
                            '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th',
                            '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th',
                            '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th',
                            '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th'];





function UpdateEvolutionChart(result) {
  for(i = 0; i < 50; i++) {
    if(EvolutionDatasets.length == i) EvolutionDatasets.push([]);
    for(j = 0; j < result.evolution_v.length; j++) {
      var Value = 20 * Math.log10(result['evolution_v'][j][i]);
      EvolutionDatasets[i].push([new Date(result.evolution_time[j] * 1000 + 1000), Value]);
      if(EvolutionDatasets[i].length > 720) EvolutionDatasets[i].shift();
    }
  }
  LastEvolutionSample = result['evolution_time'][result.evolution_time.length - 1];
  if(EvolutionCharts.length == 0) {
    for(i = 0; i < 50; i++) {
      var Str = '<div style="position: relative">' +
                '  <div id="HarmonicsEvolutionChart' + i + '" class="chart" style="height: 50px' + ((i == 0) ? '; margin-top: 16px; border-top: 1px solid #999"' : '"') + '></div>' +       
                '  <div style="pointer-events: none; position: absolute; top: -2px; text-align: left; left: 2px" id="HarmonicsEvolutionChartLegend' + i + '"></div>' +                    
                '</div>';
      $("#EvolutionPanel").append(Str);
      EvolutionCharts[i] = new Dygraph(document.getElementById("HarmonicsEvolutionChart" + i),
        EvolutionDatasets[i],
        { strokeWidth: 0.5,
          labels: ["Time", HarmonicsNames[i]],
          colors: ["#d9534f"],
          gridLineColor: "#999",
          gridLinePattern: [4, 4],
          highlightCircleSize: 0,
          interactionModel: { },
          labelsDiv: 'HarmonicsEvolutionChartLegend' + i,
          legend: 'always',
          legendFormatter: function(data) {
            if(data.x == null) {
              return '<span style="font-size: 18px; font-weight: bold">' + data.series[0].label + '</span>';
            } else {
              return '<span style="font-size: 15px; font-weight: bold">' + data.series[0].label + '</span><br/>' +
                      data.xHTML + data.series[0].yHTML;
            }
          },
          xAxisHeight: 0.1,
          xLabelHeight: 1,
          plotter: barChartPlotterCubism,
          dateWindow: [(LastEvolutionSample * 1000) - 43200000, LastEvolutionSample * 1000],
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
                 axisLabelFormatter: function(x, i) {
                   if(this.attributes_.labels_[0] == HarmonicsNames[0]) {
                     var date = new Date(x);
                     if((date.getHours() == 0) && (date.getMinutes() == 0) && (date.getSeconds() == 0)) {
                       return '<div style="font-size: 10px; margin-top: -63px; line-height: 10px; font-weight: bold">' + date.toLocaleDateString() + '</div>';
                     } else {
                       return '<div style="font-size: 10px; margin-top: -63px; line-height: 10px">' + date.toLocaleTimeString() + '</div>';                       
                     }
                   } else {
                     return null;
                   }
                 }
            }
          }
        }
      );
    }
    Dygraph.synchronize(EvolutionCharts, { range: false, zoom: false });
  } else {
    for(i = 0; i < 50; i++) {
      EvolutionCharts[i].updateOptions({ file: EvolutionDatasets[i], dateWindow: [(LastEvolutionSample * 1000) - 43200000, LastEvolutionSample * 1000] });
    }
  }
}

function AjaxSuccess60S(result) {
  if(result['evolution_time']) {
    UpdateEvolutionChart(result);
    $('#HarmonicEvolutionBtn').removeClass("disabled");
    $('#HarmonicEvolutionLoad').addClass("hidden");
  }
  $('#voltage_hour').text(Round2(result['hour_rms']));
  $('#voltage_last_day').text(Round2(result['day_rms']));
  $('#voltage_hour_min').text(Round2(result['hour_min']));
  $('#voltage_hour_max').text(Round2(result['hour_max']));
  $('#voltage_min_day').text(Round2(result['day_min']));
  $('#voltage_max_day').text(Round2(result['day_max']));
  UpdateHeader(result);
}
function GetValues60S(getHarmonics = true) {
  $.ajax({ url: "analysis/ajaxUpdateValues60sVoltage",
         timeout: 60000,
         type: "POST",
         dataType: "json",
         data: { lastevolutiontimestamp: LastEvolutionSample, harmonics: getHarmonics },
         success: AjaxSuccess60S,
         complete: function (x) {
                     setTimeout(GetValues60S, getHarmonics ? 60000 : 100);
                     DetailSpinner.stop();
                     $('#SpinnerDivDetail').remove();
                     $('#DetailPanel').css('opacity', '1.0');
                     $('#DetailPanel').find("*").prop("disabled", false);
                   }
      });
}
function TabChange() {
  $('#Tabs > div').addClass('hidden');
  if($(this).val() == 'events') {
    $('#EventsPanel').removeClass('hidden');
    $(eventChart.graphDiv).parent().removeAttr('style');
    $('#btnPause').removeClass('active');
    $('#btnPause').addClass('disabled');
    $('#btnFundamental').removeClass('active');
    $('#btnFundamental').addClass('disabled');
    $('#btnCurrent').removeClass('active');
    $('#btnCurrent').addClass('disabled');
    $('#btnExport').addClass('disabled');
  } else if($(this).val() == 'harmonics') {
    $('#HarmonicsPanel').removeClass('hidden');
    $(HarmonicsChart.graphDiv).parent().removeAttr('style');
    if(Paused) $('#btnPause').addClass('active');
    $('#btnPause').removeClass('disabled');
    if(HideFundamentals) $('#btnFundamental').addClass('active');
    $('#btnFundamental').removeClass('disabled');
    $('#btnCurrent').removeClass('active');
    $('#btnCurrent').addClass('disabled');
    $('#btnExport').removeClass('disabled');
  } else if($(this).val() == 'evolution') {
    $('#EvolutionPanel').removeClass('hidden');
    $('#EvolutionPanel').removeAttr('style');
    $('#btnPause').removeClass('active');
    $('#btnPause').addClass('disabled');
    $('#btnFundamental').removeClass('active');
    $('#btnFundamental').addClass('disabled');
    $('#btnCurrent').removeClass('active');
    $('#btnCurrent').addClass('disabled');
    $('#btnExport').removeClass('disabled');
  } else if($(this).val() == 'fft') {
    $('#FFTPanel').removeClass('hidden');
    $(FFTChart.graphDiv).parent().removeAttr('style');
    if(Paused) $('#btnPause').addClass('active');
    $('#btnPause').removeClass('disabled');
    $('#btnFundamental').removeClass('active');
    $('#btnFundamental').addClass('disabled');
    $('#btnCurrent').removeClass('active');
    $('#btnCurrent').addClass('disabled');
    $('#btnExport').removeClass('disabled');
  } 
  
    if(Paused) $('#btnPause').addClass('active');
    $('#btnPause').removeClass('disabled');
    $('#btnFundamental').removeClass('active');
    $('#btnFundamental').addClass('disabled');
    if(ShowCurrent) $('#btnCurrent').addClass('active');
    $('#btnCurrent').removeClass('disabled');
    $('#btnExport').removeClass('disabled');


  window.dispatchEvent(new Event('resize'));
}



$(':input[name="detail_change"]').change(TabChange);
$('#aggreg_modal_close').click(function() {
  $('#aggreg_modal').modal('hide');
});
$('#btnPause').change(function () {
  Paused = !Paused;
  if(Paused) {
    $('#btnPause span.live-pause').hide();
    $('#btnPause span.live-resume').show();
  } else {
    $('#btnPause span.live-pause').show();
    $('#btnPause span.live-resume').hide();
  }
});
$('#btnFundamental').click(function () {
  HideFundamentals = !HideFundamentals;
  UpdateHarmonicsChart([]);
});
$('#btnCurrent').click(function () {
  ShowCurrent = !ShowCurrent;
  UpdateWaveformChart([]);
});
$('#btnExport').click(function () {
  var Data = [];
  var Filename;
  if($('#WaveformPanel').hasClass('hidden') == false) {
    Filename = 'voltage_waveform_' + LastTimestamp + '.csv'
    Data.push([['Index', 'Voltage', 'Current']]);
    Data = Data.concat(WaveformDataset);
  } else if($('#FFTPanel').hasClass('hidden') == false) {
    Filename = 'voltage_fft_' + LastTimestamp + '.csv';
    Data = [['Index', 'dB', 'V_rms', 'Note: This is the result of our FFT algorithm. The index colum should be multiplied by 5 in order to get the frequencies.']];
    for(i = 0; i < FFTDataset.length; i++) Data.push([i, FFTDataset[i][0]- FFTOffset, FFTDataset[i][1]]);
  } else if($('#HarmonicsPanel').hasClass('hidden') == false) {
    Filename = 'voltage_harmonics_' + LastTimestamp + '.csv';
    Data = [['Order', 'dB', 'V_rms', ' Freq = ' + $('#frequency_indicator').text(), 'Note: This frequency is the result of our zero crossing algorithm for the last 10 seconds, as stated by IEC 61000-4-30.']];
    for(i = 0; i < HarmonicsDataset.length; i++) Data.push([(i + 1), HarmonicsDataset[i][0] - HarmonicsOffset, HarmonicsDataset[i][1]])  
  } else if($('#EvolutionPanel').hasClass('hidden') == false) {
    Filename = 'voltage_harmonics_evolution' + LastTimestamp + '.csv';
    Data = [['Date/Time']];
    Data[0] = Data[0].concat(HarmonicsNames);
    for(i = 0; i <  EvolutionDatasets[0].length; i++) {
      var Row = [EvolutionDatasets[0][i][0]];
      for(j = 0; j <  EvolutionDatasets.length; j++) {
        Row.push(EvolutionDatasets[j][i][1])
      }
      Data.push(Row);
    }
  }
  exportToCsv(Filename, Data);
  $(this).removeClass('active');
  $(this).blur();
});
$('#maximizeVoltage').click(function() {
  $('#GraphDialogTitle').text('Voltage');
  $('#GraphDialog').modal('show');
  PrevDialogContainer = $('#VoltageCol');
  DialogElement = $('#VoltageMaximizeDiv').detach();
  DialogPrevResizeHandler = VoltageResizeEvent;
  DialogResizeHandler = function() {
    VoltageChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
  };
  $(window).unbind('resize', VoltageResizeEvent);
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

//$(document).ajaxError(function (event, jqxhr, settings, exception) {
//  //if(jqxhr.status === 200) window.location.href = 'login';
//});
//$(document).ready(VoltageGetData(86400, 0, true));
//$(document).ready(GetValues60S(false));
//$(document).ready(GetValues5S());
















function fillEventsTable(events_list) {
    var table = $('table#events_table tbody');
    table.html('');
    var newEvents = [];
    for (var i in events_list) {
        //for (var i in events_list[type]) {
        newEvents.push(events_list[i]);
        var startTime = new Date(+events_list[i].timestamp);
        var duration = formatTimeUnit(events_list[i].duration * 1e6);
        var diff = roundTo((Math.abs(events_list[i].voltage - 230)), 2).toFixed(2);
        var percent = roundTo(100 - events_list[i].percent, 2).toFixed(2);
        table.append('<tr>\
        <td>' + startTime.toLocaleString() + '.' + startTime.getMilliseconds() + '</td>\
        <td>' + events_list[i].type + '</td>\
        <td>' + duration + '</td>\
        <td>' + diff + ' V' + '</td>\
        <td>' + percent + '%' + '</td>\
        <td><button class="btn btn-xs btn-primary get_samples"><i class="fa fa-line-chart fa-fw"></i> Get samples</button></td>\
        </tr>')
        //}
    }
    events = newEvents;
}
function generateEventTable(startDate, endDate) {
    var request = $.ajax({
        url: "analysis/ajaxGetEvents",
        timeout: 20000,
        type: "POST",
        dataType: "json",
        data: {
            startDate: startDate,
            endDate: endDate,
            getRVC: true
        }
    });
    request.done(function (result) {
        table_events.clear();
        table_events.destroy();
        fillEventsTable(result);
        table_events = $('table#events_table').DataTable(
                {
                    responsive: true,
                    "columnDefs": [
                        {width: '2%', "targets": -1}
                    ]
                });
        if ($('#event_chart div.no-data').length == 0) {
            $('#event_chart').append('<div class="no-data event_instructions"><div class="alert alert-info">Press <strong><em>Get samples</em></strong> to view the waveform of an event.</div></div>');
        }
    });
}


/**
 * Events
 */

$("table#events_table").on('click', 'button.get_samples', function () {
    var tr = table_events.row($(this).parents('tr'));
    $('#event_chart div.no-data').fadeOut(200, function () {
        $(this).remove();
    });
    table_events.rows().nodes().to$().removeClass('info');
    $(tr.node()).addClass('info');
    var index = tr.index();
    spinner.spin(eventChart.graphDiv);
    var request = $.ajax({
        url: "events/ajaxGetEventSamples",
        type: "POST",
        dataType: "json",
        data: {
            timestamp: +events[index].timestamp
        }
    });
    request.done(function (result) {
        spinner.stop();
        var eventDataset = [];
        var len = result.length;
        for (var i = 0; i < len / 2; i++) {
            eventDataset.push([i/*new Date(+result[i].timestamp)*/, result[i]/*.value*/]);
        }
        for (var j = 0; j < 100; j++) {
            eventDataset.push([i++, null]);
        }
        for (i = i; i < len; i++) {
            eventDataset.push([i/*new Date(+result[i].timestamp)*/, result[i - 100]/*.value*/]);
        }
        eventChart.updateOptions({'file': eventDataset, dateWindow: [0, result.length], underlayCallback: function (canvas, area, g) {
                var bottom_left = g.toDomCoords(len / 2, -20);
                var top_right = g.toDomCoords(len / 2 + 100, +20);

                var left = bottom_left[0];
                var right = top_right[0];

                canvas.fillStyle = "rgba(240, 240, 240, 1.0)";
                canvas.fillRect(left, area.y, right - left, area.h);
            }});
    });
});

var eventChart = new Dygraph(document.getElementById("event_chart"),
        [[0, 0]],
        {
            strokeWidth: 2,
            labels: ["Sample index", "Voltage"],
            colors: ["#d9534f"],
            ylabel: 'V',
            xlabel: 'Sample index',
            animatedZooms: true,
            interactionModel: {
//                'mousedown': mouseDownOZM,
//                'mousemove': mouseMoveOZM,
//                'mouseup': mouseUpOZM,
//                'click': clickV3,
//                'dblclick': dblClickV4,
//                'mousewheel': scrollV3
            },
            axes: {
                y: {
                    valueFormatter: function (x) {
                        return roundTo(x, 2).toFixed(2) + ' <em class="small">V</em>';
                    },
                    axisLabelFormatter: function (x) {
                        return '<small>' + roundTo(x, 2).toFixed(2) + ' <em class="small">V</em></small>';
                    }
                },
                x: {
                    valueFormatter: function (y) {
                        return y;
                    },
                    axisLabelFormatter: function (y) {
                        return y;
                    }
                }
            }
        }
);
//$('#event_chart').append('<div class="no-data event_instructions"><div class="alert alert-info">Press <strong><em>Get samples</em></strong> to view the waveform of an event.</div></div>');
//fillEventsTable(events);
//var table_events = $('table#events_table').DataTable(
//        {
//            responsive: true,
//            "columnDefs": [
//                {width: '2%', "targets": -1}
//            ],
//            "order": [0, 'desc']
//        }
//);

$('[data-toggle="popover"]').popover();

/* Secuencia de inicio para cargar la pagina */
function UpdatePage() {
  if(Nodes['Devices'][Node]['Type'] === '3Phase') {
    $('#btnShowVoltageRPhase').removeClass('disabled');
    $('#btnShowVoltageSPhase').removeClass('disabled');
    $('#btnShowVoltageTPhase').removeClass('disabled');
    $('#btnShowDetailRPhase').removeClass('disabled');
    $('#btnShowDetailSPhase').removeClass('disabled');
    $('#btnShowDetailTPhase').removeClass('disabled');
    var ShowR = (Cookies.get('ShowR') === 'true');
    var ShowS = (Cookies.get('ShowS') === 'true');
    var ShowT = (Cookies.get('ShowT') === 'true');
    if((ShowR !== true) && (ShowS !== true) && (ShowT !== true)) ShowR = true;
    Cookies.set('ShowR', (ShowR === true)); 
    Cookies.set('ShowS', (ShowS === true));
    Cookies.set('ShowT', (ShowT === true));
  }
  AjaxGetVoltage();
  AjaxGetVoltageCounters();
  AjaxGetDetails();
//  ajaxGetEvolution();
}