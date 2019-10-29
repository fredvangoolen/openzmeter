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
          <toggle class="float-right" onText="Yes" offText="No" :value="Settings.AutoStart" v-on:change="Settings.AutoStart=$event"></toggle>
          <hr style="margin: 5px 0px 5px 0px"/>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Voltage</label>
          <input-spin :min="10" :max="350" :value="Settings.Voltage" :increment="10" :decimals="2" prepend="V" v-on:change="Settings.Voltage=$event"></input-spin>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Frequency</label>
          <input-spin :min="0" :max="100" :value="Settings.Frequency" :increment="1" :decimals="2" prepend="Hz" v-on:change="Settings.Frequency=$event"></input-spin>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Current</label>
          <input-spin :min="0.1" :max="1000" :value="Settings.Current" :increment="0.1" :decimals="2" prepend="A" v-on:change="Settings.Current=$event"></input-spin>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Current phase</label>
          <input-spin :min="-180" :max="180" :value="Settings.CurrentPhase" :increment="0.1" :decimals="2" prepend="°" v-on:change="Settings.CurrentPhase=$event"></input-spin>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Mode</label>
          <option-picker :options="['1PHASE', '3PHASE', '3PHASE+N']" :selected="Settings.Mode" v-on:change="Settings.Mode=$event" :big="true"></option-picker>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Samplerate</label>
          <input-spin :min="10000" :max="100000" :value="Settings.SampleRate" :increment="100" :decimals="0" prepend="Hz" v-on:change="Settings.SampleRate=$event"></input-spin>
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
      return { Settings: {}, ShowSpinner: false, ShowSpinner: false, Error: '' };
    },
    created: function() {
      this.Settings = this.settings;
    },
    components: { Spinner, Toggle, OptionPicker, InputSpin }
  }
</script>