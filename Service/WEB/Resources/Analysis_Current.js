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

/* Grafica de corriente */
var LastCurrentSample   = 0;
var CurrentChart        = null;
var CurrentData         = [];
var CurrentSpinner      = new Spinner(SpinnerOpts);
var CurrentTimer        = null;
function AjaxGetCurrent() {
  if(CurrentTimer !== null) clearInterval(CurrentTimer);
  $('#btnCurrentNow').addClass('disabled');
  if(LastCurrentSample === 0) ShowSpinner(CurrentSpinner, 'CurrentSpinner');
  var Period = parseInt(localStorage.getItem('GraphPeriod'));
  if(!Period) {
    Period = 86400000;
    localStorage.setItem('GraphPeriod', Period);
  }
  var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
  if(!GraphEnd) {
    GraphEnd = 0;
    localStorage.setItem('GraphEnd', 0);
  } 
  var GraphAggregation = localStorage.getItem('GraphAggregation');
  if(!GraphAggregation || (GetAggregText(GraphAggregation) === false)) {
    GraphAggregation = '10m';
    localStorage.setItem('GraphAggregation', GraphAggregation);
  }
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: Period, To: GraphEnd, Aggreg: GraphAggregation, LastSample: LastCurrentSample, Series: 'rms_i', Node: Node}),
           success: AjaxCurrent,
           complete: function (x) {
             $('#btnCurrentNow').removeClass('disabled');
             if(parseInt(localStorage.getItem('GraphEnd')) === 0) CurrentTimer = setTimeout(AjaxGetCurrent, 3000);
           }
  });
}
function AjaxCurrent(result) {
  if(LastCurrentSample === 0) {
    StopSpinner(CurrentSpinner, 'CurrentSpinner');
    CurrentData = [];
  }
  if(result['Sampletime'] !== null) {
    for(i = 0; i < result['Sampletime'].length; i++) CurrentData.push([new Date(result['Sampletime'][i])].concat(result['rms_i'][i]));
    while(CurrentData.length > result['MaxSamples']) CurrentData.shift();
    LastCurrentSample = result['Sampletime'][result['Sampletime'].length - 1];
  }
  $('#CurrentAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
  if(CurrentChart == null) {
    CurrentChart = new Dygraph(document.getElementById("CurrentChart"),
      CurrentData,
      { strokeWidth: 1,
        labelsDiv: "CurrentLegend",
        labels: (Nodes['Devices'][Node]['Type'] == '3Phase') ? ['Time', 'Current (R)', 'Current (S)', 'Current (T)', 'Current (N)'] :['Time', 'Current'],
        stepPlot: true,
        fillGraph: false,
        colors: ["#428bca", "#42ca81", "#ca428b", "#ca8142"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        dateWindow: [result['From'], result['To']],
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
        underlayCallback: function(canvas, area, g) {
//          var YellowLowA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.07);
//          var YellowHeightA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.10) - YellowLowA;
//          var YellowLowB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.07);
//          var YellowHeightB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.10) - YellowLowB;
//          var RedLowA = g.toDomYCoord(1000);
//          var RedHeightA = g.toDomYCoord(NominalVoltage + NominalVoltage * 0.10) - RedLowA;
//          var RedLowB = g.toDomYCoord(NominalVoltage - NominalVoltage * 0.10);
//          var RedHeightB = g.toDomYCoord(0) - RedLowB;
//          canvas.fillStyle = "rgba(255, 255, 0, 0.15)";
//          canvas.fillRect(0, YellowLowA, 10000, YellowHeightA);
//          canvas.fillRect(0, YellowLowB, 10000, YellowHeightB);
//          canvas.fillStyle = "rgba(180, 0, 0, 0.15)";
//          canvas.fillRect(0, RedLowA, 10000, RedHeightA);
//          canvas.fillRect(0, RedLowB, 10000, RedHeightB);
        },
        zoomCallback: function(minDate, maxDate, yRanges) {
          clearInterval(CurrentTimer);
          $('#btnCurrentNow').removeClass('active');
          localStorage.setItem('GraphEnd', maxDate);
          localStorage.setItem('GraphPeriod', maxDate - minDate);
          LastCurrentSample = 0;
          AjaxGetCurrent();
        },
        interactionModel: {
          mousedown: mouseDownOZM,
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              $('#btnCurrentNow').removeClass('active');
              localStorage.setItem('GraphEnd', g.dateWindow_[1]);
              localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastCurrentSample = 0;
              AjaxGetCurrent();
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            $('#btnCurrentNow').removeClass('active');
            localStorage.setItem('GraphEnd', g.dateWindow_[1]);
            localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
            LastCurrentSample = 0;
            AjaxGetCurrent();
          }
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">A</em>';
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
    $(window).bind('resize', CurrentResizeEvent);
    window.dispatchEvent(new Event('resize'));
  } else {
    CurrentChart.updateOptions({'file': CurrentData, dateWindow: [result['From'], result['To']]});
  }
  SetCurrentVisibleSeries();
  $('#btnShowCurrentYear').removeClass('active');
  $('#btnShowCurrentMonth').removeClass('active');
  $('#btnShowCurrentWeek').removeClass('active');
  $('#btnShowCurrentDay').removeClass('active');
  if((result['To'] - result['From']) === 86400000) {
    $('#btnShowCurrentDay').addClass('active');
  } else if((result['To'] - result['From']) === 604800000) {
    $('#btnShowCurrentWeek').addClass('active');
  } else if((result['To'] - result['From']) === 2592000000) {
    $('#btnShowCurrentMonth').addClass('active');
  } else if((result['To'] - result['From']) === 31536000000) {
    $('#btnShowCurrentYear').addClass('active');
  }
  var GraphAggregation = localStorage.getItem('GraphAggregation');
  if(GraphAggregation !== result['Aggregation_Source']) {
    $('#aggreg_modal').modal('show');
    GraphAggregation = result['Aggregation_Source'];
  }
  $('#btnCurrent3S').removeClass('active');
  $('#btnCurrent1M').removeClass('active');
  $('#btnCurrent10M').removeClass('active');
  $('#btnCurrent1H').removeClass('active');
  if(GraphAggregation == '3s') {
    $('#btnCurrent3S').addClass('active');
  } else if(GraphAggregation == '1m') {
    $('#btnCurrent1M').addClass('active');
  } else if(GraphAggregation == '10m') {
    $('#btnCurrent10M').addClass('active');
  } else if(GraphAggregation == '1h') {
    $('#btnCurrent1H').addClass('active');
  }
  var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
  if(GraphEnd === 0) {
    $('#btnCurrentNow').addClass('active');
  } else {
    $('#btnCurrentNow').removeClass('active');
  }
  localStorage.setItem('GraphAggregation', GraphAggregation);
}
function CurrentResizeEvent() {
  var Height = $('#PanelAvg').height() + $('#PanelMin').height() + $('#PanelMax').height();
  CurrentChart.resize($('#CurrentCol').width(), Height - 4);
}
function SetCurrentVisibleSeries() {
  if(Nodes['Devices'][Node]['Type'] == '3Phase') {
    var ShowR = (localStorage.getItem('ShowR') === 'true');
    var ShowS = (localStorage.getItem('ShowS') === 'true');
    var ShowT = (localStorage.getItem('ShowT') === 'true');
    var ShowN = (localStorage.getItem('ShowN') === 'true');
    var VisibleSeries = [ShowR, ShowS, ShowT, ShowN];
    var Visibles = [];
    if(ShowR) Visibles.push(1);
    if(ShowS) Visibles.push(2);
    if(ShowT) Visibles.push(3);
    if(ShowN) Visibles.push(4);
    CurrentChart.updateOptions({ visibility: VisibleSeries, valueRange: CalcRanges(CurrentData, Visibles, false) });
    if(ShowR === true)
      $('#btnShowCurrentRPhase').addClass('active');
    else    
      $('#btnShowCurrentRPhase').removeClass('active');
    if(ShowS === true)
      $('#btnShowCurrentSPhase').addClass('active');
    else    
      $('#btnShowCurrentSPhase').removeClass('active');
    if(ShowT === true)
      $('#btnShowCurrentTPhase').addClass('active');
    else    
      $('#btnShowCurrentTPhase').removeClass('active');
    if(ShowN === true)
      $('#btnShowCurrentNPhase').addClass('active');
    else    
      $('#btnShowCurrentNPhase').removeClass('active');
  } else {
    CurrentChart.updateOptions({ valueRange: CalcRanges(CurrentData, [1], false) });
  }
}
function ChooseCurrentChannels() {
  var ShowR = (localStorage.getItem('ShowR') === 'true');
  var ShowS = (localStorage.getItem('ShowS') === 'true');
  var ShowT = (localStorage.getItem('ShowT') === 'true');
  var ShowN = (localStorage.getItem('ShowN') === 'true');
  if(this.innerText === 'R') ShowR = !ShowR;
  if(this.innerText === 'S') ShowS = !ShowS;
  if(this.innerText === 'T') ShowT = !ShowT;
  if(this.innerText === 'N') ShowN = !ShowN;
  if((this.innerText === 'R') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true) && (ShowN !== true)) ShowR = true;
  if((this.innerText === 'S') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true) && (ShowN !== true)) ShowS = true;
  if((this.innerText === 'T') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true) && (ShowN !== true)) ShowT = true;
  if((this.innerText === 'N') && (ShowR !== true) && (ShowS !== true) && (ShowT !== true) && (ShowN !== true)) ShowN = true;
  localStorage.setItem('ShowR', ShowR);
  localStorage.setItem('ShowS', ShowS);
  localStorage.setItem('ShowT', ShowT);
  localStorage.setItem('ShowN', ShowN);
  SetCurrentVisibleSeries();
}
$('#btnShowCurrentRPhase').click(ChooseCurrentChannels);
$('#btnShowCurrentSPhase').click(ChooseCurrentChannels);
$('#btnShowCurrentTPhase').click(ChooseCurrentChannels);
$('#btnShowCurrentNPhase').click(ChooseCurrentChannels);
$('#btnCurrent3S').click(function() {
  localStorage.setItem('GraphAggregation', '3s');
  LastCurrentSample = 0;
  AjaxGetCurrent();
});
$('#btnCurrent1M').click(function() {
  localStorage.setItem('GraphAggregation', '1m');
  LastCurrentSample = 0;
  AjaxGetCurrent();
});
$('#btnCurrent10M').click(function() {
  localStorage.setItem('GraphAggregation', '10m');
  LastCurrentSample = 0;
  AjaxGetCurrent();
});
$('#btnCurrent1H').click(function() {
  localStorage.setItem('GraphAggregation', '1h');
  LastCurrentSample = 0;
  AjaxGetCurrent();
});
$('#btnShowCurrentYear').click(function() {
  localStorage.setItem('GraphPeriod', 31536000000);
  LastCurrentSample = 0;
  AjaxGetCurrent();
  localStorage.setItem('CountersPeriod', 'Year');
  AjaxGetCurrentCounters();
});
$('#btnShowCurrentMonth').click(function() {
  localStorage.setItem('GraphPeriod', 2592000000);
  LastCurrentSample = 0;
  AjaxGetCurrent();
  localStorage.setItem('CountersPeriod', 'Month');
  AjaxGetCurrentCounters();
});
$('#btnShowCurrentWeek').click(function() {
  localStorage.setItem('GraphPeriod', 604800000);
  LastCurrentSample = 0;
  AjaxGetCurrent();
  localStorage.setItem('CountersPeriod', 'Week');
  AjaxGetCurrentCounters();
});
$('#btnShowCurrentDay').click(function() {
  localStorage.setItem('GraphPeriod', 86400000);
  LastCurrentSample = 0;
  AjaxGetCurrent();
  localStorage.setItem('CountersPeriod', 'Day');
  AjaxGetCurrentCounters();
});
$('#btnCurrentNow').click(function() {
  var End = parseInt(localStorage.getItem('GraphEnd'));
  if(End === 0) {
    $('#btnCurrentNow').removeClass('active');
    localStorage.setItem('GraphEnd', CurrentChart.getOption('dateWindow')[1]);
    if(CurrentTimer !== null) clearInterval(CurrentTimer);
  } else {
    $('#btnCurrentNow').addClass('active');
    localStorage.setItem('GraphEnd', 0);
    LastCurrentSample = 0;
    AjaxGetCurrent();
  }
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
$('#btnExportCurrent').click(function() {
  var Data = [];
  var Filename = "Current_" + LastCurrentTime + '.csv';
  Data.push([['Index', 'Current']]);
  Data = Data.concat(CurrentData);
  exportToCsv(Filename, Data);
  $(this).blur();
});

/* Contadores de corriente */
var CurrentCountersTimer  = null;
function AjaxGetCurrentCounters() {
  if(CurrentCountersTimer !== null) clearInterval(CurrentCountersTimer);
  CurrentCountersTimer = setTimeout(AjaxGetCurrentCounters, 60000);
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
           data: JSON.stringify({ Series: "avg(rms_i), max(rms_i), min(rms_i)", Aggreg: Aggreg, Period: Period, To: Current + Period, Node: Node }),
           success: CurrentUpdateCountersCurrent
  });
  $.ajax({ url: "getSeriesAggreg",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Series: "avg(rms_i), max(rms_i), min(rms_i)", Aggreg: Aggreg, Period: Period, To: Current, Node: Node }),
           success: CurrentUpdateCountersPrevs
  });
}
function CurrentUpdateCountersCurrent(result) {
  var CountersPeriod = localStorage.getItem('CountersPeriod');
  if(CountersPeriod === 'Week') {
    $('#labelCurrentPeriodAVG').text('This week average');
    $('#labelCurrentPeriodMAX').text('This week maximum');
    $('#labelCurrentPeriodMIN').text('This week minimum');
  } else if(CountersPeriod === 'Month') {
    $('#labelCurrentPeriodAVG').text('This month average');
    $('#labelCurrentPeriodMAX').text('This month maximum');
    $('#labelCurrentPeriodMIN').text('This month minimum');
  } else if(CountersPeriod === 'Year') {
    $('#labelCurrentPeriodAVG').text('This year average');
    $('#labelCurrentPeriodMAX').text('This year maximum');
    $('#labelCurrentPeriodMIN').text('This year minimum');
  } else if(CountersPeriod === 'Day') {
    $('#labelCurrentPeriodAVG').text('Today average');
    $('#labelCurrentPeriodMAX').text('Today maximum');
    $('#labelCurrentPeriodMIN').text('Today minimum');
  }
  if(result['avg_rms_v'] === null) {
    $('#labelCurrentCurrentAVG').text('?');
    $('#labelCurrentCurrentMAX').text('?');
    $('#labelCurrentCurrentMIN').text('?');
  } else {
    $('#labelCurrentCurrentAVG').html(Array_Round2(result['avg_rms_i'], '&nbsp;&nbsp;', '', ["#428bca", "#42ca81", "#ca428b", "#ca8142"]));
    $('#labelCurrentCurrentMAX').html(Array_Round2(result['max_rms_i'], '&nbsp;&nbsp;', '', ["#428bca", "#42ca81", "#ca428b", "#ca8142"]));
    $('#labelCurrentCurrentMIN').html(Array_Round2(result['min_rms_i'], '&nbsp;&nbsp;', '', ["#428bca", "#42ca81", "#ca428b", "#ca8142"]));
  }
}
function CurrentUpdateCountersPrevs(result) {
  var CountersPeriod = localStorage.getItem('CountersPeriod');
  if(CountersPeriod === 'Week') {
    $('#labelCurrentPrevPeriodAVG').text('Previous week average');
    $('#labelCurrentPrevPeriodMAX').text('Previous week maximum');
    $('#labelCurrentPrevPeriodMIN').text('Previous week minimum');
  } else if(CountersPeriod === 'Month') {
    $('#labelCurrentPrevPeriodAVG').text('Previous month average');
    $('#labelCurrentPrevPeriodMAX').text('Previous month maximum');
    $('#labelCurrentPrevPeriodMIN').text('Previous month minimum');
  } else if(CountersPeriod === 'Year') {
    $('#labelCurrentPrevPeriodAVG').text('Previous year average');
    $('#labelCurrentPrevPeriodMAX').text('Previous year maximum');
    $('#labelCurrentPrevPeriodMIN').text('Previous year minimum');
  } else if(CountersPeriod === 'Day') {
    $('#labelCurrentPrevPeriodAVG').text('Yesterday average');
    $('#labelCurrentPrevPeriodMAX').text('Yesterday maximum');
    $('#labelCurrentPrevPeriodMIN').text('Yesterday minimum');
  }
  if(result['rms_rms_v'] === null) {
    $('#labelCurrentPrevAVG').text('?');
    $('#labelCurrentPrevMAX').text('?');
    $('#labelCurrentPrevMIN').text('?');
  } else {
    $('#labelCurrentPrevAVG').html(Array_Round2(result['avg_rms_i'], '&nbsp;&nbsp;', '', ["#428bca", "#42ca81", "#ca428b", "#ca8142"]));
    $('#labelCurrentPrevMAX').html(Array_Round2(result['max_rms_i'], '&nbsp;&nbsp;', '', ["#428bca", "#42ca81", "#ca428b", "#ca8142"]));
    $('#labelCurrentPrevMIN').html(Array_Round2(result['min_rms_i'], '&nbsp;&nbsp;', '', ["#428bca", "#42ca81", "#ca428b", "#ca8142"]));
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
var DetailSpinner       = new Spinner(SpinnerOpts);
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
           error: function() {
             $('#WaveformTime').text('No stored data');
             $('#HarmonicsTime').text('No stored data');
             $('#FFTTime').text('No stored data');
           },
           complete: function (x) {
             setTimeout(AjaxGetDetails, 500);
           }
  });
}





