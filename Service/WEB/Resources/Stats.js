/*
 * Copyright (C) 2015 Pedro Sánchez Alguacil
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

;(function($) {
  $.fn.calendar.dates['custom'] = {
	  days: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"],
		daysShort: ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"],
		daysMin: ["Su", "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"],
		months: ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"],
		monthsShort: ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"],
		weekShort: 'W',
		weekStart: 1
	};
}(jQuery));
var Calendar = null;
var WeekTable = null;
var MonthTable = null;
var WeekDayTable = null;
var DayArray   = [];
var WeekDayArray  = [];
var WeekEndArray  = [];
var MonthWeekDayArray = [];
var MonthWeekEndArray = [];
var WeekDayNumArray = [];
var WeekDayCountArray = [];
var Spinner = new Spinner(SpinnerOpts);
function GetDataRanges() {
  ShowSpinner(Spinner, 'StatsSpinner');
  $.ajax({ url: "getSamplesRange",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Aggreg: '1h', Node: Node}),
           success: function(result) {
             for(year = new Date(result['From']).getFullYear(); year <= new Date(result['To']).getFullYear(); year++) $('#YearSelector').append('<button type="button" class="btn btn-default btn-xs" id="btnSelYear' + year + '" onclick="AjaxGetEnergy(' + year + ')">' + year + '</button>');
             AjaxGetEnergy(new Date(result['To']).getFullYear());
           }
  }); 
}
function AjaxGetEnergy(selected_year) {
  $('#YearSelector button').removeClass('active');
  $('#btnSelYear' + selected_year).addClass('active');
  ShowSpinner(Spinner, 'StatsSpinner');
  var To = new Date(Date.UTC(selected_year, 11, 31, 23, 59, 59, 999)).getTime();
  var From = new Date(Date.UTC(selected_year, 0, 1, 0, 0, 0, 0)).getTime();
  $.ajax({ url: "getSeries",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: To - From, To: To, Aggreg: '1h', LastSample: 0, Series: 'active_energy', Node: Node}),
           success: function(result) {
             DayArray   = [];
             WeekDayArray  = [];
             WeekEndArray  = [];
             MonthWeekDayArray = [];
             MonthWeekEndArray = [];
             WeekDayNumArray = [];
             WeekDayCountArray = [];
             for(i = 0; i < result['Sampletime'].length; i++) {
               var Day = GetDayOfYear(result['Sampletime'][i]);
               var Week = GetWeekOfYear(result['Sampletime'][i]);
               var DayOfWeek =  new Date(result['Sampletime'][i]).getDay();
               var Month = new Date(result['Sampletime'][i]).getMonth();
               if(!DayArray[Day]) DayArray[Day] = [0, 0, 0];
               if(!WeekDayArray[Week]) WeekDayArray[Week] = [0, 0, 0];
               if(!WeekEndArray[Week]) WeekEndArray[Week] = [0, 0, 0];
               if(!WeekDayNumArray[DayOfWeek]) WeekDayNumArray[DayOfWeek] = [0, 0, 0];
               if(!MonthWeekDayArray[Month]) MonthWeekDayArray[Month] = [0, 0, 0];
               if(!MonthWeekEndArray[Month]) MonthWeekEndArray[Month] = [0, 0, 0];
               if(!WeekDayCountArray[DayOfWeek]) WeekDayCountArray[DayOfWeek] = 0;
               for(n = 0; n < result['active_energy'][i].length; n++) {
                 DayArray[Day][n] = DayArray[Day][n] + result['active_energy'][i][n];
                 if((DayOfWeek === 0) || (DayOfWeek === 6)) {
                   WeekEndArray[Week][n] = WeekEndArray[Week][n] + result['active_energy'][i][n];
                   MonthWeekEndArray[Month][n] = MonthWeekEndArray[Month][n] + result['active_energy'][i][n];
                 } else {
                   WeekDayArray[Week][n] = WeekDayArray[Week][n] + result['active_energy'][i][n];
                   MonthWeekDayArray[Month][n] = MonthWeekDayArray[Month][n] + result['active_energy'][i][n];
                 }
                 WeekDayNumArray[DayOfWeek][n] = WeekDayNumArray[DayOfWeek][n] + result['active_energy'][i][n];
               }
               WeekDayCountArray[DayOfWeek] = WeekDayCountArray[DayOfWeek] + 1;
             }
             AjaxEnergy(selected_year, result);
           }
  }); 
}

function ShowColorScale(scale, outerdiv) {
  scale.out('hex');
  var width = outerdiv.width() - 1;
  for(i = 0; i < width; i++) outerdiv.append('<div style="width:1px; height:10px; display:inline-block; background:' + scale(i / width) + '"></div>');
}

function AjaxEnergy(selected_year, result) {
  var ColorScale = chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]);
  if(Calendar === null) {
    Calendar = $('#Calendar').calendar({ language: "custom" });
    ShowColorScale(ColorScale, $('#CalendarScale'));
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in DayArray) {
    Max = Math.max(Max, DayArray[i][0]);
    Min = Math.min(Min, DayArray[i][0]);
  }
  Min = 0;
  var Units = GetBestUnitsEnergy(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#CalendarMin').html(Round2(Min / Units[0]));
  $('#CalendarMax').html(Round2(Max / Units[0]));
  $('#CalendarUnits').html('<em>' + Units[1] + 'Wh</em>');
  Calendar.setYear(selected_year);
  var Disabled = [];
  Calendar.setCustomDayRenderer(function(element, date) {
    var Day = GetDayOfYear(date.getTime());
    if(DayArray[Day] === undefined) {
      Disabled.push(new Date(date));
    } else {
      var Energy = DayArray[Day][0];
      var Color = ColorScale((Energy - Min) / (Max - Min));
      var Text = chroma(Color).luminance() > 0.5 ? '#000' : '#FFF';
      
      $(element).css('background-color', Color);
      $(element).css('color', Text);
      $(element).css('border', '1px solid #fff');
      var Units = 'Wh';
      if(Energy > 1000) {
        Units = 'kWh';
        Energy = Energy / 1000;
      }
      $(element).qtip( {
        content: { text: "<div>" + Round2(Energy) + Units + "</div>" },
        style: { classes: 'qtip-bootstrap' },
        show: { event: 'mouseover', delay: 0  },
        hide: { event: 'mouseout' }
      });
    }
  });
  Calendar.setDisabledDays(Disabled);
  
  if(WeekTable !== null) {
    WeekTable.clear();
    WeekTable.destroy();
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in WeekDayArray) {
    Max = Math.max(Max, WeekDayArray[i][0] + WeekEndArray[i][0]);
    Min = Math.min(Min, WeekDayArray[i][0] + WeekEndArray[i][0]);
  }
  Min = 0;
  var Units = GetBestUnitsEnergy(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#WeekUnits').html('<em>' + Units[1] + 'Wh</em>');
  var Contents = $('#WeekTable tbody');
  for(i = 1; ;i++) {
    var startDate = GetDateOfISOWeek(i, selected_year);
    var endDate = new Date(startDate.getFullYear(), startDate.getMonth(), startDate.getDate() + 6);
    if(endDate.getFullYear() !== selected_year) break;
    var WeekDay = (WeekDayArray[i] !== undefined) ? WeekDayArray[i][0] : 0;
    var WeekEnd = (WeekEndArray[i] !== undefined) ? WeekEndArray[i][0] : 0;
    Contents.append('<tr>' +
      '<td id="Week_' + i + '">' + i + '</td>' + 
      '<td><div class="progress" style="margin-bottom:0px; height: 16px; position:relative">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * WeekDay / Max) + '%"></div>' +
      '<div class="progress-bar" role="progressbar" style="background-color: #7eb0db; width:' + (100 * WeekEnd / Max) + '%"></div>' +
      '</div></td>' +
      '<td data-sort="' + WeekDay + '">' + Round2(WeekDay / Units[0]) + '</td>' +
      '<td data-sort="' + WeekEnd + '">' + Round2(WeekEnd / Units[0]) + '</td>' +
      '<td data-sort="' + (WeekDay + WeekEnd) + '">' + Round2((WeekDay + WeekEnd) / Units[0]) + '</td>' +
      '</tr>');
      $('#Week_' + i).qtip({
        content: { text: "<div>From " + startDate.toLocaleDateString() + " to " + endDate.toLocaleDateString() + "</div>" },
        style: { classes: 'qtip-bootstrap' },
        show: { event: 'mouseover', delay: 0  },
        hide: { event: 'mouseout' }
      });
  }
  WeekTable = $('#WeekTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                          columnDefs: [ { targets: [1], sortable: false }, { targets: [0, 2, 3, 4], sortable: true, className: "text-center" } ],
                                          order: [0, 'asc']
  });
  if(MonthTable !== null) {
    MonthTable.clear();
    MonthTable.destroy();
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in MonthWeekDayArray) {
    Max = Math.max(Max, MonthWeekDayArray[i][0] + MonthWeekEndArray[i][0]);
    Min = Math.min(Min, MonthWeekDayArray[i][0] + MonthWeekEndArray[i][0]);
  }
  Min = 0;
  var Units = GetBestUnitsEnergy(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#MonthUnits').html('<em>' + Units[1] + 'Wh</em>');
  var Contents = $('#MonthTable tbody');
  for(i = 0; i < 12; i++) {
    var startDate = new Date(startDate.getFullYear(), i, 1);
    var endDate = new Date(startDate.getFullYear(), i + 1, 0);
    var WeekDay = (MonthWeekDayArray[i] !== undefined) ? MonthWeekDayArray[i][0] : 0;
    var WeekEnd = (MonthWeekEndArray[i] !== undefined) ? MonthWeekEndArray[i][0] : 0;
    Contents.append('<tr>' +
      '<td data-sort="' + i + '">' + $.fn.calendar.dates['custom']['monthsShort'][i] + '</td>' + 
      '<td><div class="progress" style="margin-bottom:0px; position:relative">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * WeekDay / Max) + '%"></div>' +
      '<div class="progress-bar" role="progressbar" style="background-color: #7eb0db; width:' + (100 * WeekEnd / Max) + '%"></div>' +
      '</div></td>' +
      '<td data-sort="' + WeekDay + '">' + Round2(WeekDay / Units[0]) + '</td>' +
      '<td data-sort="' + WeekEnd + '">' + Round2(WeekEnd / Units[0]) + '</td>' +
      '<td data-sort="' + (WeekDay + WeekEnd) + '">' + Round2((WeekDay + WeekEnd) / Units[0]) + '</td>' +
      '</tr>');
  }
  MonthTable = $('#MonthTable').DataTable({ responsive: true, paging: false, info: false, searching: false, 
                                            columnDefs: [ { targets: [1], sortable: false }, { targets: [0, 2, 3, 4], sortable: true, className: "text-center" } ],
                                            order: [0, 'asc']
  });
  if(WeekDayTable !== null) {
    WeekDayTable.clear();
    WeekDayTable.destroy();
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in WeekDayNumArray) {
    Max = Math.max(Max, WeekDayNumArray[i][0] / ((WeekDayCountArray[i] !== undefined) ? WeekDayCountArray[i] / 24 : 0));
    Min = Math.min(Min, WeekDayNumArray[i][0] / ((WeekDayCountArray[i] !== undefined) ? WeekDayCountArray[i] / 24 : 0));
  }
  Min = 0;
  var Units = GetBestUnitsEnergy(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#WeekDayUnits').html('<em>' + Units[1] + 'Wh</em>');
  var Contents = $('#WeekDayTable tbody');
  for(i = 0; i < 7; i++) {
    var Energy = (WeekDayNumArray[i] !== undefined) ? WeekDayNumArray[i][0] : 0;
    var Count  = (WeekDayCountArray[i] !== undefined) ? WeekDayCountArray[i] / 24 : 0;
    Contents.append('<tr>' +
      '<td data-sort="' + ((i === 0) ? 7 : i) + '">' + $.fn.calendar.dates['custom']['daysShort'][i] + '</td>' + 
      '<td data-sort="' + Energy + '"><div class="progress" style="margin-bottom:0px; position:relative">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * (Energy / Count) / Max) + '%"></div>' +
      '</div></td>' + 
      '<td data-sort="' + (Energy / Count) + '">' + Round2((Energy / Count) / Units[0]) + '</td>' +
      '<td data-sort="' + Energy + '">' + Round2(Energy / Units[0]) + '</td>' +
      '</tr>');
  }
  WeekDayTable = $('#WeekDayTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                                columnDefs: [ { targets: [1], sortable: false }, { targets: [2, 3], sortable: true, className: "text-right" }, { targets: [0], sortable: true, className: "text-center" } ],
                                                order: [0, 'asc']
  });
  var Max = -Infinity;
  var Min =  Infinity;
  for(i = 0; i < result['active_energy'].length; i++) {
    Max = Math.max(Max, result['active_energy'][i][0]);
    Min = Math.min(Min, result['active_energy'][i][0]);
  }
  Min = 0;
  var Units = GetBestUnitsEnergy(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#HeatMap tbody').html("");
  var Contents = $('#HeatMap tbody');

  var PrevDate = null;
  for(i = 0; i < result['active_energy'].length; i++) {
    var Day = new Date(result['Sampletime'][i]);
    var DayNum = GetDayOfYear(Day.getTime());
    if(DayNum === PrevDate) continue;
    var Row = '<td style="width:22px; font-weight: bold">' + $.fn.calendar.dates['custom']['daysShort'][Day.getDay()] + '</td><td style="width: 54px">' + Day.toLocaleDateString() + '</td>';
    if(Day.getFullYear() !== selected_year) break;
    for(n = 0; n < 24; n++) Row = Row + ('<td style="background-color: #EEE" id="Day_' + DayNum + '_Hour_' + n + '"></td>');
    Contents.append('<tr>' + Row + '</tr>');
    PrevDate = DayNum;
  }

  for(i = 0; i < result['active_energy'].length; i++) {
    var Day = new Date(result['Sampletime'][i]);
    var DayNum = GetDayOfYear(Day.getTime());
    var HourNum = Day.getHours();
    $('#Day_' + DayNum + '_Hour_' + HourNum).css('background-color', ColorScale((result['active_energy'][i][0] - Min) / (Max - Min)));
    $('#Day_' + DayNum + '_Hour_' + HourNum).qtip({
        content: { text: "<div>" + Round2(result['active_energy'][i][0] / Units[0]) + Units[1] + "Wh</div>" },
        style: { classes: 'qtip-bootstrap' },
        show: { event: 'mouseover', delay: 0  },
        hide: { event: 'mouseout' }
      });
    
  }
  
  StopSpinner(Spinner, 'StatsSpinner');
}
$('#btnShowAsTable').click(function() {
  $('#btnShowAsTable').addClass('active');
  $('#btnShowAsHeatmap').removeClass('active');
  $('#ShowAsTables').removeClass('hidden');
  $('#ShowAsHeatmap').addClass('hidden');
  localStorage.setItem("StatsView", "Tables");
});
$('#btnShowAsHeatmap').click(function() {
  $('#btnShowAsHeatmap').addClass('active');
  $('#btnShowAsTable').removeClass('active');
  $('#ShowAsTables').addClass('hidden');
  $('#ShowAsHeatmap').removeClass('hidden');
  localStorage.setItem("StatsView", "Heatmap");
});

function UpdatePage() {
  if(localStorage.getItem("StatsView") === "Tables") 
    $('#btnShowAsTable').click();
  else
    $('#btnShowAsHeatmap').click();
  
  GetDataRanges();
}