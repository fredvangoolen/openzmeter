<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Last alarms</div>
      <div class="header-buttons clearfix">
        <toolbar-pager :pageSize="10" :input="Alarms" :output.sync="FilterAlarms" :filtercols="['Date', 'Name']"></toolbar-pager>
        <div class="btn-group">
          <button-export :rows="Export" file="Alarms"></button-export>
        </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="alarms==null"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" :data="FilterAlarms" class="table small table-striped">
      </vuetable>
    </div>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object, alarms: Object },
    data: function() {
      return { Alarms: [], FilterAlarms: [],
               Columns: [
                 { name: 'Name', title: 'Name' },
                 { name: 'StartDate', title: 'Trigger Date' },
                 { name: 'EndDate', title: 'Release Date' }
              ]
      };
    },
    updated: function() {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    watch: {
      alarms: function() { this.SetAlarms(); },
      analyzer: function() { this.SetAlarms(); },
    },
    computed: {
      ToShowEvents: function() {
        return this.Events.slice(this.Page * this.PageSize, (this.Page + 1) * this.PageSize);
      }
    },
    methods: {
      SetAlarms: function() {
        this.Alarms = [];
        if((this.alarms == null) || (this.analyzer == null)) return;
        var IDs   = [];
        var Names = [];
        for(var n = 0; n < this.analyzer.Alarms.length; n++) {
          IDs.push(this.analyzer.Alarms[n].AlarmID);
          Names.push(this.analyzer.Alarms[n].Name);
        }
        var IDCol    = this.alarms.Headers.indexOf('AlarmID');
        var TriggerCol = this.alarms.Headers.indexOf('Trigger');
        var ReleaseCol = this.alarms.Headers.indexOf('Release');
        for(var n = 0; n < this.alarms.Rows.length; n++) {
          var Index = IDs.indexOf(this.alarms.Rows[n][IDCol]);
          this.Alarms.push({ StartDate: this.analyzer.FormatDateTime(this.alarms.Rows[n][TriggerCol]), EndDate: (this.alarms.Rows[n][ReleaseCol] == null) ? '-' : this.analyzer.FormatDateTime(this.alarms.Rows[n][ReleaseCol]), Name: Names[Index] });
        }
      },
      Export: function() {
        var Export = [['Name', 'Trigger Time', 'Release Time']];
        if((this.alarms == null) || (this.analyzer == null)) return;
        var IDs   = [];
        var Names = [];
        for(var n = 0; n < this.analyzer.Alarms.length; n++) {
          IDs.push(this.analyzer.Alarms[n].AlarmID);
          Names.push(this.analyzer.Alarms[n].Name);
        }
        var IDCol    = this.alarms.Headers.indexOf('AlarmID');
        var TriggerCol = this.alarms.Headers.indexOf('Trigger');
        var ReleaseCol = this.alarms.Headers.indexOf('Release');
        for(var n = 0; n < this.alarms.Rows.length; n++) {
          var Index = IDs.indexOf(this.alarms.Rows[n][IDCol]);
          Export.push([Names[Index], this.analyzer.FormatDateTime(this.alarms.Rows[n][TriggerCol]), (this.alarms.Rows[n][ReleaseCol] == null) ? '-' : this.analyzer.FormatDateTime(this.alarms.Rows[n][ReleaseCol])]);
        }
        return Object.seal(Export);
      }
    },
    components: { Vuetable }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(2), .table >>> th:nth-child(3) {
      width: 160px;
    }
  }
</style>
