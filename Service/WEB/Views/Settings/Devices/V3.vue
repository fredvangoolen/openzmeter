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
      var Phase3N = 0;
      var Phase3  = 0;
      var Phase1  = 0;
      if(this.Settings.Outputs.lengt == 0) this.$set(this.Settings, 'Outputs', [ { Mode: '3PHASE+N', Voltage: ['CH1', 'CH2', 'CH3'], Current: ['+CH1', '+CH2', '+CH3', '+CH4'] } ]);
      for(var idx in this.Settings.Outputs) {
        if(this.Settings.Outputs[idx].Mode == '3PHASE+N') Phase3N++;
        if(this.Settings.Outputs[idx].Mode == '3PHASE') Phase3++;
        if(this.Settings.Outputs[idx].Mode == '1PHASE') Phase1++;
      }
      if((Phase3N == 0) && (Phase3 == 0) && (Phase1 == 0)) this.OutputMode = 'NONE';
      if((Phase3N == 1) && (Phase3 == 0) && (Phase1 == 0)) this.OutputMode = '3PHASE+N';
      if((Phase3N == 0) && (Phase3 == 1) && (Phase1 == 0)) this.OutputMode = '3PHASE';
      if((Phase3N == 0) && (Phase3 == 1) && (Phase1 == 1)) this.OutputMode = '3PHASE + 1PHASE';
      if((Phase3N == 0) && (Phase3 == 0) && (Phase1 == 1)) this.OutputMode = '1PHASE';
      if((Phase3N == 0) && (Phase3 == 0) && (Phase1 == 2)) this.OutputMode = '1PHASE + 1PHASE';
      if((Phase3N == 0) && (Phase3 == 0) && (Phase1 == 3)) this.OutputMode = '1PHASE + 1PHASE + 1PHASE';
      if((Phase3N == 0) && (Phase3 == 0) && (Phase1 == 4)) this.OutputMode = '1PHASE + 1PHASE + 1PHASE + 1PHASE';
    },
    computed: {
      AvaliableVoltageChannels: function() {
        var Result = [];
        if(this.CalcCh.CH1.Count > 1) Result.push({name: 'CH1', value: 'CH1'});
        if(this.CalcCh.CH2.Count > 1) Result.push({name: 'CH2', value: 'CH2'});
        if(this.CalcCh.CH3.Count > 1) Result.push({name: 'CH3', value: 'CH3'});
        return Result;
      }
    },
    watch: {
      OutputMode: function(val, old) {
        if(old == null) return;
        this.$set(this.Settings, 'Outputs', []);
        if(val == 'NONE') {
          this.$set(this.Settings, 'Outputs', [ ]);
        } else if(val == '3PHASE+N') {
          this.$set(this.Settings, 'Outputs', [ { Mode: '3PHASE+N', Voltage: ['CH1', 'CH2', 'CH3'], Current: ['+CH1', '+CH2', '+CH3', '+CH4'] } ]);
        } else if(val == '3PHASE') {
          this.$set(this.Settings, 'Outputs', [ { Mode: '3PHASE', Voltage: ['CH1', 'CH2', 'CH3'], Current: ['+CH1', '+CH2', '+CH3'] } ]);
        } else if(val == '3PHASE + 1PHASE') {
          this.$set(this.Settings, 'Outputs', [ { Mode: '3PHASE', Voltage: ['CH1', 'CH2', 'CH3'], Current: ['+CH1', '+CH2', '+CH3'] }, { Mode: '1PHASE', Voltage: 'CH1', Current: '+CH4' } ]);
        } else if(val == '1PHASE') {
          this.$set(this.Settings, 'Outputs', [ { Mode: '1PHASE', Voltage: 'CH1', Current: '+CH1' } ]);
        } else if(val == '1PHASE + 1PHASE') {
          this.$set(this.Settings, 'Outputs', [ { Mode: '1PHASE', Voltage: 'CH1', Current: '+CH1' }, { Mode: '1PHASE', Voltage: 'CH2', Current: '+CH2' } ]);
        } else if(val == '1PHASE + 1PHASE + 1PHASE')  {
          this.$set(this.Settings, 'Outputs', [ { Mode: '1PHASE', Voltage: 'CH1', Current: '+CH1' }, { Mode: '1PHASE', Voltage: 'CH2', Current: '+CH2' }, { Mode: '1PHASE', Voltage: 'CH3', Current: '+CH3' } ]);
        } else if(val == '1PHASE + 1PHASE + 1PHASE + 1PHASE') {
          this.$set(this.Settings, 'Outputs', [ { Mode: '1PHASE', Voltage: 'CH1', Current: '+CH1' }, { Mode: '1PHASE', Voltage: 'CH2', Current: '+CH2' }, { Mode: '1PHASE', Voltage: 'CH3', Current: '+CH3' }, { Mode: '1PHASE', Voltage: 'CH1', Current: '+CH4' } ]);
        }
      },
      Settings: {
        deep: true,
        handler: function() {
          this.CalcCh = null;
          var VoltageCh = { CH1: { Count: 0, CH1: false, CH2: false, CH3: false, CH4: false },
                            CH2: { Count: 0, CH1: false, CH2: false, CH3: false, CH4: false },
                            CH3: { Count: 0, CH1: false, CH2: false, CH3: false, CH4: false },
                            Enabled: false };
          for(var idx in this.Settings.Outputs) {
            if(this.Settings.Outputs[idx].Mode != '1PHASE') continue;
            if(VoltageCh[this.Settings.Outputs[idx].Voltage][this.Settings.Outputs[idx].Current.substr(1)] == false) {
              VoltageCh[this.Settings.Outputs[idx].Voltage][this.Settings.Outputs[idx].Current.substr(1)] = true;
              VoltageCh[this.Settings.Outputs[idx].Voltage].Count++;
              if(VoltageCh[this.Settings.Outputs[idx].Voltage].Count > 1) VoltageCh.Enabled = true;
            }
          }
          this.CalcCh = VoltageCh;
          if(VoltageCh[this.Settings.Calculated.Voltage].Count < 2) this.Settings.Calculated.Voltage = (this.AvaliableVoltageChannels.length > 0) ? this.AvaliableVoltageChannels[0].name : "CH1";
          if(this.CalcCh[this.Settings.Calculated.Voltage].CH1 == false) this.Settings.Calculated.Current[0] = "UNUSED";
          if(this.CalcCh[this.Settings.Calculated.Voltage].CH2 == false) this.Settings.Calculated.Current[1] = "UNUSED";
          if(this.CalcCh[this.Settings.Calculated.Voltage].CH3 == false) this.Settings.Calculated.Current[2] = "UNUSED";
          if(this.CalcCh[this.Settings.Calculated.Voltage].CH4 == false) this.Settings.Calculated.Current[3] = "UNUSED";
        }
      }
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
        this.$set(this.Settings.CurrentOffset, index, -this.Settings.Wave_Average.Current[index]);
        this.Settings.Wave_Average.Current[index] = 0.0;
      },
      SetVoltageOffset: function(index) {
        this.$set(this.Settings.VoltageOffset, index, -this.Settings.Wave_Average.Voltage[index]);
        this.Settings.Wave_Average.Voltage[index] = 0.0;
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