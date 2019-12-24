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
      <tr v-for="(day, idxd) in Days" :key="idxd">
        <td>{{ day.DayOfWeek }}</td>
        <td>{{ day.Date }}</td>
        <td v-for="(block, idx) in day.Cells" :key="idx" :colspan="(aggreg === '1H') ? 4 : 1" :title="day.Date + ' - ' + block.Date + '\x0A' + block.Text" v-bind:style="{ 'border': '1px solid', 'border-color': block.BorderColor, 'background': block.Color }"></td>
      </tr>
    </tbody>
  </table>
</template>

<script>
  import moment from 'moment';
  import chroma from 'JS/chroma.js';
  import * as oZm from 'JS/openZmeter.js';
  var DaysNames = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"];
  export default {
    props: { data: { type: Object, default: null }, aggreg: { type: String, default: '1H'} },
    data: function() {
      return { Days: {} };
    },
    watch: {
      aggreg: function() { this.CalcOptions(); },
      data: function() { this.CalcOptions(); }
    },
    mounted: function() {
      this.CalcOptions();
    },
    methods: {
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
        this.Days = [];
        if((this.data == null) || (this.aggreg == null)) return;
        var Tmp = { };
        for(var i in this.data.Sampletime) {
          var RowDate = moment(this.data.Sampletime[i]);
          if(Tmp.Date != RowDate.format('DD/MM/YYYY')) {
            this.Days.push(Object.assign({}, Tmp));
            Tmp = { };
          }
          if(Tmp.Date == undefined) {
            Tmp = { DayOfWeek: DaysNames[RowDate.day()], Date: RowDate.format('DD/MM/YYYY'), Cells: [] };
            for(var n = 0; n < 24; n++) {
              if(this.aggreg === '1H') {
                Tmp.Cells.push({ Value: undefined, Date: 'Hour ' + n });
              } else {
                for(var q = 1; q < 5; q++) Tmp.Cells.push({ Value: undefined, Date: 'Hour ' + n + '(' + q + 'Q)' });  
              }
            }
          }
          var Cell = (this.aggreg === '15M') ? RowDate.hour() * 4 + Math.trunc(RowDate.minute() / 15) : RowDate.hour();
          for(var c in this.data.Active_Power[i]) {
            if(this.data.Active_Power[i][c] != null) Tmp.Cells[Cell].Value = (Tmp.Cells[Cell].Value === undefined) ? this.data.Active_Power[i][c] / 4.0 : Tmp.Cells[Cell].Value + this.data.Active_Power[i][c] / 4.0;
          }
        }
        if(Tmp.Date != undefined) this.Days.push(Tmp);
        var Max = -Infinity;
        var Min =  Infinity;
        for(var i in this.Days) {
          for(var n in this.Days[i].Cells) {
            if(this.Days[i].Cells[n].Value === undefined) continue;
            Max = Math.max(Max, this.Days[i].Cells[n].Value);
            Min = Math.min(Min, this.Days[i].Cells[n].Value);
          }
        }
        this.$emit('range', [Min, Max]);
        var ColorScale = chroma.bezier(['#00429d', '#96ffea', '#ffffe0', '#ff005e', '#93003a']);
        for(var i in this.Days) {
          for(var n in this.Days[i].Cells) {
            if(this.Days[i].Cells[n].Value === undefined) {
              this.Days[i].Cells[n].Color = "url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAQAAAAECAIAAAAmkwkpAAAAJElEQVQImT2JsQ0AMAyDyP832q+QIWqZEJDEx6gA0JZr9/mmLvBPKENuQmcdAAAAAElFTkSuQmCC')";
              this.Days[i].Cells[n].BorderColor = '#EEE'
              this.Days[i].Cells[n].Text = 'No data';
            } else {
              this.Days[i].Cells[n].Color = ColorScale((this.Days[i].Cells[n].Value - Min) / (Max - Min));
              this.Days[i].Cells[n].BorderColor = chroma(this.Days[i].Cells[n].Color).darken(0.1);
              var Units = oZm.GetBestUnits(this.Days[i].Cells[n].Value);
              this.Days[i].Cells[n].Text = oZm.RoundPlain(oZm.ScaleToUnits(this.Days[i].Cells[n].Value, Units), 2) + ' ' + Units + 'Wh';

            }
          }
        }
        this.Days = this.Days.reverse();
      }
    }
  }
</script>

<style scoped>
  .heatmap_table {
    width: 100%; 
    font-size: 9px; 
    text-align: center; 
    table-layout: fixed;
    border-collapse: separate;
  }
  .heatmap_table tbody td {
    background-repeat: repeat-x;
  }

</style>