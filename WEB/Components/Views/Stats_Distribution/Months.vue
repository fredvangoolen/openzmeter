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
    created: function() {
      this.Months = null;
      this.Periods = null;
    },
    methods: {
      GetData: function(data) {
        data.push(['Month'].concat(this.Periods));
        for(var month in this.Months) {
          var Row = [this.Months[month].Month];
          for(var period in this.Periods) Row.push(this.Months[month][this.Periods[period]]);
          data.push(Row);
        }
        return data;
      },
      SetData: function(result) {
        this.Months = [];
        this.Periods = [];
        var Max = 0.0;
        for(var i = 0; i < result['Active_Power'].length; i++) {
          var Month = moment(result.Sampletime[i]).month();
          if(this.Months[Month] === undefined) this.Months[Month] = { };
          var Period = (result.Period_Name[i] === null) ? 'NONE' : result.Period_Name[i];
          if(this.Periods.includes(Period) === false) this.Periods.push(Period);
          if(this.Months[Month][Period] === undefined) this.Months[Month][Period] = null;
          if(result['Active_Power'][i] !== null) this.Months[Month][Period] = (this.Months[Month][Period] === null) ? result['Active_Power'][i] : this.Months[Month][Period] + result['Active_Power'][i];
          Max = Math.max(Max, Math.abs(this.Months[Month][Period]));
        }
        this.Units = oZm.GetBestUnits(Max);
        this.Periods = this.Periods.sort();
        this.Columns = [ { name: 'Month', title: 'Month' } ]; 
        for(var Period in this.Periods) this.Columns.push({ name: this.Periods[Period], title: this.Periods[Period] });
        this.$nextTick(function() {
          this.$refs.table.normalizeFields();
        });
        for(var Month in this.Months) {
          for(var Period in this.Months[Month]) this.Months[Month][Period] = (this.Months[Month][Period] === null) ? '-' : oZm.Round2Plain(oZm.ScaleToUnits(this.Months[Month][Period], this.Units));
          this.Months[Month].Month = Names[Month];          
        }
        this.$refs.table.setData({ data: this.Months });
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