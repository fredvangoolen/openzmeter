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
var DaysNames = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"];
var EnergyData = [];
var SelectedYear;
function AjaxGetEnergy(selected_year) {
  $('#StatsSpinner').show();
  $('#YearSelector button').removeClass('active');
  $('#btnSelYear' + selected_year).addClass('active');
  SelectedYear = selected_year;
  $('#StatsSpinner').show();
  var To = new Date(selected_year, 11, 31, 23, 59, 59, 999).getTime();
  var From = new Date(selected_year, 0, 1, 0, 0, 0, 0).getTime();
  Ajax.GetSeries(To - From, To, '10m', 0, 'active_energy',
    function(result) {
      EnergyData = GraphDataAppend(result, 'active_energy', []);
      $('#StatsSpinner').hide();
      EnergySeries();      
    }
  );
}
function ShowColorScale(scale, outerdiv) {
  scale.out('hex');
  var width = outerdiv.width() - 1;
  for(i = 0; i <= width; i++) outerdiv.append('<div style="width:1px; height:8px; display:inline; background:' + scale(i / width) + '"></div>');
}
function SumPhases(array, visibleSeries) {
  var Energy = 0.0;
  for(phase = 0; phase < visibleSeries.length; phase++) {
    if(visibleSeries[phase] === true) Energy = Energy + EnergyData[i][phase + 1];
  }
  return Energy;
}
function UpdateHeatMap(visibleSeries) {
  var Count = 1;
  if($('#btnHeatmap1Hour').hasClass('active')) Count = 6;
  if($('#btnHeatmap30Min').hasClass('active')) Count = 3;
  if($('#btnHeatmap20Min').hasClass('active')) Count = 2;
  if($('#btnHeatmap10Min').hasClass('active')) Count = 1;
  Count = 1;
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
  var ColorScale = chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]);
  Days.forEach(function(Day) {
    var Row = '<td>' + Day.WeekDay + '</td><td>' + Day.Date + '</td>';
    Day.Cells.forEach(function(Cell) {
      var Color = (Cell === undefined) ? '#EEE' : ColorScale((Cell - Min) / (Max - Min));
      Row = Row + '<td style="background-color: ' + Color + '" colspan="' + Count + '" data-toogle="tooltip" title="' + ((Cell === undefined) ? 'No data for this period' : Round2Plain(ScaleToUnits(Cell, Units)) + Units + 'Wh') + '"></td>';
    });
    Body.append('<tr>' + Row + '</tr>');
  });
}
function UpdateView(event) {
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
  if(Analyzers[Analyzer]['Mode'] === '3Phase') {
    $('#btnStatsALLPhase').removeClass('disabled');
    $('#btnStatsRPhase').removeClass('disabled');
    $('#btnStatsSPhase').removeClass('disabled');
    $('#btnStatsTPhase').removeClass('disabled');

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
  UpdateHeatMap();
}
//$('#btnHeatmap1Hour').click(ButtonClick);
//$('#btnHeatmap30Min').click(ButtonClick);
//$('#btnHeatmap20Min').click(ButtonClick);
//$('#btnHeatmap10Min').click(ButtonClick);

function EnergySeries(event) {
  GroupButtons($('#groupEnergyPhases'), 'VisibleStatsPhases', 3, false, UpdateHeatMap, (event !== undefined) ? event.target : undefined);
}
$('#groupEnergyPhases').on('click', 'button', EnergySeries);
function UpdatePage() {
  Ajax.GetSeriesRange('10m', 
    function(result) {
      for(year = new Date(result['From']).getFullYear(); year <= new Date(result['To']).getFullYear(); year++) {
        $('#YearSelector').append('<button type="button" class="btn btn-default btn-xs" id="btnSelYear' + year + '" onclick="AjaxGetEnergy(' + year + ')">' + year + '</button>');
      }
      AjaxGetEnergy(new Date(result['To']).getFullYear());
    }
  );
}
