<template>
  <div>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != ''">{{ Error }}</div>
    <detail-tabs :tabs="['Inputs', 'Outputs']" v-model="View"></detail-tabs>
    <div class="row" v-if="View=='Inputs'">
      <div class="col-xs-12" style="margin-top:5px;">
        <label class="control-label" style="line-height: 22px">Start on boot</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="value.AutoStart"></toggle>
      </div>          
      <div class="col-xs-12">
        <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Device inputs can be calibrated to compensate components derating over time.</div>
      </div>
      <div class="col-xs-12">
        <table>
          <tr><td colspan="4" class="border-bottom">Voltage channels</td></tr>
          <tr>
            <td></td>
            <td class="text-right"><strong>ADC Offset</strong> <img :src="require('Imgs/help16.png')" data-toggle="tooltip" title="Add or substract value to ADC read, to adjust 0 point."></td>
            <td></td>
            <td class="text-right"><strong>Multiplier</strong> <img :src="require('Imgs/help16.png')" data-toggle="tooltip" title="Multiply read value by a number to adjust input across voltage sensing transformer or for apply calibrations."></td>
          </tr>
          <tr v-for="index in [0, 1, 2]">
            <td class="text-center"><strong>CH{{ index + 1 }}</strong></td>
            <td><input-spin :min="-200" :max="200" v-model="value.VoltageOffset[index]" :increment="0.01"></input-spin></td>
            <td><button type="button" class="btn btn-default btn-xs" style="height:34px" v-on:click="SetVoltageOffset(index)" v-bind:disabled="value.Wave_Average.Voltage[index] == 0.0" data-toggle="tooltip" :title="'Last second offset: ' + Round(-value.Wave_Average.Voltage[index], 2)"><img :src="require('Imgs/wand.png')"></button></td>
            <td><input-spin :min="0.1" :max="1000" v-model="value.VoltageGain[index]" :increment="0.001" :decimals="3" prepend="V/V"></input-spin></td>
          </tr>
        </table>
        <table>
          <tr><td colspan="5" class="border-bottom">Current channels</td></tr>
          <tr><td colspan="5">
            <label class="control-label" style="line-height: 22px">Full scale mode</label>
            <toggle class="float-right toggle-gain" onText="333mV" onStyle="primary" offText="625mV" offStyle="primary" v-model="value.HighGain"></toggle>
          </td></tr>
          <tr>
            <td></td>
            <td class="text-right"><strong>Offset <img :src="require('Imgs/help16.png')" data-toggle="tooltip" title="Add or substract value to ADC read, to adjust 0 point."></strong></td>
            <td></td>
            <td class="text-right"><strong>Multiplier</strong></td>
          </tr>
          <tr v-for="index in [0, 1, 2, 3]">
            <td class="text-center"><strong>CH{{ index + 1 }}</strong></td>
            <td><input-spin :min="-200" :max="200" v-model="value.CurrentOffset[index]" :increment="0.01"></input-spin></td>
            <td><button type="button" class="btn btn-default btn-xs" style="height:34px" v-on:click="SetCurrentOffset(index)" data-toggle="tooltip" :title="'Last second offset: ' + Round(-value.Wave_Average.Current[index], 2)"><img :src="require('Imgs/wand.png')"></button></td>
            <td><input-spin :min="0.00001" :max="20" v-model="value.CurrentGain[index]" :increment="0.00001" :decimals="5" prepend="A/mV"></input-spin></td>
          </tr>
        </table>
      </div>
    </div>
    <div class="row" v-if="View=='Outputs'">
      <div class="col-xs-12" style="margin-top:5px;">
        <div style="display: flex">
          <label class="control-label" style="line-height: 22px; min-width: 180px">Outputs configuration model</label>
          <option-picker :options="[ {name: '3PHASE+N', value: '3PHASE+N'}, {name: '3PHASE', value: '3PHASE'}, {name: '3PHASE + 1PHASE', value: '3PHASE + 1PHASE'}, {name: '1PHASE', value: '1PHASE'}, {name: '1PHASE + 1PHASE', value: '1PHASE + 1PHASE'}, {name: '1PHASE + 1PHASE + 1PHASE', value: '1PHASE + 1PHASE + 1PHASE'}, {name: '1PHASE + 1PHASE + 1PHASE + 1PHASE', value: '1PHASE + 1PHASE + 1PHASE + 1PHASE'}]" v-model="OutputMode"></option-picker>
        </div>
      </div>          
      <div class="col-xs-12">
        <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Current channels can not be used twice<br/>Voltage channels can not be used twice in same analyzer</div>
      </div>
      <div class="col-xs-12">
        <div class="output-table" v-for="item in value.Outputs">
          <table v-if="item.Mode == '3PHASE+N'">
            <tr><td colspan="7" class="border-bottom">3PHASE+N<span class="serial-number">{{ value.Serial }}_3PN</span></td></tr>
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
            <tr><td colspan="6" class="border-bottom">3PHASE<span class="serial-number">{{ value.Serial }}_3P</span></td></tr>
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
            <tr><td colspan="2" class="border-bottom">1PHASE<span class="serial-number">{{ value.Serial }}_1P_{{ item.Current.substr(1) }}</span></td></tr>
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
  import DetailTabs from 'Components/Tabs.vue';
  export default {
    props: ['value'],
    data() {
      return { View: 'Inputs', ShowSpinner: false, Error: '', OutputMode: null };
    },
    created: function() {
      var Phase3N = 0;
      var Phase3  = 0;
      var Phase1  = 0;
      if(this.value.Outputs.lengt == 0) this.$set(this.value, 'Outputs', [ { Mode: '3PHASE+N', Voltage: ['CH1', 'CH2', 'CH3'], Current: ['+CH1', '+CH2', '+CH3', '+CH4'] } ]);
      for(var idx in this.value.Outputs) {
        if(this.value.Outputs[idx].Mode === '3PHASE+N') Phase3N++;
        if(this.value.Outputs[idx].Mode === '3PHASE') Phase3++;
        if(this.value.Outputs[idx].Mode === '1PHASE') Phase1++;
      }
      if((Phase3N === 1) && (Phase3 === 0) && (Phase1 === 0)) this.OutputMode = '3PHASE+N';
      if((Phase3N === 0) && (Phase3 === 1) && (Phase1 === 0)) this.OutputMode = '3PHASE';
      if((Phase3N === 0) && (Phase3 === 1) && (Phase1 === 1)) this.OutputMode = '3PHASE + 1PHASE';
      if((Phase3N === 0) && (Phase3 === 0) && (Phase1 === 1)) this.OutputMode = '1PHASE';
      if((Phase3N === 0) && (Phase3 === 0) && (Phase1 === 2)) this.OutputMode = '1PHASE + 1PHASE';
      if((Phase3N === 0) && (Phase3 === 0) && (Phase1 === 3)) this.OutputMode = '1PHASE + 1PHASE + 1PHASE';
      if((Phase3N === 0) && (Phase3 === 0) && (Phase1 === 4)) this.OutputMode = '1PHASE + 1PHASE + 1PHASE + 1PHASE';
    },
    watch: {
      OutputMode: function(val, old) {
        if(old === null) return;
        this.$set(this.value, 'Outputs', []);
        if(val === '3PHASE+N') {
          this.$set(this.value, 'Outputs', [ { Mode: '3PHASE+N', Voltage: ['CH1', 'CH2', 'CH3'], Current: ['+CH1', '+CH2', '+CH3', '+CH4'] } ]);
        } else if(val === '3PHASE') {
          this.$set(this.value, 'Outputs', [ { Mode: '3PHASE', Voltage: ['CH1', 'CH2', 'CH3'], Current: ['+CH1', '+CH2', '+CH3'] } ]);
        } else if(val === '3PHASE + 1PHASE') {
          this.$set(this.value, 'Outputs', [ { Mode: '3PHASE', Voltage: ['CH1', 'CH2', 'CH3'], Current: ['+CH1', '+CH2', '+CH3'] }, { Mode: '1PHASE', Voltage: 'CH1', Current: '+CH4' } ]);
        } else if(val === '1PHASE') {
          this.$set(this.value, 'Outputs', [ { Mode: '1PHASE', Voltage: 'CH1', Current: '+CH1' } ]);
        } else if(val === '1PHASE + 1PHASE') {
          this.$set(this.value, 'Outputs', [ { Mode: '1PHASE', Voltage: 'CH1', Current: '+CH1' }, { Mode: '1PHASE', Voltage: 'CH2', Current: '+CH2' } ]);
        } else if(val === '1PHASE + 1PHASE + 1PHASE')  {
          this.$set(this.value, 'Outputs', [ { Mode: '1PHASE', Voltage: 'CH1', Current: '+CH1' }, { Mode: '1PHASE', Voltage: 'CH2', Current: '+CH2' }, { Mode: '1PHASE', Voltage: 'CH3', Current: '+CH3' } ]);
        } else if(val === '1PHASE + 1PHASE + 1PHASE + 1PHASE') {
          this.$set(this.value, 'Outputs', [ { Mode: '1PHASE', Voltage: 'CH1', Current: '+CH1' }, { Mode: '1PHASE', Voltage: 'CH2', Current: '+CH2' }, { Mode: '1PHASE', Voltage: 'CH3', Current: '+CH3' }, { Mode: '1PHASE', Voltage: 'CH1', Current: '+CH4' } ]);
        }
      }
    },
    methods: {
      Round: oZm.RoundPlain,
      SetCurrentOffset: function(index) {
        this.$set(this.value.CurrentOffset, index, -this.value.Wave_Average.Current[index]);
        this.value.Wave_Average.Current[index] = 0.0;
      },
      SetVoltageOffset: function(index) {
        this.$set(this.value.VoltageOffset, index, -this.value.Wave_Average.Voltage[index]);
        this.value.Wave_Average.Voltage[index] = 0.0;
      }
    },
    components: { InputSpin, Toggle, OptionPicker }
  }
</script>

<style scoped>
  table {
     width: 100%;
     border-radius: 6px;
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
    float:right;
    font-weight: normal;
  }
  .toggle-gain >>> .toggle {
    width: 60px !important;
  }
</style>