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
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  var Names = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"];
  export default {
    props: { data: Object, analyzer: Object },
    data() {
      return { Columns: [], Units: '', Days: [], Periods: [], Export: [] };
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
      SetData: function() {
        this.Days = [];
        this.Periods = [];
        var Max = 0.0;
        for(var i in this.data.Active_Power) {
          var Day = moment(this.data.Sampletime[i]).day();
          if(this.Days[Day] === undefined) this.Days[Day] = {};
          var Period = (this.data.Period_Name[i] === null) ? 'NONE' : this.data.Period_Name[i];
          if(this.Periods.includes(Period) === false) this.Periods.push(Period);
          if(this.data.Active_Power[i] !== null) this.Days[Day][Period] = (this.Days[Day][Period] == undefined) ? this.data.Active_Power[i] : this.Days[Day][Period] + this.data.Active_Power[i];
          Max = Math.max(Max, Math.abs(this.Days[Day][Period]));
        }
        this.Units = oZm.GetBestUnits(Max);
        this.Periods = this.Periods.sort();
        this.Columns = [ { name: 'Day', title: 'Day' } ];
        for(var Period in this.Periods) this.Columns.push({ name: this.Periods[Period], title: this.Periods[Period] });
        this.$nextTick(() => { this.$refs.table.normalizeFields(); });
        this.Export = [['Week day'].concat(this.Periods)];
        for(var Day in this.Days) {
          var Row = [Names[Day]];
          for(var Period in this.Days[Day]) {
            this.Days[Day][Period] = (this.Days[Day][Period] === null) ? '-' : oZm.Round2Plain(oZm.ScaleToUnits(this.Days[Day][Period], this.Units));
            Row.push(this.Days[Day][Period]);
          }
          this.Days[Day].Day = Names[Day];
          this.Export.push(Row);
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