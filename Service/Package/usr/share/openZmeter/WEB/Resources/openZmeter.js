/* default options for spinner */
var SpinnerOpts         = { lines: 10,  // The number of lines to draw
                            length: 10, // The length of each line
                            width: 5,   // The line thickness
                            radius: 10  // The radius of the inner circle
                          };
/* offline.js configuration and tools */
Offline.options = {requests: false};
Offline.on('confirmed-down', function() { $('#OfflineDiv').css('display', 'inherit'); });
Offline.on('confirmed-up',   function() {  $('#OfflineDiv').css('display', 'none'); });
                          
/* Series names */
var SeriesNames = {
  Harmonics: [ '1st',  '2nd',  '3rd',  '4th',  '5th',  '6th',  '7th',  '8th',  '9th', '10th',
               '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th',
               '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th',
               '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th',
               '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th'],
  Voltage: function() { return (Analyzers[Analyzer]['Mode'] === '1PHASE') ? ['L'] : ['R', 'S', 'T']; },
  Current: function() { return (Analyzers[Analyzer]['Mode'] === '1PHASE') ? ['L'] : (Analyzers[Analyzer]['Mode'] === '3PHASE') ? ['(R)', '(S)', '(T)'] : ['(R)', '(S)', '(T)', '(N)']; },
  ActiveEnergy: function() { return (Analyzers[Analyzer]['Mode'] === '1Phase') ? ['L']  : ['P (R)', 'P (S)', 'P (T)']; },
  ReactiveEnergy: function() { return (Analyzers[Analyzer]['Mode'] === '1Phase') ? ['L']  : ['Reactive Energy (R)', 'Reactive Energy (S)', 'Reactive Energy (T)']; },
  ActivePower: function() { return (Analyzers[Analyzer]['Mode'] === '1Phase') ? ['L']  : ['R', 'Active Power (S)', 'Active Power (T)']; },
  ReactivePower: function() { return (Analyzers[Analyzer]['Mode'] === '1Phase') ? ['L']  : ['R', 'Reactive Power (S)', 'Reactive Power (T)']; }
};
  
/* redirect ajax error to login page */
$(document).ajaxError(function (event, jqxhr, settings, exception) { if(jqxhr.status === 401) window.location.href = 'Login'; });
/* handle buttons events to avoid 'selected' effect */
$(document).on("mousedown", ":button", function(e) { e.preventDefault(); this.blur(); } );

/* Ajax functions */
var Ajax = {
  GetSeries: function(period, to, aggreg, lastsample, series, onSuccess, onCompletion) { $.ajax({ url: "getSeries", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion, data: JSON.stringify( { Period: period, To: to, Aggreg: aggreg, LastSample: lastsample, Series: series, Analyzer: Analyzers[Analyzer].ID } ) }); },
  GetSeriesRange: function(aggreg, onSuccess, onCompletion) { $.ajax({ url: "getSeriesRange", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion, data: JSON.stringify({ Aggreg: '1h', Analyzer: Analyzers[Analyzer].ID}) }); },
  GetSeriesNow: function(series, onSuccess, onCompletion) { $.ajax({ url: "getSeriesNow", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion, data: JSON.stringify( { Series: series, Analyzer: Analyzers[Analyzer].ID} ) }); },
  GetSeriesAggreg: function(period, to, aggreg, series, onSuccess, onCompletion) { $.ajax({ url: "getSeriesAggreg", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion, data: JSON.stringify( { Period: period, To: to, Aggreg: aggreg, Series: series, Analyzer: Analyzers[Analyzer].ID } ) }); }  
};


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
  Result.SecondaryPeriod = Result.PrimaryPeriod;
  Result.SecondaryTo = Result.PrimaryTo - Result.PrimaryPeriod;
  Result.PreferedAggreg = '1m';
  Result.PrimaryText = 'Today';
  Result.SecondaryText = 'Yesterday';
  return Result;
}

