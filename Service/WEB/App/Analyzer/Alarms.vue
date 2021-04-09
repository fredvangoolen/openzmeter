<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="Alarm==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Alarms and events</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save alarm and events config"></button>
          <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="Add alarm"></button>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close', null)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
        </div>
      </template>
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
      <vuetable ref="table" :fields="columns" :api-mode="false" :data="Analyzer.Alarms" track-by="ID" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" @click="Edit(props.rowIndex)">Edit</button>
            <button type="button" class="btn btn-danger btn-xs" @click="Delete(props.rowIndex)">Delete</button>
          </div>
        </template>
      </vuetable>
    </dlg>
    <analyzer-alarm v-if="Alarm!=null" @update="Update($event)" :analyzer="analyzer" :alarm="Alarm"></analyzer-alarm>
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
      return { Analyzer: null, ShowSpinner: false, Alarm: null,
        columns: [
          { name: 'Enabled', title: 'Enabled', callback: this.FormatEnabled },
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
    },
    methods: {
      UUID: function() {
        return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) { var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8); return v.toString(16); });
      },
      FormatEnabled: function(value, close) {
        return (value === true) ? '&#9635;' : '&#9633;';
      },
      Update: function(alarm) {
        this.Analyzer.Alarms[this.Alarm] = alarm;
        this.Alarm = null;
      },
      Add: function() {
        this.Analyzer.Alarms.push({ Name: 'Unnamed alarm', AlarmID: this.UUID(), Priority: 1, Aggreg: '1M', Samples: 10, Window: 20, Expression: '' });
        this.Alarm = this.Analyzer.Alarms.length - 1;
      },
      Edit: function(row) {
        this.Alarm = row
      },
      Delete: function(row) {
        if(confirm("Delete '" + this.Analyzer.Alarms[row].Name + "' alarm?") == false) return;
        this.Analyzer.Alarms.splice(row, 1);
      },
      Save: function() {
        this.ShowSpinner = true;
        this.Post('setAnalyzer', this.Analyzer).then(
          () => { this.$emit('close', this.Analyzer); },
          (fail) => {
            this.ShowSpinner = false;
            this.$toast.open({type:'error', message: "Failed to save analyzer config<br>" + error.message, duration: 3000});
          }
        );
      }
    },
    components: { vuetable, AnalyzerAlarm }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1) {
      width: 60px;
    }
    .table >>> td:nth-child(1) {
      text-align: center;
    }
    .table >>> th:nth-child(3) {
      width: 94px;
    }
    .table >>> td:nth-child(3) {
      text-align: right;
    }
  }
  .help {
    font-size: 80%;
  }
</style>
