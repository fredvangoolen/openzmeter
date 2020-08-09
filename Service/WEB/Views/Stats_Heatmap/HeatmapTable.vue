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
  import chroma from 'chroma-js';
  import * as oZm from 'JS/openZmeter.js';
  var DaysNames = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"];
  export default {
    props: { analyzer: Object, data: Array, aggreg: String },
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
      CalcOptions: function() {
        this.Days = [];
        if((this.data == null) || (this.aggreg == null)) return;
        var Headers   = this.data[0];
        var Rows      = this.data.slice(1);
        var Tmp       = { };
        var TimeCol   = Headers.indexOf('Sampletime');
        var EnergyCol = Headers.indexOf('Active_Energy');
        for(var i = 0; i < Rows.length; i++) {
          var RowDate = moment(Rows[i][TimeCol]).tz(this.analyzer.TimeZone);
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
          for(var c = 0; c < Rows[i][EnergyCol].length; c++) {
            Tmp.Cells[Cell].Value = (Tmp.Cells[Cell].Value == null) ? Rows[i][EnergyCol][c] : Tmp.Cells[Cell].Value + Rows[i][EnergyCol][c];
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
        this.$emit('update:range', [Min, Max]);
        var ColorScale = chroma.bezier(['#00429d', '#96ffea', '#ffffe0', '#ff005e', '#93003a']);
        var Export = [];
        var Row = ['Day', 'Date'];
        for(var h = 0; h < 24; h++) {
          if(this.aggreg == '15M') {
            for(var q = 1; q < 5; q++) Row.push(("0" + h).slice(-2) + ' Q' + q);
          } else {
            Row.push(("0" + h).slice(-2));
          }
        }
        Export.push(Row);
        for(var i in this.Days) {
          var Row = [this.Days[i].DayOfWeek, this.Days[i].Date];
          for(var n in this.Days[i].Cells) {
            Row.push(this.Round(this.Days[i].Cells[n].Value, 3));
            if(this.Days[i].Cells[n].Value === undefined) {
              this.Days[i].Cells[n].Color = "url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAQAAAAECAIAAAAmkwkpAAAAJElEQVQImT2JsQ0AMAyDyP832q+QIWqZEJDEx6gA0JZr9/mmLvBPKENuQmcdAAAAAElFTkSuQmCC')";
              this.Days[i].Cells[n].BorderColor = '#EEE'
              this.Days[i].Cells[n].Text = 'No data';
            } else {
              this.Days[i].Cells[n].Color = ColorScale((this.Days[i].Cells[n].Value - Min) / (Max - Min));
              this.Days[i].Cells[n].BorderColor = chroma(this.Days[i].Cells[n].Color).darken(0.1);
              var Units = this.BestUnits(this.Days[i].Cells[n].Value);
              this.Days[i].Cells[n].Text = this.Round(this.ScaleUnits(this.Days[i].Cells[n].Value, Units), 2, false) + ' ' + Units + 'Wh';

            }
          }
          if(Row.length > 2) Export.push(Row);
        }
        this.Days = this.Days.reverse();
        this.$emit('update:export', Export);
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