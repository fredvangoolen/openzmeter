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
var LastFrequencySample   = 0;
var FrequencyChart        = null;
var FrequencyData         = [];
var FrequencySpinner      = new Spinner(SpinnerOpts);
var FrequencyTimer        = null;
function AjaxGetFrequency() {
  if(FrequencyTimer !== null) clearInterval(FrequencyTimer);
  $('#btnFrequencyNow').addClass('disabled');
  if(LastFrequencySample === 0) ShowSpinner(FrequencySpinner, 'FrequencySpinner');
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
    data: JSON.stringify({ Period: Period, To: GraphEnd, Aggreg: GraphAggregation, LastSample: LastFrequencySample, Series: 'freq', Node: Node}),
    success: AjaxFrequency,
    complete: function() {
      $('#btnFrequencyNow').removeClass('disabled');
      if(parseInt(localStorage.getItem('GraphEnd')) === 0) setTimeout(AjaxGetFrequency, 3000);
    }
 });
}
function AjaxFrequency(result) {
  if(LastFrequencySample === 0) {
    StopSpinner(FrequencySpinner, 'FrequencySpinner');
    FrequencyData = [];
  }
  if(result['Sampletime'] !== null) {
    for(i = 0; i < result['Sampletime'].length; i++) FrequencyData.push([new Date(result['Sampletime'][i]), result['freq'][i]]);
    while(FrequencyData.length > result['MaxSamples']) FrequencyData.shift();
    LastFrequencySample = result['Sampletime'][result['Sampletime'].length - 1];
  }  
  $('#FrequencyAggregation').text('From ' + GetAggregText(result['Aggregation_Source']) + ' aggregation');
//  var UpperRange = -Infinity;
//  var LowerRange = Infinity;
//  for (i = 0; i < FrequencyData.length; i++) {
//    if(FrequencyData[i][1] > UpperRange) UpperRange = FrequencyData[i][1];
//    if(FrequencyData[i][1] < LowerRange) LowerRange = FrequencyData[i][1];
//  }
//  var Variation = (UpperRange - LowerRange);
//  if(Variation < 1) Variation = 1.0;
//  UpperRange = UpperRange + Variation * 0.1;
//  LowerRange = LowerRange - Variation * 0.1;
  if(FrequencyChart == null) {
    FrequencyChart = new Dygraph(document.getElementById("FrequencyChart"),
      FrequencyData,
      { strokeWidth: 1,
        labelsDiv: "FrequencyLegend",
        labels: ["Time", "Frequency"],
        stepPlot: true,
        fillGraph: false,
        colors: ["#5cb85c"],
        gridLineColor: "#999",
        gridLinePattern: [4, 4],
        valueRange: CalcRanges(FrequencyData, [1], false),
        dateWindow: [result['From'], result['To']],
        plugins: [
          new Dygraph.Plugins.Crosshair({
            direction: "vertical"
          })
        ],
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
        zoomCallback: function(minDate, maxDate, yRanges) {
          $('#btnFrequencyNow').removeClass('active');
          localStorage.setItem('GraphEnd', maxDate);
          localStorage.setItem('GraphPeriod', maxDate - minDate);
          LastFrequencySample = 0;
          AjaxGetFrequency();
        },
        interactionModel: {
          mousedown: mouseDownOZM,
          mousemove: mouseMoveOZM,
          mouseup: function(event, g, contex) {
            mouseUpOZM(event, g, contex);
            if((contex.isPanning) && (contex.regionWidth > 0)) {
              $('#btnFrequencyNow').removeClass('active');
              localStorage.setItem('GraphEnd', g.dateWindow_[1]);
              localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
              LastFrequencySample = 0;
              AjaxGetFrequency();
            }            
          },
          dblclick: function(event, g, context) {
            dblClickOZM(event, g, context);
            $('#btnFrequencyNow').removeClass('active');
            localStorage.setItem('GraphEnd', g.dateWindow_[1]);
            localStorage.setItem('GraphPeriod', g.dateWindow_[1] - g.dateWindow_[0]);
            LastFrequencySample = 0;
            AjaxGetFrequency();
          }
        },
        axes: {
          y: { pixelsPerLabel: 15,
               axisLabelWidth: 34,
               valueFormatter: function (x) {
                 return Round2(x) + ' <em class="x-small">Hz</em>';
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
    $(window).bind('resize', FrequencyResizeEvent);
    window.dispatchEvent(new Event('resize'));
  } else {
    FrequencyChart.updateOptions({'file': FrequencyData, valueRange: CalcRanges(FrequencyData, [1], false), dateWindow: [result['From'], result['To']]});
  }
  $('#btnShowFrequencyYear').removeClass('active');
  $('#btnShowFrequencyMonth').removeClass('active');
  $('#btnShowFrequencyWeek').removeClass('active');
  $('#btnShowFrequencyDay').removeClass('active');
  if((result['To'] - result['From']) === 86400000) {
    $('#btnShowFrequencyDay').addClass('active');
  } else if((result['To'] - result['From']) === 604800000) {
    $('#btnShowFrequencyWeek').addClass('active');
  } else if((result['To'] - result['From']) === 2592000000) {
    $('#btnShowFrequencyMonth').addClass('active');
  } else if((result['To'] - result['From']) === 31536000000) {
    $('#btnShowFrequencyYear').addClass('active');
  }
  var GraphAggregation = localStorage.getItem('GraphAggregation');
  if(GraphAggregation !== result['Aggregation_Source']) {
    $('#aggreg_modal').modal('show');
    GraphAggregation = result['Aggregation_Source'];
  }
  $('#btnFrequency3S').removeClass('active');
  $('#btnFrequency1M').removeClass('active');
  $('#btnFrequency10M').removeClass('active');
  $('#btnFrequency1H').removeClass('active');
  if(GraphAggregation == '3s') {
    $('#btnFrequency3S').addClass('active');
  } else if(GraphAggregation == '1m') {
    $('#btnFrequency1M').addClass('active');
  } else if(GraphAggregation == '10m') {
    $('#btnFrequency10M').addClass('active');
  } else if(GraphAggregation == '1h') {
    $('#btnFrequency1H').addClass('active');
  }
  var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
  if(GraphEnd === 0) {
    $('#btnFrequencyNow').addClass('active');
  } else {
    $('#btnFrequencyNow').removeClass('active');
  }
  localStorage.setItem('GraphAggregation', GraphAggregation);
}
function FrequencyResizeEvent() {
  var Height = $('#FreqAvgPanel').height() + $('#FreqMaxPanel').height() + $('#FreqMinPanel').height();
  FrequencyChart.resize($('#FrequencyCol').width(), Height - 4);
}
$('#btnFrequency3S').click(function() {
  localStorage.setItem('GraphAggregation', '3s');
  LastFrequencySample = 0;
  AjaxGetFrequency();
});
$('#btnFrequency1M').click(function() {
  localStorage.setItem('GraphAggregation', '1m');
  LastFrequencySample = 0;
  AjaxGetFrequency();
});
$('#btnFrequency10M').click(function() {
  localStorage.setItem('GraphAggregation', '10m');
  LastFrequencySample = 0;
  AjaxGetFrequency();
});
$('#btnFrequency1H').click(function() {
  localStorage.setItem('GraphAggregation', '1h');
  LastFrequencySample = 0;
  AjaxGetFrequency();
});
$('#btnShowFrequencyYear').click(function() {
  localStorage.setItem('GraphPeriod', 31536000000);
  LastFrequencySample = 0;
  AjaxGetFrequency();
  localStorage.setItem('CountersPeriod', 'Year');
  AjaxGetFrequencyCounters();
});
$('#btnShowFrequencyMonth').click(function() {
  localStorage.setItem('GraphPeriod', 2592000000);
  LastFrequencySample = 0;
  AjaxGetFrequency();
  localStorage.setItem('CountersPeriod', 'Month');
  AjaxGetFrequencyCounters();
});
$('#btnShowFrequencyWeek').click(function() {
  localStorage.setItem('GraphPeriod', 604800000);
  LastFrequencySample = 0;
  AjaxGetFrequency();
  localStorage.setItem('CountersPeriod', 'Week');
  AjaxGetFrequencyCounters();
});
$('#btnShowFrequencyDay').click(function() {
  localStorage.setItem('GraphPeriod', 86400000);
  LastFrequencySample = 0;
  AjaxGetFrequency();
  localStorage.setItem('CountersPeriod', 'Day');
  AjaxGetFrequencyCounters();
});
$('#btnFrequencyNow').click(function() {
  var End = parseInt(localStorage.getItem('GraphEnd'));
  if(End === 0) {
    $('#btnFrequencyNow').removeClass('active');
    localStorage.setItem('GraphEnd', FrequencyChart.getOption('dateWindow')[1]);
    if(FrequencyTimer !== null) clearInterval(FrequencyTimer);
  } else {
    $('#btnFrequencyNow').addClass('active');
    localStorage.setItem('GraphEnd', 0);
    LastFrequencySample = 0;
    AjaxGetFrequency();
  }
});
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
function AjaxGetFrequencyCounters() {
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
  AjaxGetFrequency();
  AjaxGetFrequencyCounters();
}