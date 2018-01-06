/*
 * Copyright (C) 2017 Eduardo Viciana Gamez
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
var LoadWeek;
var LastWeek;
var LastYear;
function SetWeekEvents(result) {
  var Table = $('#weeks_table tbody');
  var VoltagePercent = (result['Samples'] - result['Voltage']) * 100 / result['Samples'];
  var THDPercent = (result['Samples'] - result['THD']) * 100 / result['Samples'];
  var HarmonicsPercent = (result['Samples'] - result['Harmonics']) * 100 / result['Samples'];
  var Row = '<tr><td class="text-center"><strong>' + result['Week'] + "</strong>&nbsp;(" + result['Year'] + ')</td>' +
            '<td class="text-center">' + Round2(result['Samples'] * 100 / 1008) + '%</td>';
  if(result['Samples'] > 0) {
    Row += '<td>' +
           '  <div class="progress text-center" style="margin-bottom:0px; position: relative">' +
           '    <div class="progress-bar" role="progressbar" style="width:' + Math.min(100, VoltagePercent * 20) + '%; background-color: #5cb85c"></div>' +
           '    <span style="top: 2px; left: 0px; position: absolute; width: 100%; mix-blend-mode: multiply">' + Round2(VoltagePercent) + '% of time outside range</span>' +
           '  </div>' +
           '</td>' +
           '<td>' +
           '  <div class="progress text-center" style="margin-bottom:0px; position: relative">' +
           '    <div class="progress-bar" role="progressbar" style="width:' + Math.min(100, THDPercent * 20) + '%; background-color: #5cb85c"></div>' +
           '    <span style="top: 2px; left: 0px; position: absolute; width: 100%; mix-blend-mode: multiply">' + Round2(THDPercent) + '% of time outside range</span>' +
           '  </div>' +
           '</td>' +
           '<td>' +
           '  <div class="progress text-center" style="margin-bottom:0px; position: relative">' +
           '    <div class="progress-bar" role="progressbar" style="width:' + Math.min(100, HarmonicsPercent * 20) + '%; background-color: #5cb85c"></div>' +
           '    <span style="top: 2px; left: 0px; position: absolute; width: 100%; mix-blend-mode: multiply">' + Round2(HarmonicsPercent) + '% of time outside range</span>' +
           '  </div>' +
           '</td>';
  } else {
    Row += '<td colspan="4" class="text-center">No samples for this week</td>';
  }
  Table.append(Row + '</tr>');
  LastWeek = result['Week'];
  LastYear = result['Year'];
  if(result['Week'] === 1) {
    $('#weeks_table tfoot').hide();
  } else if(LoadWeek === 1) {
    $('#RowLoading').addClass('hide');
    $('#RowButton').removeClass('hide');
  } else {
    LoadWeek--;
    GetWeekInfo(result['Year'], LastWeek - 1);
  }
}
function GetWeekInfo(reqyear, reqweek) {
  $('#RowLoading').removeClass('hide');
  $('#RowButton').addClass('hide');
  $.ajax({ url: "events/getWeekEvents",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Year: reqyear, Week: reqweek }),
           success: SetWeekEvents
         });
}
function SetYearEvents(result) {
  $('#YearLabel').text('Year ' + result['Year']);
  var Day = result['Day'];
  var YearProgress = Day * 100 / result['Days'];
  var Dip = result['Dip'];
  var DipProgress = Dip * 100 / 1000;
  var Short = result['Short'];
  var ShortProgress = Short * 100 / 100;
  var Long = result['Long'];
  var LongProgress = Dip * 100 / 50;
  var FreqProgress = 100 - (10512000 - result['Freq1']) * 100 / 10512000;
  $('#dips_year').css('width', YearProgress  +'%');
  $('#dip').css('width', DipProgress  +'%');
  $('#dip_text').text(Dip + ' events in ' + Day + ' days');
  if(DipProgress > YearProgress) $('#dip').css('background-color', '#f0ad4e');
  if(Dip > 1000) $('#dip').css('background-color', '#d9534f"');
  $('#short_year').css('width', YearProgress  +'%');
  $('#short').css('width', ShortProgress  +'%');
  $('#short_text').text(Short + ' events in ' + Day + ' days');
  if(ShortProgress > YearProgress) $('#short').css('background-color', '#f0ad4e');
  if(Short > 100) $('#short').css('background-color', '#d9534f"');
  $('#long_year').css('width', YearProgress  +'%');
  $('#long').css('width', LongProgress  +'%');
  $('#long_text').text(Long + ' events in ' + Day + ' days');
  if(LongProgress > YearProgress) $('#long').css('background-color', '#f0ad4e');
  if(Long > 100) $('#long').css('background-color', '#d9534f"');  

  $('#freq_year').css('width', YearProgress  +'%');
  $('#freq').css('width', Math.min(100, FreqProgress * 200)  +'%');
  $('#freq_text').text(Math.round(result['Freq1'] / 20) + ' minutes out of range (' + Round2(FreqProgress) + '% of year)');
  if(FreqProgress > 0.048) $('#freq').css('background-color', '#f0ad4e');
  if(FreqProgress > 0.05) $('#freq').css('background-color', '#d9534f');
  
  $('#weeks_table tbody').html('');
  LoadWeek = 5;
  GetWeekInfo(result['Year'], 0);
}
function GetYearInfo(reqyear) {
  var End = GetEndYearTimestamp();
  var Start = GetYearTimestamp();
  $.ajax({ url: "getEvents",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Node : Node, To: End, Period: End - Start }),
           success: SetYearEvents
         });
}
function UpdatePage() {

$(document).ready(GetYearInfo(0));
$('#LoadNext').click(function() {
  LoadWeek = 5;
  GetWeekInfo(LastYear, LastWeek);
});
$('#LoadAll').click(function() {
  LoadWeek = 60;
  GetWeekInfo(LastYear, LastWeek);
});
}