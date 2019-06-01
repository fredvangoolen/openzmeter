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
    created: function() {
      this.Days = null;
      this.Periods = null;
    },
    methods: {
      GetData: function(data) {
        data.push(['Week day'].concat(this.Periods));
        for(var day in this.Days) {
          var Row = [this.Days[day].Day];
          for(var period in this.Periods) Row.push(this.Days[day][this.Periods[period]]);
          data.push(Row);
        }
        return data;
      },
      SetData: function(result) {
        this.Days = [];
        this.Periods = [];
        var Max = 0.0;
        for(var i = 0; i < result['Active_Power'].length; i++) {
          var Day = moment(result.Sampletime[i]).day();
          if(this.Days[Day] === undefined) this.Days[Day] = {};
          var Period = (result.Period_Name[i] === null) ? 'NONE' : result.Period_Name[i];
          if(this.Periods.includes(Period) === false) this.Periods.push(Period);
          if(this.Days[Day][Period] === undefined) this.Days[Day][Period] = null;
          if(result['Active_Power'][i] !== null) this.Days[Day][Period] = (this.Days[Day][Period] === null) ? result['Active_Power'][i] : this.Days[Day][Period] + result['Active_Power'][i];
          Max = Math.max(Max, Math.abs(this.Days[Day][Period]));
        }
        this.Units = oZm.GetBestUnits(Max);
        this.Periods = this.Periods.sort();
        this.Columns = [ { name: 'Day', title: 'Day' } ]; 
        for(var Period in this.Periods) this.Columns.push({ name: this.Periods[Period], title: this.Periods[Period] });
        this.$nextTick(function() {    
          this.$refs.table.normalizeFields();
        });
        for(var Day in this.Days) {
          for(var Period in this.Days[Day]) this.Days[Day][Period] = (this.Days[Day][Period] === null) ? '-' : oZm.Round2Plain(oZm.ScaleToUnits(this.Days[Day][Period], this.Units));
           this.Days[Day].Day = Names[Day];          
        }
        this.$refs.table.setData({ data: this.Days });
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