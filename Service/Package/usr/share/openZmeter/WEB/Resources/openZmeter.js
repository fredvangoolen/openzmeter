/* default options for spinner */
var SpinnerOpts         = { lines: 10,  // The number of lines to draw
                            length: 10, // The length of each line
                            width: 5,   // The line thickness
                            radius: 10  // The radius of the inner circle
                          };
                          
/* harmonics names */
var HarmonicsNames      = [ '1st',  '2nd',  '3rd',  '4th',  '5th',  '6th',  '7th',  '8th',  '9th', '10th',
                            '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th',
                            '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th',
                            '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th',
                            '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th'];
/* Series phase names */
var VoltageNames;
var CurrentNames;
var ActivePowerNames;
var ActiveEnergyNames;

/* offline.js configuration and tools */
Offline.options = {requests: false};
Offline.on('confirmed-down', function() { $('#OfflineDiv').css('display', 'inherit'); });
Offline.on('confirmed-up',   function() {  $('#OfflineDiv').css('display', 'none'); });

/* redirect ajax error to login page */
$(document).ajaxError(function (event, jqxhr, settings, exception) { if(jqxhr.status === 401) window.location.href = 'Login'; });
/* handle buttons events to avoid 'selected' effect */
$(document).on("mousedown", ":button", function(e) { e.preventDefault(); } );
/* getSeries function */
function GetSeries(period, to, aggreg, lastsample, series, onSuccess, onCompletion, onError) {
  $.ajax({ url: "getSeries", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion,
           data: JSON.stringify( { Period: period, To: to, Aggreg: aggreg, LastSample: lastsample, Series: series, Analyzer: Analyzers[Analyzer].ID } )
           
  });
}
/*getSeriesRange function */
function GetSeriesRange(aggreg, onSuccess, onCompletion, onError) {
  $.ajax({ url: "getSeriesRange", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion,
           data: JSON.stringify({ Aggreg: '1h', Analyzer: Analyzers[Analyzer].ID})
  });
}
/* getSeriesNow encapsulation */
function GetSeriesNow(series, onSuccess, onCompletion, onError) {
  $.ajax({ url: "getSeriesNow", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion,
           data: JSON.stringify( { Series: series, Analyzer: Analyzers[Analyzer].ID} )
  });
}
/* getSeriesNow encapsulation */
function GetSeriesAggreg(period, to, aggreg, series, onSuccess, onCompletion, onError) {
  $.ajax({ url: "getSeriesAggreg", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion,
           data: JSON.stringify( { Period: period, To: to, Aggreg: aggreg, Series: series, Analyzer: Analyzers[Analyzer].ID } )
  });
}
/* Return full text from aggregation short name */
function GetAggregLongName(shortText) {
  return (shortText === '1h') ? '1 hour' : (shortText === '10m') ? '10 minutes' : (shortText === '1m') ? '1 minute' : (shortText === '3s') ? '3 seconds' : false;
}
/* Return short name from full aggregation text*/
function GetAggregShortName(longText) {
  return (longText === '1 hour') ? '1h' : (longText === '10 minutes') ? '10m' : (longText === '1 minute') ? '1m' : (longText === '3 seconds') ? '3s' : false;
}
/* Return period name from MS time */
function GetPeriodNameFromMS(elapsed) {
  return (elapsed === 86400000) ? 'Day' : (elapsed === 604800000) ? 'Week' : (elapsed === 2592000000) ? 'Month' : (elapsed === 31536000000) ? 'Year' : 'Custom';
}
/* Return MS time from period name */
function GetMSFromPeriodName(elapsed) {
  return (elapsed === 'Day') ? 86400000 : (elapsed === 'Week') ? 604800000 : (elapsed === 'Month') ? 2592000000 : 31536000000;
}
/* return end end time and duration of current counters */
function GetCountersTimePeriod() {
  var Period = localStorage.getItem('CountersPeriod');
  var Result = (Period === 'Year') ? GetYearTimePeriod() : (Period === 'Month') ? GetMonthTimePeriod() : (Period === 'Week') ? GetWeekTimePeriod() : GetDayTimePeriod();
  return Result;
}
/* return end time and duration of custom period selection (include other useful values) */
function GetCustomTimeDuration() {
  var Result = {};
  
}
/* return end time and duration of current and previous year (include other useful values) */
function GetYearTimePeriod() {
  var Result = {};
  var now = new Date();
  Result.PrimaryTo = new Date(now.getFullYear() + 1, 1, 0, 23, 59, 59, 999).getTime();
  Result.PrimaryPeriod = Result.PrimaryTo - new Date(now.getFullYear(), 1, 1, 0, 0, 0, 0).getTime();
  Result.SecondaryTo = new Date(now.getFullYear(), 1, 0, 23, 59, 59, 999).getTime();;
  Result.SecundaryPeriod = Result.SecondaryTo - new Date(now.getFullYear() - 1, 1, 1, 0, 0, 0, 0).getTime();
  Result.PreferedAggreg = '1h';
  Result.PrimaryText = 'This year';
  Result.SecondaryText = 'Previous year';
  return Result;
}
/* return end time and duration of current and previous month (include other useful values) */
function GetMonthTimePeriod() {
  var Result = {};
  var now = new Date();
  Result.PrimaryTo = new Date(now.getFullYear(), now.getMonth() + 1, 0, 23, 59, 59, 999).getTime();
  Result.PrimaryPeriod = Result.PrimaryTo - new Date(now.getFullYear(), now.getMonth(), 1, 0, 0, 0, 0).getTime();
  Result.SecondaryTo = new Date(now.getFullYear(), now.getMonth(), 0, 23, 59, 59, 999).getTime();;
  Result.SecundaryPeriod = Result.SecondaryTo - new Date(now.getFullYear(), now.getMonth() - 1, 1, 0, 0, 0, 0).getTime();
  Result.PreferedAggreg = '10m';
  Result.PrimaryText = 'This month';
  Result.SecondaryText = 'Previous month';
  return Result;
}
/* return end time and duration of current and previous week (include other useful values) */
function GetWeekTimePeriod() {
  var Result = {};
  var now = new Date();
  Result.PrimaryPeriod = 86400000 * 7;
  Result.PrimaryTo = new Date(now.getFullYear(), now.getMonth(), now.getDate() + 7 - now.getDay(), 23, 59, 59, 999).getTime();
  Result.SecundaryPeriod = Result.CurrentDuration;
  Result.SecondaryTo = Result.CurrentEnd - Result.CurrentDuration;
  Result.PreferedAggreg = '1m';
  Result.PrimaryText = 'This week';
  Result.SecondaryText = 'Previous week';
  return Result;
}
/* return end time and duration of current and previous day (include other useful values) */
function GetDayTimePeriod() {
  var Result = {};
  var now = new Date();
  Result.PrimaryPeriod = 86400000;
  Result.PrimaryTo = new Date(now.getFullYear(), now.getMonth(), now.getDate(), 23, 59, 59, 999).getTime();
  Result.SecundaryPeriod = Result.CurrentDuration;
  Result.SecondaryTo = Result.CurrentEnd - Result.CurrentDuration;
  Result.PreferedAggreg = '1m';
  Result.PrimaryText = 'Today';
  Result.SecondaryText = 'Yesterday';
  return Result;
}
/* getSeries function than take params from controls to manage scrollable graphs */
var ScrollableLastSample = 0;
var ScrollableTimer = null;
function GetSeriesScrollable() {
  if(ScrollableTimer !== null) clearInterval(ScrollableTimer);
  $('#btnScrollableNow').addClass('disabled');
  if(ScrollableLastSample === 0) Scrollable_ShowSpinner();
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
  if(!GraphAggregation || (GetAggregLongName(GraphAggregation) === false)) {
    GraphAggregation = '10m';
    localStorage.setItem('GraphAggregation', GraphAggregation);
  }
  GetSeries(Period, GraphEnd, GraphAggregation, ScrollableLastSample, ScrollableSeries, 
    function(result) {
      Scrollable_StopSpinner();
      Scrollable_OnSuccess(result);
      ScrollableLastSample = LastSampletime(result, ScrollableLastSample);
      $('#btnScrollableAggreg div:first').text(GetAggregLongName(result['Aggregation_Source']));
      $('#btnScrollableAggreg').next().children().removeClass('active');
      $('#btnScrollableAggreg').next().children(':contains(' + GetAggregLongName(result['Aggregation_Source']) + ')').addClass('active');
      var Period = result['To'] - result['From'];
      var PeriodName = GetPeriodNameFromMS(Period);
      $('#btnScrollablePeriod div:first').text(PeriodName);
      $('#btnScrollablePeriod').next().children().removeClass('active');
      $('#btnScrollablePeriod').next().children(':contains(' + PeriodName + ')').addClass('active');
      var GraphAggregation = localStorage.getItem('GraphAggregation');
      if(GraphAggregation !== result['Aggregation_Source']) {
        $('#dlgAggregWarning').modal('show');
        GraphAggregation = result['Aggregation_Source'];
      }
      var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
      if(GraphEnd === 0) {
        $('#btnScrollableNow').addClass('active');
      } else {
        $('#btnScrollableNow').removeClass('active');
      }
      localStorage.setItem('GraphAggregation', GraphAggregation);    
    }, 
    function() { 
      $('#btnScrollableNow').removeClass('disabled');
      if(parseInt(localStorage.getItem('GraphEnd')) === 0) ScrollableTimer = setTimeout(GetSeriesScrollable, 3000);
    }
  );  
}
function Scrollable_Zoom(minDate, maxDate, yRanges) {
  clearInterval(ScrollableTimer);
  $('#btnScrollableNow').removeClass('active');
  localStorage.setItem('GraphEnd', maxDate);
  localStorage.setItem('GraphPeriod', maxDate - minDate);
  ScrollableLastSample = 0;
  GetSeriesScrollable();
}
function Scrollable_MouseUp(event, g, contex) {
  mouseUpOZM(event, g, contex);
  if((contex.isPanning) && (contex.regionWidth > 0)) Scrollable_Zoom(g.dateWindow_[0], g.dateWindow_[1]);
}
function Scrollable_DoubleClick(event, g, context) {
  dblClickOZM(event, g, context);
  Scrollable_Zoom(g.dateWindow_[0], g.dateWindow_[1]);
}
$(document).on("click", "#btnScrollableNow", function() {
  var End = parseInt(localStorage.getItem('GraphEnd'));
  clearInterval(ScrollableTimer);
  if(End === 0) {
    $('#ScrollableTimer').removeClass('active');
    localStorage.setItem('GraphEnd', EnergyChart.getOption('dateWindow')[1]);
  } else {
    $('#btnScrollableNow').addClass('active');
    localStorage.setItem('GraphEnd', 0);
    ScrollableLastSample = 0;
    GetSeriesScrollable();
  }
});
$(document).on("click", "#btnScrollablePeriod + ul li a", function() {
  var PeriodName = $(this).text();
  var Period = GetMSFromPeriodName(PeriodName);
  localStorage.setItem('GraphPeriod', Period);
  ScrollableLastSample = 0;
  GetSeriesScrollable();
  localStorage.setItem('CountersPeriod', PeriodName);
  Scrollable_UpdateCounters(true);
});
$(document).on("click", "#btnScrollableAggreg + ul li a", function() {
  localStorage.setItem('GraphAggregation', GetAggregShortName($(this).text()));
  ScrollableLastSample = 0;
  GetSeriesScrollable();
});



