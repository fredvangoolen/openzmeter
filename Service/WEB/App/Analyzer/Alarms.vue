<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="Alarm==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Alarms and events</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save alarm and events config"></button>
          <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="Add alarm"></button>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close', Analyzer.Analyzer)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
        </div>
      </template>
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null">{{ Error }}</div>
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label" style="line-height: 22px">Events detection</label>
          <toggle class="float-right" onText="Yes" offText="No" v-model="Analyzer.EventsEnabled"></toggle>
          <div class="help">Enable standar event detection according to IEC 61000-4-30</div>
        </div>
        <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
        <div class="col-xs-12">
          <label class="control-label" style="line-height: 22px">Alarms detection</label>
          <toggle class="float-right" onText="Yes" offText="No" v-model="Analyzer.AlarmsEnabled"></toggle>
          <div class="help">Evaluate math expressions with analysis params and generate alarms</div>
        </div>
      </div>
      <vuetable ref="table" :fields="columns" :api-mode="false" :data="Alarms" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" @click="Edit(props.rowData)">Edit</button>
            <button type="button" class="btn btn-danger btn-xs" @click="Delete(props.rowData)">Delete</button>
          </div>
        </template>
      </vuetable>
    </dlg>
    <analyzer-alarm v-if="Alarm!=null" @close="GetAlarms" :analyzer="analyzer" :alarm="Alarm"></analyzer-alarm>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import AnalyzerAlarm from './Alarm.vue';
  export default {
    props: { analyzer: Object },
    data() {
      return { Analyzer: null, ShowSpinner: false, Alarms: [], Alarm: null, Error: null,
        columns: [
          { name: 'Name', title: 'Name' },
          { name: '__slot:actions', title: '' }
        ]
      };
    },
    updated: function() {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    created: function() {
      this.Analyzer = Object.assign({}, this.analyzer);
      this.GetAlarms();
    },
    methods: {
      GetAlarms: function() {
        this.Alarm = null;
        this.ShowSpinner = true;
        this.$http.post('getAlarmsDef', { Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.Alarms = result.data;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      Add: function() {
        this.Alarm = { Name: 'Unnamed alarm', Priority: 1, Aggreg: '1M', Samples: 10, Window: 20, Expression: '' };
      },
      Edit: function(row) {
        this.ShowSpinner = true;
        this.Alarm = row
      },
      Delete: function(row) {
        if(confirm("Delete '" + row.Name + "' alarm?") == false) return;
        this.ShowSpinner = true;
        this.$http.post('delAlarmDef', { Analyzer: this.analyzer.Analyzer, AlarmID: row.AlarmID }).then(
          (result) => { this.GetAlarms(); },
          (fail) => { this.ShowSpinner = false; }
        );
      },
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post('setAnalyzer', this.Analyzer).then(
          (result) => { this.$emit('close', this.Analyzer); },
          (fail) => {
            this.ShowSpinner = false;
            this.Error = 'Failed to save analyzer!';
            setTimeout(() => { this.Error = null; }, 5000);
          }
        );
      }
    },
    components: { vuetable, AnalyzerAlarm }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(2) {
      width: 94px;
    }
    .table >>> td:nth-child(2) {
      text-align: right;
    }
  }
  .help {
    font-size: 80%;
  }
</style>
