<template>
  <div class="row">
    <div class="col-xs-12">
      <label class="control-label" style="line-height: 22px">Start on boot</label>
      <toggle class="float-right" onText="Yes" offText="No" :value="Settings.AutoStart" v-on:change="AutoStartChange"></toggle>
      <hr style="margin: 5px 0px 5px 0px"/>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Voltage</label>
      <input-spin :min="10" :max="350" :value="Settings.Voltage" :increment="10" :decimals="2" prepend="V" v-on:change="VoltageChange"></input-spin>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Frequency</label>
      <input-spin :min="10" :max="100" :value="Settings.Frequency" ref="freq" :increment="1" :decimals="2" prepend="Hz" v-on:change="FrequencyChange"></input-spin>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Current</label>
      <input-spin :min="0.1" :max="1000" :value="Settings.Current" :increment="0.1" :decimals="2" prepend="A" v-on:change="CurrentChange"></input-spin>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Current phase</label>
      <input-spin :min="-180" :max="180" :value="Settings.CurrentPhase" :increment="0.1" :decimals="2" prepend="°" v-on:change="CurrentPhaseChange"></input-spin>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Mode</label>
      <option-picker :options="['1PHASE', '3PHASE', '3PHASE+N']" :selected="Settings.Mode" v-on:OptionChanged="ModeChanged" :big="true"></option-picker>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Samplerate</label>
      <input-spin :min="10000" :max="100000" :value="Settings.Samplerate" :increment="100" :decimals="0" prepend="Hz" v-on:change="SamplerateChange"></input-spin>
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
    
    methods: {
      AutoStartChange: function(val) {
        this.Settings.AutoStart = val;
      },
      ModeChanged: function(val) {
        this.Settings.Mode = val;  
      },
      VoltageChange: function(val) {
        this.Settings.Voltage = val;
      },
      CurrentChange: function(val) {
        this.Settings.Current = val;
      },
      FrequencyChange: function(val) {
        this.Settings.Frequency = val;
      },
      CurrentPhaseChange: function(val) {
        this.Settings.CurrentPhase = val;
      },
      SamplerateChange: function(val) {
        this.Settings.Samplerate = val;
      }
    },
    components: { Toggle, OptionPicker, InputSpin }
  }
</script>