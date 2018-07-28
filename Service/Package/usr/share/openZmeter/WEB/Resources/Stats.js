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
var Calendar     = null;
var WeekTable    = null;
var MonthTable   = null;
var WeekDayTable = null;
var Spinner      = new Spinner(SpinnerOpts);
var EnergyData   = [];
var SelectedYear;
var SelectedPhase = [];
function GetDataRanges() {
  ShowSpinner(Spinner, 'StatsSpinner');
  GetSeriesRange('1h', 
    function(result) {
      for(year = new Date(result['From']).getFullYear(); year <= new Date(result['To']).getFullYear(); year++) $('#YearSelector').append('<button type="button" class="btn btn-default btn-xs" id="btnSelYear' + year + '" onclick="AjaxGetEnergy(' + year + ')">' + year + '</button>');
      AjaxGetEnergy(new Date(result['To']).getFullYear());
    }
  );
}
function AjaxGetEnergy(selected_year) {
  $('#YearSelector button').removeClass('active');
  $('#btnSelYear' + selected_year).addClass('active');
  SelectedYear = selected_year; 
  ShowSpinner(Spinner, 'StatsSpinner');
  var To = new Date(selected_year, 11, 31, 23, 59, 59, 999).getTime();
  var From = new Date(selected_year, 0, 1, 0, 0, 0, 0).getTime();
  GetSeries(To - From, To, '10m', 0, 'active_energy', 
    function(result) {
      EnergyData = [];
      for(i = 0; i < result['Sampletime'].length; i++) EnergyData.push([result['Sampletime'][i], result['active_energy'][i]]);
      ButtonClick();
      StopSpinner(Spinner, 'StatsSpinner');
    }
  );
}
function ShowColorScale(scale, outerdiv) {
  scale.out('hex');
  var width = outerdiv.width() - 1;
  for(i = 0; i < width; i++) outerdiv.append('<div style="width:1px; height:10px; display:inline-block; background:' + scale(i / width) + '"></div>');
}
function UpdateCalendar() {
  var DayArray = [];
  var ColorScale = chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]);
  if(Calendar === null) {
    Calendar = $('#Calendar').calendar({ language: "custom" });
    ShowColorScale(ColorScale, $('#CalendarScale'));
  }
  for(i = 0; i < EnergyData.length; i++) {
    var Day = GetDayOfYear(EnergyData[i][0]);
    if(!DayArray[Day]) DayArray[Day] = 0;
    DayArray[Day] = DayArray[Day] + EnergyData[i][1][SelectedPhase];
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in DayArray) {
    Max = Math.max(Max, DayArray[i]);
    Min = Math.min(Min, DayArray[i]);
  }
  Min = 0;
  var Units = GetBestUnitsEnergy(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#CalendarMin').html(Round2(Min / Units[0]));
  $('#CalendarMax').html(Round2(Max / Units[0]));
  $('#CalendarUnits').html('<em>' + Units[1] + 'Wh</em>');
  Calendar.setYear(SelectedYear);
  var Disabled = [];
  Calendar.setCustomDayRenderer(function(element, date) {
    var Day = GetDayOfYear(date.getTime());
    if(DayArray[Day] === undefined) {
      Disabled.push(new Date(date));
    } else {
      var Energy = DayArray[Day];
      var Color = ColorScale((Energy - Min) / (Max - Min));
      var Text = chroma(Color).luminance() > 0.5 ? '#000' : '#FFF';
      $(element).css('background-color', Color);
      $(element).css('color', Text);
      $(element).css('border', '1px solid #fff');
      $(element).prop('data-toggle', 'tooltip');
      var Units = 'Wh';
      if(Energy > 1000) {
        Units = 'kWh';
        Energy = Energy / 1000;
      }
      $(element).prop('title', Round2Plain(Energy) + Units);
    }
  });
  Calendar.setDisabledDays(Disabled);
}
function UpdateWeeks() {
  if(WeekTable !== null) {
    WeekTable.clear();
    WeekTable.destroy();
  }
  var WeekDayArray  = [];
  var WeekEndArray  = [];
  for(i = 0; i < EnergyData.length; i++) {
    var Week = GetWeekOfYear(EnergyData[i][0]);
    var DayOfWeek =  new Date(EnergyData[i][0]).getDay();
    if(!WeekDayArray[Week]) WeekDayArray[Week] = 0.0;
    if(!WeekEndArray[Week]) WeekEndArray[Week] = 0.0;
    if((DayOfWeek === 0) || (DayOfWeek === 6)) {
      WeekEndArray[Week] = WeekEndArray[Week] + EnergyData[i][1][SelectedPhase];
    } else {
      WeekDayArray[Week] = WeekDayArray[Week] + EnergyData[i][1][SelectedPhase];
    }
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in WeekDayArray) {
    Max = Math.max(Max, WeekDayArray[i] + WeekEndArray[i]);
    Min = Math.min(Min, WeekDayArray[i] + WeekEndArray[i]);
  }
  Min = 0;
  var Units = GetBestUnitsEnergy(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#WeekUnits').html('<em>' + Units[1] + 'Wh</em>');
  var Contents = $('#WeekTable tbody');
  for(i = 1; ;i++) {
    var startDate = GetDateOfISOWeek(i, SelectedYear);
    var endDate = new Date(startDate.getFullYear(), startDate.getMonth(), startDate.getDate() + 6);
    if(endDate.getFullYear() !== SelectedYear) break;
    var WeekDay = (WeekDayArray[i] !== undefined) ? WeekDayArray[i] : 0;
    var WeekEnd = (WeekEndArray[i] !== undefined) ? WeekEndArray[i] : 0;
    Contents.append('<tr>' +
      '<td data-toogle="tooltip" title="From ' + startDate.toLocaleDateString() + ' to ' + endDate.toLocaleDateString() + '">' + i + '</td>' + 
      '<td><div class="progress" style="margin-bottom:0px; position:relative">' +
      '<div class="progress-bar" role="progressbar" style="width:' + (100 * WeekDay / Max) + '%"></div>' +
      '<div class="progress-bar" role="progressbar" style="background-color: #7eb0db; width:' + (100 * WeekEnd / Max) + '%"></div>' +
      '</div></td>' +
      '<td data-sort="' + WeekDay + '">' + Round2(WeekDay / Units[0]) + '</td>' +
      '<td data-sort="' + WeekEnd + '">' + Round2(WeekEnd / Units[0]) + '</td>' +
      '<td data-sort="' + (WeekDay + WeekEnd) + '">' + Round2((WeekDay + WeekEnd) / Units[0]) + '</td>' +
      '</tr>');
  }
  WeekTable = $('#WeekTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                          columnDefs: [ { targets: [1], sortable: false }, { targets: [0, 2, 3, 4], sortable: true, className: "text-center" } ],
                                          order: [0, 'asc']
  });
}
function UpdateMonths() {
  if(MonthTable !== null) {
    MonthTable.clear();
    MonthTable.destroy();
  }
  var MonthWeekDayArray = [];
  var MonthWeekEndArray = [];
  for(i = 0; i < EnergyData.length; i++) {
    var DayOfWeek =  new Date(EnergyData[i][0]).getDay();
    var Month = new Date(EnergyData[i][0]).getMonth();
    if(!MonthWeekDayArray[Month]) MonthWeekDayArray[Month] = 0.0;
    if(!MonthWeekEndArray[Month]) MonthWeekEndArray[Month] = 0.0;
    if((DayOfWeek === 0) || (DayOfWeek === 6)) {
      MonthWeekEndArray[Month] = MonthWeekEndArray[Month] + EnergyData[i][1][SelectedPhase];
    } else {
      MonthWeekDayArray[Month] = MonthWeekDayArray[Month] + EnergyData[i][1][SelectedPhase];
    }
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in MonthWeekDayArray) {
    Max = Math.max(Max, MonthWeekDayArray[i] + MonthWeekEndArray[i]);
    Min = Math.min(Min, MonthWeekDayArray[i] + MonthWeekEndArray[i]);
  }
  Min = 0;
  var Units = GetBestUnitsEnergy(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#MonthUnits').html('<em>' + Units[1] + 'Wh</em>');
  var Contents = $('#MonthTable tbody');
  for(i = 0; i < 12; i++) {
    var startDate = new Date(SelectedYear, i, 1);
    var endDate = new Date(SelectedYear, i + 1, 0);
    var WeekDay = (MonthWeekDayArray[i] !== undefined) ? MonthWeekDayArray[i] : 0;
    var WeekEnd = (MonthWeekEndArray[i] !== undefined) ? MonthWeekEndArray[i] : 0;
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
}      
function UpdateWeekDays() {
  if(WeekDayTable !== null) {
    WeekDayTable.clear();
    WeekDayTable.destroy();
  }
  WeekDayNumArray = [];
  WeekDayCountArray = [];
  for(i = 0; i < EnergyData.length; i++) {
    var DayOfWeek =  new Date(EnergyData[i][0]).getDay();
    if(!WeekDayNumArray[DayOfWeek]) WeekDayNumArray[DayOfWeek] = 0.0;
    if(!WeekDayCountArray[DayOfWeek]) WeekDayCountArray[DayOfWeek] = 0;   
    WeekDayNumArray[DayOfWeek] = WeekDayNumArray[DayOfWeek] + EnergyData[i][1][SelectedPhase];
    WeekDayCountArray[DayOfWeek] = WeekDayCountArray[DayOfWeek] + 1;
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in WeekDayNumArray) {
    Max = Math.max(Max, WeekDayNumArray[i] / ((WeekDayCountArray[i] !== undefined) ? WeekDayCountArray[i] / 144 : 0));
    Min = Math.min(Min, WeekDayNumArray[i] / ((WeekDayCountArray[i] !== undefined) ? WeekDayCountArray[i] / 144 : 0));
  }
  Min = 0;
  var Units = GetBestUnitsEnergy(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#WeekDayUnits').html('<em>' + Units[1] + 'Wh</em>');
  var Contents = $('#WeekDayTable tbody');
  for(i = 0; i < 7; i++) {
    var Energy = (WeekDayNumArray[i] !== undefined) ? WeekDayNumArray[i] : 0;
    var Count  = (WeekDayCountArray[i] !== undefined) ? WeekDayCountArray[i] / 144 : 0;
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
}             
function UpdateHeatMap() {
  var Count = 1;
  if($('#btnHeatmap1Hour').hasClass('active')) Count = 6;
  if($('#btnHeatmap30Min').hasClass('active')) Count = 3;
  if($('#btnHeatmap20Min').hasClass('active')) Count = 2;
  if($('#btnHeatmap10Min').hasClass('active')) Count = 1;
  var Header = $('#HeatMap thead tr');
  Header.html('<td style="width:22px">Day</td><td style="width:54px">Date</td>');
  for(i = 0; i < 24; i++) Header.append('<td colspan="' + (6 / Count) + '" style="border-left: 1px solid #888">' + ((i < 10) ? '0' + i : i) + '</td>');
  var Body = $('#HeatMap tbody');
  Body.html("");
  var FirstDay = null;
  var DayNum = null;
  var PrevDay = null;
  var Cells = [];
  for(i = 0; i < EnergyData.length; i++) {
    var Day = new Date(EnergyData[i][0]);
    DayNum = GetDayOfYear(Day.getTime());
    if(FirstDay == null) FirstDay = DayNum;
    var DayStart = new Date(Day.getFullYear(), 0, DayNum, 0, 0, 0, 0);
    var Slot = Math.trunc((EnergyData[i][0] - DayStart) / (600000 * Count));
    var SlotName = DayNum + '_' + Slot;
    if(!Cells[SlotName]) Cells[SlotName] = 0.0;
    Cells[SlotName] = Cells[SlotName] + EnergyData[i][1][SelectedPhase];
  }
  var Max = -Infinity;
  var Min =  Infinity;
  for(i in Cells) {
    Max = Math.max(Max, Cells[i]);
    Min = Math.min(Min, Cells[i]);
  }
  Min = 0;
  var Units = GetBestUnitsEnergy(Math.max(Math.abs(Min), Math.abs(Max)));
  var ColorScale = chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]);
  for(; FirstDay <= DayNum; FirstDay++) {
    var Day = new Date(SelectedYear, 0, FirstDay);
    var Row = '<td style="width:22px; font-weight: bold">' + $.fn.calendar.dates['custom']['daysShort'][Day.getDay()] + '</td><td style="width: 54px">' + Day.toLocaleDateString() + '</td>';
    for(n = 0; n < 24 * (6 / Count); n++) {
      var SlotName = FirstDay + '_' + n;
      var Color = (!Cells[SlotName] ? '#EEE' : ColorScale((Cells[SlotName] - Min) / (Max - Min)));
      Row = Row + ('<td style="background-color: ' + Color + '" data-toogle="tooltip" title="' + (!Cells[SlotName] ? 'No data for this period' : Round2Plain(Cells[SlotName] / Units[0]) + Units[1] + 'Wh') + '"></td>');
    }
    Body.append('<tr>' + Row + '</tr>');
  }
}
function ButtonClick(event) {
  if(this.id === 'btnShowAsTable')   localStorage.setItem('StatView', 'Table');
  if(this.id === 'btnShowAsHeatmap') localStorage.setItem('StatView', 'Heatmap');
  if(this.id === 'btnHeatmap1Hour') localStorage.setItem('StatBlocks', '1h');
  if(this.id === 'btnHeatmap30Min') localStorage.setItem('StatBlocks', '30m');
  if(this.id === 'btnHeatmap20Min') localStorage.setItem('StatBlocks', '20m');
  if(this.id === 'btnHeatmap10Min') localStorage.setItem('StatBlocks', '10m');
  if(this.id === 'btnStatsALLPhase') localStorage.setItem('StatPhase', 'ALL');
  if(this.id === 'btnStatsRPhase') localStorage.setItem('StatPhase', 'R');
  if(this.id === 'btnStatsSPhase') localStorage.setItem('StatPhase', 'S');
  if(this.id === 'btnStatsTPhase') localStorage.setItem('StatPhase', 'T');
  
  $('#ShowAsHeatmap').addClass('hidden');
  $('#ShowAsTables').addClass('hidden');
  $('#btnShowAsTable').removeClass('active');
  $('#btnShowAsHeatmap').removeClass('active');
  $('#btnHeatmap1Hour').addClass('disabled');
  $('#btnHeatmap30Min').addClass('disabled');
  $('#btnHeatmap20Min').addClass('disabled');
  $('#btnHeatmap10Min').addClass('disabled');
  $('#btnHeatmap1Hour').removeClass('active');
  $('#btnHeatmap30Min').removeClass('active');
  $('#btnHeatmap20Min').removeClass('active');
  $('#btnHeatmap10Min').removeClass('active');
  $('#btnStatsALLPhase').removeClass('active');
  $('#btnStatsRPhase').removeClass('active');
  $('#btnStatsSPhase').removeClass('active');
  $('#btnStatsTPhase').removeClass('active');
  $('#btnStatsALLPhase').addClass('disabled');
  $('#btnStatsRPhase').addClass('disabled');
  $('#btnStatsSPhase').addClass('disabled');
  $('#btnStatsTPhase').addClass('disabled');
  switch(localStorage.getItem('StatView')) {
    case 'Table' : 
      $('#btnShowAsTable').addClass('active');
      $('#ShowAsTables').removeClass('hidden');
      break;
    default :
      localStorage.setItem('StatView', 'Heatmap');
      $('#btnShowAsHeatmap').addClass('active');
      $('#ShowAsHeatmap').removeClass('hidden');
      $('#btnHeatmap1Hour').removeClass('disabled');
      $('#btnHeatmap30Min').removeClass('disabled');
      $('#btnHeatmap20Min').removeClass('disabled');
      $('#btnHeatmap10Min').removeClass('disabled');
      break;
  } 
  if(Nodes['Devices'][Node]['Type'] === '3Phase') {
    $('#btnStatsALLPhase').removeClass('disabled');
    $('#btnStatsRPhase').removeClass('disabled');
    $('#btnStatsSPhase').removeClass('disabled');
    $('#btnStatsTPhase').removeClass('disabled');
  }
  if(Nodes['Devices'][Node]['Type'] === '3Phase') {
    switch(localStorage.getItem('StatPhase')) {
      case 'R' :
        $('#btnStatsRPhase').addClass('active');
        SelectedPhase = [0];
        break;
      case 'S' :
        $('#btnStatsSPhase').addClass('active');
        SelectedPhase = [1];
        break;
      case 'T' :
        $('#btnStatsTPhase').addClass('active');
        SelectedPhase = [2];
        break;
      default :
        $('#btnStatsALLPhase').addClass('active');
        SelectedPhase = [0, 1, 2];
        break;
    }
  } else {
    SelectedPhase = [0];
  }
  if(localStorage.getItem('StatView') === 'Heatmap') {
    switch(localStorage.getItem('StatBlocks')) {
      case '30m' :
        $('#btnHeatmap30Min').addClass('active');
        break;
      case '20m' :
        $('#btnHeatmap20Min').addClass('active');
        break;
      case '10m' :
        $('#btnHeatmap10Min').addClass('active');
        break;
      default :
        $('#btnHeatmap1Hour').addClass('active');
        break;
    }
  }
  if((this.id === 'btnShowAsTable') || (this.id === 'btnShowAsHeatmap')) return;
  UpdateHeatMap();
  if((this.id === 'btnHeatmap1Hour') || (this.id === 'btnHeatmap30Min') || (this.id === 'btnHeatmap20Min') || (this.id === 'btnHeatmap10Min')) return;
  UpdateCalendar();
  UpdateWeeks();
  UpdateMonths();
  UpdateWeekDays();
}
$('#btnShowAsTable').click(ButtonClick);
$('#btnShowAsHeatmap').click(ButtonClick);
$('#btnStatsRPhase').click(ButtonClick);
$('#btnStatsSPhase').click(ButtonClick);
$('#btnStatsTPhase').click(ButtonClick);
$('#btnStatsALLPhase').click(ButtonClick);
$('#btnHeatmap1Hour').click(ButtonClick);
$('#btnHeatmap30Min').click(ButtonClick);
$('#btnHeatmap20Min').click(ButtonClick);
$('#btnHeatmap10Min').click(ButtonClick);
function UpdatePage() {
  GetDataRanges();  
}