/* Scrollable help class */
function Scrollable(opt) {
  this.LastSample = 0;
  this.Timer = null;
  this.Opt = Object.assign({NowBtn: '#btnScrollableNow', AggregBtn: '#btnScrollableAggreg', PeriodBtn: '#btnScrollablePeriod', Spinners: [], Series: ''}, opt);
  this.Range = [0, 0];
  var Self = this;
  Self.XAxis = { valueFormatter: function (x) { return new Date(x).toLocaleDateString() + ' - ' + new Date(x).toLocaleTimeString(); },
                 axisLabelFormatter: function (x) { return new Date(x).toLocaleTimeString() + '<br/>' + new Date(x).toLocaleDateString(); }
               };
  $(Self.Opt.NowBtn).click(function() {
    var End = parseInt(localStorage.getItem('GraphEnd'));
    clearInterval(Self.Timer);
    if(End === 0) {
      $(Self.Opt.NowBtn).removeClass('active');
      localStorage.setItem('GraphEnd', Self.Range[1]);
    } else {
      $(Self.Opt.NowBtn).addClass('active');
      localStorage.setItem('GraphEnd', 0);
      Self.LastSample = 0;
      Self.GetSeries();
    }
  });
  $(Self.Opt.PeriodBtn + ' + ul li a').click(function(event) {
    event.preventDefault();
    var PeriodName = $(this).text();
    var Period = GetMSFromPeriodName(PeriodName);
    localStorage.setItem('GraphPeriod', Period);
    Self.LastSample = 0;
    Self.GetSeries();
    localStorage.setItem('CountersPeriod', PeriodName);
    Scrollable_UpdateCounters(true);
  });
  $(Self.Opt.AggregBtn + ' + ul li a').click(function(event) {
    event.preventDefault();
    localStorage.setItem('GraphAggregation', GetAggregShortName($(this).text()));
    Self.LastSample = 0;
    Self.GetSeries();
  });
  Self.GetSeries = function() {
    if(Self.Timer !== null) clearInterval(Self.Timer);
    $(Self.Opt.NowBtn).addClass('disabled');
    if(Self.LastSample === 0) {
      for(i in Self.Opt.Spinners) $(Self.Opt.Spinners[i]).show();
    }
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
    Ajax.GetSeries(Period, GraphEnd, GraphAggregation, Self.LastSample, Self.Opt.Series, 
      function(result) {
        for(i in Self.Opt.Spinners) $(Self.Opt.Spinners[i]).hide();
        Self.Opt.OnSuccess(result, Self.LastSample === 0);
        Self.LastSample = LastSampletime(result, Self.LastSample);
        $(Self.Opt.AggregBtn + ' div:first').text(GetAggregLongName(result['Aggregation_Source']));
        $(Self.Opt.AggregBtn).next().children().removeClass('active');
        $(Self.Opt.AggregBtn).next().children(':contains(' + GetAggregLongName(result['Aggregation_Source']) + ')').addClass('active');
        Self.Range = [result['From'], result['To']];
        var Period = result['To'] - result['From'];
        var PeriodName = GetPeriodNameFromMS(Period);
        $(Self.Opt.PeriodBtn + ' div:first').text(PeriodName);
        $(Self.Opt.PeriodBtn).next().children().removeClass('active');
        $(Self.Opt.PeriodBtn).next().children(':contains(' + PeriodName + ')').addClass('active');
        var GraphAggregation = localStorage.getItem('GraphAggregation');
        if(GraphAggregation !== result['Aggregation_Source']) {
          $('#dlgAggregWarning').modal('show');
          GraphAggregation = result['Aggregation_Source'];
        }
        var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
        if(GraphEnd === 0) {
          $(Self.Opt.NowBtn).addClass('active');
        } else {
          $(Self.Opt.NowBtn).removeClass('active');
        }
        localStorage.setItem('GraphAggregation', GraphAggregation);    
      }, 
      function() { 
        $(Self.Opt.NowBtn).removeClass('disabled');
        if(parseInt(localStorage.getItem('GraphEnd')) === 0) Self.Timer = setTimeout(Self.GetSeries, 3000);
      }
    );  
  };
  Self.GetSeries();
  Self.ZoomEvt = function(minDate, maxDate, yRanges) {
    clearInterval(Self.Timer);
    $(Self.Opt.NowBtn).removeClass('active');
    localStorage.setItem('GraphEnd', maxDate);
    localStorage.setItem('GraphPeriod', maxDate - minDate);
    Self.Range = [minDate, maxDate];
    Self.LastSample = 0;
    Self.GetSeries();
  };
  Self.MouseUpEvt = function(event, g, contex) {
    mouseUpOZM(event, g, contex);
    if((contex.isPanning) && (contex.regionWidth > 0)) Self.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
  };
  Self.DoubleClickEvt = function(event, g, context) {
    dblClickOZM(event, g, context);
    Self.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
  };
}

