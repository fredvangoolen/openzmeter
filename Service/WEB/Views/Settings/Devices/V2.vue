<template>
  <dlg :loading="Loading">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Device &raquo; {{ Settings.Serial }}</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Inputs' }" @click="View='Inputs'">Main</button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Outputs' }" @click="View='Outputs'">Outputs</button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', false)"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
    </template>
    <template v-slot:footer>
      <div class="row" style="text-align: center">
        <div class="col-xs-4">Device is {{ Settings.MainsPowered ? '' : ' NOT ' }} mains powered</div>
        <div class="col-xs-4">Battery is {{ Settings.Charging ? '' : ' NOT ' }} charging </div>
        <div class="col-xs-4">Battery voltage is {{ Round(Settings.BatteryVoltage, 2) }} V</div>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null">{{ Error }}</div>
    <div class="row" v-if="View=='Inputs'">
      <div class="col-xs-12" style="margin-top:5px;">
        <label class="control-label" style="line-height: 22px">Start on boot</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Settings.Enabled"></toggle>
        <hr style="margin: 5px 0px 5px 0px"/>
        <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Device inputs can be calibrated to compensate components derating over time.</div>
      </div>
      <div class="col-xs-12">
        <table>
          <tr><td colspan="4" class="border-bottom">Voltage channels</td></tr>
          <tr>
            <td></td>
            <td class="text-right">
              <strong>ADC Offset</strong>
              <img :src="require('Imgs/help16.png')" title="Add or substract value to ADC read, to adjust 0 point.">
            </td>
            <td></td>
            <td class="text-right">
              <strong>Multiplier</strong>
              <img :src="require('Imgs/help16.png')" title="Multiply read value by a number to adjust input across voltage sensing transformer or for apply calibrations.">
            </td>
          </tr>
          <tr v-for="index in [0, 1, 2]" :key="index">
            <td class="text-center"><strong>CH{{ index + 1 }}</strong></td>
            <td>
              <input-spin :min="-200" :max="200" v-model="Settings.VoltageOffset[index]" :increment="0.01" :decimals="2"></input-spin>
            </td>
            <td>
              <button type="button" class="btn btn-default btn-xs" @click="SetVoltageOffset(index)" :disabled="Settings.Wave_Average.Voltage[index] == 0.0" :title="'Last second offset: ' + Round(-Settings.Wave_Average.Voltage[index], 2, false)"><img :src="require('Imgs/wand.png')"></button>
            </td>
            <td>
              <input-spin :min="0.1" :max="1000" v-model="Settings.VoltageGain[index]" :increment="0.001" :decimals="3" prepend="V/V"></input-spin>
            </td>
          </tr>
        </table>
        <table>
          <tr><td colspan="5" class="border-bottom">Current channels</td></tr>
          <tr>
            <td colspan="5">
              <label class="control-label" style="line-height: 22px">Full scale mode</label>
              <toggle class="float-right toggle-gain" onText="333mV" onStyle="primary" offText="625mV" offStyle="primary" v-model="Settings.HighGain"></toggle>
            </td>
          </tr>
          <tr>
            <td></td>
            <td class="text-right">
              <strong>Offset <img :src="require('Imgs/help16.png')" title="Add or substract value to ADC read, to adjust 0 point."></strong>
            </td>
            <td></td>
            <td class="text-right">
              <strong>Delay</strong>
            </td>
            <td class="text-right">
              <strong>Multiplier</strong>
            </td>
          </tr>
          <tr v-for="index in [0, 1, 2, 3]" :key="index">
            <td class="text-center"><strong>CH{{ index + 1 }}</strong></td>
            <td>
              <input-spin :min="-200" :max="200" v-model="Settings.CurrentOffset[index]" :increment="0.01" :decimals="2"></input-spin>
            </td>
            <td>
              <button type="button" class="btn btn-default btn-xs" @click="SetCurrentOffset(index)" :title="'Last second offset: ' + Round(-Settings.Wave_Average.Current[index], 2, false)"><img :src="require('Imgs/wand.png')"></button>
            </td>
            <td>
              <input-spin :min="0" :max="200" v-model="Settings.CurrentDelay[index]" :increment="1" :decimals="0"></input-spin>
            </td>
            <td>
              <input-spin :min="0.00001" :max="20" v-model="Settings.CurrentGain[index]" :increment="0.00001" :decimals="5" prepend="A/mV"></input-spin>
            </td>
          </tr>
        </table>
      </div>
    </div>
    <div class="row" v-if="View=='Outputs'">
      <div class="col-xs-12" style="margin-top:5px;">
        <div style="display: flex">
          <label class="control-label" style="line-height: 22px; min-width: 180px">Outputs configuration model</label>
          <option-picker :options="[ {name: 'NONE', value: 'NONE'}, {name: '3PHASE+N', value: '3PHASE+N'}, {name: '3PHASE', value: '3PHASE'}, {name: '3PHASE + 1PHASE', value: '3PHASE + 1PHASE'}, {name: '1PHASE', value: '1PHASE'}, {name: '1PHASE + 1PHASE', value: '1PHASE + 1PHASE'}, {name: '1PHASE + 1PHASE + 1PHASE', value: '1PHASE + 1PHASE + 1PHASE'}, {name: '1PHASE + 1PHASE + 1PHASE + 1PHASE', value: '1PHASE + 1PHASE + 1PHASE + 1PHASE'}]" v-model="OutputMode"></option-picker>
        </div>
      </div>
      <div class="col-xs-12">
        <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Current channels can not be used twice<br/>Voltage channels can not be used twice in same analyzer</div>
      </div>
      <div class="col-xs-12">
        <div class="output-table" v-for="(item, idx) in Settings.Outputs" :key="idx">
          <table v-if="item.Mode == '3PHASE+N'">
            <tr><td colspan="7" class="border-bottom">3PHASE+N<span class="serial-number">{{ Settings.Serial }}_3PN</span></td></tr>
            <tr>
              <td class="text-center border-right" colspan="3"><strong>Voltage</strong></td>
              <td class="text-center" colspan="4"><strong>Current</strong></td>
            </tr>
            <tr>
              <td class="text-center"><strong>L1</strong></td>
              <td class="text-center"><strong>L2</strong></td>
              <td class="text-center border-right "><strong>L3</strong></td>
              <td class="text-center "><strong>L1</strong></td>
              <td class="text-center "><strong>L2</strong></td>
              <td class="text-center "><strong>L3</strong></td>
              <td class="text-center "><strong>N</strong></td>
            </tr>
            <tr>
              <td class="text-right"><option-picker :options="[{name: 'CH1', value: 'CH1'}, {name: 'CH2', value: 'CH2'}, {name: 'CH3', value: 'CH3'}]" v-model="item.Voltage[0]"></option-picker></td>
              <td class="text-right"><option-picker :options="[{name: 'CH1', value: 'CH1'}, {name: 'CH2', value: 'CH2'}, {name: 'CH3', value: 'CH3'}]" v-model="item.Voltage[1]"></option-picker></td>
              <td class="text-right border-right"><option-picker :options="[{name: 'CH1', value: 'CH1'}, {name: 'CH2', value: 'CH2'}, {name: 'CH3', value: 'CH3'}]" v-model="item.Voltage[2]"></option-picker></td>
              <td class="text-right"><option-picker :options="[{name: '+CH1', value: '+CH1'}, {name: '-CH1', value: '-CH1'}, {name: '+CH2', value: '+CH2'}, {name: '-CH2', value: '-CH2'}, {name: '+CH3', value: '+CH3'}, {name: '-CH3', value: '-CH3'}, {name: '+CH4', value: '+CH4'}, {name: '-CH4', value: '-CH4'}]" v-model="item.Current[0]"></option-picker></td>
              <td class="text-right"><option-picker :options="[{name: '+CH1', value: '+CH1'}, {name: '-CH1', value: '-CH1'}, {name: '+CH2', value: '+CH2'}, {name: '-CH2', value: '-CH2'}, {name: '+CH3', value: '+CH3'}, {name: '-CH3', value: '-CH3'}, {name: '+CH4', value: '+CH4'}, {name: '-CH4', value: '-CH4'}]" v-model="item.Current[1]"></option-picker></td>
              <td class="text-right"><option-picker :options="[{name: '+CH1', value: '+CH1'}, {name: '-CH1', value: '-CH1'}, {name: '+CH2', value: '+CH2'}, {name: '-CH2', value: '-CH2'}, {name: '+CH3', value: '+CH3'}, {name: '-CH3', value: '-CH3'}, {name: '+CH4', value: '+CH4'}, {name: '-CH4', value: '-CH4'}]" v-model="item.Current[2]"></option-picker></td>
              <td class="text-right"><option-picker :options="[{name: '+CH1', value: '+CH1'}, {name: '-CH1', value: '-CH1'}, {name: '+CH2', value: '+CH2'}, {name: '-CH2', value: '-CH2'}, {name: '+CH3', value: '+CH3'}, {name: '-CH3', value: '-CH3'}, {name: '+CH4', value: '+CH4'}, {name: '-CH4', value: '-CH4'}]" v-model="item.Current[3]"></option-picker></td>
            </tr>
          </table>
          <table v-if="item.Mode == '3PHASE'">
            <tr><td colspan="6" class="border-bottom">3PHASE<span class="serial-number">{{ Settings.Serial }}_3P</span></td></tr>
            <tr>
              <td class="text-center border-right" colspan="3"><strong>Voltage</strong></td>
              <td class="text-center" colspan="3"><strong>Current</strong></td>
            </tr>
            <tr>
              <td class="text-center"><strong>L1</strong></td>
              <td class="text-center"><strong>L2</strong></td>
              <td class="text-center border-right "><strong>L3</strong></td>
              <td class="text-center "><strong>L1</strong></td>
              <td class="text-center "><strong>L2</strong></td>
              <td class="text-center "><strong>L3</strong></td>
            </tr>
            <tr>
              <td class="text-right"><option-picker :options="[{name: 'CH1', value: 'CH1'}, {name: 'CH2', value: 'CH2'}, {name: 'CH3', value: 'CH3'}]" v-model="item.Voltage[0]"></option-picker></td>
              <td class="text-right"><option-picker :options="[{name: 'CH1', value: 'CH1'}, {name: 'CH2', value: 'CH2'}, {name: 'CH3', value: 'CH3'}]" v-model="item.Voltage[1]"></option-picker></td>
              <td class="text-right border-right"><option-picker :options="[{name: 'CH1', value: 'CH1'}, {name: 'CH2', value: 'CH2'}, {name: 'CH3', value: 'CH3'}]" v-model="item.Voltage[2]"></option-picker></td>
              <td class="text-right"><option-picker :options="[{name: '+CH1', value: '+CH1'}, {name: '-CH1', value: '-CH1'}, {name: '+CH2', value: '+CH2'}, {name: '-CH2', value: '-CH2'}, {name: '+CH3', value: '+CH3'}, {name: '-CH3', value: '-CH3'}, {name: '+CH4', value: '+CH4'}, {name: '-CH4', value: '-CH4'}]" v-model="item.Current[0]"></option-picker></td>
              <td class="text-right"><option-picker :options="[{name: '+CH1', value: '+CH1'}, {name: '-CH1', value: '-CH1'}, {name: '+CH2', value: '+CH2'}, {name: '-CH2', value: '-CH2'}, {name: '+CH3', value: '+CH3'}, {name: '-CH3', value: '-CH3'}, {name: '+CH4', value: '+CH4'}, {name: '-CH4', value: '-CH4'}]" v-model="item.Current[1]"></option-picker></td>
              <td class="text-right"><option-picker :options="[{name: '+CH1', value: '+CH1'}, {name: '-CH1', value: '-CH1'}, {name: '+CH2', value: '+CH2'}, {name: '-CH2', value: '-CH2'}, {name: '+CH3', value: '+CH3'}, {name: '-CH3', value: '-CH3'}, {name: '+CH4', value: '+CH4'}, {name: '-CH4', value: '-CH4'}]" v-model="item.Current[2]"></option-picker></td>
            </tr>
          </table>
          <table v-if="item.Mode == '1PHASE'">
            <tr><td colspan="2" class="border-bottom">1PHASE<span class="serial-number">{{ Settings.Serial }}_{{ item.Current.substr(1) }}_1P</span></td></tr>
            <tr>
              <td class="text-center border-right"><strong>Voltage</strong></td>
              <td class="text-center"><strong>Current</strong></td>
            </tr>
            <tr>
              <td class="text-right border-right"><option-picker :options="[{name: 'CH1', value: 'CH1'}, {name: 'CH2', value: 'CH2'}, {name: 'CH3', value: 'CH3'}]" v-model="item.Voltage"></option-picker></td>
              <td class="text-right"><option-picker :options="[{name: '+CH1', value: '+CH1'}, {name: '-CH1', value: '-CH1'}, {name: '+CH2', value: '+CH2'}, {name: '-CH2', value: '-CH2'}, {name: '+CH3', value: '+CH3'}, {name: '-CH3', value: '-CH3'}, {name: '+CH4', value: '+CH4'}, {name: '-CH4', value: '-CH4'}]" v-model="item.Current"></option-picker></td>
            </tr>
          </table>
        </div>
        <table v-if="CalcCh != null && CalcCh.Enabled">
          <tr><td colspan="6" class="border-bottom">1PHASE<span class="serial-number">{{ Settings.Serial }}_CAL_1P</span></td></tr>
          <tr>
            <td class="text-center border-right"><strong>Enabled</strong></td>
            <td class="text-center border-right"><strong>Voltage</strong></td>
            <td class="text-center"><strong>Current CH1</strong></td>
            <td class="text-center"><strong>Current CH2</strong></td>
            <td class="text-center"><strong>Current CH3</strong></td>
            <td class="text-center"><strong>Current CH4</strong></td>
          </tr>
          <tr>
            <td class="text-center border-right"><toggle onText="Yes" offText="No" v-model="Settings.Calculated.Enabled"></toggle></td>
            <td class="text-right border-right"><option-picker :options="AvaliableVoltageChannels" v-model="Settings.Calculated.Voltage"></option-picker></td>
            <td class="text-right">
              <option-picker v-if="CalcCh[Settings.Calculated.Voltage].CH1" :options="[{name: 'UNUSED', value: 'UNUSED'}, {name: 'ADD', value: 'ADD'}, {name: 'SUB', value: 'SUB'}]" v-model="Settings.Calculated.Current[0]"></option-picker>
              <div v-else class="text-left input-group-xs"><span class="form-control">UNUSED</span></div>
            </td>
            <td class="text-right">
              <option-picker v-if="CalcCh[Settings.Calculated.Voltage].CH2" :options="[{name: 'UNUSED', value: 'UNUSED'}, {name: 'ADD', value: 'ADD'}, {name: 'SUB', value: 'SUB'}]" v-model="Settings.Calculated.Current[1]"></option-picker>
              <div v-else class="text-left input-group-xs"><span class="form-control">UNUSED</span></div>
            </td>
            <td class="text-right">
              <option-picker v-if="CalcCh[Settings.Calculated.Voltage].CH3" :options="[{name: 'UNUSED', value: 'UNUSED'}, {name: 'ADD', value: 'ADD'}, {name: 'SUB', value: 'SUB'}]" v-model="Settings.Calculated.Current[2]"></option-picker>
              <div v-else class="text-left input-group-xs"><span class="form-control">UNUSED</span></div>
            </td>
            <td class="text-right">
              <option-picker v-if="CalcCh[Settings.Calculated.Voltage].CH4" :options="[{name: 'UNUSED', value: 'UNUSED'}, {name: 'ADD', value: 'ADD'}, {name: 'SUB', value: 'SUB'}]" v-model="Settings.Calculated.Current[3]"></option-picker>
              <div v-else class="text-left input-group-xs"><span class="form-control">UNUSED</span></div>
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
        this.$http.post('setDevice', this.Settings, { headers: { 'Accept': 'application/json', 'Content-Type': 'application/json' } }).then(
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