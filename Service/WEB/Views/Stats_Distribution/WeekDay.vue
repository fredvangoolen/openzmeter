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
  import moment from 'moment';
  import 'moment-timezone';
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  var Names = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"];
  export default {
    props: { data: Array, analyzer: Object },
    data() {
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
        var Max = 0.0;
        var TimeCol = this.data[0].indexOf('Time');
        var PeriodCol = this.data[0].indexOf('Period');
        var EnergyCol = this.data[0].indexOf('Active_Energy');
        for(var i = 1; i < this.data.length; i++) {
          var Day = moment(this.data[i][TimeCol]).day();
          if(this.Days[Day] == null) this.Days[Day] = { };
          var Period = (this.data[i][PeriodCol] == null) ? 'NONE' : 'P' + this.data[i][PeriodCol];
          if(this.Periods.includes(Period) == false) this.Periods.push(Period);
          if(this.data[i][EnergyCol] != null) this.Days[Day][Period] = (this.Days[Day][Period] == null) ? this.data[i][EnergyCol] : this.Days[Day][Period] + this.data[i][EnergyCol];
        }
        var Max = 0;
        for(var Day in this.Days) {
          for(var Period in this.Days[Day]) Max = Math.max(Max, this.Days[Day][Period]);
        }
        this.Units = this.BestUnits(Max);
        this.Periods = this.Periods.sort();
        this.Columns = [ { name: 'Day', title: 'Day' } ];
        for(var Period in this.Periods) this.Columns.push({ name: this.Periods[Period], title: this.Periods[Period] });
        this.$nextTick(() => { this.$refs.table.normalizeFields(); });
        this.ExportData = [['Week day'].concat(this.Periods)];
        for(var Day in this.Days) {
          var Row = [Names[Day]];
          for(var Period in this.Days[Day]) {
            Row.push(this.Round(this.Days[Day][Period], 3));
            this.Days[Day][Period] = (this.Days[Day][Period] == null) ? '-' : this.Round(this.ScaleUnits(this.Days[Day][Period], this.Units), 2, true);
          }
          this.Days[Day].Day = Names[Day];
          this.ExportData.push(Row);
        }
      }
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    components: { vuetable }
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