<template>
  <div class="row">
    <div class="col-xs-12 col-sm-6">
      <label class="control-label" style="line-height: 22px">Start on boot</label>
      <toggle class="float-right" onText="Yes" offText="No" ref="autoStart"></toggle>
    </div>
    <div class="col-xs-6 col-sm-3">
      <button type="button" class="btn btn-default btn-xs" style="width: 100%" v-bind:class="{ active: View=='Inputs' }" v-on:click="View='Inputs'">Inputs</button>
    </div>
    <div class="col-xs-6 col-sm-3">
      <button type="button" class="btn btn-default btn-xs" style="width: 100%" v-bind:class="{ active: View=='Outputs' }" v-on:click="View='Outputs'">Outputs</button>
    </div>
    <div class="col-xs-12">
      <hr style="margin: 5px 0px 5px 0px"/>
    </div>
    <div v-bind:style="{ display: (View=='Inputs') ? 'block' : 'none' }">
      <div class="col-xs-12">
        <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Device inputs can be calibrated to compensate components derating over time.</div>
      </div>
      <div v-for="(v, index) in Settings.VoltageGain">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Voltage {{ index + 1 }} correction</label>
          <input-spin :min="-10" :max="10" :value="Settings.VoltageGain[index]" :increment="0.001" :decimals="3" prepend="%" v-on:change="Settings.VoltageGain[index] = $event"></input-spin>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Voltage {{ index + 1 }} offset</label>
          <input-spin :min="-20" :max="20" :value="Settings.VoltageOffset[index]" :increment="0.01" prepend="V" v-on:change="Settings.VoltageOffset[index] = $event"></input-spin>
        </div>
      </div>
      <div v-for="(v, index) in Settings.CurrentGain">
        <div class="col-xs-12 col-sm-4">
          <label class="control-label">Current {{ index}} level</label>
          <input-spin :min="-1" :max="1" :value="Settings.CurrentOffset[index]" :increment="0.001" :decimals="3" prepend="μV/A" v-on:change="Settings.CurrentLevel[index] = $event"></input-spin>
        </div>
        <div class="col-xs-12 col-sm-4">
          <label class="control-label">Current {{ index }} correction</label>
          <input-spin :min="-10" :max="10" :value="Settings.CurrentGain[index]" :increment="0.001" :decimals="3" prepend="%" v-on:change="Settings.CurrentGain[index] = $event"></input-spin>
        </div>
        <div class="col-xs-12 col-sm-4">
          <label class="control-label">Current {{ index }} offset</label>
          <input-spin :min="-1" :max="1" :value="Settings.CurrentOffset[index]" :increment="0.01" prepend="A" v-on:change="Settings.CurrentOffset[index] = $event"></input-spin>
        </div>
      </div>
    </div>
    <div v-bind:style="{ display: (View=='Outputs') ? 'block' : 'none' }">
      <div v-for="index in [0, 1, 2, 3]">
        <div class="col-xs-12 col-sm-4">
          <label class="control-label">Output {{ index }} mode</label>
          <option-picker :options="['1PHASE', '3PHASE', '3PHASE+N']" selected="1PHASE" v-on:OptionChanged="InvertChanged" ref="invert" :big="true"></option-picker>
        </div>
        <div class="col-xs-12 col-sm-4">
          <label class="control-label">Output {{ index }} voltage channels</label>
          <option-picker :options="['NO INVERT', 'INVERT']" selected="NO INVERT" v-on:OptionChanged="InvertChanged" ref="invert" :big="true"></option-picker>
        </div>
        <div class="col-xs-12 col-sm-4">
          <label class="control-label">Output {{ index }} current channels</label>
          <option-picker :options="['NO INVERT', 'INVERT']" selected="NO INVERT" v-on:OptionChanged="InvertChanged" ref="invert" :big="true"></option-picker>
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
  export default {
    props: ['settings'],
    data() {
      return { View: 'Inputs', Settings: {} };
    },
    created: function() {
      this.Settings = this.settings;
    },
    mounted: function() {
      this.$refs.autoStart.SetState(this.Settings.AutoStart);
      this.$refs.invert.SetOption(this.Settings.CurrentInvert ? 'INVERT' : 'NO INVERT');
    },
    methods: {
//      VoltageGainChange: function(val) {
//        this.Settings.VoltageGain = val;
//      },
//      VoltageOffsetChange: function(val) {
//        this.Settings.VoltageOffset = val;
//      },
//      CurrentLevelChange: function(val, index) {
//        this.Settings.CurrentLevel[index] = val;
//      },
//      CurrentGainChange: function(val, index) {
//        this.Settings.CurrentGain[index] = val;
//      },
//      CurrentOffsetChange: function(val, index) {
//        this.Settings.CurrentOffset[index] = val;
//      },
//      InvertChanged: function(val) {
//        this.Settings.CurrentInvert = (val === 'INVERT');
//      }
    },
    components: { InputSpin, Toggle, OptionPicker }
  }
</script>
