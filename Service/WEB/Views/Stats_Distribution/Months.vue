<template>
  <div class="panel panel-other">
    <div class="panel-heading">By month
      <div class="pull-right">
        {{ Units }}Wh
        <button-export file="Distribution" :rows="Export" style="margin-top: -4px"></button-export>
      </div>
    </div>
    <div class="panel-body">
      <vuetable ref="table" :fields="Columns" :api-mode="false" :data="Months" class="table small table-striped"></vuetable>
    </div>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  import moment from 'moment';
  import 'moment-timezone';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  var Names = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
  export default {
    props: { data: Array, analyzer: Object },
    data: function() {
      return { Units: '', Columns: [], Months: [], Periods: [], ExportData: [] };
    },
    watch: {
      data: function() { this.SetData(); }
    },
    mounted: function() {
      if(this.data != null) this.SetData();
    },
    methods: {
      Export: function() {
        return this.ExportData;
      },
      SetData: function() {
        this.Months = [];
        this.Periods = [];
        if(this.data == null) return;
        var Max = 0.0;
        var TimeCol = this.data[0].indexOf('Time');
        var PeriodCol = this.data[0].indexOf('Period');
        var EnergyCol = this.data[0].indexOf('Active_Energy');
        for(var i = 1; i < this.data.length; i++) {
          var Month = moment(this.data[i][TimeCol]).tz(this.analyzer.TimeZone).month();
          if(this.Months[Month] == null) this.Months[Month] = { };
          var Period = (this.data[i][PeriodCol] === null) ? 'NONE' : 'P' + this.data[i][PeriodCol];
          if(this.Periods.includes(Period) === false) this.Periods.push(Period);
          if(this.data[i][EnergyCol] != null) this.Months[Month][Period] = (this.Months[Month][Period] == null) ? this.data[i][EnergyCol] : this.Months[Month][Period] + this.data[i][EnergyCol];
        }
        var Max = 0;
        for(var Month in this.Months) {
          for(var Period in this.Months[Month]) Max = Math.max(Max, this.Months[Month][Period]);
        }
        this.Units = this.BestUnits(Max);
        this.Periods = this.Periods.sort();
        this.Columns = [ { name: 'Month', title: 'Month' } ];
        for(var Period in this.Periods) this.Columns.push({ name: this.Periods[Period], title: this.Periods[Period] });
        this.$nextTick(() => { this.$refs.table.normalizeFields(); });
        this.ExportData = [['Month'].concat(this.Periods)];
        for(var Month in this.Months) {
          var Row = [Names[Month]];
          for(var Period in this.Months[Month]) {
            Row.push(this.Round(this.Months[Month][Period], 3));
            this.Months[Month][Period] = (this.Months[Month][Period] === null) ? '-' : this.Round(this.ScaleUnits(this.Months[Month][Period], this.Units), 2, true);
          }
          this.Months[Month].Month = Names[Month];
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