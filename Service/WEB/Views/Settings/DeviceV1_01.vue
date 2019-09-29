<template>
  <div class="modal-content">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Device &raquo; {{ Settings.Serial }}</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-on:click="$parent.Save"><img :src="require('Imgs/diskette.png')"></button>
        <button type="button" class="btn btn-default btn-xs" v-on:click="$parent.Close"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != ''">{{ Error }}</div>
      <spinner v-if="ShowSpinner"></spinner>
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label" style="line-height: 22px">Start on boot</label>
          <toggle class="float-right" onText="Yes" offText="No" :value="this.Settings.AutoStart" v:on:change="this.Settings.AutoStart=$event"></toggle>
          <hr style="margin: 5px 0px 5px 0px"/>
          <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Device inputs can be calibrated to compensate components derating over time.</div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Voltage channel correction</label>
          <input-spin :min="-10" :max="10" :value="Settings.VoltageGain" :increment="0.001" :decimals="3" prepend="%" v-on:change="Settings.VoltageGain=$event"></input-spin>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Voltage channel offset</label>
          <div style="display: flex">
            <input-spin :min="-20" :max="20" :value="Settings.VoltageOffset" ref="VoltageOffset" :increment="0.01" prepend="V" v-on:change="Settings.VoltageOffset=$event" style="flex-grow: 100"></input-spin>
            <button type="button" class="btn btn-default btn-xs" v-on:click="SetVoltageOffset" data-toggle="tooltip" :title="'Last second offset: ' + OffsetV + ' V'"><img :src="require('Imgs/wand.png')"></button>
          </div>       
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Current channel correction</label>
          <div style="display: flex">
            <div style="min-width: 134px">
              <option-picker :options="['NO INVERT', 'INVERT']" :selected="Settings.CurrentInvert?'INVERT':'NO INVERT'" v-on:change="Settings.CurrentInvert=($event=='INVERT')" :big="true"></option-picker>
            </div>
            <div>
              <input-spin :min="-10" :max="10" :value="Settings.CurrentGain" :increment="0.001" :decimals="3" prepend="%" v-on:change="Settings.CurrentGain=$event"></input-spin>
            </div>
          </div>          
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Current channel offset</label>
          <div style="display: flex">
            <input-spin :min="-1" :max="1" :value="Settings.CurrentOffset" ref="CurrentOffset" :increment="0.01" prepend="A" v-on:change="Settings.CurrentOffset=$event" style="flex-grow: 100"></input-spin>
            <button type="button" class="btn btn-default btn-xs" v-on:click="SetCurrentOffset" data-toggle="tooltip" :title="'Last second offset: ' + OffsetC + ' A'"><img :src="require('Imgs/wand.png')"></button>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Vue from 'vue';
  import * as oZm from 'JS/openZmeter.js';
  import OptionPicker from 'Components/OptionPicker.vue';
  import InputSpin from 'Components/InputSpin.vue';
  import Toggle from 'Components/Toggle.vue';
  import Spinner from 'Components/Spinner.vue';
  export default {
    props: ['settings'],
    data() {
      return { Settings: {}, ShowSpinner: false, Error: '', OffsetV: '', OffsetC: '' };
    },
    created: function() {
      this.Settings = this.settings;
      this.OffsetV = oZm.RoundPlain(this.settings.Wave_Average.Voltage);
      this.OffsetC = oZm.RoundPlain(this.settings.Wave_Average.Current);
    },
    methods: {
      SetCurrentOffset: function() {
        this.$refs.CurrentOffset.SetValue(-this.settings.Wave_Average.Current + this.settings.CurrentOffset);
      },
      SetVoltageOffset: function() {
        this.$refs.VoltageOffset.SetValue(-this.settings.Wave_Average.Voltage + this.settings.VoltageOffset);
      }
    },
    components: { Spinner, InputSpin, Toggle, OptionPicker }
  }
</script>