/* append samples of given series to array of graph data, and shift if needed */
function GraphDataAppend(result, serie, data) {
  if(result['Sampletime'] !== null && result['Sampletime'].length > 0) {
    if(data.length === 0 || data[data.length - 1][0] < result['Sampletime'][0]) {
      for(i = 0; i < result['Sampletime'].length; i++) {
        data.push([new Date(result['Sampletime'][i])].concat(result[serie][i]));
      }
      ShiftArray(data, result['MaxSamples']);
    }
  }
  return data;
}

/* remove old items from array*/
function ShiftArray(array, len) {
  while(array.length > len) array.shift();
}

/* return last timestamp from results obtained with GetSeries and others */
function LastSampletime(result, prev) {
  if(result['Sampletime'].length === 0) return prev;
  return Math.max(prev, result['Sampletime'][result['Sampletime'].length - 1]);
}

function GraphHBackgroundRanges(ranges, graph, canvas) {
  ranges.forEach(function(range) {
    var Top = (range[0] === Infinity) ? 0 : (range[0] === -Infinity) ? canvas.canvas.height : graph.toDomYCoord(range[0]);
    var Height = ((range[1] === Infinity) ? 0 : (range[1] === -Infinity) ? canvas.canvas.height : graph.toDomYCoord(range[1])) - Top;
    canvas.fillStyle = range[2];
    canvas.fillRect(0, Top, canvas.canvas.width, Height);
  });
}

