<template>
  <div>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="value.Error != null">{{ value.Error }}</div>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Start on boot</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="value.Enabled"></toggle>
        <hr style="margin: 5px 0px 5px 0px"/>
        <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Device inputs can be calibrated to compensate components derating over time.</div>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Voltage channel correction</label>
        <input-spin :min="-10" :max="10" v-model="value.VoltageGain" :increment="0.001" :decimals="3" prepend="%"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Voltage channel offset</label>
        <div style="display: flex">
          <input-spin :min="-20" :max="20" v-model="value.VoltageOffset" :increment="0.01" prepend="V" style="flex-grow: 100"></input-spin>
          <button type="button" class="btn btn-default btn-xs" v-on:click="SetVoltageOffset" data-toggle="tooltip" v-bind:disabled="value.Wave_Average.Voltage == 0.0" :title="'Last second offset: ' + Round(value.Wave_Average.Voltage, 2) + ' V'"><img :src="require('Imgs/wand.png')"></button>
        </div>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Current channel correction</label>
        <div style="display: flex">
          <div style="min-width: 134px">
            <option-picker :options="[ {name: 'NO INVERT', value: false}, {name: 'INVERT', value: true}]" v-model="value.CurrentInvert" :big="true"></option-picker>
          </div>
          <div>
            <input-spin :min="-10" :max="10" v-model="value.CurrentGain" :increment="0.001" :decimals="3" prepend="%"></input-spin>
          </div>
        </div>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Current channel offset</label>
        <div style="display: flex">
          <input-spin :min="-1" :max="1" v-model="value.CurrentOffset" :increment="0.01" prepend="A" style="flex-grow: 100"></input-spin>
          <button type="button" class="btn btn-default btn-xs" v-on:click="SetCurrentOffset" data-toggle="tooltip" v-bind:disabled="value.Wave_Average.Current == 0.0" :title="'Last second offset: ' + Round(value.Wave_Average.Current, 2) + ' A'"><img :src="require('Imgs/wand.png')"></button>
        </div>
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
    props: { value: { type: Object, default: {}} },
    data() {
      return { Error: '' };
    },
    methods: {
      Round: oZm.RoundPlain,
      SetCurrentOffset: function() {
        this.value.CurrentOffset = -this.value.Wave_Average.Current + this.value.CurrentOffset;
        this.value.Wave_Average.Current = 0.0;
      },
      SetVoltageOffset: function() {
        this.value.VoltageOffset = -this.value.Wave_Average.Voltage + this.value.VoltageOffset;
        this.value.Wave_Average.Voltage = 0.0;
      }
    },
    components: { InputSpin, Toggle, OptionPicker, oZm }
  }
</script>