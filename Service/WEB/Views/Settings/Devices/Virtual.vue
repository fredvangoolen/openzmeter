<template>
  <div>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="error!=''">{{ error }}</div>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Start on boot</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="value.AutoStart"></toggle>
        <hr style="margin: 5px 0px 5px 0px"/>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Voltage</label>
        <input-spin :min="10" :max="350" v-model="value.Voltage" :increment="10" :decimals="2" prepend="V"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Frequency</label>
        <input-spin :min="0" :max="100" v-model="value.Frequency" :increment="1" :decimals="2" prepend="Hz"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Current</label>
        <input-spin :min="0.1" :max="1000" v-model="value.Current" :increment="0.1" :decimals="2" prepend="A"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Current phase</label>
        <input-spin :min="-180" :max="180" v-model="value.CurrentPhase" :increment="0.1" :decimals="2" prepend="°"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Mode</label>
        <option-picker :options="[ {name: '1PHASE', value: '1PHASE'}, {name: '3PHASE', value: '3PHASE'}, {name: '3PHASE+N', value: '3PHASE+N'} ]" v-model="value.Mode" :big="true"></option-picker>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Samplerate</label>
        <input-spin :min="10000" :max="100000" v-model="value.SampleRate" :increment="100" :decimals="0" prepend="Hz"></input-spin>
      </div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import OptionPicker from 'Components/OptionPicker.vue';
  import InputSpin from 'Components/InputSpin.vue';
  import Toggle from 'Components/Toggle.vue';
  export default {
    props: { value: Object, error: String },
    data() {
      return { };
    }
  }
</script>