/* if called from a button update variable contents to reflect new selections. In all cases update graph visible series */
function GraphPhaseButtons(graph, buttongroup, variable, unique, rangechecker, sender) {
  var State = localStorage.getItem(variable);
  var NumElements = buttongroup.children().length;
  if(State <= 0 || State >= Math.pow(2, graph.attributes_.labels_.length)) State = 1;
  if(sender !== undefined) {
    var Index = $(sender).index();
    if(State == Math.pow(2, Index)) { //only one iten selected
      //do nothing!, i cant deselect if only one item is selected
    } else if(State & Math.pow(2, Index)) {
      State = State & ~(Math.pow(2, Index));
    } else {
      State = State | Math.pow(2, Index);
    }
    if(unique == true) State = State & (Math.pow(2, Index));  
    localStorage.setItem(variable, State);
  }
  if(Analyzers[Analyzer].Mode !== '1PHASE') {
    buttongroup.removeClass('hidden');
    var VisibleSeries = [];
    var Visibles = [];
    for(i = 0; i < NumElements; i++) {
      if(i >= graph.attributes_.labels_.length) {
        $(buttongroup.children()[i]).addClass('disabled');
      } else if(State & Math.pow(2, i)) {
        $(buttongroup.children()[i]).addClass('active');
        $(buttongroup.children()[i]).removeClass('disabled');
        VisibleSeries.push(true);
        Visibles.push(i + 1);
      } else {
        $(buttongroup.children()[i]).removeClass('active');
        $(buttongroup.children()[i]).removeClass('disabled');
        VisibleSeries.push(false);
      }
    }
    graph.updateOptions({ visibility: VisibleSeries, valueRange: rangechecker(Visibles) });
  } else {
    buttongroup.hide();
    graph.updateOptions({ valueRange: rangechecker([1]) });
  }
}

/* find max value of array */
function ArrayMax(array) {
  var Max = 0;
  for(i = 0; i < array.length; i++) {
    Max = Math.max(Max, array[i]);
  }
  return Max;
}
/* sum array */
function ArraySum(array) {
  var Sum = 0;
  for(i = 0; i < array.length; i++) {
    Sum += array[i];
  }
  return Sum;
}

/* Find the max value of graph series data (array of arrays where first column is time) */
function ArrayGraphMax(array) {
  var Max = 0;
  for(i = 0; i < array.length; i++) {
    for(n = 1; n < array[i].length; n++) {      
      Max = Math.max(Max, array[i][n]);
    }
  }
  return Max;
}

/* Find the max,min value of graph visible series data (array of arrays where first column is time) */
function ArrayGraphMinMax(graph, accum) {
  var Result = {VisibleNames: [], Min: Infinity, Max: -Infinity, MinTime: 0, MaxTime: 0};
  for(i = 0; i < graph.attrs_.visibility.length; i++) {
    if(graph.attrs_.visibility[i] === true) Result.VisibleNames.push(graph.attributes_.labels_[i]);
  }
  for(i = 0; i < graph.rawData_.length; i++) {
    if(accum === true) {
      var Value = 0;
      for(n = 1; n < graph.rawData_[i].length; n++) {      
        if(graph.attrs_.visibility[n - 1] === true) Value = Value + graph.rawData_[i][n];
      }
      if(Value < Result.Min) {
        Result.Min = Value;  
        Result.MinTime = graph.rawData_[i][0];
      }
      if(Value > Result.Max) {
        Result.Max = Value;  
        Result.MaxTime = graph.rawData_[i][0];
      }
    } else {
      for(n = 1; n < graph.rawData_[i].length; n++) {      
        if(graph.attrs_.visibility[n - 1] === true) {
          var Value = Value + graph.rawData_[i][n];
          if(Value < Result.Min) {
            Result.Min = Value;  
            Result.MinTime = graph.rawData_[i][0];
          }
          if(Value > Result.Max) {
            Result.Max = Value;  
            Result.MaxTime = graph.rawData_[i][0];
          }
        }
      }
    }
  }
  return Result;
}

