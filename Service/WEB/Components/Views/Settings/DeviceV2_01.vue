<template>
  <div class="row">
    <div class="col-xs-12 col-sm-6">
      <label class="control-label" style="line-height: 22px">Start on boot</label>
      <toggle class="float-right" onText="Yes" offText="No" ref="autoStart"></toggle>
    </div>
    <div class="col-xs-6 col-sm-3">
      <div class="btn-group" style="width: 100%">
        <button type="button" class="btn btn-default btn-xs" style="width: 100%" v-bind:class="{ active: View=='Inputs' }" v-on:click="ShowInputs($event)">Calibration</button>
      </div>
    </div>
    <div class="col-xs-6 col-sm-3">
      <div class="btn-group" style="width: 100%; display: flex">
        <button type="button" class="btn btn-default btn-xs" style="flex-grow: 100" v-bind:class="{ active: View=='Outputs' }" v-on:click="ShowOutputs($event)">Outputs</button>
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
    <div v-bind:style="{ display: (View=='Inputs') ? 'block' : 'none' }">
      <div class="col-xs-12">
        <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">Device inputs can be calibrated to compensate components derating over time.</div>
      </div>
      <div class="col-xs-12">
        <table>
          <tr><td colspan="3" class="border-bottom">Voltage channels</td></tr>
          <tr>
            <td></td>
            <td class="text-right"><strong>Correction</strong></td>
            <td class="text-right"><strong>Offset</strong></td>
          </tr>
          <tr v-for="index in [0, 1, 2]">
            <td class="text-center"><strong>CH{{ index + 1 }}</strong></td>
            <td><input-spin :min="-10" :max="10" :value="Settings.VoltageGain[index]" :increment="0.001" :decimals="3" prepend="%" v-on:change="Settings.VoltageGain[index] = $event"></input-spin></td>
            <td><input-spin :min="-20" :max="20" :value="Settings.VoltageOffset[index]" :increment="0.01" prepend="V" v-on:change="Settings.VoltageOffset[index] = $event"></input-spin></td>
          </tr>
        </table>
        <table>
          <tr><td colspan="4" class="border-bottom">Current channels</td></tr>
          <tr>
            <td></td>
            <td class="text-right"><strong>Level</strong></td>
            <td class="text-right"><strong>Correction</strong></td>
            <td class="text-right"><strong>Offset</strong></td>
          </tr>
          <tr v-for="index in [0, 1, 2, 3]">
            <td class="text-center"><strong>CH{{ index + 1 }}</strong></td>
            <td><input-spin :min="0.001" :max="1000000" :value="Settings.CurrentLevel[index]" :increment="0.001" :decimals="3" prepend="μV/A" v-on:change="Settings.CurrentLevel[index] = $event"></input-spin></td>
            <td><input-spin :min="-10" :max="10" :value="Settings.CurrentGain[index]" :increment="0.001" :decimals="3" prepend="%" v-on:change="Settings.CurrentGain[index] = $event"></input-spin></td>
            <td><input-spin :min="-10" :max="10" :value="Settings.CurrentOffset[index]" :increment="0.01" prepend="A" v-on:change="Settings.CurrentOffset[index] = $event"></input-spin></td>
          </tr>
        </table>
      </div>
    </div>
    <div v-bind:style="{ display: (View=='Outputs') ? 'block' : 'none' }">
      <div class="col-xs-12">
        <table v-if="Phase3N != null">
          <tr><td colspan="7" class="border-bottom">3PHASE+N ({{ Phase3N.Serial }})</td></tr>
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
            <td class="text-right"><option-picker :options="['CH1', 'CH2', 'CH3']" selected="CH1" v-on:change="InvertChanged" ref="invert"></option-picker></td>
            <td class="text-right"><option-picker :options="['CH1', 'CH2', 'CH3']" selected="CH2" v-on:change="InvertChanged" ref="invert"></option-picker></td>
            <td class="text-right border-right"><option-picker :options="['CH1', 'CH2', 'CH3']" selected="CH3" v-on:change="InvertChanged" ref="invert"></option-picker></td>
            <td class="text-right"><option-picker :options="['+CH1', '-CH1', '+CH2', '-CH2', '+CH3', '-CH3', '+CH4', '-CH4']" selected="CH1" v-on:change="InvertChanged" ref="invert"></option-picker></td>
            <td class="text-right"><option-picker :options="['+CH1', '-CH1', '+CH2', '-CH2', '+CH3', '-CH3', '+CH4', '-CH4']" selected="CH2" v-on:change="InvertChanged"></option-picker></td>
            <td class="text-right"><option-picker :options="['+CH1', '-CH1', '+CH2', '-CH2', '+CH3', '-CH3', '+CH4', '-CH4']" selected="CH3" v-on:change="InvertChanged"></option-picker></td>
            <td class="text-right"><option-picker :options="['+CH1', '-CH1', '+CH2', '-CH2', '+CH3', '-CH3', '+CH4', '-CH4']" selected="CH4" v-on:change="InvertChanged"></option-picker></td>
          </tr>
        </table>
        <table v-if="Phase3 != null">
          <tr><td colspan="6" class="border-bottom">3PHASE ({{ Phase3.Serial }})</td></tr>
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
            <td class="text-right"><option-picker :options="['CH1', 'CH2', 'CH3']" selected="CH1" v-on:change="InvertChanged" ref="invert"></option-picker></td>
            <td class="text-right"><option-picker :options="['CH1', 'CH2', 'CH3']" selected="CH2" v-on:change="InvertChanged" ref="invert"></option-picker></td>
            <td class="text-right border-right"><option-picker :options="['CH1', 'CH2', 'CH3']" selected="CH3" v-on:change="InvertChanged" ref="invert"></option-picker></td>
            <td class="text-right"><option-picker :options="['CH1', 'CH2', 'CH3', 'CH4']" selected="CH1" v-on:change="InvertChanged" ref="invert"></option-picker></td>
            <td class="text-right"><option-picker :options="['CH1', 'CH2', 'CH3', 'CH4']" selected="CH2" v-on:change="InvertChanged" ref="invert"></option-picker></td>
            <td class="text-right"><option-picker :options="['CH1', 'CH2', 'CH3', 'CH4']" selected="CH3" v-on:change="InvertChanged" ref="invert"></option-picker></td>
          </tr>
        </table>
        <table v-for="index in Phase1">
          <tr><td colspan="2" class="border-bottom">1PHASE (ghjghj)</td></tr>
          <tr>
            <td class="text-center border-right"><strong>Voltage</strong></td>
            <td class="text-center"><strong>Current</strong></td>
          </tr>
          <tr>
            <td class="text-right border-right"><option-picker :options="['CH1', 'CH2', 'CH3']" selected="CH1" v-on:change="InvertChanged" ref="invert"></option-picker></td>
            <td class="text-right"><option-picker :options="['CH1', 'CH2', 'CH3', 'CH4']" selected="CH1" v-on:change="InvertChanged" ref="invert"></option-picker></td>
          </tr>
        </table>
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
  export default {
    props: ['settings'],
    data() {
      return { View: 'Inputs', Settings: {}, Phase3N: {}, Phase3: null, Phase1: [] };
    },
    created: function() {
      this.Settings = this.settings;
    },
    mounted: function() {
      this.$refs.autoStart.SetState(this.Settings.AutoStart);
//      this.$refs.invert.SetOption(this.Settings.CurrentInvert ? 'INVERT' : 'NO INVERT');
    },
    methods: {
      ShowInputs: function(evt) {
        $(evt.target).blur();
        this.View = 'Inputs';
      },
      ShowOutputs: function(evt) {
        $(evt.target).blur();
        this.View = 'Outputs';
      },
      SetMode: function(val) {
        if(val == '3PHASE+N') {
          this.Phase3 = null;
          this.Phase1 = [];
          if(this.Phase3N == null) this.Phase3N = {};
        } else if(val == '3PHASE') {
          this.Phase3N = null;
          this.Phase1 = [];
          if(this.Phase3 == null) this.Phase3 = {};
        } else if(val == '3PHASE + 1PHASE') {
          this.Phase3N = null;
          this.Phase1 = [{}];
          if(this.Phase3 == null) this.Phase3 = {};
        } else if(val == '1PHASE') {
          this.Phase3N = null;
          this.Phase3 = null;    
          this.Phase1 = [{}];
        } else if(val == '1PHASE + 1PHASE') {
          this.Phase3N = null;
          this.Phase3 = null;    
          this.Phase1 = [{}, {}];
        } else if(val == '1PHASE + 1PHASE + 1PHASE')  {
          this.Phase3N = null;
          this.Phase3 = null;    
          this.Phase1 = [{}, {},{}];
        } else if(val == '1PHASE + 1PHASE + 1PHASE + 1PHASE') {
          this.Phase3N = null;
          this.Phase3 = null;    
          this.Phase1 = [{}, {},{},{}];
        }
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
  table:not(:first-child) {
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
</style>