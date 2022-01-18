<template>
  <div class="panel panel-other">
    <div class="panel-heading">By week day
      <div class="float-right">
        {{ Units }}Wh
        <button-export file="Distribution" :rows="Export" style="margin-top: -4px"></button-export>
      </div>
    </div>
    <div class="panel-body">
      <vuetable ref="table" :fields="Columns" :data="Days" :api-mode="false" class="table small table-striped"></vuetable>
    </div>
  </div>
</template>

<script>
  //import moment from 'moment';
  //import 'moment-timezone';
  let Names = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"];
  export default {
    props: { data: Array },
    data: function() {
      return { Columns: [], Units: '', Days: [], Periods: [], ExportData: [] };
    },
    watch: {
      data: function() {
        this.SetData();
      }
    },
    mounted: function() {
      if(this.data != null) this.SetData();
    },
    methods: {
      Export: function() {
        return this.ExportData;
      },
      SetData: function() {
        this.Days = [];
        this.Periods = [];
        if(this.data == null) return;
        let TimeCol = this.data[0].indexOf('Time');
        let PeriodCol = this.data[0].indexOf('Period');
        let EnergyCol = this.data[0].indexOf('Active_Energy');
        for(let i = 1; i < this.data.length; i++) {
          let Day = moment(this.data[i][TimeCol]).day();
          if(this.Days[Day] == null) this.Days[Day] = { };
          let Period = (this.data[i][PeriodCol] == null) ? 'NONE' : 'P' + this.data[i][PeriodCol];
          if(this.Periods.includes(Period) == false) this.Periods.push(Period);
          if(this.data[i][EnergyCol] != null) this.Days[Day][Period] = (this.Days[Day][Period] == null) ? this.data[i][EnergyCol] : this.Days[Day][Period] + this.data[i][EnergyCol];
        }
        let Max = 0.0;
        for(let Day in this.Days) {
          for(let Period in this.Days[Day]) Max = Math.max(Max, this.Days[Day][Period]);
        }
        this.Units = this.BestUnits(Max);
        this.Periods = this.Periods.sort();
        this.Columns = [ { name: 'Day', title: 'Day' } ];
        for(let Period in this.Periods) this.Columns.push({ name: this.Periods[Period], title: this.Periods[Period] });
        this.$nextTick(() => { this.$refs.table.normalizeFields(); });
        this.ExportData = [['Week day'].concat(this.Periods)];
        for(let Day in this.Days) {
          let Row = [Names[Day]];
          for(let Period in this.Days[Day]) {
            Row.push(this.Round(this.Days[Day][Period], 3));
            this.Days[Day][Period] = (this.Days[Day][Period] == null) ? '-' : this.Round(this.ScaleUnits(this.Days[Day][Period], this.Units), 2, true);
          }
          this.Days[Day].Day = Names[Day];
          this.ExportData.push(Row);
        }
      }
    }
  }
</script>

<style scoped>
@media screen and (min-width: 600px) {
  .table >>> th:not(:first-child) {
    text-align: right;
    width: 80px;
  }
  .table >>> td:not(:first-child) {
    text-align: right;
  }
}
</style>