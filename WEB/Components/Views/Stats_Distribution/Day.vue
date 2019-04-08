<template>
  <div class="panel panel-other">
    <div class="panel-heading">By week day<div class="float-right">{{ Units }}Wh</div></div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive"></vuetable>
    </div>
  </div>
</template>

<script>
  import moment from 'JS/moment.js';
  import vuetable from 'vuetable-2';
  import * as oZm from 'JS/openZmeter.js';
  import Spinner from 'Components/Spinner.vue';
  var Names = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"];
  export default {
    data() {
      return { ShowSpinner: true, Columns: [], Units: '' };
    },
    methods: {
      SetData(result) {
        var Days = [];
        var Periods = [];
        var Max = 0.0;
        for(var i = 0; i < result['Active_Power'].length; i++) {
          var Day = moment(result.Sampletime[i]).day();
          if(Days[Day] === undefined) Days[Day] = {};
          var Period = (result.Period_Name[i] === null) ? 'NONE' : result.Period_Name[i];
          if(Periods.includes(Period) === false) Periods.push(Period);
          if(Days[Day][Period] === undefined) Days[Day][Period] = null;
          if(result['Active_Power'][i] !== null) Days[Day][Period] = (Days[Day][Period] === null) ? result['Active_Power'][i] : Days[Day][Period] + result['Active_Power'][i];
          Max = Math.max(Max, Math.abs(Days[Day][Period]));
        }
        this.Units = oZm.GetBestUnits(Max);
        Periods = Periods.sort();
        this.Columns = [ { name: 'Day', title: 'Day' } ]; 
        for(var Period in Periods) this.Columns.push({ name: Periods[Period], title: Periods[Period] });
        this.$nextTick(function() {    
          this.$refs.table.normalizeFields();
        });
        for(var Day in Days) {
          for(var Period in Days[Day]) Days[Day][Period] = (Days[Day][Period] === null) ? '-' : oZm.Round2Plain(oZm.ScaleToUnits(Days[Day][Period], this.Units));
           Days[Day].Day = Names[Day];          
        }
        this.$refs.table.setData({ data: Days });
        this.ShowSpinner = false;
      }
    },
    components: { vuetable, Spinner }
  }
</script>

<style scoped>
  .table >>> th:not(:first-child) {
    text-align: right;
    width: 80px;
  }
  .table >>> td:not(:first-child) {
    text-align: right;
  }
</style>