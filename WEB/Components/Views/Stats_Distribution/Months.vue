<template>
  <div class="panel panel-other">
    <div class="panel-heading">By month<div class="pull-right">{{ Units }}Wh</div></div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive"></vuetable>
    </div>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  import Spinner from 'Components/Spinner.vue';
  var Names = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
  export default {
    data: function() {
      return { ShowSpinner: true, Units: '', Columns: [] };
    },
    methods: {
      SetData: function(result) {
        var Months = [];
        var Periods = [];
        var Max = 0.0;
        for(var i = 0; i < result['Active_Power'].length; i++) {
          var Month = moment(result.Sampletime[i]).month();
          if(Months[Month] === undefined) Months[Month] = { };
          var Period = (result.Period_Name[i] === null) ? 'NONE' : result.Period_Name[i];
          if(Periods.includes(Period) === false) Periods.push(Period);
          if(Months[Month][Period] === undefined) Months[Month][Period] = null;
          if(result['Active_Power'][i] !== null) Months[Month][Period] = (Months[Month][Period] === null) ? result['Active_Power'][i] : Months[Month][Period] + result['Active_Power'][i];
          Max = Math.max(Max, Math.abs(Months[Month][Period]));
        }
        this.Units = oZm.GetBestUnits(Max);
        Periods = Periods.sort();
        this.Columns = [ { name: 'Month', title: 'Month' } ]; 
        for(var Period in Periods) this.Columns.push({ name: Periods[Period], title: Periods[Period] });
        this.$nextTick(function() {
          this.$refs.table.normalizeFields();
        });
        for(var Month in Months) {
          for(var Period in Months[Month]) Months[Month][Period] = (Months[Month][Period] === null) ? '-' : oZm.Round2Plain(oZm.ScaleToUnits(Months[Month][Period], this.Units));
           Months[Month].Month = Names[Month];          
        }
        this.$refs.table.setData({ data: Months });
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