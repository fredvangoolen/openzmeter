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
/* Calendar locales */
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
} (jQuery)); 
var Calendar = null, WeekTable = null, MonthTable = null, WeekDayTable = null, HourTable = null, EnergyData = [], SelectedYear;
function AjaxGetEnergy(selected_year) {
  $('#StatsSpinner').show();
  $('#btnYear + ul > li').removeClass('active');
  $('#btnYear + ul > li > a:contains(' + selected_year +')').parent().addClass('active');
  $('#btnYear span:first').text(selected_year);
  SelectedYear = selected_year;
  $('#StatsSpinner').show();
  var To = new Date(selected_year, 11, 31, 23, 59, 59, 999).getTime();
  var From = new Date(selected_year, 0, 1, 0, 0, 0, 0).getTime();
  Ajax.GetSeries(To - From, To, '1h', 0, 'active_energy',
    function(result) {
      EnergyData = GraphDataAppend(result, 'active_energy', []);
      $('#StatsSpinner').hide();
      EnergySeries();      
    }
  );
}
function SumPhases(array, visibleSeries) {
  var Energy = 0.0;
  for(phase = 0; phase < visibleSeries.length; phase++) {
    if(visibleSeries[phase] === true) Energy = Energy + EnergyData[i][phase + 1];
  }
  return Energy;
}
function UpdateCalendar(visibleSeries) {
  var ColorScale = chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]), DayArray = [];
  for(i = 0; i < EnergyData.length; i++) {
    var Day = GetDayOfYear(EnergyData[i][0]);
    if(DayArray[Day] === undefined) DayArray[Day] = 0.0;
    DayArray[Day] = DayArray[Day] + SumPhases(EnergyData[i], visibleSeries);
  }
  var Max = ArrayMax(DayArray), Min = ArrayMin(DayArray), Units = GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
  if(Calendar === null) {
    Calendar = $('#Calendar').calendar({ language: "custom" });
    ShowColorScale(ColorScale, $('#CalendarScale'));
  }
  $('#CalendarMin').html(Round2(ScaleToUnits(Min, Units)));
  $('#CalendarMax').html(Round2(ScaleToUnits(Max, Units)));
  $('#CalendarUnits').html(Units + 'Wh');
  Calendar.setYear(SelectedYear);
  var Disabled = [];
  Calendar.setCustomDayRenderer(function(element, date) {
    var Day = GetDayOfYear(date.getTime());
    if(DayArray[Day] === undefined) {
      Disabled.push(new Date(date));
    } else {
      var Energy = DayArray[Day];
      var Color = ColorScale((Energy - Min) / (Max - Min));
      $(element).css({ 'background-color': Color, 'color': chroma(Color).luminance() > 0.5 ? '#000' : '#FFF', 'border': '1px solid #fff'});
      $(element).prop({'data-toggle': 'tooltip', 'title': Round2Plain(ScaleToUnits(Energy, Units)) + ' ' + Units + 'Wh'} );
    }
  });
  Calendar.setDisabledDays(Disabled);
}
function UpdateWeeks(visibleSeries) {
  var WeekDay = [], WeekEnd = [];
  if(WeekTable !== null) {
    WeekTable.clear();
    WeekTable.destroy();
  }
  for(i = 0; i < EnergyData.length; i++) {
    var Week      = GetWeekOfYear(EnergyData[i][0]);
    var DayOfWeek = new Date(EnergyData[i][0]).getDay();
    if(WeekDay[Week] === undefined) WeekDay[Week] = 0.0;
    if(WeekEnd[Week] === undefined) WeekEnd[Week] = 0.0;
    if((DayOfWeek === 0) || (DayOfWeek === 6)) {
      WeekEnd[Week] = WeekEnd[Week] + SumPhases(EnergyData[i], visibleSeries);
    } else {
      WeekDay[Week] = WeekDay[Week] + SumPhases(EnergyData[i], visibleSeries);
    }
  }
  var Max = -Infinity, Min =  Infinity;
  for(i in WeekDay) {
    Max = Math.max(Max, WeekDay[i] + WeekEnd[i]);
    Min = Math.min(Min, WeekDay[i] + WeekEnd[i]);
  }
  var Units = GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#WeekUnits').html('<em>' + Units + 'Wh</em>');
  var Contents = $('#WeekTable tbody');
  for(i = 1; ;i++) {
    var startDate = GetDateOfISOWeek(i, SelectedYear);
    var endDate = new Date(startDate.getFullYear(), startDate.getMonth(), startDate.getDate() + 6);
    if(endDate.getFullYear() !== SelectedYear) break;
    var E0 = (WeekDay[i] !== undefined) ? WeekDay[i] : 0, E1 = (WeekEnd[i] !== undefined) ? WeekEnd[i] : 0, E2 = E0 + E1;
    Contents.append('<tr>' +
      '<td data-toogle="tooltip" title="From ' + startDate.toLocaleDateString() + ' to ' + endDate.toLocaleDateString() + '">' + i + '</td>' + 
      '<td><div class="progress" style="margin-bottom:0px; position:relative; height: 16px">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * E0 / Max) + '%"></div>' +
      '<div class="progress-bar" role="progressbar" style="background-color: #7eb0db; width:' + (100 * E1 / Max) + '%"></div>' +
      '</div></td>' +
      '<td data-sort="' + E0 + '">' + ((WeekDay[i] !== undefined) ? Round2(ScaleToUnits(E0, Units)) : '-') + '</td>' +
      '<td data-sort="' + E1 + '">' + ((WeekDay[i] !== undefined) ? Round2(ScaleToUnits(E1, Units)) : '-') + '</td>' +
      '<td data-sort="' + E2 + '">' + ((WeekDay[i] !== undefined) ? Round2(ScaleToUnits(E2, Units)) : '-') + '</td>' +
      '</tr>');
  }
  WeekTable = $('#WeekTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                          columnDefs: [ { targets: [1], sortable: false }, { targets: [0, 2, 3, 4], sortable: true, className: "text-center" } ],
                                          order: [0, 'asc']
  });
}
function UpdateMonths(visibleSeries) {
  var WeekDay = [], WeekEnd = [];
  if(MonthTable !== null) {
    MonthTable.clear();
    MonthTable.destroy();
  }
  for(i = 0; i < EnergyData.length; i++) {
    var Month     = new Date(EnergyData[i][0]).getMonth();
    var DayOfWeek =  new Date(EnergyData[i][0]).getDay();
    var Energy = 0;
    if(WeekDay[Month] === undefined) WeekDay[Month] = 0.0;
    if(WeekEnd[Month] === undefined) WeekEnd[Month] = 0.0;
    if((DayOfWeek === 0) || (DayOfWeek === 6)) {
      WeekEnd[Month] = WeekEnd[Month] + SumPhases(EnergyData[i], visibleSeries);
    } else {
      WeekDay[Month] = WeekDay[Month] + SumPhases(EnergyData[i], visibleSeries);
    }
  }
  var Max = -Infinity, Min =  Infinity;
  for(i in WeekDay) {
    Max = Math.max(Max, WeekDay[i] + WeekEnd[i]);
    Min = Math.min(Min, WeekDay[i] + WeekEnd[i]);
  }
  var Units = GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#MonthUnits').html('<em>' + Units + 'Wh</em>');
  var Contents = $('#MonthTable tbody');
  for(i = 0; i < 12; i++) {
    var startDate = new Date(SelectedYear, i, 1);
    var endDate = new Date(SelectedYear, i + 1, 0);
    var E0 = (WeekDay[i] !== undefined) ? WeekDay[i] : 0, E1 = (WeekEnd[i] !== undefined) ? WeekEnd[i] : 0, E2 = E0 + E1;
    Contents.append('<tr>' +
      '<td data-sort="' + i + '">' + $.fn.calendar.dates['custom']['monthsShort'][i] + '</td>' + 
      '<td><div class="progress" style="margin-bottom:0px; position:relative; height: 16px">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * E0 / Max) + '%"></div>' +
      '<div class="progress-bar" role="progressbar" style="background-color: #7eb0db; width:' + (100 * E1 / Max) + '%"></div>' +
      '</div></td>' +
      '<td data-sort="' + E0 + '">' + ((WeekDay[i] !== undefined) ? Round2(ScaleToUnits(E0, Units)) : '-') + '</td>' +
      '<td data-sort="' + E1 + '">' + ((WeekDay[i] !== undefined) ? Round2(ScaleToUnits(E1, Units)) : '-') + '</td>' +
      '<td data-sort="' + E2 + '">' + ((WeekDay[i] !== undefined) ? Round2(ScaleToUnits(E2, Units)) : '-') + '</td>' +
      '</tr>');
  }
  MonthTable = $('#MonthTable').DataTable({ responsive: true, paging: false, info: false, searching: false, 
                                            columnDefs: [ { targets: [1], sortable: false }, { targets: [0, 2, 3, 4], sortable: true, className: "text-center" } ],
                                            order: [0, 'asc']
  });
}      
function UpdateWeekDays(visibleSeries) {
  var WeekDay = Array(7).fill(0), WeekDayCount = Array(7).fill(0);
  if(WeekDayTable !== null) {
    WeekDayTable.clear();
    WeekDayTable.destroy();
  }
  for(i = 0; i < EnergyData.length; i++) {
    var DayOfWeek = new Date(EnergyData[i][0]).getDay();
    WeekDay[DayOfWeek] = WeekDay[DayOfWeek] + SumPhases(EnergyData[i], visibleSeries);
    WeekDayCount[DayOfWeek] = WeekDayCount[DayOfWeek] + 1;
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in WeekDay) {
    if(WeekDayCount[i] === 0) continue;
    Max = Math.max(Max, WeekDay[i] / WeekDayCount[i]);
    Min = Math.min(Min, WeekDay[i] / WeekDayCount[i]);
  }
  var Units = GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#WeekDayUnits').html('<em>' + Units + 'Wh</em>');
  var Contents = $('#WeekDayTable tbody');
  for(i = 0; i < 7; i++) {
    var Energy = WeekDay[i];
    var Count  = WeekDayCount[i];
    Contents.append('<tr>' +
      '<td data-sort="' + ((i === 0) ? 7 : i) + '">' + $.fn.calendar.dates['custom']['daysShort'][i] + '</td>' + 
      '<td data-sort="' + Energy + '"><div class="progress" style="margin-bottom:0px; position:relative; height: 16px">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * (Energy / Count) / Max) + '%"></div>' +
      '</div></td>' + 
      '<td data-sort="' + (Energy / Count) + '">' + ((WeekDayCount[i] === 0) ? '-' : Round2(ScaleToUnits(Energy / Count, Units))) + '</td>' +
      '<td data-sort="' + Energy + '">' + ((WeekDayCount[i] === 0) ? '-' : Round2(ScaleToUnits(Energy, Units))) + '</td>' +
      '</tr>');
  }
  WeekDayTable = $('#WeekDayTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                                columnDefs: [ { targets: [1], sortable: false }, { targets: [2, 3], sortable: true, className: "text-right" }, { targets: [0], sortable: true, className: "text-center" } ],
                                                order: [0, 'asc']
  });
}
function UpdateHours(visibleSeries) {
  var Hours = Array(24).fill(0), HoursCount = Array(24).fill(0);
  if(HourTable !== null) {
    HourTable.clear();
    HourTable.destroy();
  }
  for(i = 0; i < EnergyData.length; i++) {
    var Hour = new Date(EnergyData[i][0]).getHours();
    Hours[Hour] = Hours[Hour] + SumPhases(EnergyData[i], visibleSeries);
    HoursCount[Hour] = HoursCount[Hour] + 1;
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in Hours) {
    if(Hours[i] === 0) continue;
    Max = Math.max(Max, Hours[i] / HoursCount[i]);
    Min = Math.min(Min, Hours[i] / HoursCount[i]);
  }
  var Units = GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#HourUnits').html('<em>' + Units + 'Wh</em>');
  var Contents = $('#HourTable tbody');
  for(i = 0; i < 24; i++) {
    var Energy = Hours[i];
    var Count  = HoursCount[i];
    Contents.append('<tr>' +
      '<td data-sort="' + i + '">' + i + '</td>' + 
      '<td data-sort="' + Energy + '"><div class="progress" style="margin-bottom:0px; position:relative; height: 16px">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * (Energy / Count) / Max) + '%"></div>' +
      '</div></td>' + 
      '<td data-sort="' + (Energy / Count) + '">' + ((HoursCount[i] === 0) ? '-' : Round2(ScaleToUnits(Energy / Count, Units))) + '</td>' +
      '<td data-sort="' + Energy + '">' + ((HoursCount[i] === 0) ? '-' : Round2(ScaleToUnits(Energy, Units))) + '</td>' +
      '</tr>');
  }
  HourTable = $('#HourTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                                columnDefs: [ { targets: [1], sortable: false }, { targets: [2, 3], sortable: true, className: "text-right" }, { targets: [0], sortable: true, className: "text-center" } ],
                                                order: [0, 'asc']
  });
}
function EnergySeries(event) {
  GroupButtons($('#groupEnergyPhases'), 'VisibleStatsPhases', 3, false, 
  function(visibleSeries) {
    UpdateCalendar(visibleSeries);
    UpdateWeeks(visibleSeries);
    UpdateMonths(visibleSeries);
    UpdateWeekDays(visibleSeries);
    UpdateHours(visibleSeries);
  }, (event !== undefined) ? event.target : undefined);
}
$('#groupEnergyPhases').on('click', 'button', EnergySeries);
function UpdatePage() {
  Ajax.GetSeriesRange('1h', 
    function(result) {
      for(year = new Date(result['From']).getFullYear(); year <= new Date(result['To']).getFullYear(); year++) {
        $('#btnYear + ul').append('<li><a href="#">' + year + '</a></li>');
      }
      $('#btnYear + ul > li > a').click(function(evt) { 
        evt.preventDefault();
        AjaxGetEnergy(parseInt(this.text));
      });
      AjaxGetEnergy(new Date(result['To']).getFullYear());
    }
  );
}
