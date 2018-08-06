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
var VoltageChart     = null;
var VoltageData      = [];
var ScrollableSeries = "rms_v";
function Scrollable_ShowSpinner() {
  $('#VoltageSpinner').show();
}
function Scrollable_StopSpinner() {
  $('#VoltageSpinner').hide();
}
function Scrollable_OnSuccess(result) {
  if(ScrollableLastSample === 0) VoltageData = [Array(VoltageNames.length + 1).fill(0)];
  VoltageData = GraphDataAppend(result, 'rms_v', VoltageData);
  if(VoltageChart == null) {
    VoltageChart = new Dygraph(document.getElementById("VoltageChart"), VoltageData,
      { strokeWidth: 0.5, stepPlot: true, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4],
        legend: 'follow', legendFormatter: LegendFormatter_Follow, labels: ['Time'].concat(VoltageNames), dateWindow: [result['From'], result['To']],
        zoomCallback: Scrollable_Zoom, interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup: Scrollable_MouseUp, dblclick: Scrollable_DoubleClick },  
        colors: ColorArray($('#VoltagePanel').css('border-color'), 3),
        plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
        underlayCallback: function(canvas, area, g) {
          var NV = Analyzers[Analyzer].NominalVoltage, Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
          GraphHBackgroundRanges([[NV * 1.1, NV * 1.07, Yellow], [NV * 0.93, NV * 0.9, Yellow], [Infinity, NV * 1.1, Red], [NV * 0.9, -Infinity, Red]], g, canvas);
        },
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { return Round2(x) + ' <em class="x-small">V</em>'; },
               axisLabelFormatter: function (x) { return Round2(x); }
             },
          x: { valueFormatter: function (x) { return new Date(x).toLocaleDateString() + ' - ' + new Date(x).toLocaleTimeString(); },
               axisLabelFormatter: function (x) { return new Date(x).toLocaleTimeString() + '<br/>' + new Date(x).toLocaleDateString(); }
             }
          }
        }  
    );
    $(window).bind('resize', VoltageResizeEvent);
    window.dispatchEvent(new Event('resize'));
  } else {
    VoltageChart.updateOptions({file: VoltageData, dateWindow: [result['From'], result['To']]});
  }
  VoltageSeries();
}
function VoltageResizeEvent() {
  var Height = $('#VoltageAvgPanel').height() + $('#VoltageMaxPanel').height() + $('#VoltageMinPanel').height();
  VoltageChart.resize($('#VoltageCol').width(), Height - 4);
}
function VoltageSeries(event) {
  GraphPhaseButtons(VoltageChart, $('#groupVoltagePhases'), 'VisibleVoltageMainPhases', false, function(series) { return CalcRanges(VoltageData, series, false); }, (event !== undefined) ? event.target : undefined); 
}
$('#groupVoltagePhases').on('click', 'button', VoltageSeries);
$('#btnExportVoltage').click(function() {
  exportToCsv("Voltage_" + Node + '_' + LastVoltageSample + '.csv', [['Time'].concat(VoltageNames)].concat(VoltageData));
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

/* Contadores de voltage */
var VoltageCountersTimer  = null;
function AjaxGetVoltageCounters() {
  if(VoltageCountersTimer !== null) clearInterval(VoltageCountersTimer);
  VoltageCountersTimer = setTimeout(AjaxGetVoltageCounters, 60000);
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
           data: JSON.stringify({ Series: "avg(rms_v), max(rms_v), min(rms_v)", Aggreg: Aggreg, Period: Period, To: Current + Period, Analyzer: Analyzers[Analyzer].ID }),
           success: VoltageUpdateCountersCurrent
  });
  $.ajax({ url: "getSeriesAggreg",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: "avg(rms_v), max(rms_v), min(rms_v)", Aggreg: Aggreg, Period: Period, To: Current, Analyzer: Analyzers[Analyzer].ID }),
           success: VoltageUpdateCountersPrevs
  });
}
function VoltageUpdateCountersCurrent(result) {
  var CountersPeriod = localStorage.getItem('CountersPeriod');
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
    $('#labelVoltageCurrentAVG').html(Array_Round2(result['avg_rms_v'], '&nbsp;&nbsp;', ["#d9534f", "#534fd9", "#4fd953"]));
    $('#labelVoltageCurrentMAX').html(Array_Round2(result['max_rms_v'], '&nbsp;&nbsp;', ["#d9534f", "#534fd9", "#4fd953"]));
    $('#labelVoltageCurrentMIN').html(Array_Round2(result['min_rms_v'], '&nbsp;&nbsp;', ["#d9534f", "#534fd9", "#4fd953"]));
  }
}
function VoltageUpdateCountersPrevs(result) {
  var CountersPeriod = localStorage.getItem('CountersPeriod');
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
var DetailTimer   = null;
var DetailTimestamp = 0;
function AjaxGetDetails() {
//  if(WaveformChart === null) ShowSpinner(DetailSpinner, 'DetailSpinner');
  $('#btnPauseDetail').addClass('disabled');
  $.ajax({ url: "getSeriesNow",
           timeout: 60000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: 'wave_v, wave_i, harmonics_v, harmonics_i, fft_v, fft_i, power_factor, thd_v, phi, rms_v, rms_i', Analyzer: Analyzers[Analyzer].ID}),
           success: function(result) {
             if(WaveformChart === null) $('#DetailSpinner').hide();
             DetailTimestamp = result['Sampletime'];
             UpdateWaveformChart(result);
             UpdateHarmonicsChart(result);
             UpdateFFTChart(result);
             UpdatePhasorChart(result);
             $('#PowerFactorNow').html(Array_Round2(result['power_factor'], '&nbsp;&nbsp;', ["#d9534f", "#534fd9", "#4fd953"]));
             for(i = 0; i < result['thd_v'].length; i++) result['thd_v'][i] = result['thd_v'][i] * 100;
             $('#THDNow').html(Array_Round2(result['thd_v'], '&nbsp;&nbsp;', ["#d9534f", "#534fd9", "#4fd953"]));
             $('#PHINow').html(Array_Round2(result['phi'], '&nbsp;&nbsp;', ["#d9534f", "#534fd9", "#4fd953"], " (CAP)", " (IND)"));
           },
           error: function() {
             $('#WaveformTime').text('Node Offline');
             $('#HarmonicsTime').text('Node Offline');
             $('#FFTTime').text('Node Offline');
             $('#PhasorTime').text('Node Offline');
             $('#PowerFactorNow').text('Offline');
             $('#THDNow').text('Offline');
             $('#PHINow').text('Offline');
           },
           complete: function (x) {
             $('#btnPauseDetail').removeClass('disabled');
             DetailTimer = setTimeout(AjaxGetDetails, 3000);
           }
        });
}
var DetailPaused = false;
function DetailsButtonClick(event) {
  if(this.id === 'btnShowDetailWaveform') localStorage.setItem('VoltageDetailView', 'Waveform');
  if(this.id === 'btnShowDetailHarmonics') localStorage.setItem('VoltageDetailView', 'Harmonics');
  if(this.id === 'btnShowDetailEvolution') localStorage.setItem('VoltageDetailView', 'Evolution');
  if(this.id === 'btnShowDetailAdvanced') localStorage.setItem('VoltageDetailView', 'Advanced');
  if(this.id === 'btnShowDetailPhasors') localStorage.setItem('VoltageDetailView', 'Phasors');
  if(this.id === 'btnShowDetailParameters') localStorage.setItem('VoltageDetailView', 'Parameters');
  if(this.id === 'btnShowDetailEvents') localStorage.setItem('VoltageDetailView', 'Events');
  if(this.id === 'btnShowDetail1Harmonic') localStorage.setItem('VoltageDetailFirstHarmonic', (localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') ? 'false' : 'true');
  if(this.id === 'btnPauseDetail') {
    DetailPaused = !DetailPaused;
    if(DetailPaused) {
      $('#btnPauseDetail').addClass('active');
      if(DetailTimer !== null) clearInterval(DetailTimer);
    } else {
      $('#btnPauseDetail').removeClass('active');
      AjaxGetDetails();
    }
  }
  if(this.id === 'btnShowDetailRPhase') {
     if((localStorage.getItem('VoltageDetailView') === 'Evolution') || (localStorage.getItem('VoltageDetailView') === 'Parameters')) {
       localStorage.setItem('VoltageDetailEvolutionPhase', 'PhaseR');
     } else {
       localStorage.setItem('VoltageDetailPhaseR', (localStorage.getItem('VoltageDetailPhaseR') === 'true') ? 'false' : 'true');
     }
  }
  if(this.id === 'btnShowDetailSPhase') {
    if((localStorage.getItem('VoltageDetailView') === 'Evolution') || (localStorage.getItem('VoltageDetailView') === 'Parameters')) {
       localStorage.setItem('VoltageDetailEvolutionPhase', 'PhaseS');
     } else {
       localStorage.setItem('VoltageDetailPhaseS', (localStorage.getItem('VoltageDetailPhaseS') === 'true') ? 'false' : 'true');
     }
  }
  if(this.id === 'btnShowDetailTPhase') {
    if((localStorage.getItem('VoltageDetailView') === 'Evolution') || (localStorage.getItem('VoltageDetailView') === 'Parameters')) {
       localStorage.setItem('VoltageDetailEvolutionPhase', 'PhaseT');
     } else {
       localStorage.setItem('VoltageDetailPhaseT', (localStorage.getItem('VoltageDetailPhaseT') === 'true') ? 'false' : 'true');
     }
  }
  if(this.id === 'btnShowDetailVoltageAndCurrent') {
    localStorage.setItem('VoltageDetailShowCurrent', (localStorage.getItem('VoltageDetailShowCurrent') === 'true') ? 'false' : 'true');
    WaveformChart.updateOptions({ showRangeSelector: false});
    WaveformChart.updateOptions({ showRangeSelector: true});
    HarmonicsChart.updateOptions({ showRangeSelector: true});
    HarmonicsChart.updateOptions({ showRangeSelector: false});
    FFTChart.updateOptions({ showRangeSelector: false});
    FFTChart.updateOptions({ showRangeSelector: true});
  }
  $('#btnShowDetailWaveform').removeClass('active');
  $('#btnShowDetailHarmonics').removeClass('active');
  $('#btnShowDetailEvolution').removeClass('active');
  $('#btnShowDetailAdvanced').removeClass('active');
  $('#btnShowDetailPhasors').removeClass('active');
  $('#btnShowDetailParameters').removeClass('active');
  $('#btnShowDetailEvents').removeClass('active');
  switch(localStorage.getItem('VoltageDetailView')) {
    case 'Harmonics' :
      $('#btnShowDetailHarmonics').addClass('active');
      break;
    case 'Evolution' :
      $('#btnShowDetailEvolution').addClass('active');
      break;
    case 'Advanced' :
      $('#btnShowDetailAdvanced').addClass('active');
      break;
    case 'Events' :
      $('#btnShowDetailEvents').addClass('active');
      break;
    case 'Waveform' :
      $('#btnShowDetailWaveform').addClass('active');
      break;
    case 'Parameters' :
      $('#btnShowDetailParameters').addClass('active');
      break;
    case 'Phasors' :
      $('#btnShowDetailPhasors').addClass('active');
      break;
    default :
      $('#btnShowDetailWaveform').addClass('active');
      localStorage.setItem('VoltageDetailView', 'Waveform');
      break;
  }
  
  if((Analyzers[Analyzer]['Mode'] !== '3Phase') || (localStorage.getItem('VoltageDetailView') === 'Events')) {
    $('#btnShowDetailRPhase').addClass('disabled');
    $('#btnShowDetailSPhase').addClass('disabled');
    $('#btnShowDetailTPhase').addClass('disabled');
  } else {
    $('#btnShowDetailRPhase').removeClass('disabled');
    $('#btnShowDetailSPhase').removeClass('disabled');
    $('#btnShowDetailTPhase').removeClass('disabled');   
  }
  
  $('#btnShowDetailRPhase').removeClass('active');
  $('#btnShowDetailSPhase').removeClass('active');
  $('#btnShowDetailTPhase').removeClass('active');
  if(Analyzers[Analyzer]['Type'] === '3Phase') {
    switch(localStorage.getItem('VoltageDetailView')) {
      case 'Waveform' :
      case 'Harmonics' :
      case 'Phasors' :
      case 'Advanced' :
        var ShowR = (localStorage.getItem('VoltageDetailPhaseR') === 'true');
        var ShowS = (localStorage.getItem('VoltageDetailPhaseS') === 'true');
        var ShowT = (localStorage.getItem('VoltageDetailPhaseT') === 'true');
        if((ShowR === false) && (ShowS === false) && (ShowT === false)) {
          ShowR = true;
          localStorage.setItem('VoltageDetailPhaseR', 'true');
        }
        if(ShowR === true) $('#btnShowDetailRPhase').addClass('active');
        if(ShowS === true) $('#btnShowDetailSPhase').addClass('active');
        if(ShowT === true) $('#btnShowDetailTPhase').addClass('active');
        break;
      case 'Parameters' :
      case 'Evolution' :
        switch(localStorage.getItem('VoltageDetailEvolutionPhase')) {
          case 'PhaseR' :
            $('#btnShowDetailRPhase').addClass('active');
            break;
          case 'PhaseS' :
            $('#btnShowDetailSPhase').addClass('active');
            break;
          case 'PhaseT':
            $('#btnShowDetailTPhase').addClass('active');
            break;
          default:
            localStorage.setItem('VoltageDetailEvolutionPhase', 'PhaseR');
            $('#btnShowDetailRPhase').addClass('active');
            break;
        }
    }
  }
  $('#btnShowDetail1Harmonic').addClass('disabled');
  $('#btnShowDetail1Harmonic').removeClass('active');
  if(localStorage.getItem('VoltageDetailView') === 'Harmonics') {
    $('#btnShowDetail1Harmonic').removeClass('disabled');
    if(localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') {
      $('#btnShowDetail1Harmonic').addClass('active');
    } else {
      $('#btnShowDetail1Harmonic').removeClass('active');
    }
  }
  $('#btnShowDetailVoltageAndCurrent').addClass('disabled');
  $('#btnShowDetailVoltageAndCurrent').removeClass('active');
  switch(localStorage.getItem('VoltageDetailView')) {
    case 'Waveform' :
    case 'Harmonics' :
    case 'Advanced' :
      $('#btnShowDetailVoltageAndCurrent').removeClass('disabled');
      if(localStorage.getItem('VoltageDetailShowCurrent') === 'true') {
        $('#btnShowDetailVoltageAndCurrent').addClass('active');
      } else {
        $('#btnShowDetailVoltageAndCurrent').removeClass('active');
      }
      break;
  }
  $('#btnMaximizeDetail').addClass('disabled');
  switch(localStorage.getItem('VoltageDetailView')) {
    case 'Waveform' :
    case 'Harmonics' :
    case 'Advanced' :
      $('#btnMaximizeDetail').removeClass('disabled');
      break;
    case 'Events' :
      if(EventLoaded) $('#btnMaximizeDetail').removeClass('disabled');
      break;
  }
  $('#btnExportDetail').addClass('disabled');
    switch(localStorage.getItem('VoltageDetailView')) {
    case 'Waveform' :
    case 'Harmonics' :
    case 'Advanced' :
      $('#btnExportDetail').removeClass('disabled');
      break;
  }
  $('#Tabs > div').addClass('hidden');
  switch(localStorage.getItem('VoltageDetailView')) {
    case 'Waveform' :
      $('#WaveformPanel').removeClass('hidden');
      if(WaveformChart !== null) $(WaveformChart.graphDiv).parent().removeAttr('style');
      if(WaveformChart !== null) UpdateWaveformChart(null);
      break;
    case 'Harmonics' :
      $('#HarmonicsPanel').removeClass('hidden');
      if(HarmonicsChart !== null) $(HarmonicsChart.graphDiv).parent().removeAttr('style');
      if(HarmonicsChart !== null) UpdateHarmonicsChart(null);
      break;
    case 'Evolution' :
      $('#EvolutionPanel').removeClass('hidden');
      break;
    case 'Phasors' :
      $('#PhasorPanel').removeClass('hidden');
      if(PhasorChart !== null) $(PhasorChart.graphDiv).parent().attr('style', 'width: 404px; height: 394px');
      break;
    case 'Parameters' :
      $('#ParametersPanel').removeClass('hidden');
      if(PowerFactorChart !== null) UpdatePowerFactorChart(null);
      if(THDChart !== null) UpdateTHDChart(null);
      if(PHIChart !== null) UpdatePHIChart(null);
      break;
    case 'Advanced' :
      $('#FFTPanel').removeClass('hidden');
      if(FFTChart !== null) $(FFTChart.graphDiv).parent().removeAttr('style');
      if(FFTChart !== null) UpdateFFTChart(null);
      break;
    case 'Events' :
      $('#EventsPanel').removeClass('hidden');
      if(EventChart !== null) $(EventChart.graphDiv).parent().removeAttr('style');
      break;
  }
  window.dispatchEvent(new Event('resize'));
}
$('#btnShowDetailWaveform').click(DetailsButtonClick);
$('#btnShowDetailHarmonics').click(DetailsButtonClick);
$('#btnShowDetailEvolution').click(DetailsButtonClick);
$('#btnShowDetailPhasors').click(DetailsButtonClick);
$('#btnShowDetailParameters').click(DetailsButtonClick);
$('#btnShowDetailAdvanced').click(DetailsButtonClick);
$('#btnShowDetailAdvanced').click(DetailsButtonClick);
$('#btnShowDetailEvents').click(DetailsButtonClick);
$('#btnShowDetailRPhase').click(DetailsButtonClick);
$('#btnShowDetailSPhase').click(DetailsButtonClick);
$('#btnShowDetailTPhase').click(DetailsButtonClick);
$('#btnShowDetailVoltageAndCurrent').click(DetailsButtonClick);
$('#btnShowDetail1Harmonic').click(DetailsButtonClick);
$('#btnPauseDetail').click(DetailsButtonClick);

/* Grafica de forma de onda */
var WaveformChart       = null;
var WaveformDataset     = [];
function UpdateWaveformChart(result) {
  return;
  if(result !== null) {
    var date = new Date(result['Sampletime']);
    $('#WaveformTime').text('Last update at ' + date.toLocaleTimeString() + '.' + date.getMilliseconds());
    WaveformDataset = [];
    for(i = 0; i < result['wave_v'][0].length; i++) {
      if(Analyzers[Analyzer]['Type'] === '3Phase') {
        WaveformDataset.push([i, result['wave_v'][0][i], result['wave_v'][1][i], result['wave_v'][2][i], result['wave_i'][0][i], result['wave_i'][1][i], result['wave_i'][2][i]]);
      } else {
        WaveformDataset.push([i, result['wave_v'][0][i], result['wave_i'][0][i]]);      
      }
    }
  }
  var ShowR = (localStorage.getItem("VoltageDetailPhaseR") === 'true');
  var ShowS = (localStorage.getItem("VoltageDetailPhaseS") === 'true');
  var ShowT = (localStorage.getItem("VoltageDetailPhaseT") === 'true');
  var ShowCurrent = (localStorage.getItem("VoltageDetailShowCurrent") === 'true');
  var VoltageSeries = [];
  var CurrentSeries = [];
  var Visibility    = [];
  if(Analyzers[Analyzer]['Type'] === '3Phase') {
    if(ShowR) VoltageSeries.push(1);
    if(ShowS) VoltageSeries.push(2);
    if(ShowT) VoltageSeries.push(3);
    if(ShowR && ShowCurrent) CurrentSeries.push(4);
    if(ShowS && ShowCurrent) CurrentSeries.push(5);
    if(ShowT && ShowCurrent) CurrentSeries.push(6);
    Visibility = [ShowR, ShowS, ShowT, ShowR && ShowCurrent, ShowS && ShowCurrent, ShowT && ShowCurrent];
  } else {
    VoltageSeries.push(1);
    CurrentSeries.push(2);
    Visibility = [true, ShowCurrent];
  }
  var Max_Voltage = CalcRanges(WaveformDataset, VoltageSeries);
  var Max_Current = CalcRanges(WaveformDataset, CurrentSeries);
  var Options = { 
    strokeWidth: 1,
    labelsDiv: "WaveformLegend",  
    labels: (Analyzers[Analyzer]['Type'] === '3Phase') ? ['Time', 'Voltage (R)', 'Voltage (S)', 'Voltage (T)', 'Current (R)', 'Current (S)', 'Current (T)'] : ['Time', 'Voltage', 'Current'],
    colors: (Analyzers[Analyzer]['Type'] === '3Phase') ? ['#d9534f', '#534fd9', '#4fd953', '#428bca', '#42ca81', '#ca428b'] : ['#d9534f', '#428bca'],
    visibility: Visibility,
    gridLineColor: "#999",
    gridLinePattern: [4, 4],
    showRangeSelector: true,
    rangeSelectorHeight: 20,
    rangeSelectorPlotFillColor: "#FFFFFF",
    rangeSelectorPlotStrokeColor: "#d9534f",
    xAxisHeight: 10,
    interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
    series: (Nodes[Node]['Type'] === '3Phase') ? { 'Voltage (R)': { showInRangeSelector: true }, 'Current (R)': { axis: 'y2' }, 'Current (S)': { axis: 'y2' }, 'Current (T)': { axis: 'y2'  } } : { 'Voltage': { showInRangeSelector: true }, 'Current': { axis: 'y2' } },
    plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
    axes: {
      y: { pixelsPerLabel: 15,
           axisLabelWidth: 34,
           valueRange: Max_Voltage, 
           valueFormatter: function (x) {
             return Round2(x) + ' <em class="x-small">V</em>';
           },
           axisLabelFormatter: function (x) {
             return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
           }
         },
      y2: { pixelsPerLabel: 15,
            axisLabelWidth: ShowCurrent ? 34 : 0,
            valueRange: Max_Current, 
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
             for(var i = 1; i <= Math.floor(max / Nodes[Node]['SampleFreq'] * Nodes['NominalFreq']); i++) {
               ticks.push({ v: i * Nodes[Node]['SampleFreq'] / Nodes['NominalFreq'], label: '<div style="font-size: 10px; margin-top: -2px; text-align: center">' + Round2(i / Nodes['NominalFreq'] * 1000) + '</div>' });
             }
             return ticks;
           },
           valueFormatter: function (x) {
             return Round2((x * 1000) / Nodes[Node]['SampleFreq']) + ' <em class="x-small">ms</em>';
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
      if(Analyzers[Analyzer]['Mode'] === '3Phase') {
        HarmonicsDataset.push([result['harmonics_v'][0][i], result['harmonics_v'][1][i], result['harmonics_v'][2][i], result['harmonics_i'][0][i], result['harmonics_i'][1][i], result['harmonics_i'][2][i]]);
      } else {
        HarmonicsDataset.push([result['harmonics_v'][0][i], result['harmonics_i'][0][i]]);      
      }
    }
  }
  var ShowR = (localStorage.getItem("VoltageDetailPhaseR") === 'true');
  var ShowS = (localStorage.getItem("VoltageDetailPhaseS") === 'true');
  var ShowT = (localStorage.getItem("VoltageDetailPhaseT") === 'true');
  var ShowCurrent = (localStorage.getItem("VoltageDetailShowCurrent") === 'true');
  var VoltageSeries = [];
  var CurrentSeries = [];
  var Visibility    = [];
  if(Analyzers[Analyzer]['Mode'] === '3Phase') {
    if(ShowR) VoltageSeries.push(0);
    if(ShowS) VoltageSeries.push(1);
    if(ShowT) VoltageSeries.push(2);
    if(ShowR && ShowCurrent) CurrentSeries.push(3);
    if(ShowS && ShowCurrent) CurrentSeries.push(4);
    if(ShowT && ShowCurrent) CurrentSeries.push(5);
    Visibility = [ShowR, ShowS, ShowT, ShowR && ShowCurrent, ShowS && ShowCurrent, ShowT && ShowCurrent];
  } else {
    VoltageSeries.push(0);
    CurrentSeries.push(1);
    Visibility = [true, ShowCurrent];
  }
  var ToShow = [];
  for(i = (localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') ? 0 : 1; i < HarmonicsDataset.length; i++) {
    ToShow.push([]);
    var Pos = (localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') ? i : i - 1
    for(n = 0; n < HarmonicsDataset[i].length; n++) ToShow[Pos][n] = 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][n]));
  }
  var VoltageRange = CalcRanges(ToShow, VoltageSeries);
  var CurrentRange = CalcRanges(ToShow, CurrentSeries);
  HarmonicsVOffset = (VoltageRange[0] < 0.0) ? -VoltageRange[0] : 0.0;
  HarmonicsCOffset = (CurrentRange[0] < 0.0) ? -CurrentRange[0] : 0.0;
  for(i = 0; i < ToShow.length; i++) {
    if(Analyzers[Analyzer]['Mode'] === '3Phase') {
      ToShow[i] = [i, ToShow[i][0] + HarmonicsVOffset, ToShow[i][1] + HarmonicsVOffset, ToShow[i][2] + HarmonicsVOffset, ToShow[i][3] + HarmonicsCOffset, ToShow[i][4] + HarmonicsCOffset, ToShow[i][5] + HarmonicsCOffset];
    } else {
      ToShow[i] = [i, ToShow[i][0] + HarmonicsVOffset, ToShow[i][1] + HarmonicsCOffset];
    }
  }
  var Options = { 
    strokeWidth: 1,
    labelsDiv: "HarmonicsLegend",  
    labels: (Analyzers[Analyzer]['Mode'] === '3Phase') ? ['Time', 'Voltage (R)', 'Voltage (S)', 'Voltage (T)', 'Current (R)', 'Current (S)', 'Current (T)'] : ['Time', 'Voltage', 'Current'],
    colors: (Analyzers[Analyzer]['Mode'] === '3Phase') ? ['#d9534f', '#534fd9', '#4fd953', '#428bca', '#42ca81', '#ca428b'] : ['#d9534f', '#428bca'],
    visibility: Visibility,  
    plotter: multiColumnBarPlotter,
    dateWindow: [-0.5, (localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') ? 49.5 : 48.5],
    gridLineColor: "#999",
    gridLinePattern: [4, 4],
    interactionModel: { },
    series: (Analyzers[Analyzer]['Mode'] === '3Phase') ? { 'Current (R)': { axis: 'y2' }, 'Current (S)': { axis: 'y2' }, 'Current (T)': { axis: 'y2'  } } : { 'Current': { axis: 'y2' } },
    axes: { 
      y: { pixelsPerLabel: 15,
           axisLabelWidth: 20,
           valueRange: [VoltageRange[0] + HarmonicsVOffset, VoltageRange[1] + HarmonicsVOffset], 
           ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             for(var i = Math.floor(min - HarmonicsVOffset); i < Math.ceil(max - HarmonicsVOffset); i++) {
               if((i % 10) == 0) ticks.push({ v: i + HarmonicsVOffset, label: '<div style="font-size: 10px; margin-top: -2px; text-align: right">' + i + '</div>' });
             }
             return ticks;
           },
           valueFormatter: function (x, a, b, c, row, col) {
             var Row = ((localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') ? 0 : 1) + row;
             return Round2(x - HarmonicsVOffset) + '&nbsp;<em class="small">dB</em>&nbsp;(' + (Round2(HarmonicsDataset[Row][col - 1])) + '<em class="small">&nbsp;V</em>)';
           }
         },
      y2: { pixelsPerLabel: 15,
            axisLabelWidth: ShowCurrent ? 15 : 0,
            valueRange: [CurrentRange[0] + HarmonicsCOffset, CurrentRange[1] + HarmonicsCOffset],
            valueFormatter: function (x, a, b, c, row, col) {
              var Row = ((localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') ? 0 : 1) + row;
              return Round2(x - HarmonicsCOffset) + '&nbsp;<em class="small">dB</em>&nbsp;(' + (Round2(HarmonicsDataset[Row][col - 1])) + '<em class="small">&nbsp;A</em>)';
            },
            ticker: function(min, max, pixels, opts, dygraph, vals) {
              var ticks = [];
              if(ShowCurrent == true) {
                for(var i = Math.floor(min - HarmonicsCOffset); i < Math.ceil(max - HarmonicsCOffset); i++) {
                  if((i % 10) == 0) ticks.push({ v: i + HarmonicsCOffset, label: '<div style="font-size: 10px; margin-top: -2px; text-align: right">' + i + '</div>' });
                }
              }
              return ticks;
            }
          },
      x: { drawGrid: false, 
           axisLabelFormatter: function (x) {
             if(localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') return '<div style="font-size: 10px; margin-top: -2px">' + HarmonicsNames[x] + '</div>';
             return '<div style="font-size: 10px; margin-top: -2px">' + HarmonicsNames[x + 1] + '</div>';
           },
           valueFormatter: function (y) {
             if(localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') return HarmonicsNames[y] + ' Harmonic (' + (50 * (y + 1)) + '<em class="small"> Hz</em>)';
             return HarmonicsNames[y+1] + ' Harmonic (' + (50 * (y + 1)) + '<em class="small"> Hz</em>)';
           },
           ticker: function(min, max, pixels, opts, dygraph, vals) {
              var ticks = [];
              for(var i = 0; i < HarmonicsNames.length; i++) {
                if(localStorage.getItem('VoltageDetailFirstHarmonic') === 'true') {
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
  return;
  if(result !== null) {
    var date = new Date(result['Sampletime']);
    $('#FFTTime').text('Last update at ' + date.toLocaleTimeString() + '.' + date.getMilliseconds());
    FFTDataset = [];
    for(i = 0; i < result['fft_v'][0].length; i++) {
      if(Nodes[Node]['Type'] === '3Phase') {
        FFTDataset.push([result['fft_v'][0][i], result['fft_v'][1][i], result['fft_v'][2][i], result['fft_i'][0][i], result['fft_i'][1][i], result['fft_i'][2][i]]);
      } else {
        FFTDataset.push([result['fft_v'][0][i], result['fft_i'][0][i]]);
      }
    }
  }
  var ShowR = (localStorage.getItem("VoltageDetailPhaseR") === 'true');
  var ShowS = (localStorage.getItem("VoltageDetailPhaseS") === 'true');
  var ShowT = (localStorage.getItem("VoltageDetailPhaseT") === 'true');
  var ShowCurrent = (localStorage.getItem("VoltageDetailShowCurrent") === 'true');
  var VoltageSeries = [];
  var CurrentSeries = [];
  var Visibility    = [];
  if(Nodes[Node]['Type'] === '3Phase') {
    if(ShowR) VoltageSeries.push(0);
    if(ShowS) VoltageSeries.push(1);
    if(ShowT) VoltageSeries.push(2);
    if(ShowR && ShowCurrent) CurrentSeries.push(3);
    if(ShowS && ShowCurrent) CurrentSeries.push(4);
    if(ShowT && ShowCurrent) CurrentSeries.push(5);
    Visibility = [ShowR, ShowS, ShowT, ShowR && ShowCurrent, ShowS && ShowCurrent, ShowT && ShowCurrent];
  } else {
    VoltageSeries.push(0);
    CurrentSeries.push(1);
    Visibility = [true, ShowCurrent];
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
    if(Nodes[Node]['Type'] === '3Phase') {
      ToShow[i] = [i, ToShow[i][0] + FFTVOffset, ToShow[i][1] + FFTVOffset, ToShow[i][2] + FFTVOffset, ToShow[i][3] + FFTCOffset, ToShow[i][4] + FFTCOffset, ToShow[i][5] + FFTCOffset];
    } else {
      ToShow[i] = [i, ToShow[i][0] + FFTVOffset, ToShow[i][1] + FFTCOffset];
    }
  }
  var Options = { 
    strokeWidth: 1,
    labelsDiv: "FFTLegend",  
    labels: (Nodes[Node]['Type'] === '3Phase') ? ['Time', 'Voltage (R)', 'Voltage (S)', 'Voltage (T)', 'Current (R)', 'Current (S)', 'Current (T)'] : ['Time', 'Voltage', 'Current'],
    colors: (Nodes[Node]['Type'] === '3Phase') ? ['#d9534f', '#534fd9', '#4fd953', '#428bca', '#42ca81', '#ca428b'] : ['#d9534f', '#428bca'],
    visibility: Visibility,
    fillGraph: true,
    gridLineColor: "#999",
    gridLinePattern: [4, 4],
    showRangeSelector: true,
    rangeSelectorHeight: 20,
    rangeSelectorPlotFillColor: "#FFFFFF",
    rangeSelectorPlotStrokeColor: "#d9534f",
    interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
    series: (Nodes[Node]['Type'] === '3Phase') ? { 'Voltage (R)': { showInRangeSelector: true }, 'Current (R)': { axis: 'y2' }, 'Current (S)': { axis: 'y2' }, 'Current (T)': { axis: 'y2'  } } : { 'Current': { axis: 'y2' } },
    plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
    axes: { 
      y: { pixelsPerLabel: 15,
           axisLabelWidth: 20,
           valueRange: [VoltageRange[0] + FFTVOffset, VoltageRange[1] + FFTVOffset], 
           ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             for(var i = Math.floor(min - FFTVOffset); i < Math.ceil(max - FFTVOffset); i++) {
               if((i % 10) == 0) ticks.push({ v: i + FFTVOffset, label: '<div style="font-size: 10px; margin-top: -2px; text-align: right">' + i + '</div>' });
             }
             return ticks;
           },
           valueFormatter: function (x, a, b, c, row, col) {
             return Round2(x - FFTVOffset) + '&nbsp;<em class="small">dB</em>&nbsp;(' + (Round2(FFTDataset[row][col - 1])) + '<em class="small">&nbsp;V</em>)';
           }
         },
      y2: { pixelsPerLabel: 15,
            axisLabelWidth: ShowCurrent ? 15 : 0,
            valueRange: [CurrentRange[0] + FFTCOffset, CurrentRange[1] + FFTCOffset],
            valueFormatter: function (x, a, b, c, row, col) {
              return Round2(x - FFTCOffset) + '&nbsp;<em class="small">dB</em>&nbsp;(' + (Round2(FFTDataset[row][col - 1])) + '<em class="small">&nbsp;A</em>)';
            },
            ticker: function(min, max, pixels, opts, dygraph, vals) {
              var ticks = [];
              if(ShowCurrent == true) {
                for(var i = Math.floor(min - FFTCOffset); i < Math.ceil(max - FFTCOffset); i++) {
                  if((i % 10) == 0) ticks.push({ v: i + FFTCOffset, label: '<div style="font-size: 10px; margin-top: -2px; text-align: right">' + i + '</div>' });
                }
              }
              return ticks;
            }
          },
      x: { drawGrid: true,
           axisLabelFormatter: function (y) {
             y = (y / 10 * Nodes['NominalFreq']);
             if(y >= 1000) {
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

/* Grafico de fasores */
var PhasorChart = null;
var PhasorTable = null;
function UpdatePhasorChart(result) {
  var date = new Date(result['Sampletime']);
  $('#PhasorTime').text('Last update at ' + date.toLocaleTimeString() + '.' + date.getMilliseconds());
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
    strokeWidth: 0.5,
    labels: ['Index', 'Phase', 'Magnitude'],
    colors: ['#d9534f', '#428bca'],
    gridLineColor: "#999",
    gridLinePattern: [4, 4],
    highlightCircleSize: 0,
    interactionModel: { },
    legend: 'never',
    valueRange: [-115, 115],
    dateWindow: [-115, 115],
    plugins: [ new Dygraph.Plugins.PolarCrosshair( { } ) ],
    plotter: polarLinePlotter,
    xAxisHeight: 0.1,
    xLabelHeight: 0.1,
    axes: {
      y: { drawAxis: true,
           drawGrid:false,
           axisLabelWidth: 0,
           axisLineColor: "#FFF",
           ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             var Width = document.getElementById("PhasorChart").clientWidth / 2 - 1;
             for(var i = 1; i <= 4; i++) {
               ticks.push({ v: i * 25, label: '<div style="font-size: 11px; background: rgba(255, 255, 255, 0.8); margin-left: ' + Width + 'px; text-align: left; width: 44px; line-height: 12px">' + Round2(i / 4 * MaxVoltage) + '<em>V</em></div>' });
             }
             return ticks;
           },            
         },
      x: { drawAxis: true,
           drawGrid:false,
           axisLabelWidth: 0,
           axisLineColor: "#FFF",
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
    if(Analyzers[Analyzer]['Mode'] === '3Phase') {
      if(i === 0) PhaseName = "R";
      if(i === 1) PhaseName = "S";
      if(i === 2) PhaseName = "T";
    }
    PhasorTable.row.add( [PhaseName, "Voltage", Round2(i * 120) + 'º', Round2(result['rms_v'][i]) + '<em> V</em>'] ).draw(false);
    PhasorTable.row.add( [PhaseName, "Current", Round2(i * 120 + -result['phi'][i]) + 'º', Round2(result['rms_i'][i]) + '<em> A</em>'] ).draw();
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
           data: JSON.stringify({ Period: 86400000, To: 0, Aggreg: '1m', LastSample: LastEvolutionSample, Series: 'power_factor, thd_v, phi, harmonics_v', Analyzer: Analyzers[Analyzer].ID}),
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
  var Visibility = (Analyzers[Analyzer]['Mode'] === '3Phase') ? [localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseR', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseS', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseT'] : [true];
  if(PowerFactorChart === null) {
    PowerFactorChart = new Dygraph(document.getElementById("PowerFactorChart"),
      PowerFactorDataset,
      { strokeWidth: 0.5,
        labels: (Analyzers[Analyzer]['Mode'] === '3Phase') ? ['Time', 'Power Factor (R)', 'Power Factor (S)', 'Power Factor (T)'] : ['Time', 'Power Factor'],
        colors: (Analyzers[Analyzer]['Mode'] === '3Phase') ? ['#d9534f', '#d9534f', '#d9534f'] : ['#d9534f'],
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
    for(i = 0; i < result['thd_v'].length; i++) {
      THDDataset.push([new Date(result['Sampletime'][i])].concat(result['thd_v'][i]));
    }
    while(THDDataset.length > result['MaxSamples']) THDDataset.shift();
  }
  var Visibility = (Analyzers[Analyzer]['Mode'] === '3Phase') ? [localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseR', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseS', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseT'] : [true];
  if(THDChart === null) {
    THDChart = new Dygraph(document.getElementById("THDChart"),
      THDDataset,
      { strokeWidth: 0.5,
        labels: (Analyzers[Analyzer]['Mode'] === '3Phase') ? ['Time', 'THD (R)', 'THD (S)', 'THD (T)'] : ['Time', 'THD'],
        colors: (Analyzers[Analyzer]['Mode'] === '3Phase') ? ['#d9534f', '#d9534f', '#d9534f'] : ['#d9534f'],
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
  var Visibility = (Analyzers[Analyzer]['Mode'] === '3Phase') ? [localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseR', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseS', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseT'] : [true];
  if(PHIChart === null) {
    PHIChart = new Dygraph(document.getElementById("PHIChart"),
      PHIDataset,
      { strokeWidth: 0.5,
        labels: (Analyzers[Analyzer]['Mode'] === '3Phase') ? ['Time', 'PHI (R)', 'PHI (S)', 'PHI (T)'] : ['Time', 'PHI'],
        colors: (Analyzers[Analyzer]['Mode'] === '3Phase') ? ['#d9534f', '#d9534f', '#d9534f'] : ['#d9534f'],
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
      for(i = 0; i < result['harmonics_v'].length; i++) {
        var Row = [new Date(result['Sampletime'][i])];
        for(j = 0; j < result['harmonics_v'][i].length; j++) Row.push(20 * Math.log10(result['harmonics_v'][i][j][k]));
        HarmonicsDatasets[k].push(Row);
      }
      while(HarmonicsDatasets[k].length > result['MaxSamples']) HarmonicsDatasets[k].shift();
    }
  }
  var Visibility = (Analyzers[Analyzer]['Mode'] === '3Phase') ? [localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseR', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseS', localStorage.getItem('VoltageDetailEvolutionPhase') === 'PhaseT'] : [true];
  if(HarmonicsCharts.length == 0) {
    for(i = 0; i < 50; i++) {
      HarmonicsCharts[i] = new Dygraph(document.getElementById("HarmonicsEvolutionChart_" + i),
        HarmonicsDatasets[i],
        { strokeWidth: 0.5,
          labels: (Analyzers[Analyzer]['Mode'] === '3Phase') ? ['Time', HarmonicsNames[i] + ' (R)', HarmonicsNames[i] + ' (S)', HarmonicsNames[i] + ' (T)'] : ['Time', HarmonicsNames[i]],
          colors: (Analyzers[Analyzer]['Mode'] === '3Phase') ? ['#d9534f', '#d9534f', '#d9534f'] : ['#d9534f'],
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

/* Tabla de eventos */
var EventLoaded        = false;
var LastEventStartTime = 0;
var EventsDatasheet    = [];
var EventsTable        = null;
var EventChart         = null;
var EventSpinner       = new Spinner(SpinnerOpts);
function AjaxGetEvents(period, to, node) {
  $.ajax({ url: "getEvents",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: period, To: to, LastEvent: LastEventStartTime, Analyzer: Analyzers[Analyzer].ID}),
           success: AjaxEvents,
           complete: function() { 
             $('#btnVoltageNow').removeClass('disabled');
             if(parseInt(localStorage.getItem('GraphEnd')) === 0) VoltageTimer = setTimeout(AjaxGetVoltage, 3000);
           }
    });
}
function AjaxEvents(result) {
  if(EventsTable === null) {
    EventsTable = $('#EventsTable').DataTable({ responsive: false, paging: false, scrollCollapse: true, scrollY: '224px', dom: '<"top float-left"i><"top float-right"f><"clear">t<"clear">', language: { info: "_MAX_ events in selected period", infoEmpty: "No events in selection"},
                                                columnDefs: [ { targets: [5], orderable: false }, { targets: [0, 1, 2, 3, 4], searchable: true } ],
                                                order: [0, 'desc']
    });
  }
  if(LastEventStartTime === 0) {
    EventsTable.clear().draw();
    EventsDatasheet = [];
  }
  if(result['Starttime'] !== null) {
    for(i = 0; i < result['Starttime'].length; i++) {
      var Time = new Date(result['Starttime'][i]);
      var RowText = $('<tr>').append('<td data-search="' + result['Starttime'][i] + '">' + Time.toLocaleString() + '.' + Time.getMilliseconds() + '</td><td>' + result['Type'][i] + '</td><td>' + Round2(result['Duration'][i] / Nodes[Node]['SampleFreq']) + '</td><td>' + Round2(result['Voltage'][i]) + '</td><td>' + Round2(result['Percentage'][i]) + '</td><td><button class="btn btn-xs btn-primary get_samples"><i class="fa fa-line-chart fa-fw"></i> Get samples</button></td>');
      EventsTable.row.add(RowText).draw();
      LastEventStartTime = result['Starttime'][i];
      EventsDatasheet.push(result['Starttime'][i]);
    }
  }
  if(EventChart === null) {
    EventChart = new Dygraph(document.getElementById("EventChart"), (Nodes[Node]['Type'] == '3Phase') ? [[0, 0, 0, 0]] : [[0, 0]], {
      strokeWidth: 0.5,
      gridLineColor: "#999",
      gridLinePattern: [4, 4],
      labels: (Nodes[Node]['Type'] == '3Phase') ? ['Time', 'Voltage (R)', 'Voltage (S)', 'Voltage (T)'] :['Time', 'Voltage'],
      colors: ["#d9534f", "#534fd9", "#4fd953"],
      interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
      labelsDiv: "EventChartLegend"
    });
    $('#EventChart').append('<div class="no-data event_instructions"><div class="alert alert-info">Press <strong><em>Get samples</em></strong> to view the waveform of an event.</div></div>');
  }
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

/* Exportar y ampliar graficas de detalles */
$('#btnExportDetail').click(function () {
  var Data = [];
  var Filename;
  if($('#WaveformPanel').hasClass('hidden') === false) {
    exportToCsv("Voltage_Waveform_" + Node + '_' + DetailTimestamp + '.csv', ((Nodes[Node]['Type'] === '3Phase') ? [['DateTime', 'Voltage R', 'Voltage S', 'Voltage T', 'Current R', 'Current S', 'Current T']] : [['DateTime', 'Voltage', 'Current']]).concat(WaveformDataset));
  } else if($('#FFTPanel').hasClass('hidden') === false) {
    Data = [['Index'].concat(((Nodes[Node]['Type'] === '3Phase') ? ['dB (R)', 'V_rms (R)', 'dB (S)', 'V_rms (S)', 'dB (T)', 'V_rms (T)', 'dB (R)', 'I_rms (R)', 'dB (S)', 'I_rms (S)', 'dB (T)', 'I_rms (T)'] : ['dB', 'V_rms', 'dB', 'I_rms'])).concat('Note: This is the result of our FFT algorithm. The index colum should be multiplied by 5 in order to get the frequencies.')];
    for(i = 0; i < FFTDataset.length; i++) {
      if(Nodes[Node]['Type'] === '3Phase') {    
        Data.push([i, 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][0])), FFTDataset[i][0], 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][1])), FFTDataset[i][1],
                      20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][2])), FFTDataset[i][2], 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][3])), FFTDataset[i][3],
                      20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][4])), FFTDataset[i][4], 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][5])), FFTDataset[i][5]]);
      } else {
        Data.push([i, 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][0])), FFTDataset[i][0], 20.0 * Math.log10(Math.max(0.000000001, FFTDataset[i][1])), FFTDataset[i][1]]);
      }
    }
    exportToCsv("Voltage_FFT_" + Node + '_' + DetailTimestamp + '.csv', Data);
  } else if($('#HarmonicsPanel').hasClass('hidden') == false) {
    Data = [['Harmonic'].concat(((Nodes[Node]['Type'] === '3Phase') ? ['dB (R)', 'V_rms (R)', 'dB (S)', 'V_rms (S)', 'dB (T)', 'V_rms (T)', 'dB (R)', 'I_rms (R)', 'dB (S)', 'I_rms (S)', 'dB (T)', 'I_rms (T)'] : ['dB', 'V_rms', 'dB', 'I_rms'])).concat('Note: This is the result of our FFT algorithm.')];
    for(i = 0; i < HarmonicsDataset.length; i++) {
      if(Nodes[Node]['Type'] === '3Phase') {
        Data.push([HarmonicsNames[i], 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][0])), HarmonicsDataset[i][0], 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][1])), HarmonicsDataset[i][1],
                                      20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][2])), HarmonicsDataset[i][2], 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][3])), HarmonicsDataset[i][3],
                                      20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][4])), HarmonicsDataset[i][4], 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][5])), HarmonicsDataset[i][5]]);
      } else {
        Data.push([HarmonicsNames[i], 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][0])), HarmonicsDataset[i][0], 20.0 * Math.log10(Math.max(0.000000001, HarmonicsDataset[i][1])), HarmonicsDataset[i][1]]);
      }
    }
    exportToCsv("Voltage_Harmonics_" + Node + '_' + DetailTimestamp + '.csv', Data);
  } else if($('#EvolutionPanel').hasClass('hidden') == false) {
    Filename = 'Voltage_Harmonics_Evolution' + LastTimestamp + '.csv';
    Data = [['Date/Time']];
    Data[0] = Data[0].concat(HarmonicsNames);
    for(i = 0; i <  EvolutionDatasets[0].length; i++) {
      var Row = [EvolutionDatasets[0][i][0]];
      for(j = 0; j <  EvolutionDatasets.length; j++) {
        Row.push(EvolutionDatasets[j][i][1])
      }
      Data.push(Row);
    }
    exportToCsv("Voltage_Harmonics_Evolution" + ((Nodes[Node]['Type'] === '3Phase') ? '(' + localStorage.getItem('VoltageDetailEvolutionPhase') + ')_' : '_') + Node + '_' + DetailTimestamp + '.csv', Data);  
  }
});
$('#btnMaximizeDetail').click(function() {
  if(localStorage.getItem('VoltageDetailView') === 'Waveform') {
    $('#GraphDialogTitle').text('Waveform');
    PrevDialogContainer = $('#WaveformPanel');
  } else if(localStorage.getItem('VoltageDetailView') === 'Harmonics') {
    $('#GraphDialogTitle').text('Harmonics');
    PrevDialogContainer = $('#HarmonicsPanel');
  } else if(localStorage.getItem('VoltageDetailView') === 'Advanced') {
    $('#GraphDialogTitle').text('Advanced');
    PrevDialogContainer = $('#FFTPanel');
  } else if(localStorage.getItem('VoltageDetailView') === 'Events') {
    $('#GraphDialogTitle').text('Event waveform');
    PrevDialogContainer = $('#EventWaveformPanel');
  }
  $('#GraphDialog').modal('show');
  if(localStorage.getItem('VoltageDetailView') === 'Waveform') {
    DialogElement = $('#WaveformMaximize').detach();
    DialogPrevResizeHandler = function() {
      $(WaveformChart.graphDiv).parent().removeAttr('style');
    };
    DialogResizeHandler = function() {
      WaveformChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
    };
  } else if(localStorage.getItem('VoltageDetailView') === 'Harmonics') {
    DialogElement = $('#HarmonicsMaximize').detach();
    DialogPrevResizeHandler = function() {
      $(HarmonicsChart.graphDiv).parent().removeAttr('style');
    };
    DialogResizeHandler = function() {
      HarmonicsChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
    };
  } else if(localStorage.getItem('VoltageDetailView') === 'Advanced') {
    DialogElement = $('#FFTMaximize').detach();
    DialogPrevResizeHandler = function() {
      $(FFTChart.graphDiv).parent().removeAttr('style');
    };
    DialogResizeHandler = function() {
      FFTChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
    };
  } else if(localStorage.getItem('VoltageDetailView') === 'Events') {
    DialogElement = $('#EventWaveformMaximize').detach();
    DialogPrevResizeHandler = function() {
      $(EventChart.graphDiv).parent().removeAttr('style');
    };
    DialogResizeHandler = function() {
      EventChart.resize($('#GraphDialogBody').width(), $('#GraphDialogContents').height() - 110);
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

/* Secuencia de inicio para cargar la pagina */
function UpdatePage() {
  DetailsButtonClick();
  GetSeriesScrollable();
  AjaxGetVoltageCounters();
  AjaxGetDetails();
  AjaxGetEvolution();
}
//1752