<template>
  <dlg :loading="Loading">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Device &raquo; {{ Settings.Serial }}</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', false)"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
    </template>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Start on boot</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Settings.Enabled"></toggle>
        <hr style="margin: 5px 0px 5px 0px"/>
        <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Device inputs can be calibrated to compensate components derating over time.</div>
        <table>
          <tr class="border-bottom">
            <td class="border-right"><strong>Channel</strong></td>
            <td colspan="2" class="text-right border-right">
              <strong>ADC Offset</strong>
              <img :src="require('Imgs/help16.png')" title="Add or substract value to ADC read, to adjust 0 point.">
            </td>
            <td class="text-right">
              <strong>Multiplier</strong>
              <img :src="require('Imgs/help16.png')" title="Multiply read value by a number to adjust input or for apply calibrations.">
            </td>
            <td><strong>Enabled</strong></td>
          </tr>
          <tr class="separator-bottom">
            <td><strong>Voltage</strong></td>
            <td>
              <input-spin :min="-200" :max="200" v-model="Settings.VoltageOffset" :increment="0.01" :decimals="2"></input-spin>
            </td>
            <td>
              <button type="button" class="btn btn-default btn-xs" @click="SetVoltageOffset()" :disabled="Settings.Wave_Average.Voltage == 0.0" :title="'Last second offset: ' + Round(-Settings.Wave_Average.Voltage, 2, false)"><img :src="require('Imgs/wand.png')"></button>
            </td>
            <td>
              <input-spin :min="90" :max="110" v-model="Settings.VoltageGain" :increment="0.001" :decimals="3" prepend="%"></input-spin>
            </td>
            <td></td>
          </tr>
          <tr v-for="(item, idx) in Settings.Outputs" :key="idx">
            <td><strong>Current CH{{ idx + 1 }}</strong></td>
            <td>
              <input-spin :min="-200" :max="200" v-model="Settings.Outputs[idx].CurrentOffset" :increment="0.01" :decimals="2"></input-spin>
            </td>
            <td>
              <button type="button" class="btn btn-default btn-xs" @click="SetCurrentOffset(idx)" :disabled="Settings.Wave_Average.Current[idx] == 0.0" :title="'Last second offset: ' + Round(-Settings.Wave_Average.Current[idx], 2, false)"><img :src="require('Imgs/wand.png')"></button>
            </td>
            <td>
              <input-spin :min="90" :max="110" v-model="Settings.Outputs[idx].CurrentGain" :increment="0.001" :decimals="3" prepend="%"></input-spin>
            </td>
            <td>
              <toggle class="float-right" onText="Yes" offText="No" v-model="Settings.Outputs[idx].Enabled"></toggle>
            </td>
          </tr>
          <tr  class="separator-top">
            <td><strong>Aggregated</strong></td>
            <td></td>
            <td></td>
            <td></td>
            <td>
              <toggle class="float-right" onText="Yes" offText="No" v-model="Settings.Calculated.Enabled"></toggle>
            </td>
          </tr>
        </table>
      </div>
    </div>
  </dlg>
</template>

<script>
  export default {
    props: { settings: Object },
    data() {
      return { View: 'Inputs', Loading: false, Error: null, OutputMode: null, Settings: null, CalcCh: null };
    },
    created: function() {
      this.Settings = Object.assign({}, this.settings);
    },
    methods: {
      Save: function () {
        this.Loading = true;
        this.Post('setDevice', this.Settings).then(
          (result) => { this.$emit('close', true); },
          (fail) => {
            this.Loading = false;
            this.Error = 'Failed to save device settings!';
            setTimeout(() => { this.Error = null; }, 1000);
          }
        );
      },
      SetCurrentOffset: function(index) {
        this.$set(this.Settings.Outputs[index], 'CurrentOffset', -this.Settings.Wave_Average.Current[index]);
        this.Settings.Wave_Average.Current[index] = 0.0;
      },
      SetVoltageOffset: function() {
        this.Settings.VoltageOffset = -this.Settings.Wave_Average.Voltage;
        this.Settings.Wave_Average.Voltage = 0.0;
      }
    }
  }
</script>

<style scoped>
  table {
    width: 100%;
    border: 1px solid #ccc;
  }
  table:not(:first-child), .output-table:not(:first-child) {
    margin-top: 5px;
  }
  table tr td {
    border-spacing: 0px;
    padding: 3px;
  }
  table .separator-top {
    border-top: 1px solid #ccc;
  }
  table .separator-bottom {
    border-bottom: 1px solid #ccc;
  }
  table .border-bottom {
    border-bottom: 1px solid #ccc;
    background: #eee;
    font-weight: bold;
  }
  table .border-right {
    border-right: 1px solid #ccc;
  }
  .serial-number {
    font-size: 12px;
    float: right;
    font-weight: normal;
  }
  .toggle-gain >>> .toggle {
    width: 60px !important;
  }
</style>