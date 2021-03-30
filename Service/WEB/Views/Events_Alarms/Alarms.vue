<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Last alarms</div>
      <div class="header-buttons clearfix">
        <toolbar-pager :pageSize="50" :input="Alarms" :output.sync="FilterAlarms" :filtercols="['Date', 'Name']"></toolbar-pager>
        <div class="btn-group">
          <button class="btn btn-default btn-xs btn-danger" type="button" @click="DeleteSelected">Delete selection</button>
          <button-export :rows="Export" file="Alarms"></button-export>
        </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="(alarms==null)||ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" :data="FilterAlarms" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-danger btn-xs"  @click="Delete(props.rowData)">Delete</button>
          </div>
        </template>
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
      return { Alarms: [], FilterAlarms: [], ShowSpinner: false,
               Columns: [
                 { name: '__checkbox' },
                 { name: 'Name', title: 'Name' },
                 { name: 'StartDate', title: 'Trigger Date', callback: this.FormatDate },
                 { name: 'EndDate', title: 'Release Date', callback: this.FormatDate },
                 { name: '__slot:actions', title: '' }
              ]
      };
    },
    updated: function() {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    watch: {
      alarms: function() {
        this.Alarms = [];
        if((this.alarms == null) || (this.analyzer == null)) return;
        var IDs   = [];
        var Names = [];
        for(var n = 0; n < this.analyzer.Alarms.length; n++) {
          IDs.push(this.analyzer.Alarms[n].AlarmID);
          Names.push(this.analyzer.Alarms[n].Name);
        }
        var IDCol      = this.alarms.Headers.indexOf('AlarmID');
        var TriggerCol = this.alarms.Headers.indexOf('Trigger');
        var ReleaseCol = this.alarms.Headers.indexOf('Release');
        for(var n = 0; n < this.alarms.Rows.length; n++) {
          var Index = IDs.indexOf(this.alarms.Rows[n][IDCol]);
          this.Alarms.push({ id: n, ID: this.alarms.Rows[n][IDCol], StartDate: this.alarms.Rows[n][TriggerCol], EndDate: this.alarms.Rows[n][ReleaseCol], Name: Names[Index] });
        }
       },
    },
    methods: {
      FormatDate: function(val) {
        return (val == null) ? '-' : this.analyzer.FormatDateTime(val);
      },
      DeleteSelected: function() {
        var Selection = this.$refs.table.selectedTo;
        if(confirm('Delete ' + Selection.length + ' alarm(s)?') == false) return;
        this.ShowSpinner = true;
        var Promises = [];
        for(var n = 0; n < Selection.length; n++) Promises.push(this.Post('delAlarm', { Trigger: this.Alarms[Selection[n]].StartDate, Analyzer: this.analyzer.Analyzer, AlarmID: this.Alarms[Selection[n]].ID }));
        Promise.all(Promises).then(
          () => {
            this.$emit('alarmschanged');
            this.ShowSpinner = false;
            this.$refs.table.selectedTo = [];
          },
          () => {
            this.ShowSpinner = false;
          }
        );
      },
      Delete: function(evt) {
        if(confirm("Delete selected alarm?")) {
          this.Post('delAlarm', { Trigger: evt.StartDate, Analyzer: this.analyzer.Analyzer, AlarmID: evt.ID }).then(
            (result, n) => { this.$emit('alarmschanged'); }
          );
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
    .table >>> th:nth-child(1) {
      width: 30px;
    }
    .table >>> th:nth-child(3), .table >>> th:nth-child(4) {
      width: 160px;
    }
    .table >>> th:nth-child(5) {
      width: 60px;
    }
  }
</style>
