<template>
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
      <input-spin :min="-20" :max="20" :value="Settings.VoltageOffset" :increment="0.01" prepend="V" v-on:change="Settings.VoltageOffset=$vent"></input-spin>
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
      <input-spin :min="-1" :max="1" :value="Settings.CurrentOffset" :increment="0.01" prepend="A" v-on:change="Settings.CurrentOffset=$event"></input-spin>
    </div>
  </div>
</template>

<script>
  import Vue from 'vue';
  import * as oZm from 'JS/openZmeter.js';
  import OptionPicker from 'Components/OptionPicker.vue';
  import InputSpin from 'Components/InputSpin.vue';
  import Toggle from 'Components/Toggle.vue';
  export default {
    props: ['settings'],
    data() {
      return { Settings: {} };
    },
    created: function() {
      this.Settings = this.settings;
    },
    components: { InputSpin, Toggle, OptionPicker }
  }
</script>