/* Add a cubism like graph to a div */
function AppendCubismGraph(div, initialData, label, datewindow, yvalueformatter ) {
  var Index = div.children().length;
  var Container = $('<div class="cubism-serie"></div>');
  var GraphContainer = $('<div class="graph" id="' + div[0].id + '_' + Index + '_Graph"></div>');
  var GraphLegend = $('<div class="legend" id="' + div[0].id + '_' + Index + '_Legend"></div>');
  Container.append(GraphContainer); 
  Container.append(GraphLegend);
  $(div).append(Container);
  var Graph = new Dygraph(GraphContainer[0], initialData, 
    { strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], highlightCircleSize: 0, legend: 'always', xAxisHeight: 0.1, xLabelHeight: 1, plotter: barChartPlotterCubism, interactionModel: { },
      labels: ["Time"].concat(Array(initialData[0].length - 1).fill(label)), colors: Array(initialData[0].length - 1).fill("#d9534f"), labelsDiv: div[0].id + '_' + Index + '_Legend', dateWindow: datewindow,
      plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ],
      legendFormatter: function(data) {
        if(data.x == null) {
          return '<span style="font-size: 18px; font-weight: bold">' + data.series[0].label + '</span>';
        } else {
          for(i = 0; i < data.series.length; i++)  {
            if(data.series[i].yHTML !== undefined) return '<span style="font-size: 15px; font-weight: bold">' + data.series[0].label + '</span><br/>' + data.xHTML + data.series[i].yHTML;
          }      
        }
      },
      axes: {
        y: { drawAxis: false, drawGrid:false, valueFormatter: yvalueformatter },
        x: { valueFormatter: function (x) { return '<div style="font-size: 10px; line-height: 10px">' + new Date(x).toLocaleTimeString() + '</div>'; },
             axisLabelFormatter: (Index !== 0) ? function() { return null; } : 
             function(x, i) {
                var date = new Date(x);
                if((date.getHours() == 0) && (date.getMinutes() == 0) && (date.getSeconds() == 0)) {
                  return '<div class="timeline-day">' + date.toLocaleDateString() + '</div>';
                } else {
                  return '<div class="timeline">' + date.toLocaleTimeString() + '</div>';                       
                }
              }
            }
      }
    }
  );
  return Graph;
}

/* Scale all values of an array to units ('', 'k', 'M') */
function ScaleArrayToUnits(array, units) {
  if(units === '') return array;
  for(i = 0; i < array.length; i++) {
    array[i] = ScaleToUnits(array[i], units);
  }
  return array;
}

/* Scale value to units ('', 'k', 'M') */
function ScaleToUnits(val, units) {
  if(units === '') return val;
  if(units === 'k') {
    return val / 1000;  
  } else if(units === 'M') {
    return val / 1000000;  
  }      
}

/* Find units for high values of energy */
function GetBestUnits(value) {
  if(value > 1000000) {
   return 'M';
  } else if(value > 1000) {
    return 'k';
  } else {
    return '';
  }
}

/* Generate an array of colors from a base */
function ColorArray(base, items) {
  var Array = [];
  for(i = 0; i < items; i++) {
   Array.push(base); 
   base = chroma(base).darken();
  }
  return Array;
}

function LegendFormatter_Follow(data) {
  if(data.x === undefined || data.x === null) return "";
  var Result = '<span class="dygraph-legend-title">' + new Date(data.x).toLocaleDateString() + ' - ' + new Date(data.x).toLocaleTimeString() + '</span><br>';
  for(i = 0; i < data.series.length; i++)  {
    if(data.series[i].isVisible) {
      Result = Result + data.series[i].label + ' : ' + data.series[i].yHTML + '<br>';
    }
  }
  return Result;
}



/* Añade los flag de eventos a una grafica */
function AddEvents(result, serieName, graph) {
  var Events = graph.annotations();
  for(i = 0; i < result['Sampletime'].length; i++) {
    if(result['flag'][i] === false) continue;
    var EventID = {
      series: serieName,
      xval: result['Sampletime'][i],
      shortText: 'EVT',
      text: "This aggregation period contains an event"
    };
    Events.push(EventID);
  }
  graph.setAnnotations(Events);
}
/* Convierte un array de floats a cadena con dos decimales, separados por el caracter dado y usando los colores que se pongan (añadiendo cadena1 si es negativo y cadena2 si positivo)*/
function Array_Round2(values, separator, colors, cadena1, cadena2) {
  for(i = 0; i < values.length; i++) {
    if((colors !== undefined) && (colors[i] !== undefined)) {
     var Text = '<div style="padding: 0px 5px; border-radius: 5px; color: #FFF; background:' + colors[i] + '; display:inline-block">' + Round2(values[i]);
     if((values[i] > 0) && (cadena2 !== undefined)) Text += cadena2;
     if((values[i] < 0) && (cadena1 !== undefined)) Text += cadena1;
     values[i] = Text + '</div>';
   } else {
     var Text = Round2(values[i]);
     if((values[i] > 0) && (cadena2 !== undefined)) Text += cadena2;
     if((values[i] < 0) && (cadena1 !== undefined)) Text += cadena1;
     values[i] = Text + '</div>';
   }
  }
  return values.join(separator);
}

/* Busca el mejor divisor y unidades para un valor de entrada, ya sea B, kB, MB o GB */
function GetBestUnits2(value) {
  if(value > 1024 * 1024 * 1024) {
    return [1024 * 1024 * 1024, 'GB'];
  } else if(value > 1024 * 1024) {
    return [1024 * 1024, 'MB'];
  } else if(value > 1024) {
    return [1024, 'kB'];
  } else {
    return [1, 'B'];
  }
}



/* Formato rapido en multiplos de mil si es necesario */
function FormatKMG(value, unit, unitK, unitsM, unitsG) {
  if(value > 1000000000) return Round2(value / 1000000000) + unitG; 
  if(value > 1000000) return Round2(value / 1000000) + unitM; 
  if(value > 1000) return Round2(value / 1000) + unitK; 
  return Round2(value) + unit; 
}

