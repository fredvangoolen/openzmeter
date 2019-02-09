import chroma from 'chroma-js';
export var Series = {
  'VOLTAGE': { CSS_Class: 'panel-voltage', Title: 'Voltage', Serie: 'Voltage', Units: 'V', Link: '/Analysis/Voltage', 
    FormatValue: function(val) { return {Units: '' + V, Value: Round2(val) }; } 
  },
  'CURRENT': { CSS_Class: 'panel-current', Title: 'Current', Serie: 'Current', Units: 'A', Link: '/Analysis/Current', 
    FormatValue: function(val) { return Round2(val); } 
  },
  'ACTIVE_POWER': { CSS_Class: 'panel-active-power', Title: 'Active power', Serie: 'Active_Power', Units: 'w', Link: '/Analysis/Power', FormatValue: function(val) { return Round2(val); } },
  'FREQUENCY': { CSS_Class: 'panel-frequency', Title: 'Frequency', Serie: 'Frequency', Units: 'Hz', Link: '/Analysis/Frequency', FormatValue: function(val) { return Round2(val); }, rangeFunc: function() {} },
  'REACTIVE_POWER': { CSS_Class: '', Title: 'Reactive Power', Serie: 'Reactive_Power', Units: 'var', Link: '/Analysis/Power', FormatValue: function(val) { return Round2(val); }, rangeFunc: function() {} },
  'POWER_FACTOR': { CSS_Class: '', Title: 'Power Factor', Serie: 'Power_Factor', Units: '', Link: '', FormatValue: function(val) { return Round2(val); } },
  'PHI': { CSS_Class: '', Title: 'Phi', Serie: 'Phi', Units: '', Link: '', FormatValue: function(val) { return Round2(val); }},
  'VOLTAGE_THD': { CSS_Class: '', Title: 'Voltage THD', Serie: 'Voltage_THD', Units: '%', Link:'', FormatValue: function(val) { return Round2(val); }},
  'CURRENT_THD': { CSS_Class: '', Title: 'Current THD', Serie: 'Current_THD', Units: '%', Link:'', FormatValue: function(val) { return Round2(val); }}
};

