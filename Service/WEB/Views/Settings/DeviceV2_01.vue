<template>
  <div class="modal-content">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Device &raquo; {{ Settings.Serial }}</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-on:click="$parent.Save"><img :src="require('Imgs/diskette.png')"></button>
        <button type="button" class="btn btn-default btn-xs" v-on:click="$parent.Close"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
      <div class="btn-group pull-right">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: View=='Inputs' }" v-on:click="View='Inputs'">Main</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: View=='Outputs' }" v-on:click="View='Outputs'">Analyzers</button>
        <button type="button" class="btn btn-default btn-xs dropdown-toggle" data-toggle="dropdown"><span class="caret"></span></button>
        <ul class="dropdown-menu">
          <li><a v-on:click="SetMode('3PHASE+N')">3PHASE+N</a></li>
          <li role="separator" class="divider"></li>
          <li><a v-on:click="SetMode('3PHASE')">3PHASE</a></li>
          <li><a v-on:click="SetMode('3PHASE + 1PHASE')">3PHASE + 1PHASE</a></li>
          <li role="separator" class="divider"></li>
          <li><a v-on:click="SetMode('1PHASE')">1PHASE</a></li>
          <li><a v-on:click="SetMode('1PHASE + 1PHASE')">1PHASE + 1PHASE</a></li>
          <li><a v-on:click="SetMode('1PHASE + 1PHASE + 1PHASE')">1PHASE + 1PHASE + 1PHASE</a></li>
          <li><a v-on:click="SetMode('1PHASE + 1PHASE + 1PHASE + 1PHASE')">1PHASE + 1PHASE + 1PHASE + 1PHASE</a></li>
        </ul>
      </div>
    </div>
    <div class="modal-body">
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != ''">{{ Error }}</div>
      <spinner v-if="ShowSpinner"></spinner>
      <div class="row" v-bind:style="{ display: (View=='Inputs') ? 'block' : 'none' }">
        <div class="col-xs-12">
          <label class="control-label" style="line-height: 22px">Start on boot</label>
          <toggle class="float-right" onText="Yes" offText="No" v-model="Settings.AutoStart"></toggle>
        </div>          
        <div class="col-xs-12">
          <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Device inputs can be calibrated to compensate components derating over time.</div>
        </div>
        <div class="col-xs-12">
          <table>
            <tr><td colspan="4" class="border-bottom">Voltage channels</td></tr>
            <tr>
              <td></td>
              <td class="text-right"><strong>Correction</strong></td>
              <td class="text-right"><strong>Offset</strong></td>
              <td></td>
            </tr>
            <tr v-for="index in [0, 1, 2]">
              <td class="text-center"><strong>CH{{ index + 1 }}</strong></td>
              <td><input-spin :min="-10" :max="10" v-model="Settings.VoltageGain[index]" :increment="0.001" :decimals="3" prepend="%"></input-spin></td>
              <td><input-spin :min="-20" :max="20" v-model="Settings.VoltageOffset[index]" :increment="0.01" prepend="V"></input-spin></td>
              <td><button type="button" class="btn btn-default btn-xs" style="height:34px" v-on:click="SetVoltageOffset(index)" data-toggle="tooltip" :title="'Last second offset: ' + OffsetV[index] + ' V'"><img :src="require('Imgs/wand.png')"></button></td>
            </tr>
          </table>
          <table>
            <tr><td colspan="5" class="border-bottom">Current channels</td></tr>
              <tr><td colspan="5">
                <label class="control-label" style="line-height: 22px">Full scale mode</label>
                <toggle class="float-right toggle-gain" onText="333mV" onStyle="primary" offText="625mV" offStyle="primary" v-model="Settings.HighGain"></toggle>
              </td></tr>
            <tr>
              <td></td>
              <td class="text-right"><strong>Full scale value</strong></td>
              <td class="text-right"><strong>Correction</strong></td>
              <td class="text-right"><strong>Offset</strong></td>
              <td></td>
            </tr>
            <tr v-for="index in [0, 1, 2, 3]">
              <td class="text-center"><strong>CH{{ index + 1 }}</strong></td>
              <td><input-spin :min="0.1" :max="1000000" v-model="Settings.CurrentLevel[index]" :increment="0.1" :decimals="1" prepend="A"></input-spin></td>
              <td><input-spin :min="-10" :max="10" v-model="Settings.CurrentGain[index]" :increment="0.001" :decimals="3" prepend="%"></input-spin></td>
              <td><input-spin :min="-1000" :max="1000" ref="CurrentOffset" v-model="Settings.CurrentOffset[index]" :increment="0.01" prepend="A"></input-spin></td>
              <td><button type="button" class="btn btn-default btn-xs" style="height:34px" v-on:click="SetCurrentOffset(index)" data-toggle="tooltip" :title="'Last second offset: ' + OffsetC[index] + ' A'"><img :src="require('Imgs/wand.png')"></button></td>
            </tr>
          </table>
        </div>
      </div>
      <div class="row" v-bind:style="{ display: (View=='Outputs') ? 'block' : 'none' }">
        <div class="col-xs-12">
          <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Current channels can not be used twice<br/>Voltage channels can not be used twice in same analyzer</div>
        </div>
        <div class="col-xs-12">
          <div class="output-table" v-for="item in Settings.Outputs">
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
                <td class="text-right"><option-picker :options="['+CH1', '-CH1', '+CH2', '-CH2', '+CH3', '-CH3', '+CH4', '-CH4']" :selected="item.Current[0]" v-on:change="item.Current[0] = $event"></option-picker></td>
                <td class="text-right"><option-picker :options="['+CH1', '-CH1', '+CH2', '-CH2', '+CH3', '-CH3', '+CH4', '-CH4']" :selected="item.Current[1]" v-on:change="item.Current[1] = $event"></option-picker></td>
                <td class="text-right"><option-picker :options="['+CH1', '-CH1', '+CH2', '-CH2', '+CH3', '-CH3', '+CH4', '-CH4']" :selected="item.Current[2]" v-on:change="item.Current[2] = $event"></option-picker></td>
                <td class="text-right"><option-picker :options="['+CH1', '-CH1', '+CH2', '-CH2', '+CH3', '-CH3', '+CH4', '-CH4']" :selected="item.Current[3]" v-on:change="item.Current[3] = $event"></option-picker></td>
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
                <td class="text-right"><option-picker :options="['+CH1', '-CH1', '+CH2', '-CH2', '+CH3', '-CH3', '+CH4', '-CH4']" :selected="item.Current[0]" v-on:change="item.Current[0] = $event"></option-picker></td>
                <td class="text-right"><option-picker :options="['+CH1', '-CH1', '+CH2', '-CH2', '+CH3', '-CH3', '+CH4', '-CH4']" :selected="item.Current[1]" v-on:change="item.Current[1] = $event"></option-picker></td>
                <td class="text-right"><option-picker :options="['+CH1', '-CH1', '+CH2', '-CH2', '+CH3', '-CH3', '+CH4', '-CH4']" :selected="item.Current[2]" v-on:change="item.Current[2] = $event"></option-picker></td>
              </tr>
            </table>
            <table v-if="item.Mode == '1PHASE'">
              <tr><td colspan="2" class="border-bottom">1PHASE<span class="serial-number">{{ settings.Serial }}_1P_{{ item.Current.substr(1) }}</span></td></tr>
              <tr>
                <td class="text-center border-right"><strong>Voltage</strong></td>
                <td class="text-center"><strong>Current</strong></td>
              </tr>
              <tr>
                <td class="text-right border-right"><option-picker :options="['CH1', 'CH2', 'CH3']" v-model="item.Voltage"></option-picker></td>
              <td class="text-right"><option-picker :options="['+CH1', '-CH1', '+CH2', '-CH2', '+CH3', '-CH3', '+CH4', '-CH4']" :selected="item.Current" v-on:change="item.Current = $event"></option-picker></td>
              </tr>
            </table>
          </div>
        </div>
      </div>
    </div>
  </div>      