/* Convierte un numero a cadena con dos decimales y muestra el valor en los huecos que se pasen */
function Round2Labels(value, integer, fraction) {
  if(isNaN(value)) {
    integer.text('?');
    fraction.text('');
  } else {
    var parts = (value + "").split(".");
    integer.text(parts[0]);
    if(parts.length === 1) {
      fraction.text('.00');
    } else {
      parts[1] = parts[1] + "0";
      fraction.text('.' + parts[1].substr(0, 2));
    }
  }
}

/* Retorna un numero como cadena con dos decimales */
function Round2(value) {
  if(isNaN(value)) {
    return '?';
  } else {
    value = Math.round(value * 100);
    var Integer = Math.trunc(value / 100);
    Fraction = Math.abs(value % 100);
    Fraction = Fraction.toString();
    while(Fraction.length < 2) Fraction = '0' + Fraction;
    return ((value < 0) ? '-' : '') + Math.abs(Integer.toString()) + "<span style='font-size:80%'>." + Fraction + '</span>';
  }
}

/* Retorna un numero como cadena con dos decimales pero sin formato html */
function Round2Plain(value) {
  if(isNaN(value)) {
    return '?';
  } else {
    value = Math.round(value * 100);
    var Integer = Math.trunc(value / 100);
    Fraction = Math.abs(value % 100);
    Fraction = Fraction.toString();
    while(Fraction.length < 2) Fraction = '0' + Fraction;
    return Integer.toString() + '.' + Fraction;
  }
}


/* Update header info periodicaly */
function GetIndicators() {
  GetSeriesNow('rms_v, flag', 
    function(result) {
      var Time = new Date(result['Sampletime']);
      $('#NodeTime').text(Time.toLocaleTimeString() + ' - ' + Time.toLocaleDateString());
      $('#IndicatorVoltageValues').html("");
      for(i = 0; i < result['rms_v'].length; i++) {
        $('#IndicatorVoltageValues').append('<span class="value">' + Round2(result['rms_v'][i]) + '</span>');
      }
      $('#VoltageIndicator').html(Array_Round2(result['rms_v'], ' | '));
      $('#CurrentIndicator').html(Array_Round2(result['rms_i'], ' | '));
      var Units = 'W';
      for (i = 0; i < result['active_power'].length; i++) {
        if (result['active_power'][i] > 1000)
          Units = 'kW';
      }
      $('#PowerIndicatorUnits').text(Units);
      if (Units == 'kW') {
        for (i = 0; i < result['active_power'].length; i++)
          result['active_power'][i] = result['active_power'][i] / 1000;
      }
      $('#PowerIndicator').html(Array_Round2(result['active_power'], ' | ', ''));
      $('#FrequencyIndicator').html(Round2(result['freq']) + ' Hz');
    },
    function() { 
      setTimeout(GetIndicators, 5000); 
    }, 
    function() {
      $('#VoltageIndicator').text('Offline');
      $('#CurrentIndicator').text('Offline');
      $('#PowerIndicator').text('Offline');
      $('#FrequencyIndicator').text('Offline');
      $('NodeTime').text('Offline');   
    }
  );
}




function AjaxGetIndicatorsB() {
  var From = GetDayTimestamp();
 $.ajax({ url: "getSeriesAggreg",
    timeout: 20000,
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data: JSON.stringify({ Series: "sum(active_energy)", Aggreg: '1m', Period: 86400000, To: From + 86400000, Analyzer: Analyzers[Analyzer].ID }),
    success: function(result) {
      var Units = 'Wh';
      for(i = 0; i < result['sum_active_energy'].length; i++) {
        if(result['sum_active_energy'][i] > 1000) Units = 'kWh';
      }
      $('#EnergyIndicatorUnits').text(Units);
      if(Units == 'kWh') {
        for(i = 0; i < result['sum_active_energy'].length; i++) result['sum_active_energy'][i] = result['sum_active_energy'][i] / 1000;
      }
      $('#EnergyIndicator').html(Array_Round2(result['sum_active_energy'], ' | ', ''));
    },
    error: function() {
      $('EnergyIndicator').text('Offline');
    },
    complete: function() {
      setTimeout(AjaxGetIndicatorsB, 60000);
    }
 });
}
function AjaxGetSysStats() {
  $.ajax({ url: "getSysInfo",
    timeout: 20000,
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data: JSON.stringify({ Type: 'Performance' }),
    success: function(result) {
      var Status = [];
      if(result['CPU_Usage'] != undefined) {
        if(result['CPU_Count'] != undefined) {
          Status.push(Math.round(result['CPU_Usage'] / result['CPU_Count'] * 100) + '%(CPU)');
        } else {
          Status.push(result['CPU_Usage'] + '%(CPU)');
        }
      }
      if((result['RAM_Total'] != undefined) && (result['RAM_Total'] != undefined)) {
        Status.push(Math.round((result['RAM_Total'] - result['RAM_Free']) / result['RAM_Total'] * 100) + '%(RAM)');
      }
      if((result['Swap_Total'] != undefined) && (result['Swap_Free'] != undefined)) {      
        Status.push(Math.round((result['Swap_Total'] - result['Swap_Free']) / result['Swap_Total'] * 100) + '%(Swap)');
      }
      if(Status.length == 0) {
        $('#StatusRow').css('display', 'none');
      } else {
        $('#StatusRow').css('display', 'inherited');
        $('#UsageText').text(Status.join(' - '));
      }
//      $('#CPUPercent').css('width', );
//      $('#CPUText').html(Round2(result['CPU_Usage'] / result['CPU_Count'] * 100) + '%');
//      var Units = GetBestUnits(result['RAM_Total']);
//      $('#RAMPercent').css('width', (result['RAM_Total'] - result['RAM_Free']) / result['RAM_Total'] * 100 + '%');
//      $('#RAMText').html(Round2((result['RAM_Total'] - result['RAM_Free']) / Units[0]) + '/' + Round2(result['RAM_Total'] / Units[0]) + '<em>&nbsp;' + Units[1] + '</em>');
//      Units = GetBestUnits(result['Swap_Total']);
//      $('#SwapPercent').css('width', (result['Swap_Total'] - result['Swap_Free']) / result['Swap_Total'] * 100 + '%');
//      $('#SwapText').html(Round2((result['Swap_Total'] - result['Swap_Free']) / Units[0]) + '/' + Round2(result['Swap_Total'] / Units[0]) + '<em>&nbsp;' + Units[1] + '</em>');
//      Units = GetBestUnits(result['Disk_Total']);
      $('#DiskPercent').css('width', (result['Disk_Total'] - result['Disk_Free']) / result['Disk_Total'] * 100 + '%');
//      $('#DiskText').html(Round2((result['Disk_Total'] - result['Disk_Free']) / Units[0]) + '/' + Round2(result['Disk_Total'] / Units[0]) + '<em>&nbsp;' + Units[1] + '</em>');
      $('#UptimeText').html(moment.duration(result['UpTime'], "seconds").format("d [days] h [hours] m [minutes]"));
      $('#BatteryPercent').css('width', result['Battery_Percent'] + '%');
//      $('#BatteryText').html(Round2(result['Battery_Percent']) + '%');
    },
    complete: function() {
      setTimeout(AjaxGetSysStats, 5000);
    }
  });  
}

