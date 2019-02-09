import * as oZm from './openZmeter.js';
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
var DaysNames = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"], EnergyData = [], SelectedYear;
function AjaxGetEnergy(selected_year) {
  $('#StatsSpinner').show();
  $('#btnYear + ul > li').removeClass('active');
  $('#btnYear + ul > li > a:contains(' + selected_year +')').parent().addClass('active');
  $('#btnYear span:first').text(selected_year);
  SelectedYear = selected_year;
  $('#StatsSpinner').show();
  var To = new Date(selected_year, 11, 31, 23, 59, 59, 999).getTime();
  var From = new Date(selected_year, 0, 1, 0, 0, 0, 0).getTime();
  Analyzer.GetSeries(To - From, To, '15M', 0, ['ACTIVE_POWER'],
    function(result) {
      EnergyData = GraphDataAppend(result, 'Active_Power', []);
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
function UpdateHeatMap(visibleSeries) {
  var Block = parseInt(localStorage.getItem('StatsBlock'));
  var Count = (Block === 0) ? 6 : (Block === 1) ? 3 : (Block === 2) ? 2 : 1;
  var Body = $('#HeatMap tbody');
  Body.html("");
  var Days = [];
  for(i = 0; i < EnergyData.length; i++) {
    var Timestamp = new Date(EnergyData[i][0]);
    var Day = GetDayOfYear(Timestamp.getTime());
    if(Days[Day] === undefined) Days[Day] = {WeekDay: DaysNames[Timestamp.getDay()], Date: Timestamp.toLocaleDateString(), Cells: Array(24 * (6 / Count)).fill(undefined) };
    var Cell = Math.trunc((EnergyData[i][0].getTime() % (24 * 3600 * 1000)) / (600000 * Count));
    if((Cell > 143) || (Cell < 0)) alert(Cell);
    if(Days[Day].Cells[Cell] === undefined) Days[Day].Cells[Cell] = 0;
    Days[Day].Cells[Cell] = Days[Day].Cells[Cell] + SumPhases(EnergyData[i], visibleSeries);
  }
  var Max = -Infinity;
  var Min =  Infinity;
  Days.forEach(function(Day) {
    Day.Cells.forEach(function(Cell) {
      if(Cell === undefined) return;
      Max = Math.max(Max, Cell);
      Min = Math.min(Min, Cell);
    });
  });
  var Units = GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
  $('#MapMin').html(Round2(ScaleToUnits(Min, Units)));
  $('#MapMax').html(Round2(ScaleToUnits(Max, Units)));
  $('#MapUnits').html(Units + 'Wh');
  var ColorScale = chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]);
  var Len = 0;
  Days.slice().reverse().forEach(function(Day) {
    var Row = $('<tr></tr>');
    Row.append('<td>' + Day.WeekDay + '</td>');
    Row.append('<td style="border-right: 1px solid #333">' + Day.Date + '</td>');
    for(var i = 0; i < Day.Cells.length; i++) {
      var Cell = Day.Cells[i];
      var Tmp = $('<td colspan="' + Count + '" data-toogle="tooltip"></td>');
      var Color = ColorScale((Cell - Min) / (Max - Min));
      Tmp.css('background-color', (Cell === undefined) ? '#555' : Color);
      Tmp.attr('title', (Cell === undefined) ? 'No data for this period' : (Round2Plain(ScaleToUnits(Cell, Units)) + Units + 'Wh'));
      Tmp.css('border', '1px solid #333');
      Row.append(Tmp);
    };
     Len = Len + 1;
    Body.append(Row);
  });
}
function EnergySeries(event) {
  GroupButtons($('#groupEnergyPhases'), 'VisibleStatsPhases', 3, false, UpdateHeatMap, (event !== undefined) ? event.target : undefined);
}
$('#groupEnergyPhases').on('click', 'button', EnergySeries);
export function UpdateStatsHeatmap() {
  var State = parseInt(localStorage.getItem('StatsBlock'));
  var NumElements = $('#btnBlocks + ul').children().length;
  if(isNaN(State) || (State < 0) || (State > (NumElements - 1))) State = 0;
  $('#btnBlocks + ul > li:nth-child(' + (State + 1) + ')').addClass('active');
  $('#btnBlocks span:first').text($('#btnBlocks + ul > li:nth-child(' + (State + 1) + ') > a').text());
  localStorage.setItem('StatsBlock', State);
  Analyzer.GetSeriesRange('10M', 
    function(result) {
      for(year = new Date(result['From']).getFullYear(); year <= new Date(result['To']).getFullYear(); year++) {
        $('#btnYear + ul').append('<li><a href="#">' + year + '</a></li>');
      }
      $('#btnYear + ul > li > a').click(function(evt) { 
        evt.preventDefault();
        AjaxGetEnergy(parseInt(this.text));
      });
      $('#btnBlocks + ul > li > a').click(function(evt) {
        evt.preventDefault();
        $('#btnBlocks').parent().find('li').removeClass('active');
        $('#btnBlocks').parent().find('li > a:contains(' + this.text +')').parent().addClass('active');
        $('#btnBlocks span:first').text(this.text);
        localStorage.setItem('StatsBlock', $(this).parent().index());
        EnergySeries();
      });
      AjaxGetEnergy(new Date(result['To']).getFullYear());
      var ColorScale = chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]);
      ShowColorScale(ColorScale, $('#CalendarScale'));
    }
  );
}
