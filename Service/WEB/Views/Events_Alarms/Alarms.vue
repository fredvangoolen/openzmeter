<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Last alarms</div>
      <div class="header-buttons clearfix">
        <toolbar-pager :pageSize="10" :input="Alarms" :output.sync="FilterAlarms" :filtercols="['Date', 'Name']"></toolbar-pager>
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
                 { name: 'Priority', title: 'Priority' },
                 { name: 'StartDate', title: 'Trigger Date' },
                 { name: 'EndDate', title: 'Release Date' },
                 { name: 'Name', title: 'Name' }
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
        var Groups = {};
        var Priority = ['LOW', 'MEDIUM', 'HIGH'];
        for(var n = 0; n < this.alarms.AlarmTime.length; n++) {
          var Name = this.alarms.Name[n];
          if(Groups.hasOwnProperty(Name) == false) Groups[Name] = null;
          if((this.alarms.Triggered[n] == false) && (Groups[Name] == null)) {
            this.Alarms.push({ Priority: Priority[this.alarms.Priority[n]], StartDate: 'Unknown', EndDate: this.analyzer.FormatDateTime(this.alarms.AlarmTime[n]), Name: Name });
          } else if((this.alarms.Triggered[n] == false) && (Groups[Name] != null)) {
            this.Alarms.push({ Priority: Priority[this.alarms.Priority[n]], StartDate: this.analyzer.FormatDateTime(Groups[Name]), EndDate: this.analyzer.FormatDateTime(this.alarms.AlarmTime[n]), Name: Name });
            Groups[Name] = null;
          } else if((this.alarms.Triggered[n] == true) && (Groups[Name] != null)) {
            continue;
          } else if((this.alarms.Triggered[n] == true) && (Groups[Name] == null)) {
            Groups[Name] = this.alarms.AlarmTime[n];
          }
        }
        for(var n in Groups) {
          if(Groups[n] != null) this.Alarms.push({ Priority: '-', StartDate: this.analyzer.FormatDateTime(Groups[n]), EndDate: 'Unknown', Name: n });
        }
      },
      Export: function() {
        var Export = [['Date', 'Type', 'Duration', 'ΔU (V)', 'Notes']];
        for(var i = 0; i < this.Events.length; i++) Export.push([this.Events[i].StartTime, this.Events[i].Type, this.Round(this.Events[i].SampleDuration / this.Events[i].SampleRate, 3), this.Round(this.Events[i].Diff, 3), this.Events[i].Notes]);
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
      width: 100px;
    }
    .table >>> th:nth-child(2), .table >>> th:nth-child(3) {
      width: 160px;
    }
  }
</style>