/* Guardar array como archivo CSV, incluyendo la cabecera */
function exportToCsv(filename, rows) {
  var processRow = function (row) {
    var finalVal = '';
    for(var j = 0; j < row.length; j++) {
      var innerValue = row[j] === null ? '' : row[j].toString();
      if(row[j] instanceof Date) innerValue = row[j].toLocaleString();
      var result = innerValue.replace(/"/g, '""');
      if(result.search(/("|;|\n)/g) >= 0) result = '"' + result + '"';
      if(j > 0) finalVal += ';';
      finalVal += result;
    }
    return finalVal + '\n';
  };
  var csvFile = '';
  for (var i = 0; i < rows.length; i++) csvFile += processRow(rows[i]);
  var blob = new Blob([csvFile], {type: 'text/csv;charset=utf-8;'});
  if (navigator.msSaveBlob) { // IE 10+
    navigator.msSaveBlob(blob, filename);
  } else {
    var link = document.createElement("a");
    if(link.download !== undefined) { // feature detection
      var url = URL.createObjectURL(blob);
      link.setAttribute("href", url);
      link.setAttribute("download", filename);
      link.style.visibility = 'hidden';
      document.body.appendChild(link);
      link.click();
      document.body.removeChild(link);
    }
  }
}

/* Actualiza el desplegable de nodos disponibles y selecciona el que este activo para la sesion */
var Analyzers = [];
var Analyzer = parseInt(localStorage.getItem("SelAnalyzer"));
var TableAnalyzers = null;
function GetAnalyzersCompletes(result) {
  if(result.length > 0) $('#btnSearchAnalyzer').removeClass('disabled');
  Analyzers = result;
  TableAnalyzers = $('#tblAnalyzer').DataTable({ responsive: false, paging: false, scrollCollapse: true, scrollY: '224px', 
                                                 dom: '<"top float-left"i><"top float-right"f><"clear">t<"clear">p', 
                                                 language: { info: "_MAX_ avaliable devices", infoEmpty: "No avaliable devices", zeroRecords: " "},
                                                 columnDefs: [ { targets: 0, width: "auto" }, { targets: 1, width: "50px" }, { targets: 2, width: "100px" }, { targets: 3, width: "60px", orderable: false, searchable: false } ], order: [0, 'desc']
  });
  for(i = 0; i < result.length; i++) {
    var RowText = $('<tr>').append('<td class="active">' + result[i].Name + '</td><td>' + result[i].Mode + '</td><td style="font-family: monospace; font-size : 11px">' + result[i].ID + '</td><td><button class="btn btn-xs btn-primary" style="width: 100%">Select</button>');
    TableAnalyzers.row.add(RowText).draw();
  }
  $('#dlgSearchAnalyzer').on('shown.bs.modal',function() { TableDevices.columns.adjust(); } );
  $('#btnSearchAnalyzer').on('click', function() { $('#dlgSearchDevice').modal('show'); });
  
  if(Analyzers[Analyzer] === undefined) {
    localStorage.setItem("SelAnalyzer", 0);
    Analyzer = 0;
  }
//  for(i = 0; i < result['Devices'].length; i++) {
//    Nodes['Devices'][i].Type = "3Phase";
//    if(i === Node) {
//      $('#SourceSel').text(result['Devices'][i]['Name'] + ' (' + result['Devices'][i]['Type'] + ')');
//    } else {
//      $('#SourceList').append(' <li><a href="#" id="Node_' + i + '">' + result['Devices'][i]['Name'] + ' (' + result['Devices'][i]['Type'] + ')</a></li>');      
//    }
//  }
//  if(result['Devices'].length <= 1) {
//    $('#SourcesMenu').addClass('hidden');
//  }
//  $('#SourceList li a').click(function(event) {
//    event.preventDefault();
//    localStorage.setItem('SelNode', this.id.replace('Node_', ''));
//    location.reload(false);
//  });
  if(Analyzers.length !== 0) {
    GetIndicators();
    AjaxGetIndicatorsB();
  }
  VoltageNames      = (Analyzers[Analyzer]['Mode'] == '1PHASE') ? ['Voltage'] : ['V(R)', 'V(S)', 'V(T)'];
  CurrentNames      = (Analyzers[Analyzer]['Mode'] == '1PHASE') ? ['Current'] : (Analyzers[Analyzer]['Mode'] == '3PHASE') ? ['Current (R)', 'Current (S)', 'Current (T)'] : ['Current (R)', 'Current (S)', 'Current (T)', 'Current (N)'];
  ActivePowerNames  = (Analyzers[Analyzer]['Mode'] == '1PHASE') ? ['Power']   : ['Power (R)', 'Power (S)', 'Power (T)'];
  ActiveEnergyNames = (Analyzers[Analyzer]['Mode'] == '1Phase') ? ['Energy']  : ['Energy (R)', 'Energy (S)', 'Energy (T)'];
  AjaxGetSysStats();
  UpdatePage();
}

/* Genera la secuencia de pasos una vez cargado todo el javascript para mostrar la pagina */
var LoadedPending;
function LoadComplete() {
          $('[data-sidenav]').sidenav();
    $('#labelUserName').text(localStorage.getItem('UserName'));
    $.ajax({ url: "getAnalyzers", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), success: GetAnalyzersCompletes });

    
    
    var url = window.location;
    var element = $('ul.nav a').filter(function () {
      return this.href === url || url.href.indexOf(this.href) === 0;
    }).addClass('active').parent().parent().addClass('in').parent();
    if(element.is('div')) {
      element.addClass('active');
    } else {
      $('#side-menu li > a').first().addClass('active');
    }
}
function StartPageLoad() {
  $('.spinnerdiv').each(function(index, element) { new Spinner(SpinnerOpts).spin(element); });
  LoadedPending = $('*[data-load-block]').length;
  if(LoadedPending > 0) {
    $.ajax( { url: 'Common.html', success: 
      function(result) {
        var Template = $('<div></div>');
        Template.append(result.trim());
        $('*[data-load-block]').each(
          function(index) { 
            $(this).html(Template.children('#' + $(this).data('loadBlock')).html());
            LoadedPending--;
            if(LoadedPending === 0) LoadComplete();
          }
        );        
      } 
    });
  }
}





/* Obtiene fecha del inicio de una semana dando el numero y año */
function GetDateOfISOWeek(w, y) {
  var simple = new Date(y, 0, 1 + (w - 1) * 7);
  var dow = simple.getDay();
  var ISOweekStart = simple;
  if (dow <= 4)
    ISOweekStart.setDate(simple.getDate() - simple.getDay() + 1);
  else
    ISOweekStart.setDate(simple.getDate() + 8 - simple.getDay());
  return ISOweekStart;
}

/* Obtiene el numero de dia dentro del año de un timestamp */
function GetDayOfYear(timestamp) {
  var now = new Date(timestamp);
  var start = new Date(now.getFullYear(), 0, 0);
  var diff = (now - start);
  var oneDay = 1000 * 60 * 60 * 24;
  var day = Math.floor(diff / oneDay);
  return day;
}

/* Obtiene el numero de semana dentro del año de un timestamp */
function GetWeekOfYear(timestamp) {
  var date = new Date(timestamp);
  date.setHours(0, 0, 0, 0);
  date.setDate(date.getDate() + 3 - (date.getDay() + 6) % 7);
  var week1 = new Date(date.getFullYear(), 0, 4);
  return 1 + Math.round(((date.getTime() - week1.getTime()) / 86400000 - 3 + (week1.getDay() + 6) % 7) / 7);
}

/* Obtiene el timestamp del inicio del dia actual */
function GetDayTimestamp() {
  var now = new Date();
  return new Date(now.getFullYear(), now.getMonth(), now.getDate(), 0, 0, 0, 0).getTime();
}

/* Obtiene el timestamp del inicio del mes actual */
function GetMonthTimestamp() {
  var now = new Date();
  return new Date(now.getFullYear(), now.getMonth(), 1, 0, 0, 0, 0).getTime();
}

/* Obtiene el timestamp del inicio del año actual */
function GetYearTimestamp() {
  var now = new Date();
  return new Date(now.getFullYear(), 0, 1, 0, 0, 0, 0).getTime();
}

function GetEndYearTimestamp() {
  var now = new Date();
  return new Date(now.getFullYear() + 1, 0, 1, 0, 0, 0, 0).getTime();
}

/* Calcula rango para las graficas, la entrada es un array y un array de columnas a comprobar */
function CalcRanges(input, cols, fromzero, onlypositive, sum) {
  var UpperRange = -Infinity;
  var LowerRange =  Infinity;
  for(i = 0; i < input.length; i++) {
    if(sum == true) {
      var Sum = 0; 
      for(n = 0; n < cols.length; n++) {
        if(input[i][cols[n]] == null) continue;
        Sum += input[i][cols[n]];
      }
      if(Sum > UpperRange) UpperRange = Sum;
      if(Sum < LowerRange) LowerRange = Sum;
    } else {
      for(n = 0; n < cols.length; n++) {
        if(input[i][cols[n]] == null) continue;
        if(input[i][cols[n]] > UpperRange) UpperRange = input[i][cols[n]];
        if(input[i][cols[n]] < LowerRange) LowerRange = input[i][cols[n]];
      }
    }
  }
  var Variation = (UpperRange - LowerRange);
  if(Variation < 1) Variation = 1.0;
  if((fromzero === true) && (UpperRange > 0) && (LowerRange > 0)) {
    UpperRange = UpperRange + Variation * 0.1;
    LowerRange = 0;
  } else if((fromzero === true) && (UpperRange < 0) && (LowerRange < 0)) {
    UpperRange = 0;
    LowerRange = LowerRange - Variation * 0.1;
  } else {
    UpperRange = UpperRange + Variation * 0.1;
    LowerRange = LowerRange - Variation * 0.1;
  }
  if((onlypositive === true) && (LowerRange < 0)) {
    LowerRange = 0;
  }
  return [LowerRange, UpperRange];  
}

/* Divide por 1000 los valores de entrada de un array */
function Div1000(input) {
  var Result = [];
  for(i = 0; i < input.length; i++) {
    Result[i] = [input[i][0]];
    for(n = 1; n < input[i].length; n++) Result[i][n] = input[i][n] / 1000.0;
  }
  return Result;
}











var energyUnit = 'kWh'
var today = new Date();
//$('#side-menu').metisMenu();
//Loads the correct sidebar on window load,
//collapses the sidebar on window resize.
// Sets the min-height of #page-wrapper to window size
$(window).bind("load resize", function () {
    topOffset = 50;
    width = (this.window.innerWidth > 0) ? this.window.innerWidth : this.screen.width;
    if (width < 768) {
        $('div.navbar-collapse').addClass('collapse');
        topOffset = 100; // 2-row-menu
    }

    height = ((this.window.innerHeight > 0) ? this.window.innerHeight : this.screen.height) - 1;
    height = height - topOffset;
    if (height < 1)
        height = 1;
    if (height > topOffset) {
        $("#page-wrapper").css("min-height", (height) + "px");
    }
});
var url = window.location;
var element = $('ul.nav a').filter(function () {
    return this.href == url || url.href.indexOf(this.href) == 0;
}).addClass('active').parent().parent().addClass('in').parent();
if (element.is('div')) {
    element.addClass('active');
} else {
    $('#side-menu li > a').first().addClass('active');
}

$('.maximize').click(function () {
    var panel = $(this).parents('.panel');
    panel.addClass('maximized');
    $(this).hide();
    $('.minimize', panel).show();
    window.dispatchEvent(new Event('resize'));
});
$('.minimize').click(function () {
    var panel = $(this).parents('.panel');
    panel.removeClass('maximized');
    $(this).hide();
    $('.maximize', panel).show();
    window.dispatchEvent(new Event('resize'));
});
$('#side-menu a').click(function () {
//    $(this).addClass('cursor-wait');
//    $('body').addClass('cursor-wait');
});
function getStringEventType(type) {
    switch (type) {
        case 'INTERRUPTION':
            return "Interruption";
        case 'DIP':
            return "Dip";
        case 'SWELL':
            return "Swell";
        case 'RVC':
            return "RVC";
        case 'UNKNOWN':
            return "Unknown";
    }
}

function getStringEventNum(type) {
    switch (type) {
        case 'INTERRUPTION':
            return 0;
        case 'DIP':
            return 1;
        case 'SWELL':
            return 2;
        case 'RVC':
            return 3;
        case 'UNKNOWN':
            return 4;
    }
}

function getIconEventType(type) {
    switch (type) {
        case 'SWELL':
            return 'fa-level-up';
        case 'DIP':
            return 'fa-level-down';
        case 'INTERRUPTION':
            return "fa-window-close-o";
        case 'RVC':
            return 'fa-line-chart';
        case 'UNKNOWN':
            return 'fa-question';
    }
}

function getStringEventTypeShort(type) {
    switch (type) {
        case 0:
            return "I";
        case 1:
            return "D";
        case 2:
            return "S";
        case 3:
            return "R";
        case 4:
            return "U";
    }
}
function getStringEventTypeByNum(type) {
    switch (type) {
        case 0:
            return "Interruption";
        case 1:
            return "Dip";
        case 2:
            return "Swell";
        case 3:
            return "RVC";
        case 4:
            return "UNKNOWN";
    }
}

function ajaxGetLastEvents() {
    var request = $.ajax({
        url: "events/ajaxGetLastEvents",
        timeout: 20000,
        type: "POST",
        dataType: "json"
    });
    return request;
}

function generateEventAlertBox(event, timestamp) {
    var newEvent = "";
    if (event.timestamp > timestamp) {
        newEvent = "new_event";
    }
    var startTime = new Date(+event.timestamp);
    if (event.end_time != -1) {
        var duration = formatTimeUnit(event.duration * 1e6);
    } else {
        duration = 'In progress...'
    }
    var diff = (Math.abs(event.voltage - 230)).toFixed(2);
    var percent = (100 - event.percent).toFixed(2);
    return '<li class="event_alert ' + newEvent + '">\
                <div>\
                    <p><i class="fa ' + getIconEventType(event.type) + ' fa-fw"></i> ' + getStringEventType(event.type) + '\
                    <span class="pull-right text-muted small">' + (startTime.toLocaleString() + '.' + startTime.getMilliseconds()) + '</span></p>\
                    <div class="row small">\
                        <div class="col col-md-4">\
                            <em>Duration:</em><br> ' + duration + '\
                        </div>\
                        <div class="col col-md-5">\
                            <em>ΔU<sub>max</sub>:</em> ' + diff + ' V<br>(' + percent + '%)\
                        </div>\
                        <div class="col col-md-3">\
                            <a class="btn btn-primary btn-sm" href="events/disturbances/' + (+event.timestamp) + '" alt="See details..." title="See details..."><i class="fa fa-search-plus fa-fw"></i></a>\
                        </div>\
                    </div>\
                </div>\
            </li>\
            <li class="divider"></li>';
}

$('#eventsDropdown').on('show.bs.dropdown', function () {
    var dropdown = $('.dropdown-alerts', this).hide();
    var request = ajaxGetLastEvents();
    request.done(function (data) {
        var html = '';
        var timestamp = data.timestamp;
        for (var i in data.events) {
            //for (var j in data.events[i])
//                var generate = true;
//            if (i > 0) {
//                if (data.events[i][j - 1].time == data.events[i][j].time && data.events[i][j].end_time == -1) {
//                    generate = false;
//                }
//            }
//            if (generate) {
            html += generateEventAlertBox(data.events[i], timestamp);
//            }
        }
        if (html != '') {
            $('#eventsDropdown .dropdown-alerts').prepend(html);
        }
        $('#eventsDropdown .dropdown-alerts').slideDown(200);
    })
})

$('#eventsDropdown').on('hidden.bs.dropdown', function () {
    $('#eventsDropdown .event_alert').remove();
    $('#eventsDropdown .divider').remove();
})
