<template>
  <table class='heatmap_table'>
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
      <tr v-for="day in Days"><td>{{ day.DayOfWeek }}</td><td>{{ day.Date }}</td><td v-for="block in day.Cells" :colspan="(Aggregation === '1H') ? 4 : 1" data-toggle="tooltip" :title="block.Text" v-bind:style="{ 'background-color': block.Color }"></td></tr>
    </tbody>
  </table>
</template>

<script>
  import moment from 'JS/moment.js';
  import chroma from 'JS/chroma.js';
  import * as oZm from 'JS/openZmeter.js';
  var DaysNames = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"];
  export default {
    data: function() {
      return { Days: {}, Aggregation: '1H' };
    },
    created: function() {
      this.Data = null;
    },
    methods: {
      SetAggregation: function(aggreg) {
        this.Aggregation = aggreg;
        this.CalcOptions();
      },
      SetData: function(data) {
        this.Data = data;
        this.CalcOptions();
      },
      GetData: function() {
        var Data = [];
        var Header = ['Day', 'Date'];
        for(var h = 0; h < 24; h++) {
          if(this.Aggregation === '15M') {
            for(var q = 1; q < 5; q++) Header.push(("0" + h).slice(-2) + ' Q' + q);
          } else {
            Header.push(("0" + h).slice(-2));
          }
        }
        Data.push(Header);
        for(var Day in this.Days) {
          var Row = [this.Days[Day].DayOfWeek, this.Days[Day].Date];
          for(var i = 0; i < this.Days[Day].Cells.length; i++) Row.push((this.Days[Day].Cells[i].Value === undefined) ? '' : this.Days[Day].Cells[i].Value);
          Data.push(Row);
        }       
        return Data;
      },
      CalcOptions: function() {
        if(this.Data === null) return;
        var Days = {};
        for(var i = 0; i < this.Data.Sampletime.length; i++) {
          var Day = moment(this.Data.Sampletime[i]).dayOfYear();
          if(Days[Day] === undefined) Days[Day] = {DayOfWeek: DaysNames[moment(this.Data.Sampletime[i]).day()], Date: moment(this.Data.Sampletime[i]).format('DD/MM/YYYY'), Cells: Array((this.Aggregation === '1H') ? 24 : 96).fill({ Value: undefined })};
          var Cell = moment(this.Data.Sampletime[i]).hour();
          if(this.Aggregation === '15M') Cell = Cell * 4 + Math.trunc(moment(this.Data.Sampletime[i]).minute() / 15);
          if(Days[Day].Cells[Cell].Value === undefined) Days[Day].Cells[Cell] = { Value: 0 };
          Days[Day].Cells[Cell].Value = Days[Day].Cells[Cell].Value + this.Data.Active_Power[i][0] / 4.0;
        }
        var Max = -Infinity;
        var Min =  Infinity;
        for(var Day in Days) {    
          for(var Cell in Days[Day].Cells) {
            if(Days[Day].Cells[Cell].Value === undefined) {
              Days[Day].Cells[Cell].Text = 'No data';  
            } else {
              Max = Math.max(Max, Days[Day].Cells[Cell].Value);
              Min = Math.min(Min, Days[Day].Cells[Cell].Value);
              var Units = oZm.GetBestUnits(Days[Day].Cells[Cell].Value);
              Days[Day].Cells[Cell].Text = oZm.Round2Plain(oZm.ScaleToUnits(Days[Day].Cells[Cell].Value, Units)) + ' ' + Units + 'Wh';
            }
          };
        };
        var Units = oZm.GetBestUnits(Math.max(Math.abs(Min), Math.abs(Max)));
        this.$emit('RangeChanged', oZm.ScaleToUnits(Min, Units), oZm.ScaleToUnits(Max, Units), Units);
        var ColorScale = chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]);
        for(var Day in Days) {    
          for(var Cell in Days[Day].Cells) {
            if(Days[Day].Cells[Cell].Value === undefined) {
              Days[Day].Cells[Cell].Color = '#EEEEEE';
            } else {
              Days[Day].Cells[Cell].Color = ColorScale((Days[Day].Cells[Cell].Value - Min) / (Max - Min));
            }
          };
        };
        this.Days = Days;
      }
    }
  }
</script>

<style>
  .heatmap_table {
    width: 100%; 
    font-size: 9px; 
    text-align:center; 
    table-layout: fixed
  }
  .heatmap_table thead td:nth-child(n+3) {
     border-bottom: 1px solid #CCCCCC;
  }
  .heatmap_table tbody td:nth-child(2) {
     border-right: 1px solid #CCCCCC;
  }
  .heatmap_table tbody tr:last-child td:nth-child(n+3) {
     border-bottom: 1px solid #CCCCCC;
  }
  .heatmap_table tbody tr td:last-child {
     border-right: 1px solid #CCCCCC;
  }
</style>