////var SpinnerOpts = { lines: 10, length: 10, width: 5, radius: 10 }; // default options for spinner
///* offline.js configuration and tools */
//Offline.options = { requests: true };
//Offline.on('confirmed-down', function() { $('#OfflineDiv').css('display', 'inherit'); });
//Offline.on('confirmed-up',   function() { $('#OfflineDiv').css('display', 'none'); });
///* redirect ajax error to login page */
//$(document).ajaxError(function (event, jqxhr, settings, exception) { if(jqxhr.status === 401) window.location.href = 'Login'; });
///* handle buttons events to avoid 'selected' effect */
//$(document).on("mousedown", ":button", function(e) { e.preventDefault(); this.blur(); } );
//
///* Short aggreg to period */
//function GetPeriodFromAggreg(aggreg) {
//  return (aggreg === '1H') ? 3600000 : (aggreg === '15M') ? 900000 : (aggreg === '10M') ? 600000 : (aggreg === '1M') ? 60000 : 3000;
//}
///* Return full text from aggregation short name */
export function GetAggregLongName(shortText) {
  return (shortText === '1H') ? '1 hour' : (shortText === '15M') ? '15 minutes' : (shortText === '10M') ? '10 minutes' : (shortText === '1M') ? '1 minute' : (shortText === '3S') ? '3 seconds' : false;
}
///* Return short name from full aggregation text*/
//function GetAggregShortName(longText) {
//  return (longText === '1 hour') ? '1H' : (longText === '15 minutes') ? '15M' : (longText === '10 minutes') ? '10M' : (longText === '1 minute') ? '1M' : (longText === '3 seconds') ? '3S' : false;
//}
///* Return period name from MS time */
//function GetPeriodNameFromMS(elapsed) {
//  return (elapsed === 86400000) ? 'Day' : (elapsed === 604800000) ? 'Week' : (elapsed === 2592000000) ? 'Month' : (elapsed === 31536000000) ? 'Year' : 'Custom';
//}
///* Return MS time from period name */
//function GetMSFromPeriodName(elapsed) {
//  return (elapsed === 'Day') ? 86400000 : (elapsed === 'Week') ? 604800000 : (elapsed === 'Month') ? 2592000000 : 31536000000;
//}
///* return end time and duration of current and previous year (include other useful values) */
//function GetYearTimePeriod() {
//  var Result = {};
//  var now = new Date();
//  Result.PrimaryTo = new Date(now.getFullYear() + 1, 1, 0, 23, 59, 59, 999).getTime();
//  Result.PrimaryPeriod = Result.PrimaryTo - new Date(now.getFullYear(), 1, 1, 0, 0, 0, 0).getTime();
//  Result.SecondaryTo = new Date(now.getFullYear(), 1, 0, 23, 59, 59, 999).getTime();;
//  Result.SecundaryPeriod = Result.SecondaryTo - new Date(now.getFullYear() - 1, 1, 1, 0, 0, 0, 0).getTime();
//  Result.PreferedAggreg = '1H';
//  Result.PrimaryText = 'This year';
//  Result.SecondaryText = 'Previous year';
//  return Result;
//}
///* return end time and duration of current and previous month (include other useful values) */
//function GetMonthTimePeriod() {
//  var Result = {};
//  var now = new Date();
//  Result.PrimaryTo = new Date(now.getFullYear(), now.getMonth() + 1, 0, 23, 59, 59, 999).getTime();
//  Result.PrimaryPeriod = Result.PrimaryTo - new Date(now.getFullYear(), now.getMonth(), 1, 0, 0, 0, 0).getTime();
//  Result.SecondaryTo = new Date(now.getFullYear(), now.getMonth(), 0, 23, 59, 59, 999).getTime();;
//  Result.SecundaryPeriod = Result.SecondaryTo - new Date(now.getFullYear(), now.getMonth() - 1, 1, 0, 0, 0, 0).getTime();
//  Result.PreferedAggreg = '10M';
//  Result.PrimaryText = 'This month';
//  Result.SecondaryText = 'Previous month';
//  return Result;
//}
///* return end time and duration of current and previous week (include other useful values) */
//function GetWeekTimePeriod() {
//  var Result = {};
//  var now = new Date();
//  Result.PrimaryPeriod = 86400000 * 7;
//  Result.PrimaryTo = new Date(now.getFullYear(), now.getMonth(), now.getDate() + 7 - ((now.getDay() === 0) ? 6 : now.getDay() - 1), 23, 59, 59, 999).getTime();
//  Result.SecundaryPeriod = Result.PrimaryPeriod;
//  Result.SecondaryTo = Result.PrimaryTo - Result.PrimaryPeriod;
//  Result.PreferedAggreg = '1M';
//  Result.PrimaryText = 'This week';
//  Result.SecondaryText = 'Previous week';
//  return Result;
//}
///* return end time and duration of current and previous day (include other useful values) */
//function GetDayTimePeriod() {
//  var Result = {};
//  var now = new Date();
//  Result.PrimaryPeriod = 86400000;
//  Result.PrimaryTo = new Date(now.getFullYear(), now.getMonth(), now.getDate(), 23, 59, 59, 999).getTime();
//  Result.SecondaryPeriod = Result.PrimaryPeriod;
//  Result.SecondaryTo = Result.PrimaryTo - Result.PrimaryPeriod;
//  Result.PreferedAggreg = '1M';
//  Result.PrimaryText = 'Today';
//  Result.SecondaryText = 'Yesterday';
//  return Result;
//}
//
///* Scrollable help class 
// * NowBtn: pause the automatic update
// * AggregBtn: Selected aggregation
// * PeriodBtn: Period in view
// * Counters: Callback function called when selected period changes (called with GetTimes result) */
//function Scrollable(opt) {
//  this.LastSample = 0;
//  this.Timer = null;
//  this.Opt = Object.assign({NowBtn: '#btnScrollableNow', AggregBtn: '#btnScrollableAggreg', PeriodBtn: '#btnSearchAnalyzercrollablePeriod', Spinners: [], Series: '', AggregVariable: 'GraphAggregation'}, opt);
//  this.Range = [0, 0];
//  this.CountersTimes = null;
//  var Self = this;
//  Self.XAxis = { valueFormatter: function (x) { return new Date(x).toLocaleDateString() + ' - ' + new Date(x).toLocaleTimeString(); },
//                 axisLabelFormatter: function (x) { return new Date(x).toLocaleTimeString() + '<br/>' + new Date(x).toLocaleDateString(); }
//               };
//  $(Self.Opt.NowBtn).click(function() {
//    var End = parseInt(localStorage.getItem('GraphEnd'));
//    clearInterval(Self.Timer);
//    if(End === 0) {
//      $(Self.Opt.NowBtn).removeClass('active');
//      localStorage.setItem('GraphEnd', Self.Range[1]);
//    } else {
//      $(Self.Opt.NowBtn).addClass('active');
//      localStorage.setItem('GraphEnd', 0);
//      Self.LastSample = 0;
//      Self.GetSeries();
//    }
//  });
//  $(Self.Opt.PeriodBtn + ' + ul li a').click(function(event) {
//    event.preventDefault();
//    var PeriodName = $(this).text();
//    var Period = GetMSFromPeriodName(PeriodName);
//    localStorage.setItem('GraphPeriod', Period);
//    Self.LastSample = 0;
//    Self.GetSeries();
//  });
//  $(Self.Opt.AggregBtn + ' + ul li a').click(function(event) {
//    event.preventDefault();
//    localStorage.setItem(Self.Opt.AggregVariable, GetAggregShortName($(this).text()));
//    Self.LastSample = 0;
//    Self.GetSeries();
//  });
//  Self.GetCountersTimePeriod = function(name) {
//    if(parseInt(localStorage.getItem('GraphEnd')) === 0) {
//      if(name === 'Year')  return Object.assign(GetYearTimePeriod(),  { PreferedAggreg: localStorage.getItem(Self.Opt.AggregVariable) });
//      if(name === 'Month') return Object.assign(GetMonthTimePeriod(), { PreferedAggreg: localStorage.getItem(Self.Opt.AggregVariable) });
//      if(name === 'Week')  return Object.assign(GetWeekTimePeriod(),  { PreferedAggreg: localStorage.getItem(Self.Opt.AggregVariable) });
//      if(name === 'Day')   return Object.assign(GetDayTimePeriod(),   { PreferedAggreg: localStorage.getItem(Self.Opt.AggregVariable) });
//    }
//    var Result = {};
//    Result.PrimaryPeriod = parseInt(localStorage.getItem('GraphPeriod'));
//    Result.PrimaryTo = parseInt(localStorage.getItem('GraphEnd'));
//    Result.PreferedAggreg = localStorage.getItem(Self.Opt.AggregVariable);
//    Result.PrimaryText = 'Displayed period';
//    if(Result.PrimaryTo === 0) {
//      Result.SecondaryText = 'Last ' + moment.duration(Result.PrimaryPeriod / 1000, "seconds").format("d [days] h [hours] m [minutes]");
//    } else {
//      var From = new Date(Result.PrimaryTo - Result.PrimaryPeriod);
//      var To = new Date(Result.PrimaryTo);
//      Result.SecondaryText = 'From ' + From.toLocaleDateString() + ' - ' + From.toLocaleTimeString() + ' to ' + To.toLocaleDateString() + ' - ' + To.toLocaleTimeString();
//    }
//    return Result;    
//  };
//  Self.OnSuccess = function(result) {
//    for(var i in Self.Opt.Spinners) $(Self.Opt.Spinners[i]).hide();
//    Self.Opt.OnSuccess(result, Self.LastSample === 0);
//    Self.LastSample = LastSampletime(result, Self.LastSample);
//    $(Self.Opt.AggregBtn + ' span:first').text(GetAggregLongName(result['Aggregation_Source']));
//    $(Self.Opt.AggregBtn).next().children().removeClass('active');
//    $(Self.Opt.AggregBtn).next().children(':contains(' + GetAggregLongName(result['Aggregation_Source']) + ')').addClass('active');
//    Self.Range = [result['From'], result['To']];
//    var Period = result['To'] - result['From'];
//    var PeriodName = GetPeriodNameFromMS(Period);
//    $(Self.Opt.PeriodBtn + ' span:first').text(PeriodName);
//    $(Self.Opt.PeriodBtn).next().children().removeClass('active');
//    $(Self.Opt.PeriodBtn).next().children(':contains(' + PeriodName + ')').addClass('active');
//    if(Self.CountersTimes === null) {
//      Self.CountersTimes = Self.GetCountersTimePeriod(PeriodName);
//      if(Self.Opt.Counters !== undefined) Self.Opt.Counters(Self.CountersTimes, true);
//    } else {
//      var Times = Self.GetCountersTimePeriod(PeriodName);
//      if(Self.Opt.Counters !== undefined) Self.Opt.Counters(Times, (Times.PrimaryTo !== Self.CountersTimes.PrimaryTo) || (Times.PrimaryPeriod !== Self.CountersTimes.PrimaryPeriod));
//      Self.CountersTimes = Times;
//    }
//    if(Self.Opt.Series !== 'Prices') {
//      var GraphAggregation = localStorage.getItem(Self.Opt.AggregVariable);
//      if(GraphAggregation !== result['Aggregation_Source']) {
//        $('#dlgAggregWarning').modal('show');
//        GraphAggregation = result['Aggregation_Source'];
//      }
//      localStorage.setItem(Self.Opt.AggregVariable, GraphAggregation);
//    }      
//    var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
//    if(GraphEnd === 0) {
//      $(Self.Opt.NowBtn).addClass('active');
//    } else {
//      $(Self.Opt.NowBtn).removeClass('active');
//    }
//  };
//  Self.GetSeries = function() {
//    if(Self.Timer !== null) clearInterval(Self.Timer);
//    $(Self.Opt.NowBtn).addClass('disabled');
//    if(Self.LastSample === 0) {
//      for(var i in Self.Opt.Spinners) $(Self.Opt.Spinners[i]).show();
//    }
//    var Period = parseInt(localStorage.getItem('GraphPeriod'));
//    if(!Period) {
//      Period = 86400000;
//      localStorage.setItem('GraphPeriod', Period);
//    }
//    var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
//    if(!GraphEnd) {
//      GraphEnd = 0;
//      localStorage.setItem('GraphEnd', 0);
//    }
//    var GraphAggregation = localStorage.getItem(Self.Opt.AggregVariable);
//    if(!GraphAggregation || (GetAggregLongName(GraphAggregation) === false)) {
//      GraphAggregation = '1H';
//      localStorage.setItem(Self.Opt.AggregVariable, GraphAggregation);
//    }
//    if(Self.Opt.Series === 'Prices') {
//      Ajax.GetPrices(Period, GraphEnd, Self.LastSample,
//        Self.OnSuccess, 
//        function() { 
//          $(Self.Opt.NowBtn).removeClass('disabled');
//          if(parseInt(localStorage.getItem('GraphEnd')) === 0) Self.Timer = setTimeout(Self.GetSeries, GetPeriodFromAggreg(GraphAggregation));
//        }
//      );      
//    } else {
//      Analyzer.GetSeries(Period, GraphEnd, GraphAggregation, Self.LastSample, Self.Opt.Series, 
//        Self.OnSuccess, 
//        function() { 
//          $(Self.Opt.NowBtn).removeClass('disabled');
//          if(parseInt(localStorage.getItem('GraphEnd')) === 0) Self.Timer = setTimeout(Self.GetSeries, GetPeriodFromAggreg(GraphAggregation));
//        }
//      );
//    }
//  };
//  Self.GetSeries();
//  Self.ZoomEvt = function(minDate, maxDate, yRanges) {
//    clearInterval(Self.Timer);
//    $(Self.Opt.NowBtn).removeClass('active');
//    localStorage.setItem('GraphEnd', maxDate);
//    localStorage.setItem('GraphPeriod', maxDate - minDate);
//    Self.Range = [minDate, maxDate];
//    Self.LastSample = 0;
//    Self.GetSeries();
//  };
//  Self.MouseUpEvt = function(event, g, contex) {
//    mouseUpOZM(event, g, contex);
//    if((contex.isPanning) && (contex.regionWidth > 0)) Self.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
//  };
//  Self.DoubleClickEvt = function(event, g, context) {
//    dblClickOZM(event, g, context);
//    Self.ZoomEvt(g.dateWindow_[0], g.dateWindow_[1]);
//  };
//}
//
///* Live view help class */
//function Live(opt) {
//  this.DetailPaused = false;
//  this.LastSample = 0;
//  this.Timer = null;
//  this.Opt = Object.assign({NowBtn: '#btnPause', Spinners: [], Series: ''}, opt);
//  var Self = this;
//  $(Self.Opt.NowBtn).click(function() {
//    Self.DetailPaused = !Self.DetailPaused;
//    if(Self.DetailPaused) {
//      $(Self.Opt.NowBtn).removeClass('active');
//      if(Self.Timer !== null) clearInterval(Self.Timer);
//    } else {
//      $(Self.Opt.NowBtn).addClass('active');
//      Self.GetSeries();
//    }
//  });
//  $(Self.Opt.NowBtn).addClass('active');
//  Self.GetSeries = function() {
//    $(Self.Opt.NowBtn).addClass('disabled');
//    Analyzer.GetSeriesNow(Self.Opt.Series, 
//      function(result) {
//        for(var i in Self.Opt.Spinners) $(Self.Opt.Spinners[i]).hide();
//        Self.Opt.OnSuccess(result);
//        Self.LastSample = result['Sampletime'];
//      },
//      function() {
//        $(Self.Opt.NowBtn).removeClass('disabled');
//        Self.Timer = setTimeout(Self.GetSeries, 3000);
//      }
//    );
//  };
//  Self.GetSeries();
//}
///* Update periodically a fixed period graph(s) */
//function PeriodView(opt) {
//  this.LastSample = 0;
//  this.Timer = null;
//  this.Opt = Object.assign({Spinners: [], Series: '', Period: 86400000, Aggreg: '1M'}, opt);
//  var Self = this;
//  Self.GetSeries = function() {
//    Analyzer.GetSeries(Self.Opt.Period, 0, Self.Opt.Aggreg, Self.LastSample, Self.Opt.Series, 
//      function(result) {
//        if(result['Sampletime'] === null) return;
//        if(result['Sampletime'].length === 0) return;
//        for(var i in Self.Opt.Spinners) $(Self.Opt.Spinners[i]).hide();
//        Self.Opt.OnSuccess(result);
//        Self.LastSample = LastSampletime(result, Self.LastSample);
//      },
//      function() {
//        if(Self.Opt.Aggreg === '3S') {
//          Self.Timer = setTimeout(Self.GetSeries, 3000);
//        } else {
//          Self.Timer = setTimeout(Self.GetSeries, 60000);
//        }
//      }
//    );
//  };
//  Self.GetSeries();
//}
///* Tab selector */
//function TabSelector(variable, tab, divs, changeTabEvt) {
//  this.Variable = variable;
//  this.Tab = tab;
//  this.Divs = divs;
//  this.ChangeEvent = changeTabEvt;
//  var Self = this;
//  var Items = $(tab).children().length;
//  $(tab).scrollingTabs();
//  $(tab + ' > li > a').click(function(evt) {
//    $(Self.Divs + ' > div').hide();
//    localStorage.setItem(Self.Variable, $(evt.target).parent().index());
//    $(Self.Tab + ' > li').removeClass('active');
//    $(Self.Tab + ' > li:nth-child(' + (Self.GetSaved() + 1) + ')').addClass('active');
//    $(Self.Divs + ' > div:nth-child(' + (Self.GetSaved() + 1) + ')').show();
//    Self.ChangeEvent(Self.GetSaved());
//    window.dispatchEvent(new Event('resize'));
//  });
//  Self.GetSaved = function() {
//    var Data = parseInt(localStorage.getItem(Self.Variable));
//    if((isNaN(Data) === true) || (Data >= Items) || (Data < 0)) {
//      localStorage.setItem(Self.Variable, 0);
//      Data = 0;
//    } 
//    return Data;    
//  }
//  $(divs + ' > div').hide();
//  $(tab + ' > li:nth-child(' + (Self.GetSaved() + 1) + ')').addClass('active');
//  $(divs + ' > div:nth-child(' + (Self.GetSaved() + 1) + ')').show();
//  Self.ChangeEvent(Self.GetSaved());
//}
///* Graph maximize and export */
//function GraphTools(title, graph, toolbar, resizeHandler, exportFunction) {
//  this.Graph = graph;
//  this.Title = title;
//  this.PrevHandler = resizeHandler;
//  this.Graph = graph;
//  this.Toolbar = toolbar;
//  var Self = this;
//  Self.DialogResizeHandler = function() {
//    ((typeof(Self.Graph) === 'function') ? Self.Graph() : Self.Graph).resize($('#MaximizeDialogBody').width(), $('#MaximizeDialogContents').height() - 70);
//  }
//  $(Self.Toolbar + ' button[data-function="Maximize"]').click(function(evt) {
//    if($('#dlgMaximizeDialog').hasClass('in')) {
//      $('#MaximizeToolbar button[data-function="Maximize"] > img').attr('src', 'Imgs/arrow_out.png');
//      $('#MaximizeToolbar button[data-function="Maximize"]').attr('title', 'Maximize graph');
//      $(window).unbind('resize', Self.DialogResizeHandler);
//      $(Self.PrevContainer).append($(((typeof(Self.Graph) === 'function') ?  Self.Graph() : Self.Graph).graphDiv).parent().detach());
//      $(((typeof(Self.Graph) === 'function') ?  Self.Graph() : Self.Graph).graphDiv).parent().removeAttr('style');
//      if(Self.PrevHandler !== undefined) $(window).bind('resize', Self.PrevHandler);
//      $(Self.Toolbar).append($('#MaximizeToolbar').children().detach());
//      $('#dlgMaximizeDialog').modal('hide');
//    } else {
//      $(Self.Toolbar + ' button[data-function="Maximize"] > img').attr('src', 'Imgs/arrow_in.png');
//      $(Self.Toolbar + ' button[data-function="Maximize"]').attr('title', 'Minimize graph');
//      Self.PrevContainer = $(((typeof(Self.Graph) === 'function') ?  Self.Graph() : Self.Graph).graphDiv).parent().parent();
//      if(Self.PrevHandler !== undefined) $(window).unbind('resize', Self.PrevHandler);
//      $('#dlgMaximizeDialog').modal('show');
//      $('#MaximizeDialogTitle').text((typeof(Self.Title) === 'function') ? Self.Title() : Self.Title);
//      $('#MaximizeToolbar').append($(Self.Toolbar).children().detach());
//      $('#MaximizeDialogBody').append($(((typeof(Self.Graph) === 'function') ?  Self.Graph() : Self.Graph).graphDiv).parent().detach());
//      $(window).bind('resize', Self.DialogResizeHandler);
//    }
//    window.dispatchEvent(new Event('resize'));
//  });
//  if(exportFunction !== undefined) $(Self.Toolbar + ' button[data-function="Export"]').click(exportFunction);
//}
///* append samples of given series to array of graph data, and shift if needed */
export function GraphDataAppend(result, serie, data) {
  if(result['Sampletime'] !== null && result['Sampletime'].length > 0) {
    if(data.length === 0 || data[data.length - 1][0] < result['Sampletime'][0]) {
      for(var i = 0; i < result['Sampletime'].length; i++) {
        data.push([new Date(result['Sampletime'][i])].concat(result[serie][i]));
      }
      ShiftArray(data, result['MaxSamples']);
    }
  }
  return data;
}
///* remove old items from array*/
function ShiftArray(array, len) {
  while(array.length > len) array.shift();
}
///* return last timestamp from results obtained with GetSeries and others */
export function LastSampletime(result, prev) {
  if(result['Sampletime'].length === 0) return prev;
  return Math.max(prev, result['Sampletime'][result['Sampletime'].length - 1]);
}
///* higlight horizontal ranges in graph */
//function GraphHBackgroundRanges(ranges, graph, canvas) {
//  ranges.forEach(function(range) {
//    var Top = (range[0] === Infinity) ? 0 : (range[0] === -Infinity) ? canvas.canvas.height : graph.toDomYCoord(range[0]);
//    var Height = ((range[1] === Infinity) ? 0 : (range[1] === -Infinity) ? canvas.canvas.height : graph.toDomYCoord(range[1])) - Top;
//    canvas.fillStyle = range[2];
//    canvas.fillRect(0, Top, canvas.canvas.width, Height);
//  });
//}
///* highlight weekens in graph */
//function GraphHightlightWeekends(color, canvas, area, g) {
//  canvas.fillStyle = color;
//  var fromTimestamp = g.dateWindow_[0];
//  var toTimestamp = g.dateWindow_[1];
//  while(fromTimestamp < toTimestamp) {
//    var FromDate = new Date(fromTimestamp);
//    var Tmp = Math.min(new Date(FromDate.getFullYear(), FromDate.getMonth(), FromDate.getDate() + 1, 0, 0, 0, 0).getTime(), toTimestamp);
//    if((FromDate.getDay() === 6) || (FromDate.getDay() === 0)) {
//      var canvas_left_x = g.toDomXCoord(fromTimestamp), canvas_right_x = g.toDomXCoord(Tmp);
//      canvas.fillRect(canvas_left_x, area.y, canvas_right_x - canvas_left_x, area.h);
//    }
//    fromTimestamp = Tmp;
//  }
//}
///* Handle group button action (phases) */
//function GroupButtons(buttongroup, variable, maxItems, unique, callback, sender) {
//  var State = localStorage.getItem(variable);
//  var NumElements = buttongroup.children().length;
//  if(State <= 0 || State >= Math.pow(2, maxItems)) State = 1;
//  if(sender !== undefined) {
//    while($(sender).is("button") == false) sender = $(sender).parent();
//    var Index = $(sender).index();
//    if(State == Math.pow(2, Index)) { //only one iten selected
//      //do nothing!, i cant deselect if only one item is selected
//    } else if(State & Math.pow(2, Index)) {
//      State = State & ~(Math.pow(2, Index));
//    } else {
//      State = State | Math.pow(2, Index);
//    }
//    if(unique == true) State = State & (Math.pow(2, Index));  
//    localStorage.setItem(variable, State);
//  }
//  var VisibleSeries = [];
//  var Visibles = [];
//  if(Analyzer.GetSelected().Mode !== '1PHASE') {
//    buttongroup.removeClass('hide');
//    for(var i = 0; i < NumElements; i++) {
//      if(i >= maxItems) {
//        $(buttongroup.children()[i]).addClass('disabled');
//      } else if(State & Math.pow(2, i)) {
//        $(buttongroup.children()[i]).addClass('active');
//        $(buttongroup.children()[i]).removeClass('disabled');
//        VisibleSeries.push(true);
//        Visibles.push(i + 1);
//      } else {
//        $(buttongroup.children()[i]).removeClass('active');
//        $(buttongroup.children()[i]).removeClass('disabled');
//        VisibleSeries.push(false);
//      }
//    }
//  } else {
//    buttongroup.addClass("hide");
//    Visibles.push(1);
//    VisibleSeries.push(true);
//  }
//  if(callback !== undefined) callback(VisibleSeries, Visibles);  
//}
///* if called from a button update variable contents to reflect new selections. In all cases update graph visible series */
//function GraphPhaseButtons(graph, buttongroup, variable, unique, rangechecker, sender) {
//  GroupButtons(buttongroup, variable, graph.attributes_.labels_.length, unique, function(VisibleSeries, Visibles) { graph.updateOptions({ visibility: VisibleSeries, valueRange: rangechecker(Visibles) }); }, sender);
//}
///* Generate db ticks (integers) */ 
//function GraphDBTicks(min, max, offset) {
//  var ticks = [];
//  for(var i = Math.floor(min - offset); i < Math.ceil(max - offset); i++) {
//    if((i % 10) == 0) ticks.push({ v: i + offset, label: i });
//  }
//  return ticks;
//}
///* Generate odd harmonic ticks */
//function GraphOddHarmonics() {
//  var Ticks = [];
//  for(var i = 0; i < Analyzer.GetSeriesNames('Harmonics').length; i = i + 2) Ticks.push({ v: i, label: Analyzer.GetSeriesNames('Harmonics')[i] });
//  return Ticks;
//}
//function GraphWaveTimeTicks(max) {
//  var ticks = [];
//  for(var i = 0; i <= Math.floor(max / Analyzer.GetSelected().SampleFreq * Analyzer.GetSelected().NominalFreq); i++) {
//    ticks.push({ v: i * Analyzer.GetSelected().SampleFreq / Analyzer.GetSelected().NominalFreq, label: Round2(i / Analyzer.GetSelected().NominalFreq * 1000) });
//  }
//  return ticks;
//}
///* find abs max value of array (maybe array of arrays) */
export function ArrayAbsMax(array) {
  var Max = 0;
  for(var i in array) Max = Math.max(Max, Array.isArray(array[i]) ? ArrayMax(array[i]) : Math.abs(array[i]));
  return Max;
}
///* find max value of array (maybe array of arrays) */
//function ArrayMax(array) {
//  var Max = -Infinity;
//  for(var i in array) {
//    Max = Math.max(Max, Array.isArray(array[i]) ? ArrayMax(array[i]) : array[i]);
//  }
//  if(Max === -Infinity) return 0;
//  return Max;
//}
///* find min value of array (maybe array of arrays) */
//function ArrayMin(array) {
//  var Min = +Infinity;
//  for(var i in array) {
//    Min = Math.min(Min, Array.isArray(array[i]) ? ArrayMin(array[i]) : array[i]);
//  }
//  if(Min === +Infinity) return 0;
//  return Min;
//}
///* sum array */
//function ArraySum(array) {
//  var Sum = 0;
//  for(var i = 0; i < array.length; i++) {
//    Sum += array[i];
//  }
//  return Sum;
//}
///* Find the max value of graph series data (array of arrays where first column is time) */
//function ArrayGraphMax(array) {
//  var Max = 0;
//  for(var i = 0; i < array.length; i++) {
//    for(var n = 1; n < array[i].length; n++) {      
//      Max = Math.max(Max, Math.abs(array[i][n]));
//    }
//  }
//  return Max;
//}
///* Scale all values of an array to units ('', 'k', 'M') */
export function ScaleArrayToUnits(array, units) {
  if(units === '') return array;
  for(var i = 0; i < array.length; i++) array[i] = ScaleToUnits(array[i], units);
  return array;
}
//
///* Scale value to units ('', 'k', 'M') */
export function ScaleToUnits(val, units) {
  if(units === '') return val;
  if(units === 'k') {
    return val / 1000;  
  } else if(units === 'M') {
    return val / 1000000;  
  }      
}
///* Find units for high values of energy */
export function GetBestUnits(value) {
  if(Math.abs(value) > 1000000) {
   return 'M';
  } else if(Math.abs(value) > 1000) {
    return 'k';
  } else {
    return '';
  }
}
///* Find the max,min value of graph visible series data (array of arrays where first column is time) */
//function ArrayGraphMinMax(graph, accum) {
//  var Result = {VisibleNames: [], Min: Infinity, Max: -Infinity, MinTime: 0, MaxTime: 0};
//  for(var i = 0; i < graph.attrs_.visibility.length; i++) {
//    if(graph.attrs_.visibility[i] === true) Result.VisibleNames.push(graph.attributes_.labels_[i]);
//  }
//  for(var i = 0; i < graph.rawData_.length; i++) {
//    if(accum === true) {
//      var Value = 0;
//      for(var n = 1; n < graph.rawData_[i].length; n++) {      
//        if(graph.attrs_.visibility[n - 1] === true) Value = Value + graph.rawData_[i][n];
//      }
//      if(Value < Result.Min) {
//        Result.Min = Value;  
//        Result.MinTime = graph.rawData_[i][0];
//      }
//      if(Value > Result.Max) {
//        Result.Max = Value;  
//        Result.MaxTime = graph.rawData_[i][0];
//      }
//    } else {
//      for(var n = 1; n < graph.rawData_[i].length; n++) {      
//        if(graph.attrs_.visibility[n - 1] === true) {
//          var Value = Value + graph.rawData_[i][n];
//          if(Value < Result.Min) {
//            Result.Min = Value;  
//            Result.MinTime = graph.rawData_[i][0];
//          }
//          if(Value > Result.Max) {
//            Result.Max = Value;  
//            Result.MaxTime = graph.rawData_[i][0];
//          }
//        }
//      }
//    }
//  }
//  return Result;
//}
//function ShowColorScale(scale, outerdiv) {
//  scale.out('hex');
//  var width = outerdiv.width() - 1;
//  for(i = 0; i <= width; i++) outerdiv.append('<div style="width:1px; height:' + $(outerdiv).height() + 'px; display:inline; background:' + scale(i / width) + '"></div>');
//}
///* Generate an array of colors from a base */
export function ColorArray(base, items) {
  var Array = [];
  for(var i = 0; i < items; i++) {
   Array.push(base); 
   base = chroma(base).darken();
  }
  return Array;
}
///* Manage legend in graphs */
export function LegendFormatter_Follow(data) {
  if(data.x === undefined || data.x === null) return "";
  var Result = '<span class="dygraph-legend-title">' + data.xHTML + '</span><br>';
  for(var i = 0; i < data.series.length; i++)  {
    if(data.series[i].isVisible) {
      Result = Result + data.series[i].label + ' : ' + data.series[i].yHTML + '<br>';
    }
  }
  return Result;
}
//
//
//
//
//
//
//
///* Añade los flag de eventos a una grafica */
//function AddEvents(result, serieName, graph) {
//  var Events = graph.annotations();
//  for(i = 0; i < result['Sampletime'].length; i++) {
//    if(result['Flag'][i] === false) continue;
//    var EventID = {
//      series: serieName,
//      xval: result['Sampletime'][i],
//      shortText: 'EVT',
//      text: "This aggregation period contains an event"
//    };
//    Events.push(EventID);
//  }
//  graph.setAnnotations(Events);
//}
///* Convierte un array de floats a cadena con dos decimales, separados por el caracter dado y usando los colores que se pongan (añadiendo cadena1 si es negativo y cadena2 si positivo)*/
export function Array_Round2(values, colors, cadena1, cadena2) {
  if(Array.isArray(values) === false) values = [values];
  for(var i = 0; i < values.length; i++) {
    if((colors !== undefined) && (colors[i] !== undefined)) {
     var Text = '<div style="padding: 3px 10px; margin: 0px 5px; border-radius: 3px; color: #FFF; background:' + colors[i] + '; display:inline-block">' + Round2(values[i]);
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
  return values.join('');
}
//
///* Busca el mejor divisor y unidades para un valor de entrada, ya sea B, kB, MB o GB */
//function GetBestUnits2(value) {
//  if(value > 1024 * 1024 * 1024) {
//    return [1024 * 1024 * 1024, 'GB'];
//  } else if(value > 1024 * 1024) {
//    return [1024 * 1024, 'MB'];
//  } else if(value > 1024) {
//    return [1024, 'kB'];
//  } else {
//    return [1, 'B'];
//  }
//}
//
//
//
///* Formato rapido en multiplos de mil si es necesario */
//function FormatKMG(value, unit, unitK, unitsM, unitsG) {
//  if(value > 1000000000) return Round2(value / 1000000000) + unitG; 
//  if(value > 1000000) return Round2(value / 1000000) + unitM; 
//  if(value > 1000) return Round2(value / 1000) + unitK; 
//  return Round2(value) + unit; 
//}
//
///* Convierte un numero a cadena con dos decimales y muestra el valor en los huecos que se pasen */
//function Round2Labels(value, integer, fraction) {
//  if(isNaN(value)) {
//    integer.text('?');
//    fraction.text('');
//  } else {
//    var parts = (value + "").split(".");
//    integer.text(parts[0]);
//    if(parts.length === 1) {
//      fraction.text('.00');
//    } else {
//      parts[1] = parts[1] + "0";
//      fraction.text('.' + parts[1].substr(0, 2));
//    }
//  }
//}
//
///* Retorna un numero como cadena con dos decimales */
export function Round2(value) {
  if(value === null) {
    return '~';
  } else {
    value = Math.round(value * 100);
    var Integer = Math.trunc(value / 100);
    var Fraction = Math.abs(value % 100);
    Fraction = Fraction.toString();
    while(Fraction.length < 2) Fraction = '0' + Fraction;
    return ((value < 0) ? '-' : '') + Math.abs(Integer.toString()) + "<span style='font-size:80%'>." + Fraction + '</span>';
  }
}
//
///* Retorna un numero como cadena con dos decimales pero sin formato html */
//function Round2Plain(value) {
//  if(value === null) {
//    return '-';
//  } else {
//    value = Math.round(value * 100);
//    var Integer = Math.trunc(value / 100);
//    Fraction = Math.abs(value % 100);
//    Fraction = Fraction.toString();
//    while(Fraction.length < 2) Fraction = '0' + Fraction;
//    return Integer.toString() + '.' + Fraction;
//  }
//}
//
///* Guardar array como archivo CSV, incluyendo la cabecera */
//function exportToCsv(filename, rows) {
//  var processRow = function (row) {
//    var finalVal = '';
//    for(var j = 0; j < row.length; j++) {
//      var innerValue = row[j] === null ? '' : row[j].toString();
//      if(row[j] instanceof Date) innerValue = row[j].toLocaleString();
//      var result = innerValue.replace(/"/g, '""');
//      if(result.search(/("|;|\n)/g) >= 0) result = '"' + result + '"';
//      if(j > 0) finalVal += ';';
//      finalVal += result;
//    }
//    return finalVal + '\n';
//  };
//  var csvFile = '';
//  for (var i = 0; i < rows.length; i++) csvFile += processRow(rows[i]);
//  var blob = new Blob([csvFile], {type: 'text/csv;charset=utf-8;'});
//  if (navigator.msSaveBlob) { // IE 10+
//    navigator.msSaveBlob(blob, filename);
//  } else {
//    var link = document.createElement("a");
//    if(link.download !== undefined) { // feature detection
//      var url = URL.createObjectURL(blob);
//      link.setAttribute("href", url);
//      link.setAttribute("download", filename);
//      link.style.visibility = 'hidden';
//      document.body.appendChild(link);
//      link.click();
//      document.body.removeChild(link);
//    }
//  }
//}
//function LoadAnalyzers() {
//
//  $('#btnAnalyzersConfigBack, #btnAnalyzersACLBack, #btnAnalyzersTariffsBack').click(function(event) {
//    $('#dlgAnalyzers > div > div').addClass('hide');
//    $('#divAnalyzers').removeClass('hide');
//  });
//  $('#tblAnalyzers').on('click', 'a', function(event) {
//    event.preventDefault();
//    if($(event.target).is('a')) {
//      $('#dlgAnalyzers > div > div').addClass('hide');
//      if($(event.target).parent('li').index() === 0) {
//        $('#divAnalyzersConfig').removeClass('hide');
//      } else if($(event.target).parent('li').index() === 1) {
//        $('#divAnalyzersACL').removeClass('hide');        
//      } else if($(event.target).parent('li').index() === 2) {
//
//      } else if($(event.target).parent('li').index() === 4) {
//        $('#divAnalyzersMaintenance').removeClass('hide');
//      }
//    }
//  });
//  
//}
///* Load News and show dialog if any */
//function LoadNews() {
//  $('#btnNewsNext').click(LoadNews);
//  $.ajax({ url: "getNews", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ LastLogin: parseInt(localStorage.getItem('LastLogin')) }), success: 
//    function(result) {
//      if(result === null) {
//        $('#dlgNews').modal('hide');
//        return;
//      }
//      $('#dlgNews .modal-header h4').html(result.Header);
//      $('#dlgNews .modal-body').html(result.Contents);
//      $('#dlgNews').modal('show');
//      localStorage.setItem('LastLogin', result.Timestamp + 1);
//    }
//  }); 
//}
//function User_Manager() {
//  var Self = this;
//  Self.Edit = function() {
//  };
//  Self.Logout = function() {
//    window.location.href = 'Logout';
//  };
//  $('#labelUserName').text(localStorage.getItem('UserName'));
//  if(localStorage.getItem('UserName') === "admin") $('#menuSettings').removeClass('hide');
//  $('#btnUserEdit').click(Self.Edit);
//  $('#btnUserLogout').click(Self.Logout);
//};
//function Analyzer_Manager() {
//  var Self = this;
//  Self.AddAnalyzers = function(result) {
//    if(result.length === 0) return;
//    $('#btnAnalyzerSearch').removeClass('disabled');
//    Self.AvaliableAnalyzers = result;
////    for(var i = 0; i < Self.AvaliableAnalyzers.length; i++) {
////      var RowText = '<tr' + ((i === Self.CurrentAnalyzerIndex) ? ' class="info"' : '') + '>' +
////                    '<td>' + ((result[i].Online === true) ? 'ONLINE' : 'OFFLINE') + '</td>' +
////                    '<td>' + result[i].Name + '</td>' +
////                    '<td>' + result[i].Mode + '</td>' +
////                    '<td><div class="btn-group" data-analyzer="' + result[i].ID + '">' +
////                    '<button type="button" class="btn btn-success btn-xs' + ((i === Self.CurrentAnalyzerIndex) ? ' disabled' : '') + '">Select</button><button type="button" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false"><span class="caret"></span></button>' +
////                    '<ul class="dropdown-menu pull-right"><li><a href="#">Configure</a></li><li><a href="#">ACL</a></li><li><a href="#">Tariff</a></li><li><a href="#">Alerts</a></li><li role="separator" class="divider"></li><li><a href="#">Maintenance</a></li></ul>' +
////                    '</div></td>' +
////                    '</tr>';
////      Self.TableAnalyzers.row.add($(RowText)).draw();
////    }
//    if(Self.AvaliableAnalyzers[Self.CurrentAnalyzerIndex] === undefined) {
//      if(Self.AvaliableAnalyzers.length !== 0) $('#dlgAnalyzers').modal('show');
//      $('#NodeTime').text('Not connected!');
//    } else {
//      $('#labelAnalyzerName').text(Self.AvaliableAnalyzers[Self.CurrentAnalyzerIndex].Name);
//      $('#AnalyzerMenu').removeClass('hide');
//      $('#AnalyzerNavHeader').removeClass('hide');
//      $('.indicator-bar').removeClass('hide');
//      Self.GetIndicators();
//      Self.GetIndicatorsB();
//      Self.GetSysStats();
//    }
//  };
//  Self.Search = function() {
//    $('#dlgAnalyzers > div > div').addClass('hide');
//    $('#divAnalyzers').removeClass('hide');
//    $('#dlgAnalyzers').modal('show');
//    Self.TableAnalyzers.columns.adjust();
//  };
//  Self.Configure = function(serial) {
//    alert("Coinfigure not yet implemented!");
//  };
//  Self.ACL = function(serial) {
//    alert("ACL not yet implemented!");
//  };
//  Self.Tariffs = function(serial) {
//    $('#dlgAnalyzers > div > div').addClass('hide');
//    $('#divAnalyzersTariff').removeClass('hide');
//    $('#divAnalyzersTariffSpinner').removeClass('hide');
//    Self.TableTariffs.clear();
//    $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify( { Analyzer: Self.GetSelected().ID } ), 
//      success: function(result) {
//        for(var i = 0; i < result.length; i++) {
//          var RowText = '<tr>' +
//                        '<td>' + result[i].Name + '</td>' +
//                        '<td data-sort="' + result[i].StartDate + '">' + moment(result[i].StartDate).format('DD/MM/YYYY HH:mm:ss') + '</td>' +
//                        '<td data-sort="' + result[i].EndDate + '">' + moment(result[i].EndDate).format('DD/MM/YYYY HH:mm:ss') + '</td>' +
//                        '<td data-tariff="' + result[i].RateID + '"><div class="btn-group">' +
//                        '<button type="button" class="btn btn-default btn-xs">Edit</button><button type="button" class="btn btn-danger btn-xs">Delete</button>' +
//                        '</div></td>' +
//                        '</tr>';
//          Self.TableTariffs.row.add($(RowText)).draw();
//        }
//        Self.AvaliableTariffs = result;
//        Self.TableTariffs.columns.adjust();
//        $('#divAnalyzersTariffSpinner').addClass('hide');
//      }
//    });
//    Self.OnEditAnalyzer = serial;
//  };
//  Self.TariffDelete = function(id) {
//    alert("Not yet implemented!");
//  };
//  Self.TariffUnselect = function() {
//    $('#inputAnalyzerTariffStartDate').data("DateTimePicker").disable();
//    $('#inputAnalyzerTariffEndDate').data("DateTimePicker").disable();
//    $('#inputAnalyzerTariffStartDate').data("DateTimePicker").clear();
//    $('#inputAnalyzerTariffEndDate').data("DateTimePicker").clear();
//    $('#inputAnalyzerTariffComments').prop('disabled');
//    $('#inputAnalyzerTariffComments').html('');
//    $('#divAnalyzerTariffPeriods').html('');
//    Self.TariffValidate();
//  };
//  Self.TariffSelect = function(id) {
//    Self.OnEditTariff = Self.OnEditTariffCurrent;
//    if(id !== 'CURRENT') $.ajax({ url: "getTariffs", timeout: 20000, async: false, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ RateID: parseInt(id) }), success: function(result) { Self.OnEditTariff = result; } });
//    Self.OnEditTariff.RateID = (Self.OnEditTariffCurrent !== null) ? Self.OnEditTariffCurrent.RateID : null;
//    $('#inputAnalyzerTariffStartDate').data("DateTimePicker").enable();
//    $('#inputAnalyzerTariffEndDate').data("DateTimePicker").enable();
//    $('#inputAnalyzerTariffStartDate').data("DateTimePicker").date(moment(Self.OnEditTariff.StartDate));
//    $('#inputAnalyzerTariffEndDate').data("DateTimePicker").date(moment(Self.OnEditTariff.EndDate));
//    $('#inputAnalyzerTariffComments').removeProp('disabled');
//    $('#inputAnalyzerTariffComments').html(Self.OnEditTariff.Details.Description);
//    $('#divAnalyzerTariffPeriods').html('');
//    $('#divAnalyzerTariffPeriods').html('<div class="col-xs-12"><label class="control-label">Period settings</label></div>');
//    for(var i = 0; i < Self.OnEditTariff.Details.Periods.length; i++) {
//      $('#divAnalyzerTariffPeriods').append(
//        '<div class="col-xs-6"><div class="input-group input-group-xs"><span class="input-group-addon">P' + (i + 1) + '</span><input type="text" class="form-control" value="0.0"><span class="input-group-addon">kW</span></div></div>' +
//        '<div class="col-xs-6"><div class="input-group input-group-xs"><span class="input-group-addon">P' + (i + 1) + '</span><input type="text" class="form-control" value="0.0"><span class="input-group-addon">€/kW/day</span></div></div>' +
//        '<div class="col-xs-12" style="height: 2px"></div>');
//    }
//    Self.TariffValidate();
//  };
//  Self.TariffEdit = function(id) {
//    $('#dlgAnalyzers > div > div').addClass('hide');
//    $('#divAnalyzersTariffEdit').removeClass('hide');
//    $('#inputAnalyzerTariffStartDate').datetimepicker({ format: 'DD/MM/YYYY', viewMode: 'months' });
//    $('#inputAnalyzerTariffEndDate').datetimepicker({ format: 'DD/MM/YYYY', viewMode: 'months' });
//    Self.OnEditTariff = { RateID: null, StartDate: null, EndDate: null, Details: { Description: '', Periods: [] } };
//    Self.OnEditTariffCurrent = null;
//    Self.TariffUnselect();
//    if(id !== null) {
//      $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", async: false, dataType: "json", contentType: 'application/json', data: JSON.stringify({ RateID: parseInt(id), Analyzer: Self.OnEditAnalyzer}), success: function(result) { Self.OnEditTariff = result; }});
//      Self.OnEditTariffCurrent = Self.OnEditTariff;
//    }
//    $("#inputAnalyzerTariffBase").bsSuggest("destroy");
//    var Data = [];
//    if(Self.OnEditTariffCurrent !== null) {
//      Data.push({id: "CURRENT", name: Self.OnEditTariffCurrent.Name + ' (Copy)', description: 'From ' + moment(Self.OnEditTariffCurrent.StartDate).format('DD/MM/YYYY') + ' to ' + moment(Self.OnEditTariffCurrent.EndDate).format('DD/MM/YYYY')});
//      $("#inputAnalyzerTariffBase").attr('data-id', 'CURRENT');
//      $("#inputAnalyzerTariffBase").val(Data[0].name);    
//    } else {
//      $("#inputAnalyzerTariffBase").attr('data-id', '');
//      $("#inputAnalyzerTariffBase").val('');
//    }
//    $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", async: false, dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), success: function(result) { for(var i = 0; i < result.length; i++) Data.push({ id: result[i].RateID, name: result[i].Name, description: 'From ' + moment(result[i].StartDate).format('DD/MM/YYYY') + ' to ' + moment(result[i].EndDate).format('DD/MM/YYYY')}); } });
//    if(Self.OnEditTariffCurrent !== null) Self.TariffSelect('CURRENT');
//    $("#inputAnalyzerTariffBase").bsSuggest( { hideOnSelect: true, effectiveFields: ['name', 'description'], showHeader: false, ignorecase: true, indexId: 0, indexKey: 1, data: { value: Data } } );    
//    $("#inputAnalyzerTariffBase").on('onSetSelectValue', function (e, keyword) { Self.TariffSelect(keyword.id); });
//    $("#inputAnalyzerTariffBase").on('onUnsetSelectValue', function (e) { Self.TariffUnselect(); });
//    $('#divAnalyzersTariffEditSpinner').addClass('hide');
//    Self.TariffValidate();
//  };
//  Self.TariffValidate = function() {
//    $('#divAnalyzersTariffSuccess, #divAnalyzersTariffError').addClass('hide');
//    $('#btnAnalyzersTariffSave').addClass('disabled');
//    if($('#inputAnalyzerTariffBase').attr('data-id') == '') {
//      $('#divAnalyzersTariffError').text('Choose a tariff base!').removeClass('hide');
//      return;
//    }
//    var Overlap = false;
//    for(var i = 0; i < Self.AvaliableTariffs.length; i++) {
//      if(Self.OnEditTariff.RateID === Self.AvaliableTariffs[i].RateID) continue;
//      if((Self.OnEditTariff.StartDate < Self.AvaliableTariffs[i].EndDate) && (Self.AvaliableTariffs[i].StartDate < Self.OnEditTariff.EndDate)) Overlap = true
//    }
//    if(Overlap) {
//      $('#divAnalyzersTariffError').text('Defined tariff period overlaps with other assigned tariffs').removeClass('hide');
//      return;
//    }
//    $('#divAnalyzersTariffSuccess').text('Tariff is correctly configured').removeClass('hide');
//    $('#btnAnalyzersTariffSave').removeClass('disabled');
//  };
//  Self.TariffSave = function() {
//    
//  };  
//  Self.Maintenance = function(serial) {
//    alert("Maintenance not yet implemented!");
//  };
//  Self.GetSelected = function() {
//    return Self.AvaliableAnalyzers[Self.CurrentAnalyzerIndex];
//  };
//  Self.SetSelected = function(index) {
//    localStorage.setItem('SelAnalyzer', index);
//    location.reload();
//  };
//  Self.GetSeries = function(period, to, aggreg, lastsample, series, onSuccess, onCompletion, onError) {
//    return $.ajax({ url: "getSeries", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion, erro: onError, data: JSON.stringify( { Period: period, To: to, Aggreg: aggreg, LastSample: lastsample, Series: series, Analyzer: Self.GetSelected().ID } ) }); 
//  };
//  Self.GetSeriesNow = function(series, onSuccess, onCompletion, onError) { 
//    return $.ajax({ url: "getSeriesNow", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion, error: onError, data: JSON.stringify( { Series: series, Analyzer: Self.GetSelected().ID } ) }); 
//  };
//  Self.GetSeriesRange = function(aggreg, onSuccess, onCompletion, onError) { 
//    return $.ajax({ url: "getSeriesRange", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion, error: onError, data: JSON.stringify({ Aggreg: aggreg, Analyzer: Self.GetSelected().ID }) }); 
//  };
//  Self.GetSeriesAggreg = function(period, to, aggreg, series, onSuccess, onCompletion, onError) { 
//    return $.ajax({ url: "getSeriesAggreg", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion, error: onError, data: JSON.stringify( { Period: period, To: to, Aggreg: aggreg, Series: series, Analyzer: Self.GetSelected().ID } ) }); 
//  };
//  Self.GetEvents = function(period, to, lastevent, onSuccess, onCompletion, onError) { 
//    return $.ajax({ url: "getEvents", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion, error: onError, data: JSON.stringify( { Period: period, To: to, LastEvent: lastevent, Analyzer: Self.GetSelected().ID } ) }); 
//  };
//  Self.GetPrices = function(period, to, lastsample, onSuccess, onCompletion, onError) {
//    return $.ajax({ url: "getPrices", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', success: onSuccess, complete: onCompletion, error: onError, data: JSON.stringify( { Period: period, To: to, LastSample: lastsample, Analyzer: Self.GetSelected().ID } ) }); 
//  };
//  Self.GetIndicators = function() {
//    Self.GetSeriesNow(['VOLTAGE', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY'], 
//      function(result) {
//        vm.$refs.indicator_bar.setValue(result);
////        vm.$refs.voltage_indicator.setValue(result['Voltage']);
////        vm.$refs.current_indicator.setValue(result['Current']);
////        vm.$refs.power_indicator.setValue(result['Active_Power']);
////        vm.$refs.frequency_indicator.setValue([result['Frequency']]);
//        var Time = new Date(result['Sampletime']);
//        $('#NodeTime').text(Time.toLocaleTimeString() + ' - ' + Time.toLocaleDateString());
//      },
//      function() { 
//        setTimeout(Self.GetIndicators, 5000); 
//      }, 
//      function() {
//        vm.$refs.indicator_bar.setValue(null);
//        $('NodeTime').text('Offline');   
//      }
//    );
//  };
//  Self.GetIndicatorsB = function() {
//    var Times = GetDayTimePeriod();
//    Self.GetSeriesAggreg(Times.PrimaryPeriod, Times.PrimaryTo, Times.PreferedAggreg, "sum(active_energy)", 
//      function(result) {
//        var Units = GetBestUnits(ArrayMax(result['sum_active_energy']));
//        $('#IndicatorActiveEnergyUnits').text(Units + 'Wh');
//        $('#IndicatorTodayEnergyValues').html("");
//        result['sum_active_energy'] = ScaleArrayToUnits(result['sum_active_energy'], Units);
//        for(i = 0; i < result['sum_active_energy'].length; i++) {
//          $('#IndicatorTodayEnergyValues').append('<span class="value">' + Round2(result['sum_active_energy'][i]) + '</span>');
//        }
//      },
//      function() {
//        setTimeout(Self.GetIndicatorsB, 60000);
//      },
//      function() {
//        $('EnergyIndicator').text('Offline');
//      }
//    );
//  };
//  Self.GetSysStats = function() {
//    $.ajax({ url: "getSysInfo", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Type: 'Performance' }),
//      success: function(result) {
//        var Status = [];
//        if(result['CPU_Usage'] != undefined) {
//          if(result['CPU_Count'] != undefined) {
//            Status.push(Math.round(result['CPU_Usage'] / result['CPU_Count'] * 100) + '%(CPU)');
//          } else {
//            Status.push(result['CPU_Usage'] + '%(CPU)');
//          }
//        }
//        if((result['RAM_Total'] != undefined) && (result['RAM_Total'] != undefined)) {
//          Status.push(Math.round((result['RAM_Total'] - result['RAM_Free']) / result['RAM_Total'] * 100) + '%(RAM)');
//        }
//        if((result['Swap_Total'] != undefined) && (result['Swap_Free'] != undefined)) {      
//          Status.push(Math.round((result['Swap_Total'] - result['Swap_Free']) / result['Swap_Total'] * 100) + '%(Swap)');
//        }
//        if(Status.length == 0) {
//          $('#StatusRow').css('display', 'none');
//        } else {
//          $('#StatusRow').css('display', 'inherited');
//          $('#UsageText').text(Status.join(' - '));
//        }
//  //      $('#CPUPercent').css('width', );
//  //      $('#CPUText').html(Round2(result['CPU_Usage'] / result['CPU_Count'] * 100) + '%');
//  //      var Units = GetBestUnits(result['RAM_Total']);
//  //      $('#RAMPercent').css('width', (result['RAM_Total'] - result['RAM_Free']) / result['RAM_Total'] * 100 + '%');
//  //      $('#RAMText').html(Round2((result['RAM_Total'] - result['RAM_Free']) / Units[0]) + '/' + Round2(result['RAM_Total'] / Units[0]) + '<em>&nbsp;' + Units[1] + '</em>');
//  //      Units = GetBestUnits(result['Swap_Total']);
//  //      $('#SwapPercent').css('width', (result['Swap_Total'] - result['Swap_Free']) / result['Swap_Total'] * 100 + '%');
//  //      $('#SwapText').html(Round2((result['Swap_Total'] - result['Swap_Free']) / Units[0]) + '/' + Round2(result['Swap_Total'] / Units[0]) + '<em>&nbsp;' + Units[1] + '</em>');
//  //      Units = GetBestUnits(result['Disk_Total']);
//        $('#DiskPercent').css('width', (result['Disk_Total'] - result['Disk_Free']) / result['Disk_Total'] * 100 + '%');
//  //      $('#DiskText').html(Round2((result['Disk_Total'] - result['Disk_Free']) / Units[0]) + '/' + Round2(result['Disk_Total'] / Units[0]) + '<em>&nbsp;' + Units[1] + '</em>');
//        $('#UptimeText').html(moment.duration(result['UpTime'], "seconds").format("d [days] h [hours] m [minutes]"));
//        $('#BatteryPercent').css('width', result['Battery_Percent'] + '%');
//  //      $('#BatteryText').html(Round2(result['Battery_Percent']) + '%');
//      },
//      complete: function() {
//        setTimeout(Self.GetSysStats, 5000);
//      }
//    });  
//  };
//  Self.GetSeriesNames = function(varName) {
//    if(varName === 'Harmonics') return ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th', '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th', '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th', '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th'];
//    if(varName === 'Voltage')   return (Self.GetSelected().Mode === '1PHASE') ? ['V'] : ['V(R)', 'V(S)', 'V(T)'];
//    if(varName === 'Current')   return (Self.GetSelected().Mode === '1PHASE') ? ['I'] : (Self.GetSelected().Mode === '3PHASE') ? ['I(R)', 'I(S)', 'I(T)'] : ['I(R)', 'I(S)', 'I(T)', 'I(N)'];
//    if(varName === 'Active_Energy') return (Self.GetSelected().Mode === '1PHASE') ? ['P'] : ['P(R)', 'P(S)', 'P(T)'];
//    if(varName === 'Reactive_Energy') return (Self.GetSelected().Mode === '1PHASE') ? ['Q'] : ['Q(R)', 'Q(S)', 'Q(T)'];
//    if(varName === 'Active_Power') return (Self.GetSelected().Mode === '1PHASE') ? ['P'] : ['P(R)', 'P(S)', 'P(T)'];
//    if(varName === 'Reactive_Power') return (Self.GetSelected().Mode === '1PHASE') ? ['Q'] : ['Q(R)', 'Q(S)', 'Q(T)'];
//    if(varName === 'Power_Factor')  return (Self.GetSelected().Mode === '1PHASE') ? ['PF'] : ['PF(R)', 'PF(S)', 'PF(T)'];
//    if(varName === 'Voltage_THD')  return (Self.GetSelected().Mode === '1PHASE') ? ['THDv'] : ['THDv(R)', 'THDv(S)', 'THDv(T)'];
//    if(varName === 'Current_THD')  return (Self.GetSelected().Mode === '1PHASE') ? ['THDi'] : (Self.GetSelected().Mode) ? ['THDi(R)', 'THDi(S)', 'THDi(T)'] : ['THDi(R)', 'THDi(S)', 'THDi(T)', 'THDi(N)'];
//    if(varName === 'Phi')  return (Self.GetSelected().Mode === '1PHASE') ? ['PHI']  : ['PHI(R)', 'PHI(S)', 'PHI(T)'];
//  };
//  Self.AvaliableAnalyzers = [];
//  Self.CurrentAnalyzerIndex = parseInt(localStorage.getItem("SelAnalyzer"));
////  Self.TableAnalyzers = $('#tblAnalyzers').DataTable({ responsive: false, paging: false, scrollCollapse: true, dom: '<"top float-left"i><"top float-right"f><"clear">t<"clear">p', language: { info: "_MAX_ avaliable devices", infoEmpty: "No avaliable devices" }, columnDefs: [ { targets: [0], orderable: false, searchable: false }, { targets: [3], orderable: false, searchable: false }, { targets: [0, 2], className: 'text-center' } ], order: [1, 'desc'] });
//  Self.TableTariffs = $('#tblAnalyzersTariff').DataTable({ responsive: false, paging: false, scrollCollapse: true, dom: '<"top float-left"i><"top float-right"f><"clear">t<"clear">p', language: { info: "_MAX_ avaliable tarifs", infoEmpty: "No configured tariffs"}, columnDefs: [ { targets: 0, orderable: true, searchable: true }, { targets: [1, 2], orderable: true, className: 'text-center' }, { targets: 3, width: "80px", orderable: false, searchable: false }], order: [2, 'desc'] });
//  $.ajax({ url: "getAnalyzers", timeout: 20000, async: false, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), success: this.AddAnalyzers });
//  $('#btnAnalyzerSearch').click(function() {vm_analyzer.$refs.AnalyzersDialog.Show(Self.AvaliableAnalyzers); /*Self.Search*/ });
//  $('#tblAnalyzers').on('click', 'button:first-child', function() { Self.SetSelected(TableAnalyzers.row($(this).parents('tr')).node()._DT_RowIndex); } );
//  $('#tblAnalyzers').on('click', 'li:nth-child(1) > a', function(event) { event.preventDefault(); Self.Configure($(this).parents('div').attr('data-analyzer')); } );
//  $('#tblAnalyzers').on('click', 'li:nth-child(2) > a', function(event) { event.preventDefault(); Self.ACL($(this).parents('div').attr('data-analyzer')); } );
//  $('#tblAnalyzers').on('click', 'li:nth-child(3) > a', function(event) { event.preventDefault(); Self.Tariffs($(this).parents('div').attr('data-analyzer')); } );
//  
//  $('#tblAnalyzers').on('click', 'li:nth-child(6) > a', function(event) { event.preventDefault(); Self.Maintenance($(this).parents('div').attr('data-analyzer')); } );
//  $('#tblAnalyzersTariff').on('click', 'button:nth-child(1)', function(event) { Self.TariffEdit($(this).parents('td').attr('data-tariff')); } );
//  $('#tblAnalyzersTariff').on('click', 'button:nth-child(2)', function(event) { Self.TariffDelete($(this).parents('td').attr('data-tariff')); } );
//  $('#btnAnalyzersTariffsBack').click(Self.Search);
//  $('#btnAnalyzersTariffBack').click(function() { Self.Tariffs(Self.OnEditAnalyzer); });
//  $('#btnAnalyzersTariffsAdd').click(function() { Self.TariffEdit(null); });
//  $('#btnAnalyzersTariffSave').click(Self.TariffSave);
//}
//// JS entry point for all page loading
//if((localStorage.getItem('SelAnalyzer') === null) && (window.location.pathname !== '/Settings')) { window.location.href = 'Settings'; }
//$.ajax( { url: 'Common.html', async: false, success: 
//  function(result) {
//    var Template = $('<div></div>').append(result.trim());
//    $('*[data-load-block]').each( function(index) { $(this).html(Template.children('#' + $(this).data('loadBlock')).html()); } );        
//  } 
//});
//$('.spinnerdiv').each(function(index, element) { new Spinner(SpinnerOpts).spin(element); });
//$('[data-sidenav]').sidenav();
//$('ul.sidenav-menu li a').filter( function() { return this.href === window.location.href; }).addClass('active').parent().parent().show().parent().children('a').addClass('active');            
//var User = new User_Manager();
//var Analyzer = new Analyzer_Manager();
//
//
////LoadNews();
//UpdatePage();
//
//
//httpVueLoader.register(Vue, 'Components/NavbarIndicatorsPanel.vue');
//httpVueLoader.register(Vue, 'Components/NavbarIndicators.vue');
//httpVueLoader.register(Vue, 'Components/AnalyzersDialog.vue');
//
//
//      var vm = new Vue({
//        el: '#app'
//
//      });
//
//      var vm_analyzer = new Vue({
//        el: '#analyzer-app'
//
//      });
//
//
///* Obtiene fecha del inicio de una semana dando el numero y año */
//function GetDateOfISOWeek(w, y) {
//  var simple = new Date(y, 0, 1 + (w - 1) * 7);
//  var dow = simple.getDay();
//  var ISOweekStart = simple;
//  if (dow <= 4)
//    ISOweekStart.setDate(simple.getDate() - simple.getDay() + 1);
//  else
//    ISOweekStart.setDate(simple.getDate() + 8 - simple.getDay());
//  return ISOweekStart;
//}
//
///* Obtiene el numero de dia dentro del año de un timestamp */
//function GetDayOfYear(timestamp) {
//  var now = new Date(timestamp);
//  var start = new Date(now.getFullYear(), 0, 0);
//  var diff = (now - start);
//  var oneDay = 1000 * 60 * 60 * 24;
//  var day = Math.floor(diff / oneDay);
//  return day;
//}
//
///* Obtiene el numero de semana dentro del año de un timestamp */
//function GetWeekOfYear(timestamp) {
//  var date = new Date(timestamp);
//  date.setHours(0, 0, 0, 0);
//  date.setDate(date.getDate() + 3 - (date.getDay() + 6) % 7);
//  var week1 = new Date(date.getFullYear(), 0, 4);
//  return 1 + Math.round(((date.getTime() - week1.getTime()) / 86400000 - 3 + (week1.getDay() + 6) % 7) / 7);
//}
//
///* Obtiene el timestamp del inicio del dia actual */
//function GetDayTimestamp() {
//  var now = new Date();
//  return new Date(now.getFullYear(), now.getMonth(), now.getDate(), 0, 0, 0, 0).getTime();
//}
//
///* Obtiene el timestamp del inicio del mes actual */
//function GetMonthTimestamp() {
//  var now = new Date();
//  return new Date(now.getFullYear(), now.getMonth(), 1, 0, 0, 0, 0).getTime();
//}
//
///* Obtiene el timestamp del inicio del año actual */
//function GetYearTimestamp() {
//  var now = new Date();
//  return new Date(now.getFullYear(), 0, 1, 0, 0, 0, 0).getTime();
//}
//
//function GetEndYearTimestamp() {
//  var now = new Date();
//  return new Date(now.getFullYear() + 1, 0, 1, 0, 0, 0, 0).getTime();
//}
//
///* Calcula rango para las graficas, la entrada es un array y un array de columnas a comprobar */
//function CalcRanges(input, cols, fromzero, onlypositive, sum, noOffset) {
//  var UpperRange = -Infinity;
//  var LowerRange =  Infinity;
//  for(i = 0; i < input.length; i++) {
//    if(sum == true) {
//      var Sum = 0; 
//      for(n = 0; n < cols.length; n++) {
//        if(input[i][cols[n]] === null) continue;
//        Sum += input[i][cols[n]];
//      }
//      if(Sum > UpperRange) UpperRange = Sum;
//      if(Sum < LowerRange) LowerRange = Sum;
//    } else {
//      for(n = 0; n < cols.length; n++) {
//        if(input[i][cols[n]] === null) continue;
//        if(input[i][cols[n]] > UpperRange) UpperRange = input[i][cols[n]];
//        if(input[i][cols[n]] < LowerRange) LowerRange = input[i][cols[n]];
//      }
//    }
//  }
//  if((UpperRange === -Infinity) && (LowerRange === Infinity)) {
//    LowerRange = 0;
//    UpperRange = 0;
//  }
//  if((noOffset === undefined) || (noOffset === false)) {
//    var Variation = (UpperRange - LowerRange);
//    if(Variation < 1) Variation = 1.0;
//    if((fromzero === true) && (UpperRange >= 0) && (LowerRange >= 0)) {
//      UpperRange = UpperRange + Variation * 0.1;
//      LowerRange = 0;
//    } else if((fromzero === true) && (UpperRange <= 0) && (LowerRange <= 0)) {
//      UpperRange = 0;
//      LowerRange = LowerRange - Variation * 0.1;
//    } else {
//      UpperRange = UpperRange + Variation * 0.1;
//      LowerRange = LowerRange - Variation * 0.1;
//    }
//  }
//  if((onlypositive === true) && (LowerRange < 0)) {
//    LowerRange = 0;
//  }
//  return [LowerRange, UpperRange];  
//}
//
///* Divide por 1000 los valores de entrada de un array */
//function Div1000(input) {
//  var Result = [];
//  for(i = 0; i < input.length; i++) {
//    Result[i] = [input[i][0]];
//    for(n = 1; n < input[i].length; n++) Result[i][n] = input[i][n] / 1000.0;
//  }
//  return Result;
//}
//
//
//
//
//
//
//
//
//
//
//
//var energyUnit = 'kWh'
//var today = new Date();
////$('#side-menu').metisMenu();
////Loads the correct sidebar on window load,
////collapses the sidebar on window resize.
//// Sets the min-height of #page-wrapper to window size
//$(window).bind("load resize", function () {
//    topOffset = 50;
//    width = (this.window.innerWidth > 0) ? this.window.innerWidth : this.screen.width;
//    if (width < 768) {
//        $('div.navbar-collapse').addClass('collapse');
//        topOffset = 100; // 2-row-menu
//    }
//
//    height = ((this.window.innerHeight > 0) ? this.window.innerHeight : this.screen.height) - 1;
//    height = height - topOffset;
//    if (height < 1)
//        height = 1;
//    if (height > topOffset) {
//        $("#page-wrapper").css("min-height", (height) + "px");
//    }
//});
//var url = window.location;
//var element = $('ul.nav a').filter(function () {
//    return this.href == url || url.href.indexOf(this.href) == 0;
//}).addClass('active').parent().parent().addClass('in').parent();
//if (element.is('div')) {
//    element.addClass('active');
//} else {
//    $('#side-menu li > a').first().addClass('active');
//}
//
//$('.maximize').click(function () {
//    var panel = $(this).parents('.panel');
//    panel.addClass('maximized');
//    $(this).hide();
//    $('.minimize', panel).show();
//    window.dispatchEvent(new Event('resize'));
//});
//$('.minimize').click(function () {
//    var panel = $(this).parents('.panel');
//    panel.removeClass('maximized');
//    $(this).hide();
//    $('.maximize', panel).show();
//    window.dispatchEvent(new Event('resize'));
//});
//$('#side-menu a').click(function () {
////    $(this).addClass('cursor-wait');
////    $('body').addClass('cursor-wait');
//});
//function getStringEventType(type) {
//    switch (type) {
//        case 'INTERRUPTION':
//            return "Interruption";
//        case 'DIP':
//            return "Dip";
//        case 'SWELL':
//            return "Swell";
//        case 'RVC':
//            return "RVC";
//        case 'UNKNOWN':
//            return "Unknown";
//    }
//}
//
//function getStringEventNum(type) {
//    switch (type) {
//        case 'INTERRUPTION':
//            return 0;
//        case 'DIP':
//            return 1;
//        case 'SWELL':
//            return 2;
//        case 'RVC':
//            return 3;
//        case 'UNKNOWN':
//            return 4;
//    }
//}
//
//function getIconEventType(type) {
//    switch (type) {
//        case 'SWELL':
//            return 'fa-level-up';
//        case 'DIP':
//            return 'fa-level-down';
//        case 'INTERRUPTION':
//            return "fa-window-close-o";
//        case 'RVC':
//            return 'fa-line-chart';
//        case 'UNKNOWN':
//            return 'fa-question';
//    }
//}
//
//function getStringEventTypeShort(type) {
//    switch (type) {
//        case 0:
//            return "I";
//        case 1:
//            return "D";
//        case 2:
//            return "S";
//        case 3:
//            return "R";
//        case 4:
//            return "U";
//    }
//}
//function getStringEventTypeByNum(type) {
//    switch (type) {
//        case 0:
//            return "Interruption";
//        case 1:
//            return "Dip";
//        case 2:
//            return "Swell";
//        case 3:
//            return "RVC";
//        case 4:
//            return "UNKNOWN";
//    }
//}
//
//function ajaxGetLastEvents() {
//    var request = $.ajax({
//        url: "events/ajaxGetLastEvents",
//        timeout: 20000,
//        type: "POST",
//        dataType: "json"
//    });
//    return request;
//}
//
//function generateEventAlertBox(event, timestamp) {
//    var newEvent = "";
//    if (event.timestamp > timestamp) {
//        newEvent = "new_event";
//    }
//    var startTime = new Date(+event.timestamp);
//    if (event.end_time != -1) {
//        var duration = formatTimeUnit(event.duration * 1e6);
//    } else {
//        duration = 'In progress...'
//    }
//    var diff = (Math.abs(event.voltage - 230)).toFixed(2);
//    var percent = (100 - event.percent).toFixed(2);
//    return '<li class="event_alert ' + newEvent + '">\
//                <div>\
//                    <p><i class="fa ' + getIconEventType(event.type) + ' fa-fw"></i> ' + getStringEventType(event.type) + '\
//                    <span class="pull-right text-muted small">' + (startTime.toLocaleString() + '.' + startTime.getMilliseconds()) + '</span></p>\
//                    <div class="row small">\
//                        <div class="col col-md-4">\
//                            <em>Duration:</em><br> ' + duration + '\
//                        </div>\
//                        <div class="col col-md-5">\
//                            <em>ΔU<sub>max</sub>:</em> ' + diff + ' V<br>(' + percent + '%)\
//                        </div>\
//                        <div class="col col-md-3">\
//                            <a class="btn btn-primary btn-sm" href="events/disturbances/' + (+event.timestamp) + '" alt="See details..." title="See details..."><i class="fa fa-search-plus fa-fw"></i></a>\
//                        </div>\
//                    </div>\
//                </div>\
//            </li>\
//            <li class="divider"></li>';
//}
//
//$('#eventsDropdown').on('show.bs.dropdown', function () {
//    var dropdown = $('.dropdown-alerts', this).hide();
//    var request = ajaxGetLastEvents();
//    request.done(function (data) {
//        var html = '';
//        var timestamp = data.timestamp;
//        for (var i in data.events) {
//            //for (var j in data.events[i])
////                var generate = true;
////            if (i > 0) {
////                if (data.events[i][j - 1].time == data.events[i][j].time && data.events[i][j].end_time == -1) {
////                    generate = false;
////                }
////            }
////            if (generate) {
//            html += generateEventAlertBox(data.events[i], timestamp);
////            }
//        }
//        if (html != '') {
//            $('#eventsDropdown .dropdown-alerts').prepend(html);
//        }
//        $('#eventsDropdown .dropdown-alerts').slideDown(200);
//    })
//})
//
//$('#eventsDropdown').on('hidden.bs.dropdown', function () {
//    $('#eventsDropdown .event_alert').remove();
//    $('#eventsDropdown .divider').remove();
//})
////1446