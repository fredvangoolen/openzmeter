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
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  var Names = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
  export default {
    props: { data: Object, analyzer: Object },
    data: function() {
      return { Units: '', Columns: [], Months: [], Periods: [], Export: [] };
    },
    watch: {
      data: function() { this.SetData(); }
    },
    mounted: function() {
      if(this.data != null) this.SetData();
    },
    methods: {
      SetData: function() {
        this.Months = [];
        this.Periods = [];
        var Max = 0.0;
        for(var i in this.data.Active_Power) {
          var Month = moment(this.data.Sampletime[i]).month();
          if(this.Months[Month] === undefined) this.Months[Month] = { };
          var Period = (this.data.Period_Name[i] === null) ? 'NONE' : this.data.Period_Name[i];
          if(this.Periods.includes(Period) === false) this.Periods.push(Period);
          if(this.data.Active_Power[i] != null) this.Months[Month][Period] = (this.Months[Month][Period] == undefined) ? this.data.Active_Power[i] : this.Months[Month][Period] + this.data.Active_Power[i];
          Max = Math.max(Max, Math.abs(this.Months[Month][Period]));
        }
        this.Units = oZm.GetBestUnits(Max);
        this.Periods = this.Periods.sort();
        this.Columns = [ { name: 'Month', title: 'Month' } ];
        for(var Period in this.Periods) this.Columns.push({ name: this.Periods[Period], title: this.Periods[Period] });
        this.$nextTick(() => { this.$refs.table.normalizeFields(); });
        this.Export = [['Month'].concat(this.Periods)];
        for(var Month in this.Months) {
          var Row = [Names[Month]];
          for(var Period in this.Months[Month]) {
            this.Months[Month][Period] = (this.Months[Month][Period] === null) ? '-' : oZm.RoundPlain(oZm.ScaleToUnits(this.Months[Month][Period], this.Units), 2);
            Row.push(this.Months[Month][Period]);
          }
          this.Months[Month].Month = Names[Month];
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