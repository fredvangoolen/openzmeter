<template>
  <div class="row">
    <div class="col-xs-12">
      <label class="control-label" style="line-height: 22px">Start on boot</label>
      <toggle class="float-right" onText="Yes" offText="No"></toggle>
      <hr style="margin: 5px 0px 5px 0px"/>
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Device inputs can be calibrated to compensate components derating over time.</div>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Voltage channel correction</label>
      <input-spin :min="-10" :max="10" :value="Settings.VoltageGain" :increment="0.001" :decimals="3" ref="voltageGain" prepend="%" v-on:change="VoltageGainChange"></input-spin>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Voltage channel offset</label>
      <input-spin :min="-20" :max="20" :value="Settings.VoltageOffset" :increment="0.01" ref="voltageOffset" prepend="V" v-on:change="VoltageOffsetChange"></input-spin>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Current channel correction</label>
      <input-spin :min="-10" :max="10" :value="Settings.CurrentGain" :increment="0.001" :decimals="3" ref="currentGain" prepend="%" v-on:change="CurrentGainChange"></input-spin>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Current channel offset</label>
      <input-spin :min="-1" :max="1" :value="Settings.CurrentOffset" :increment="0.01" ref="currentOffset" prepend="A" v-on:change="CurrentOffsetChange"></input-spin>
    </div>
  </div>
</template>

<script>
  import Vue from 'vue';
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import Spinner from 'Components/Spinner.vue';
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
    components: { InputSpin, Toggle }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1) {
      width: 60px;
      text-align: center;
    }
    .table >>> td:nth-child(1) {
      text-align: center;
      font-size: 14px;
    }
    .table >>> th:nth-child(3) {
      width: 180px;
      text-align: center;
    }
    .table >>> td:nth-child(3) {
      text-align: center;
    }
    .table >>> th:nth-child(4) {
      width: 90px;
    }
  }
  
</style>