var ShowVoltage         = false;
var Paused              = false;
var HideFundamentals    = false;
var HarmonicsChart      = null;
var HarmonicsDataset    = [];
var HarmonicsOffset     = 0;
var FFTChart            = null;
var FFTDataset          = [];
var FFTOffset           = 0;
var WaveformChart       = null;
var WaveformDataset     = [];
var EvolutionCharts     = [];
var LastEvolutionSample = 0;
var EvolutionDatasets   = [];
var CurrentAggregation  = '10m';
var CurrentTimeScale    = 'Day';
var CurrentKeepNow      = true;
var SampleRate          = 15625;
var HarmonicsNames      = [ '1st',  '2nd',  '3rd',  '4th',  '5th',  '6th',  '7th',  '8th',  '9th', '10th',
                            '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th',
                            '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th',
                            '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th',
                            '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th'];

var DetailSpinner  = new Spinner(SpinnerOpts);
function AjaxSuccess5S(result) {
  if(result != null && Object.keys(result).length > 0) {
    if(result['harmonics_i']) UpdateHarmonicsChart(result);
    if(result['fft_i']) UpdateFFTChart(result);
    if(result['wave_i']) UpdateWaveformChart(result);
    LastTimestamp = result['sampletime'];
    result.lastVoltage = result['rms_v'];
    result.lastCurrent = result['rms_i'];
    result.lastPower = result['active_power'];
    result.lastFrequency = result['freq'];
  } else {
    $('#current_integer').text('?');
    $('#current_decimal').text('');
    $('#current_last_day').text('?');
    $('#current_hour_min').text('?');
    $('#current_hour_max').text('?');
    $('#current_min_day').text('?');
    $('#current_max_day').text('?');
  }
  UpdateHeader(result);
}
function UpdateHarmonicsChart(result) {
  if(result['harmonics_i']) {
    HarmonicsDataset = [];
    for(i = 0; i < result.harmonics_i.length; i++) {
      if(result.harmonics_i[i] < 0.000000001) result.harmonics_i[i] = 0.000000001;
      HarmonicsDataset.push([20.0 * Math.log10(result.harmonics_i[i]), result.harmonics_i[i]]);
    }
    HarmonicsOffset = 0;
    for(i = 0; i < 50; i++) {
      if(-HarmonicsDataset[i][0] > 0) HarmonicsOffset = Math.max(HarmonicsOffset, -HarmonicsDataset[i][0]);
    }
    for(i = 0; i < 50; i++) HarmonicsDataset[i][0] = HarmonicsDataset[i][0] + HarmonicsOffset;
  }
  var toShow = [];
  for(i = (HideFundamentals ? 1 : 0); i < 50; i++) toShow.push([i, HarmonicsDataset[i][0]]);
  if(HarmonicsChart === null) {
    HarmonicsChart = new Dygraph(document.getElementById("harmonics_chart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "HarmonicsLegend",  
        labels: ["Harmonic", "dB"],
        colors: ["#31708f"],
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
                 return Round2(x - HarmonicsOffset) + '<strong style="color: #31708f">&nbsp;&nbsp;Current</strong>: ' + (Round2(HarmonicsDataset[row + (HideFundamentals ? 1 : 0)][1])) + '<em class="small"> A</em>';
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
    var date = new Date(result.sampletime * 1000);
    $('#HarmonicsTime').text('Last update at ' + date.toLocaleTimeString() + '.' + date.getMilliseconds());
  } else {
    $('#HarmonicsTime').text('No stored data');
    HarmonicsChart.updateOptions({ file: []});
  }
}
function UpdateFFTChart(result) {
  FFTDataset = [];
  if(result['fft_i']) {
    for(i = 0; i < result.fft_i.points.length; i++) {
      if(result.fft_i.points[i] < 0.000000001) result.fft_i.points[i] = 0.000000001;
      FFTDataset.push([20.0 * Math.log10(result.fft_i.points[i]), result.fft_i.points[i]]);
    }
    FFTOffset = 0;
    for(i = 0; i < result.fft_i.points.length; i++) {
      if(-FFTDataset[i][0] > FFTOffset) FFTOffset = -FFTDataset[i][0];
    }
    for(i = 0; i < result.fft_i.points.length; i++) FFTDataset[i][0] = FFTDataset[i][0] + FFTOffset;
  }
  var toShow = [];
  for(i = 0; i < FFTDataset.length; i++) toShow.push([i, FFTDataset[i][0]]);
  if(FFTChart == null) {
    FFTChart = new Dygraph(document.getElementById("fft_chart"),
      toShow,
      { strokeWidth: 1,
        labelsDiv: "FFTLegend",  
        labels: ["Frequency", "dB"],
        colors: ["#31708f"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        showRangeSelector: true,
        rangeSelectorHeight: 20,
        rangeSelectorPlotFillColor: "#31708f",
        rangeSelectorPlotStrokeColor: "#31708f",
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
                 return Round2(x - FFTOffset) + '<strong style="color: #31708f">&nbsp;&nbsp;Current</strong>: ' + (Round2(FFTDataset[row][1])) + '<em class="small"> A</em>';
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
    var date = new Date(result.fft_i.time);
    $('#FFTTime').text('Last update at ' + date.toLocaleTimeString() + '.' + date.getMilliseconds());
  } else {
    $('#FFTTime').text('No stored data');
    FFTChart.updateOptions({ file: []});
  }
}




function UpdateWaveformChart(result) {
  if(result['wave_i']) {
    if(result['wave_i'].length != 0) {
      var date = new Date(result.fft_i.time);
      $('#WaveformTime').text('Last update at ' + date.toLocaleTimeString() + '.' + date.getMilliseconds());
    } else {
      $('#WaveformTime').text('No stored data');
      WaveformChart.updateOptions({ file: [[0, 0, 0]]});
    }
    WaveformDataset = [];
    for(i = 0; i < result.wave_i.points.length / 2; i++) {
      WaveformDataset.push([i, result.wave_i.points[i * 2 + 1], result.wave_i.points[i * 2]]);
    }
  }
  var Max_Voltage = 1.0;
  var Max_Current = 0.1;
  for(i = 0; i < WaveformDataset.length; i++) {
    if(Math.abs(WaveformDataset[i][2]) > Max_Voltage) Max_Voltage = Math.abs(WaveformDataset[i][2]);
    if(Math.abs(WaveformDataset[i][1]) > Max_Current) Max_Current = Math.abs(WaveformDataset[i][1]);
  }
  var Options = { 
    strokeWidth: 1,
    labelsDiv: "WaveformLegend",  
    labels: ["Time", "Current", "Voltage"],
    colors: ["#31708f", "#d9534f"],
    gridLineColor: "#999",
    gridLinePattern: [4, 4],
    visibility: [true, ShowVoltage],
    showRangeSelector: true,
    rangeSelectorHeight: 20,
    rangeSelectorPlotFillColor: "#FFFFFF",
    rangeSelectorPlotStrokeColor: "#31708f",
    xAxisHeight: 10,
    interactionModel: {
      mousedown: mouseDownOZM,
      mousemove: mouseMoveOZM,
      mouseup  : mouseUpOZM,
      dblclick : dblClickOZM_Restore
    },
    series: {
      'Voltage': {
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
           valueRange: [-Max_Current * 1.1, Max_Current * 1.1], 
           valueFormatter: function (x) {
             return Round2(x) + ' <em class="x-small">A</em>';
           },
           axisLabelFormatter: function (x) {
             return '<div style="font-size: 10px; margin-top: -2px">' + Round2(x) + '</div>';
           }
         },
      y2: { pixelsPerLabel: 15,
            axisLabelWidth: ShowVoltage ? 34 : 0,
            valueRange: [-Max_Voltage * 1.1, Max_Voltage * 1.1], 
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
                     for(var i = 1; i <= Math.floor(max / 312.5); i++) {
                       ticks.push({ v: i * 312.5, label: '<div style="font-size: 10px; margin-top: -2px; text-align: center">' + Round2((i * 312.5 * 1000) / SampleRate) + '</div>' });
                     }
                     return ticks;
                   },
           valueFormatter: function (x) {
             return Round2((x * 1000) / SampleRate) + ' <em class="x-small">ms</em>';
           },
           axisLabelFormatter: function (x) {
             return '<div style="font-size: 10px; margin-top: -2px; line-height: 10px">' + Round2((x * 1000) / SampleRate) + '</div>';
           }
         }
    }
  };
  if(WaveformChart == null) {
    WaveformChart = new Dygraph(document.getElementById("waveform_chart"), WaveformDataset, Options);
  } else {
    WaveformChart.updateOptions(Options);
    WaveformChart.updateOptions({ file: WaveformDataset });   
  }
  WaveformChart.updateOptions({ showRangeSelector: false});
  WaveformChart.updateOptions({ showRangeSelector: true});
  if(Max_Current > 0.1) {
    $('#LowWarning').addClass('hidden');
  } else {
    $('#LowWarning').removeClass('hidden');
  }
}
function UpdateEvolutionChart(result) {
  for(i = 0; i < 50; i++) {
    if(EvolutionDatasets.length == i) EvolutionDatasets.push([]);
    for(j = 0; j < result.evolution_i.length; j++) {
      var Value = 20 * Math.log10(result['evolution_i'][j][i]);
      EvolutionDatasets[i].push([new Date(result.evolution_time[j] * 1000 + 1000), Value]);
      if(EvolutionDatasets[i].length > 720) EvolutionDatasets[i].shift();
    }
  }
  LastEvolutionSample = result['evolution_time'][result.evolution_time.length - 1];
  if(EvolutionCharts.length == 0) {
    for(i = 0; i < 50; i++) {
      var Str = '<div style="position: relative">' +
                '  <div id="HarmonicsEvolutionChart' + i + '" class="chart" style="height: 50px' + ((i == 0) ? '; margin-top: 16px; border-top: 1px solid #999"' : '"') + '></div>' +       
                '  <div style="pointer-events: none; position: absolute; top: -2px; width: 70px; text-align: left; left: 2px" id="HarmonicsEvolutionChartLegend' + i + '"></div>' +
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
    Dygraph.synchronize(EvolutionCharts);
  } else {
    for(i = 0; i < 50; i++) {
      EvolutionCharts[i].updateOptions({ file: EvolutionDatasets[i], dateWindow: [(LastEvolutionSample * 1000) - 43200000, LastEvolutionSample * 1000] });
    }
  }
}
function GetValues5S() {
  if(HarmonicsChart == null) {
    $('#DetailPanel').find("*").prop("disabled", true);
    $('#DetailPanel').css('opacity', '0.6');
    $('#DetailPanel').append('<div id="SpinnerDivDetail" style="position:absolute; top:0; left:0; width:100%; height:100%; z-index:2;"></div>');
    DetailSpinner.spin(document.getElementById('SpinnerDivDetail'));
  }
  $.ajax({ url: "analysis/ajaxUpdateValues5sCurrent",
           timeout: 60000,
           type: "POST",
           dataType: "json",
           data: { paused: Paused },
           success: AjaxSuccess5S,
           complete: function (x) {
                       setTimeout(GetValues5S, 5000);
                       DetailSpinner.stop();
                       $('#SpinnerDivDetail').remove();
                       $('#DetailPanel').css('opacity', '1.0');
                       $('#DetailPanel').find("*").prop("disabled", false);
                     }
        });
}
function AjaxSuccess60S(result) {
  if(result['evolution_time']) {
    UpdateEvolutionChart(result);
    $('#HarmonicEvolutionBtn').removeClass("disabled");
    $('#HarmonicEvolutionLoad').addClass("hidden");
  }
  $('#current_hour').text(Round2(result['hour_rms']));
  $('#current_last_day').text(Round2(result['day_rms']));
  $('#current_hour_min').text(Round2(result['hour_min']));
  $('#current_hour_max').text(Round2(result['hour_max']));
  $('#current_min_day').text(Round2(result['day_min']));
  $('#current_max_day').text(Round2(result['day_max']));
  UpdateHeader(result);
}

function TabChange() {
  $('#Tabs > div').addClass('hidden');
  if($(this).val() == 'harmonics') {
    $('#HarmonicsPanel').removeClass('hidden');
    $(HarmonicsChart.graphDiv).parent().removeAttr('style');
    if(Paused) $('#btnPause').addClass('active');
    $('#btnPause').removeClass('disabled');
    if(HideFundamentals) $('#btnFundamental').addClass('active');
    $('#btnFundamental').removeClass('disabled');
    $('#btnVoltage').removeClass('active');
    $('#btnVoltage').addClass('disabled');
    $('#btnExport').removeClass('disabled');
  } else if($(this).val() == 'evolution') {
    $('#EvolutionPanel').removeClass('hidden');
    $('#EvolutionPanel').removeAttr('style');
    $('#btnPause').removeClass('active');
    $('#btnPause').addClass('disabled');
    $('#btnFundamental').removeClass('active');
    $('#btnFundamental').addClass('disabled');
    $('#btnVoltage').removeClass('active');
    $('#btnVoltage').addClass('disabled');
    $('#btnExport').removeClass('disabled');
  } else if($(this).val() == 'fft') {
    $('#FFTPanel').removeClass('hidden');
    $(FFTChart.graphDiv).parent().removeAttr('style');
    if(Paused) $('#btnPause').addClass('active');
    $('#btnPause').removeClass('disabled');
    $('#btnFundamental').removeClass('active');
    $('#btnFundamental').addClass('disabled');
    $('#btnVoltage').removeClass('active');
    $('#btnVoltage').addClass('disabled');
    $('#btnExport').removeClass('disabled');
  } else if($(this).val() == 'waveform') {
    $('#WaveformPanel').removeClass('hidden');
    if(WaveformChart != null) $(WaveformChart.graphDiv).parent().removeAttr('style');
    if(Paused) $('#btnPause').addClass('active');
    $('#btnPause').removeClass('disabled');
    $('#btnFundamental').removeClass('active');
    $('#btnFundamental').addClass('disabled');
    if(ShowVoltage) $('#btnVoltage').addClass('active');
    $('#btnVoltage').removeClass('disabled');
    $('#btnExport').removeClass('disabled');
  }
  window.dispatchEvent(new Event('resize'));
}
function CurrentTickAggregationButtons(aggregation) {
  $('#current_3s').removeClass('active');
  $('#current_1m').removeClass('active');
  $('#current_10m').removeClass('active');
  $('#current_1h').removeClass('active');
  if(aggregation == '3 seconds') {
    $('#current_3s').addClass('active');
    CurrentAggregation = '3s';
  } else if(aggregation == '1 minute') {
    $('#current_1m').addClass('active');
    CurrentAggregation = '1m'; 
  } else if(aggregation == '10 minutes') {
    $('#current_10m').addClass('active');
    CurrentAggregation = '10m';
  } else if(aggregation == '1 hour') {
    $('#current_1h').addClass('active');
    CurrentAggregation = '1h';
  }
}
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
$('#btnVoltage').click(function () {
  ShowVoltage = !ShowVoltage;
  UpdateWaveformChart([]);
});
$('#btnExport').click(function () {
  var Data = [];
  var Filename;
  if($('#WaveformPanel').hasClass('hidden') == false) {
    Filename = 'current_waveform_' + LastTimestamp + '.csv'
    Data.push([['Index', 'Current', 'Voltage']]);
    Data = Data.concat(WaveformDataset);
  } else if($('#FFTPanel').hasClass('hidden') == false) {
    Filename = 'current_fft_' + LastTimestamp + '.csv';
    Data = [['Index', 'dB', 'I_rms', 'Note: This is the result of our FFT algorithm. The index colum should be multiplied by 5 in order to get the frequencies.']];
    for(i = 0; i < FFTDataset.length; i++) Data.push([i, FFTDataset[i][0]- FFTOffset, FFTDataset[i][1]]);
  } else if($('#HarmonicsPanel').hasClass('hidden') == false) {
    Filename = 'current_harmonics_' + LastTimestamp + '.csv';
    Data = [['Order', 'dB', 'I_rms', ' Freq = ' + $('#frequency_indicator').text(), 'Note: This frequency is the result of our zero crossing algorithm for the last 10 seconds, as stated by IEC 61000-4-30.']];
    for(i = 0; i < HarmonicsDataset.length; i++) Data.push([(i + 1), HarmonicsDataset[i][0] - HarmonicsOffset, HarmonicsDataset[i][1]])  
  } else if($('#EvolutionPanel').hasClass('hidden') == false) {
    Filename = 'current_harmonics_evolution' + LastTimestamp + '.csv';
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




/* Secuencia de inicio para cargar la pagina */
function UpdatePage() {
  if(Nodes['Devices'][Node]['Type'] === '3Phase') {
    $('#btnShowCurrentRPhase').removeClass('disabled');
    $('#btnShowCurrentSPhase').removeClass('disabled');
    $('#btnShowCurrentTPhase').removeClass('disabled');
    $('#btnShowCurrentNPhase').removeClass('disabled');
    $('#btnShowDetailRPhase').removeClass('disabled');
    $('#btnShowDetailSPhase').removeClass('disabled');
    $('#btnShowDetailTPhase').removeClass('disabled');
    $('#btnShowDetailNPhase').removeClass('disabled');
    var ShowR = (localStorage.getItem('ShowR') === 'true');
    var ShowS = (localStorage.getItem('ShowS') === 'true');
    var ShowT = (localStorage.getItem('ShowT') === 'true');
    var ShowN = (localStorage.getItem('ShowT') === 'true');
    if((ShowR !== true) && (ShowS !== true) && (ShowT !== true) && (ShowN !== true)) ShowR = true;
    localStorage.setItem('ShowR', (ShowR === true)); 
    localStorage.setItem('ShowS', (ShowS === true));
    localStorage.setItem('ShowT', (ShowT === true));
    localStorage.setItem('ShowN', (ShowN === true));
  }
  AjaxGetCurrent();
  AjaxGetCurrentCounters();
  AjaxGetDetails();
  //ajaxGetEvolution();
}