/* Live view help class */
function Live(opt) {
  this.DetailPaused = false;
  this.LastSample = 0;
  this.Timer = null;
  this.Opt = Object.assign({PauseBtn: '#btnPause', Spinners: [], Series: ''}, opt);
  var Self = this;
  $(Self.Opt.PauseBtn).click(function() {
    Self.DetailPaused = !Self.DetailPaused;
    if(Self.DetailPaused) {
      $(Self.Opt.PauseBtn).addClass('active');
      if(Self.Timer !== null) clearInterval(Self.Timer);
    } else {
      $(Self.Opt.PauseBtn).removeClass('active');
      Selft.GetSeries();
    }
  });
  Self.GetSeries = function() {
    $(Self.Opt.PauseBtn).addClass('disabled');
    Ajax.GetSeriesNow(Self.Opt.Series, 
      function(result) {
        for(i in Self.Opt.Spinners) $(Self.Opt.Spinners[i]).hide();
        Self.Opt.OnSuccess(result);
        Self.LastSample = result['Sampletime'];
      },
      function() {
        $(Self.Opt.PauseBtn).removeClass('disabled');
        Self.Timer = setTimeout(Self.GetSeries, 3000);
      }
    );
  };
  Self.GetSeries();
}
/* Update periodically a fixed period graph(s) */
function PeriodView(opt) {
  this.LastSample = 0;
  this.Timer = null;
  this.Opt = Object.assign({Spinners: [], Series: '', Period: 86400000, Aggreg: '1m'}, opt);
  var Self = this;
  Self.GetSeries = function() {
    Ajax.GetSeries(Self.Opt.Period, 0, Self.Opt.Aggreg, Self.LastSample, Self.Opt.Series, 
      function(result) {
        if(result['Sampletime'] === null) return;
        if(result['Sampletime'].length === 0) return;
        for(i in Self.Opt.Spinners) $(Self.Opt.Spinners[i]).hide();
        Self.Opt.OnSuccess(result);
        Self.LastSample = LastSampletime(result, Self.LastSample);
      },
      function() {
        if(Self.Opt.Aggreg === '3s') {
          Self.Timer = setTimeout(Self.GetSeries, 3000);
        } else {
          Self.Timer = setTimeout(Self.GetSeries, 60000);
        }
      }
    );
  };
  Self.GetSeries();
}