</template>

<script>
  import Vue from 'vue';
  import * as oZm from 'JS/openZmeter.js';
  import OptionPicker from 'Components/OptionPicker.vue';
  import Spinner from 'Components/Spinner.vue';
  import InputSpin from 'Components/InputSpin.vue';
  import Toggle from 'Components/Toggle.vue';
  export default {
    props: ['settings'],
    data() {
      return { View: 'Inputs', Settings: {}, ShowSpinner: false, Error: '', OffsetV: [], OffsetC: [] };
    },
    created: function() {
      this.Settings = this.settings;
      for(var n = 0; n < this.settings.Wave_Average.Voltage.length; n++) this.OffsetV.push(oZm.RoundPlain(this.settings.Wave_Average.Voltage[n]));
      for(var n = 0; n < this.settings.Wave_Average.Current.length; n++) this.OffsetC.push(oZm.RoundPlain(this.settings.Wave_Average.Current[n]));
    },
    methods: {
      SetMode: function(val) {
        this.$set(this.Settings, 'Outputs', []);
        this.$nextTick(function() {
          if(val == '3PHASE+N') {
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
        });
      },
      SetCurrentOffset: function(index) {
        this.$refs.CurrentOffset[index].SetValue(-this.settings.Wave_Average.Current[index] + this.settings.CurrentOffset[index]);
      },
      SetVoltageOffset: function(index) {
        this.$refs.VoltageOffset[index].SetValue(-this.settings.Wave_Average.Voltage[index] + this.settings.VoltageOffset[index]);
      }
    },
    components: { Spinner, InputSpin, Toggle, OptionPicker }
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