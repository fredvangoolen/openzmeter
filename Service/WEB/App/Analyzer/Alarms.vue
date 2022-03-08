<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="Alarm==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Alarms and events</h4></div>
        <div class="pull-right btn-group">
          <z-button @click="Save" tooltip="Save alarm and events config" :image="require('Imgs/diskette.png')" />
          <z-button @click="Add" tooltip="Add new alarm" :image="require('Imgs/plus.png')" />
          <z-button @click="$emit('close', null)" tooltip="Back" :image="require('Imgs/arrow_undo.png')" />
        </div>
      </template>
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label" style="line-height: 22px">Events detection</label>
          <toggle class="float-right" onText="Yes" offText="No" v-model="Analyzer.Events.Enabled"></toggle>
          <div class="help">Enable standar event detection according to IEC 61000-4-30</div>
        </div>
        <div class="col-xs-6">
          <label>Save RMS values</label>
          <input-spin :min="-200" :max="200" :increment="0.01" :decimals="2" prepend="mS" v-model="Analyzer.Events.SaveRMS"></input-spin>
        </div>
        <div class="col-xs-6">
          <label>Save RAW values</label>
          <input-spin :min="-200" :max="200" :increment="0.01" :decimals="2" prepend="mS" v-model="Analyzer.Events.SaveRAW"></input-spin>
        </div>
        <vuetable :fields="columns_events" :api-mode="false" :data="EventsConfig" class="table small table-striped">
          <template slot="enable" slot-scope="props">
            <input type="checkbox" v-model="props.rowData.Enabled">
            <div class="event-type">{{props.rowData.Type}}</div>
          </template>
          <template slot="threshold" slot-scope="props">
            <input-spin :min="0" :max="100" :increment="0.01" :decimals="2" prepend="%" v-model="props.rowData.Threshold"></input-spin>
          </template>
          <template slot="hysteresis" slot-scope="props">
            <input-spin :min="0" :max="100" :increment="0.01" :decimals="2" prepend="%" v-model="props.rowData.Hysteresis"></input-spin>
          </template>
        </vuetable>
        <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
        <div class="col-xs-12">
          <label class="control-label" style="line-height: 22px">Alarms detection</label>
          <toggle class="float-right" onText="Yes" offText="No" v-model="Analyzer.AlarmsEnabled"></toggle>
          <div class="help">Evaluate math expressions with analysis params and generate alarms</div>
        </div>
      </div>
      <vuetable :fields="columns_alarms" :api-mode="false" :data="Analyzer.Alarms" track-by="ID" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <z-button @click="Edit(props.rowIndex)" caption="Edit" />
            <z-button variant="danger" @click="Delete(props.rowIndex)" caption="Delete" />
          </div>
        </template>
      </vuetable>
    </dlg>
    <analyzer-alarm v-if="Alarm!=null" @update="Update($event)" :analyzer="analyzer" :alarm="Alarm"></analyzer-alarm>
  </div>
</template>

<script>
  import AnalyzerAlarm from './Alarm.vue';
  export default {
    props: { analyzer: Object },
    data() {
      return { Analyzer: null, ShowSpinner: false, Alarm: null, EventsConfig: [],
        columns_alarms: [
          { name: 'Enabled', title: 'Enabled', callback: this.FormatEnabled, width: '60px', titleClass: 'text-center', dataClass: 'text-center' },
          { name: 'Name', title: 'Name' },
          { name: '__slot:actions', title: '', width: '96px', dataClass: 'text-right' }
        ],
        columns_events: [
          { name: '__slot:enable', title: 'Type', width: '140px' },
          { name: '__slot:threshold', title: 'Threshold' },
          { name: '__slot:hysteresis', title: 'Hysteresis' }
        ]
      };
    },
    created: function() {
      this.Analyzer = Object.assign({}, this.analyzer);
      this.EventsConfig = [
        { Type: 'RVC', Threshold: this.Analyzer.Events.RVC.Threshold, Hysteresis: this.Analyzer.Events.RVC.Hysteresis, Enabled: this.Analyzer.Events.RVC.Enabled },
        { Type: 'DIP', Threshold: this.Analyzer.Events.DIP.Threshold, Hysteresis: this.Analyzer.Events.DIP.Hysteresis, Enabled: this.Analyzer.Events.DIP.Enabled },
        { Type: 'SWELL', Threshold: this.Analyzer.Events.SWELL.Threshold, Hysteresis: this.Analyzer.Events.SWELL.Hysteresis, Enabled: this.Analyzer.Events.SWELL.Enabled },
        { Type: 'INTERRUPTION', Threshold: this.Analyzer.Events.INTERRUPTION.Threshold, Hysteresis: this.Analyzer.Events.INTERRUPTION.Hysteresis, Enabled: this.Analyzer.Events.INTERRUPTION.Enabled },
      ];
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
        this.Analyzer.Events.RVC.Threshold = this.EventsConfig[0].Threshold;
        this.Analyzer.Events.RVC.Hysteresis = this.EventsConfig[0].Hysteresis;
        this.Analyzer.Events.RVC.Enabled = this.EventsConfig[0].Enabled;
        this.Analyzer.Events.DIP.Threshold = this.EventsConfig[1].Threshold;
        this.Analyzer.Events.DIP.Hysteresis = this.EventsConfig[1].Hysteresis;
        this.Analyzer.Events.DIP.Enabled = this.EventsConfig[1].Enabled;
        this.Analyzer.Events.SWELL.Threshold = this.EventsConfig[2].Threshold;
        this.Analyzer.Events.SWELL.Hysteresis = this.EventsConfig[2].Hysteresis;
        this.Analyzer.Events.SWELL.Enabled = this.EventsConfig[2].Enabled;
        this.Analyzer.Events.INTERRUPTION.Threshold = this.EventsConfig[3].Threshold;
        this.Analyzer.Events.INTERRUPTION.Hysteresis = this.EventsConfig[3].Hysteresis;
        this.Analyzer.Events.INTERRUPTION.Enabled = this.EventsConfig[3].Enabled;
        this.Post('setAnalyzer', this.Analyzer).then(
          () => { this.$emit('close', this.Analyzer); },
          (fail) => {
            this.ShowSpinner = false;
            if(fail != null) this.$toast.open({type:'error', message: "Failed to save analyzer config<br>" + fail.message, duration: 3000});
          }
        );
      }
    },
    components: { AnalyzerAlarm }
  }
</script>

<style scoped>
  .help {
    font-size: 80%;
  }
  .event-type {
    left: 1px;
    display: inline-block;
    position: relative;
    top: -2px;
  }
</style>