/* Tab selector */
function TabSelector(variable, tab, divs, changeTabEvt) {
  this.Variable = variable;
  this.Tab = tab;
  this.Divs = divs;
  this.ChangeEvent = changeTabEvt;
  var Self = this;
  var Items = $(tab).children().length;
  $(tab).scrollingTabs();
  $(tab + ' > li > a').click(function(evt) {
    $(divs + ' > div').hide();
    localStorage.setItem(Self.Variable, $(evt.target).parent().index());
    $(divs + ' > div:nth-child(' + (Self.GetSaved() + 1) + ')').show();
    Self.ChangeEvent(Self.GetSaved());
    window.dispatchEvent(new Event('resize'));
  });
    Self.GetSaved = function() {
    var Data = parseInt(localStorage.getItem(Self.Variable));
    if((isNaN(Data) === true) || (Data >= Items) || (Data < 0)) {
      localStorage.setItem(Self.Variable, 0);
      Data = 0;
    } 
    return Data;    
  }
  $(divs + ' > div').hide();
  $(tab + ' > li:nth-child(' + (Self.GetSaved() + 1) + ')').addClass('active');
  $(divs + ' > div:nth-child(' + (Self.GetSaved() + 1) + ')').show();
  Self.ChangeEvent(Self.GetSaved());
}

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
/* higlight horizontal ranges in graph */
function GraphHBackgroundRanges(ranges, graph, canvas) {
  ranges.forEach(function(range) {
    var Top = (range[0] === Infinity) ? 0 : (range[0] === -Infinity) ? canvas.canvas.height : graph.toDomYCoord(range[0]);
    var Height = ((range[1] === Infinity) ? 0 : (range[1] === -Infinity) ? canvas.canvas.height : graph.toDomYCoord(range[1])) - Top;
    canvas.fillStyle = range[2];
    canvas.fillRect(0, Top, canvas.canvas.width, Height);
  });
}
/* highlight weekens in graph */
function GraphHightlightWeekends(color, canvas, area, g) {
  canvas.fillStyle = color;
  var fromTimestamp = g.dateWindow_[0];
  var toTimestamp = g.dateWindow_[1];
  while(fromTimestamp < toTimestamp) {
    var FromDate = new Date(fromTimestamp);
    var Tmp = Math.min(new Date(FromDate.getFullYear(), FromDate.getMonth(), FromDate.getDate() + 1, 0, 0, 0, 0).getTime(), toTimestamp);
    if((FromDate.getDay() === 6) || (FromDate.getDay() === 0)) {
      var canvas_left_x = g.toDomXCoord(fromTimestamp), canvas_right_x = g.toDomXCoord(Tmp);
      canvas.fillRect(canvas_left_x, area.y, canvas_right_x - canvas_left_x, area.h);
    }
    fromTimestamp = Tmp;
  }
}
/* Hanle grouo button action (phases) */
function GroupButtons(buttongroup, variable, maxItems, unique, callback, sender) {
  var State = localStorage.getItem(variable);
  var NumElements = buttongroup.children().length;
  if(State <= 0 || State >= Math.pow(2, maxItems)) State = 1;
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
  var VisibleSeries = [];
  var Visibles = [];
  if(Analyzers[Analyzer].Mode !== '1PHASE') {
    buttongroup.removeClass('hidden');
    for(i = 0; i < NumElements; i++) {
      if(i >= maxItems) {
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
  } else {
    buttongroup.hide();
    Visibles.push(1);
    VisibleSeries.push(true);
  }
  if(callback !== undefined) callback(VisibleSeries, Visibles);  
}
/* if called from a button update variable contents to reflect new selections. In all cases update graph visible series */
function GraphPhaseButtons(graph, buttongroup, variable, unique, rangechecker, sender) {
  GroupButtons(buttongroup, variable, graph.attributes_.labels_.length, unique, function(VisibleSeries, Visibles) { graph.updateOptions({ visibility: VisibleSeries, valueRange: rangechecker(Visibles) }); }, sender);
}
/* Generate db ticks (integers) */ 
function GraphDBTicks(min, max, offset) {
  var ticks = [];
  for(var i = Math.floor(min - offset); i < Math.ceil(max - offset); i++) {
    if((i % 10) == 0) ticks.push({ v: i + offset, label: i });
  }
  return ticks;
}
/* Generate odd harmonic ticks */
function GraphOddHarmonics() {
  var Ticks = [];
  for(i = 0; i < SeriesNames.Harmonics.length; i = i + 2) Ticks.push({ v: i, label: SeriesNames.Harmonics[i] });
  return Ticks;
}
function GraphWaveTimeTicks(max) {
  var ticks = [];
  for(var i = 0; i <= Math.floor(max / Analyzers[Analyzer].SampleFreq * Analyzers[Analyzer].NominalFreq); i++) {
    ticks.push({ v: i * Analyzers[Analyzer].SampleFreq / Analyzers[Analyzer].NominalFreq, label: Round2(i / Analyzers[Analyzer].NominalFreq * 1000) });
  }
  return ticks;
}
/* find max value of array (maybe array of arrays) */
function ArrayMax(array) {
  var Max = -Infinity;
  for(i in array) {
    Max = Math.max(Max, Array.isArray(array[i]) ? ArrayMax(array[i]) : array[i]);
  }
  return Max;
}
/* find min value of array (maybe array of arrays) */
function ArrayMin(array) {
  var Min = +Infinity;
  for(i in array) {
    Min = Math.min(Min, Array.isArray(array[i]) ? ArrayMin(array[i]) : array[i]);
  }
  return Min;
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
        y: { drawAxis: false, drawGrid:false, 
             valueFormatter: function(val) { return '<div style="font-size: 14px; line-height: 18px; font-weight: bold">' + yvalueformatter(val) + '</div>'}
           },
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
  var Result = '<span class="dygraph-legend-title">' + data.xHTML + '</span><br>';
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
     var Text = '<div style="padding: 0px 2px; margin: 0px 5px; border-radius: 5px; color: #FFF; background:' + colors[i] + '; display:inline-block">' + Round2(values[i]);
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
  Ajax.GetSeriesNow('rms_v, rms_i, active_power, freq', 
    function(result) {
      var Time = new Date(result['Sampletime']);
      $('#NodeTime').text(Time.toLocaleTimeString() + ' - ' + Time.toLocaleDateString());
      $('#IndicatorVoltageValues').html("");
      for(i = 0; i < result['rms_v'].length; i++) {
        $('#IndicatorVoltageValues').append('<span class="value">' + Round2(result['rms_v'][i]) + '</span>');
      }
      $('#IndicatorCurrentValues').html("");
      for(i = 0; i < result['rms_v'].length; i++) {
        $('#IndicatorCurrentValues').append('<span class="value">' + Round2(result['rms_i'][i]) + '</span>');
      }
      $('#IndicatorActivePowerValues').html("");
      var Units = GetBestUnits(ArrayMax(result['active_power']));
      $('#IndicatorActivePowerUnits').text(Units + "W");
      result['active_power'] = ScaleArrayToUnits(result['active_power'], Units);
      for(i = 0; i < result['active_power'].length; i++) {
        $('#IndicatorActivePowerValues').append('<span class="value">' + Round2(result['active_power'][i]) + '</span>');
      }
      $('#IndicatorFrequencyValues').html('<span class="value">' + Round2(result['freq']) + '</span>');
    },
    function() { 
      setTimeout(GetIndicators, 5000); 
    }, 
    function() {
      $('#IndicatorVoltageValues').html('<span class="value">Offline</span>');
      $('#IndicatorCurrentValues').html('<span class="value">Offline</span>');
      $('#IndicatorActivePowerValues').html('<span class="value">Offline</span>');
      $('#IndicatorFrequencyValues').html('<span class="value">Offline</span>');
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
  $("#tblAnalyzers").on('click', 'button', function () {
    var Row = TableAnalyzers.row($(this).parents('tr'));
    TableAnalyzers.rows().nodes().to$().removeClass('info');
    $(Row.node()).addClass('info');
    localStorage.setItem('SelAnalyzer', Row.node()._DT_RowIndex);
    location.reload();
  });
  if(result.length > 0) $('#btnSearchAnalyzer').removeClass('disabled');
  Analyzers = result;
  TableAnalyzers = $('#tblAnalyzers').DataTable({ responsive: false, paging: false, scrollCollapse: true, scrollY: '224px', 
                                                  dom: '<"top float-left"i><"top float-right"f><"clear">t<"clear">p', 
                                                  language: { info: "_MAX_ avaliable devices", infoEmpty: "No avaliable devices", zeroRecords: " "},
                                                  columnDefs: [ { targets: 0, width: "20px", orderable: false, searchable: false }, { targets: 1, width: "auto" }, { targets: 2, width: "40px" }, { targets: 3, width: "100px" }, { targets: 4, width: "60px", orderable: false, searchable: false } ], order: [1, 'desc']
  });
  for(i = 0; i < result.length; i++) {
    
    
    
    
    var RowText = $('<tr ' + ((i === Analyzer) ? 'class="info"' : '') + '>').append('<td class="text-center">' + ((result[i].Online === true) ? '<i class="fa fa-circle" title="Online"></i>' : '<i class="fa fa-circle-thin" title="Offline"></i>') + '</td><td>' + result[i].Name + '</td><td>' + result[i].Mode + '</td><td style="font-family: monospace; font-size : 11px">' + result[i].ID + '</td><td>' + ((i === Analyzer) ? '<div class="text-success text-center">Selected</div></td>' : '<button class="btn btn-xs btn-primary" style="width: 100%">Select</button></td>'));
    TableAnalyzers.row.add(RowText).draw();
  }
  $('#dlgSearchAnalyzer').on('shown.bs.modal',function() { TableAnalyzers.columns.adjust(); } );
  $('#btnSearchAnalyzer').on('click', function() { $('#dlgSearchAnalyzer').modal('show'); });
  if(Analyzers[Analyzer] === undefined) {
    $('#dlgSearchAnalyzer').modal('show');
    return;
  } else {
    $('#labelAnalyzerName').text(Analyzers[Analyzer].Name);
    GetIndicators();
    AjaxGetIndicatorsB();
  }
  AjaxGetSysStats();
  UpdatePage();
}

/* Genera la secuencia de pasos una vez cargado todo el javascript para mostrar la pagina */
var LoadedPending;
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
            if(LoadedPending === 0) {
              $('[data-sidenav]').sidenav();
              $('#labelUserName').text(localStorage.getItem('UserName'));
              $.ajax({ url: "getAnalyzers", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), success: GetAnalyzersCompletes });
              $('ul.sidenav-menu li a').filter( function() { return this.href === window.location.href; }).addClass('active').parent().parent().show().parent().children('a').addClass('active');
            }
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
  if((fromzero === true) && (UpperRange >= 0) && (LowerRange >= 0)) {
    UpperRange = UpperRange + Variation * 0.1;
    LowerRange = 0;
  } else if((fromzero === true) && (UpperRange <= 0) && (LowerRange <= 0)) {
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
