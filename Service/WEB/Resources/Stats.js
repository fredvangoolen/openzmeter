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
var MonthArray = [];
var WeekDayNumArray = [];
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
             //Relleno los años que tengo y selecciono el ultimo
             AjaxGetEnergy(2018);
           }
  }); 
}
function AjaxGetEnergy(selected_year) {
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
             WeekArray  = [];
             MonthArray = [];
             WeekDayNumArray = [];
             for(i = 0; i < result['Sampletime'].length; i++) {
               var Day = GetDayOfYear(result['Sampletime'][i]);
               var Week = GetWeekOfYear(result['Sampletime'][i]);
               var DayOfWeek =  new Date(result['Sampletime'][i]).getDay();
               var Month = new Date(result['Sampletime'][i]).getMonth();
               if(!DayArray[Day]) DayArray[Day] = [0, 0, 0];
               if(!WeekDayArray[Week]) WeekDayArray[Week] = [0, 0, 0];
               if(!WeekEndArray[Week]) WeekEndArray[Week] = [0, 0, 0];
               if(!WeekDayNumArray[DayOfWeek]) WeekDayNumArray[DayOfWeek] = [0, 0, 0];
               if(!MonthArray[Week]) MonthArray[Month] = [0, 0, 0];
               for(n = 0; n < result['active_energy'][i].length; n++) {
                 DayArray[Day][n] = DayArray[Day][n] + result['active_energy'][i][n];
                 if((DayOfWeek === 0) || (DayOfWeek === 6)) 
                   WeekEndArray[Week][n] = WeekEndArray[Week][n] + result['active_energy'][i][n];
                 else
                   WeekDayArray[Week][n] = WeekDayArray[Week][n] + result['active_energy'][i][n];
                 WeekDayNumArray[DayOfWeek][n] = WeekDayNumArray[DayOfWeek][n] + result['active_energy'][i][n];
                 MonthArray[Month][n] = MonthArray[Month][n] + result['active_energy'][i][n];
               }
             }
             AjaxEnergy(selected_year);
           }
  }); 
}
function AjaxEnergy(selected_year) {
  if(Calendar === null) Calendar = $('#Calendar').calendar({ language: "custom" });
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in DayArray) {
    Max = Math.max(Max, DayArray[i][0]);
    Min = Math.min(Min, DayArray[i][0]);
  }
  Calendar.setYear(selected_year);
  Calendar.setCustomDayRenderer(function(element, date) {
    var Day = GetDayOfYear(date.getTime());
    if(DayArray[Day] === undefined) {
      $(element.parentElement).addClass('disabled');
    } else {
      var Energy = DayArray[Day][0];
      var Color = 'rgba(255, 0, 0,' +  Energy / Max + ')';
      $(element).css('background-color', Color);
      $(element).css('border', '1px solid #fff');
      var Units = 'Wh';
      if(Energy > 1000) {
        Units = 'kWh';
        Energy = Energy / 1000;
      }
      $(element).on("mouseover", null, function ($e) {
        $(this).qtip({
          overwrite: false,
            content: {
              text: "<div>" + Round2(Energy) + Units + "</div>"
            },
            style: {
              classes: 'qtip-bootstrap'
            }
        });
      });
    }
  });
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
  var Contents = $('#WeekTable tbody');
  for(i = 1; ;i++) {
    var startDate = GetDateOfISOWeek(i, selected_year);
    var endDate = new Date(startDate.getFullYear(), startDate.getMonth(), startDate.getDate() + 6);
    if(endDate.getFullYear() !== selected_year) break;
    var WeekDay = (WeekDayArray[i] !== undefined) ? WeekDayArray[i][0] : 0;
    var WeekEnd = (WeekEndArray[i] !== undefined) ? WeekEndArray[i][0] : 0;
    Contents.append('<tr>' +
      '<td>' + i + '</td>' + 
      '<td data-sort="' + startDate.getTime() + '">' + startDate.toLocaleDateString() + '</td>' + 
      '<td data-sort="' + endDate.getTime() + '">' + endDate.toLocaleDateString() + '</td>' + 
      '<td data-sort="' + WeekDay + '"><div class="progress" style="margin-bottom:0px; height: 16px; position:relative">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * WeekDay / Max) + '%"></div>' +
      '<span style="position: absolute; left:0px; text-align: center; width: 100%">' + FormatKMG(WeekDay, '<em>Wh</em>', '<em>kWh</em>') + '</span>' +
      '</div></td>' +
      '<td data-sort="' + WeekEnd + '"><div class="progress" style="margin-bottom:0px; position:relative">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * WeekEnd / Max) + '%"></div>' +
      '<span style="position: absolute; left:0px;  padding-top: 2px; text-align: center; width: 100%">' + FormatKMG(WeekEnd, '<em>Wh</em>', '<em>kWh</em>') + '</span>' +
      '</div></td>' +
      '<td data-sort="' + (WeekDay + WeekEnd) + '"><div class="progress" style="margin-bottom:0px; position:relative">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * (WeekDay + WeekEnd) / Max) + '%"></div>' +
      '<span style="position: absolute; left:0px; padding-top: 2px; text-align: center; width: 100%">' + FormatKMG(WeekDay + WeekEnd, '<em>Wh</em>', '<em>kWh</em>') + '</span>' +
      '</div></td>');
  }
  WeekTable = $('#WeekTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                          columnDefs: [ { targets: [0, 1, 2, 3, 4, 5], searchable: true } ],
                                          order: [0, 'asc']
  });  
  if(MonthTable !== null) {
    MonthTable.clear();
    MonthTable.destroy();
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in MonthArray) {
    Max = Math.max(Max, MonthArray[i][0]);
    Min = Math.min(Min, MonthArray[i][0]);
  }
  var Contents = $('#MonthTable tbody');
  for(i = 0; i < 12; i++) {
    var startDate = new Date(startDate.getFullYear(), i, 1);
    var endDate = new Date(startDate.getFullYear(), i + 1, 0);
    var Energy = (MonthArray[i] !== undefined) ? MonthArray[i][0] : 0;
    Contents.append('<tr>' +
      '<td data-sort="' + i + '">' + $.fn.calendar.dates['custom']['monthsShort'][i] + '</td>' + 
      '<td data-sort="' + startDate.getTime() + '">' + startDate.toLocaleDateString() + '</td>' + 
      '<td data-sort="' + endDate.getTime() + '">' + endDate.toLocaleDateString() + '</td>' + 
      '<td data-sort="' + Energy + '"><div class="progress" style="margin-bottom:0px; position:relative">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * Energy / Max) + '%"></div>' +
      '<span style="position: absolute; left:0px; padding-top: 2px; text-align: center; width: 100%">' + FormatKMG(Energy, '<em>Wh</em>', '<em>kWh</em>') + '</span>' +
      '</div></td>');
  }
  MonthTable = $('#MonthTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                            columnDefs: [ { targets: [0, 1, 2, 3], searchable: true } ],
                                            order: [0, 'asc']
  });
  if(WeekDayTable !== null) {
    WeekDayTable.clear();
    WeekDayTable.destroy();
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in WeekDayNumArray) {
    Max = Math.max(Max, WeekDayNumArray[i][0]);
    Min = Math.min(Min, WeekDayNumArray[i][0]);
  }
  var Contents = $('#WeekDayTable tbody');
  for(i = 0; i < 7; i++) {
    var Energy = (WeekDayNumArray[i] !== undefined) ? WeekDayNumArray[i][0] : 0;
    Contents.append('<tr>' +
      '<td data-sort="' + ((i === 0) ? 7 : i) + '">' + $.fn.calendar.dates['custom']['daysShort'][i] + '</td>' + 
      '<td data-sort="' + Energy + '"><div class="progress" style="margin-bottom:0px; position:relative">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * Energy / Max) + '%"></div>' +
      '<span style="position: absolute; left:0px; padding-top: 2px; text-align: center; width: 100%">' + FormatKMG(Energy, '<em>Wh</em>', '<em>kWh</em>') + '</span>' +
      '</div></td>');
  }
  WeekDayTable = $('#WeekDayTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                                columnDefs: [ { targets: [0, 1], searchable: true } ],
                                                order: [0, 'asc']
  });
  StopSpinner(Spinner, 'StatsSpinner');
  Calendar.translate;
}
function UpdatePage() {
  GetDataRanges();
}