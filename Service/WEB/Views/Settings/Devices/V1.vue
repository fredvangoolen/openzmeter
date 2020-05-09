<template>
  <dlg :loading="Loading">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Device &raquo; {{ Settings.Serial }}</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', false)"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null">{{Error }}</div>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enabled</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Settings.Enabled"></toggle>
        <hr style="margin: 5px 0px 5px 0px"/>
        <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Device inputs can be calibrated to compensate components derating over time.</div>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Voltage channel correction</label>
        <input-spin :min="-10" :max="10" :big="true" v-model="Settings.VoltageGain" :increment="0.001" :decimals="3" prepend="%"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Voltage channel offset</label>
        <div style="display: flex">
          <input-spin :min="-20" :max="20" :big="true" v-model="Settings.VoltageOffset" :increment="0.01" :decimals="2" style="flex-grow: 100"></input-spin>
          <button type="button" class="btn btn-default btn-xs" @click="SetVoltageOffset" data-toggle="tooltip" :disabled="Settings.Wave_Average.Voltage == 0.0" :title="'Last second offset: ' + Round(Settings.Wave_Average.Voltage, 2, false)"><img :src="require('Imgs/wand.png')"></button>
        </div>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Current channel correction</label>
        <div style="display: flex">
          <div style="min-width: 134px">
            <option-picker :options="[ {name: 'NO INVERT', value: false}, {name: 'INVERT', value: true}]" v-model="Settings.CurrentInvert" :big="true"></option-picker>
          </div>
          <div>
            <input-spin :big="true" :min="-10" :max="10" v-model="Settings.CurrentGain" :increment="0.001" :decimals="3" prepend="%"></input-spin>
          </div>
        </div>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Current channel offset</label>
        <div style="display: flex">
          <input-spin :big="true" :min="-1" :max="1" v-model="Settings.CurrentOffset" :increment="0.01" :decimals="2" style="flex-grow: 100"></input-spin>
          <button type="button" class="btn btn-default btn-xs" @click="SetCurrentOffset" data-toggle="tooltip" :disabled="Settings.Wave_Average.Current == 0.0" :title="'Last second offset: ' + Round(Settings.Wave_Average.Current, 2, false)"><img :src="require('Imgs/wand.png')"></button>
        </div>
      </div>
    </div>
  </dlg>
</template>

<script>
  export default {
    props: { settings: Object },
    data() {
      return { Error: null, Loading: false, Settings: null };
    },
    created: function() {
      this.Settings = Object.assign({}, this.settings);
    },
    methods: {
      Save: function () {
        this.Loading = true;
        this.$http.post('setDevice', this.Settings).then(
          (result) => { this.$emit('close', true); },
          (fail) => {
            this.Loading = false;
            this.Error = 'Failed to save device settings!';
            setTimeout(() => { this.Error = null; }, 1000);
          }
        );
      },
      SetCurrentOffset: function() {
        this.Settings.CurrentOffset = -this.Settings.Wave_Average.Current + this.Settings.CurrentOffset;
        this.Settings.Wave_Average.Current = 0.0;
      },
      SetVoltageOffset: function() {
        this.Settings.VoltageOffset = -this.Settings.Wave_Average.Voltage + this.Settings.VoltageOffset;
        this.Settings.Wave_Average.Voltage = 0.0;
      }
    }
  }
</script>