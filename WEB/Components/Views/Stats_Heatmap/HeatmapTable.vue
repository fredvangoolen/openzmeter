<template>
  <table style="width: 100%; font-size: 9px; text-align:center; table-layout: fixed">
    <thead style="font-weight: bold">
      <tr>
        <td style="width:16px">Day</td>
        <td style="width:54px">Date</td>
        <td colspan="4">00</td>
        <td colspan="4">01</td>
        <td colspan="4">02</td>
        <td colspan="4">03</td>
        <td colspan="4">04</td>
        <td colspan="4">05</td>
        <td colspan="4">06</td>
        <td colspan="4">07</td>
        <td colspan="4">08</td>
        <td colspan="4">09</td>
        <td colspan="4">10</td>
        <td colspan="4">11</td>
        <td colspan="4">12</td>
        <td colspan="4">13</td>
        <td colspan="4">14</td>
        <td colspan="4">15</td>
        <td colspan="4">16</td>
        <td colspan="4">17</td>
        <td colspan="4">18</td>
        <td colspan="4">19</td>
        <td colspan="4">20</td>
        <td colspan="4">21</td>
        <td colspan="4">22</td>
        <td colspan="4">23</td>
      </tr>
    </thead>
    <tbody ref="rows">
    </tbody>
  </table>
</template>

<script>
  export default {
    data() {
      return { Data: [] };
    },
    methods: {
      setMode: function(mode) {
        
      },
      SetData: function(data) {
      },
      Draw: function() {
        $(this.$refs.rows).html('');
//            var Block = parseInt(localStorage.getItem('StatsBlock'));
//  var Count = (Block === 0) ? 6 : (Block === 1) ? 3 : (Block === 2) ? 2 : 1;
//  var Body = $('#HeatMap tbody');
//  Body.html("");
//  var Days = [];
//  for(i = 0; i < EnergyData.length; i++) {
//    var Timestamp = new Date(EnergyData[i][0]);
//    var Day = GetDayOfYear(Timestamp.getTime());
//    if(Days[Day] === undefined) Days[Day] = {WeekDay: DaysNames[Timestamp.getDay()], Date: Timestamp.toLocaleDateString(), Cells: Array(24 * (6 / Count)).fill(undefined) };
//    var Cell = Math.trunc((EnergyData[i][0].getTime() % (24 * 3600 * 1000)) / (600000 * Count));
//    if((Cell > 143) || (Cell < 0)) alert(Cell);
//    if(Days[Day].Cells[Cell] === undefined) Days[Day].Cells[Cell] = 0;
//    Days[Day].Cells[Cell] = Days[Day].Cells[Cell] + SumPhases(EnergyData[i], visibleSeries);
//  }
//  var Max = -Infinity;
//  var Min =  Infinity;
//  Days.forEach(function(Day) {
//    Day.Cells.forEach(function(Cell) {
//      if(Cell === undefined) return;
//      Max = Math.max(Max, Cell);
//      Min = Math.min(Min, Cell);
//    });
//  });
//  var Units = GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
//  $('#MapMin').html(Round2(ScaleToUnits(Min, Units)));
//  $('#MapMax').html(Round2(ScaleToUnits(Max, Units)));
//  $('#MapUnits').html(Units + 'Wh');
//  var ColorScale = chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]);
//  var Len = 0;
//  Days.slice().reverse().forEach(function(Day) {
//    var Row = $('<tr></tr>');
//    Row.append('<td>' + Day.WeekDay + '</td>');
//    Row.append('<td style="border-right: 1px solid #333">' + Day.Date + '</td>');
//    for(var i = 0; i < Day.Cells.length; i++) {
//      var Cell = Day.Cells[i];
//      var Tmp = $('<td colspan="' + Count + '" data-toogle="tooltip"></td>');
//      var Color = ColorScale((Cell - Min) / (Max - Min));
//      Tmp.css('background-color', (Cell === undefined) ? '#555' : Color);
//      Tmp.attr('title', (Cell === undefined) ? 'No data for this period' : (Round2Plain(ScaleToUnits(Cell, Units)) + Units + 'Wh'));
//      Tmp.css('border', '1px solid #333');
//      Row.append(Tmp);
//    };
//     Len = Len + 1;
//    Body.append(Row);
//  });

      }
      
    }
  }
</script>

<style>

</style>