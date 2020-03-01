<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Configuration</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" :class="{ disabled: ShowSpinner }" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', null)"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != '' && !ShowSpinner">{{ Error }}</div>
    <div class="row">
      <div class="col-xs-9">
        <label class="control-label">Serial number</label>
        <input type="text" class="form-control" disabled="" :value="Analyzer.Analyzer">
      </div>
      <div class="col-xs-3">
        <label class="control-label">Mode</label>
        <input type="text" class="form-control" disabled="" :value="Analyzer.Mode">
      </div>
      <div class="col-xs-12 col-sm-7">
        <label class="control-label">Display name</label>
        <input type="text" class="form-control" v-model="Analyzer.Name" required>
      </div>
      <div class="col-xs-12 col-sm-5">
        <label class="control-label">Time zone</label>
        <option-picker :options="TimeZones" v-model="Analyzer.TimeZone" :big="true"></option-picker>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Nominal voltage</label>
        <input-spin prepend="V" :big="true" :list="[{name: '120 V (L-N)', value: 120}, {name: '240 V (L-L)', value: 240}, {name: '230 V (L-N)', value: 230}]" :decimals="1" :min="10" :max="10000" :increment="1" v-model="Analyzer.NominalVoltage"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Nominal frequency</label>
        <input-spin prepend="Hz" :big="true" :list="[{name: 'DC', value: 0}, {name: '50 Hz', value: 50}, {name: '60 Hz', value: 60}]" :decimals="1" :min="0" :max="500" :increment="1" v-model="Analyzer.NominalFrequency"></input-spin>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Min current</label>
        <input-spin prepend="A" :big="true" :decimals="1" :min="0" :max="500" :increment="1" v-model="Analyzer.MinCurrent"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Max current</label>
        <input-spin prepend="A" :big="true" :decimals="1" :min="0.1" :max="500" :increment="1" v-model="Analyzer.MaxCurrent"></input-spin>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12">
        <table>
          <tr><td><strong>Tag name</strong></td><td class="text-right"><strong>Value</strong></td></tr>
          <tr v-for="(tag, idx) in Analyzer.Tags" :key="idx">
            <td>
              <div class="input-group input-group-xs">
                <span class="input-group-btn"><button class="btn btn-default" type="button" @click="Analyzer.Tags.splice(idx, 1)"><img :src="require('Imgs/cross.png')"></button></span>
                <input type="text" class="form-control" v-model="tag.Name">
              </div>
            </td>
            <td>
              <div class="input-group input-group-xs">
                <input type="text" class="form-control rounded" v-model="tag.Value">
              </div>
            </td>
          </tr>
          <tr>
            <td colspan="2">
              <button type="button" class="btn btn-default btn-xs" style="width:100%" @click="Analyzer.Tags.push({ Name: 'Unnamed tag', Value: 'none'})"><img :src="require('Imgs/plus.png')"> New tag</button>
            </td>
          </tr>
        </table>
      </div>
    </div>
  </dlg>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import InputSpin from 'Components/InputSpin.vue';
  import moment from 'moment';
  export default {
    props: { analyzer: Object },
    data() {
      return { ShowSpinner: false, Error: '', Analyzer: null,
               TimeZones: moment.tz.names().map((val) => { return { name: val, value: val }}),
               Columns: [
                 {name: 'Name', title: 'Name'},
                 {name: '__slot:power', title: 'Power mode'},
                 {name: '__slot:reactive', title: 'Reactive mode'},
                 {name: '__slot:color', title: 'Colour'}
               ]
      };
    },
    created: function() {
      this.Analyzer = Object.assign({}, this.analyzer);
      this.View = 'Basic';
    },
    methods: {
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post('setAnalyzer', this.Analyzer).then(
          (result) => { this.$emit('close', this.Analyzer.Analyzer); },
          (fail) => {
            this.ShowSpinner = false;
            this.Error = 'Failed to save analyzer!';
            setTimeout(() => { this.Error = ''; }, 5000);
          }
        );
      }
    },
    components: { InputSpin, Vuetable }
  }
</script>

<style scoped>
  table, .input-group {
    width: 100%;
  }
  table tr td {
    padding: 1px 3px;
  }
  .rounded {
    border-radius: 4px !important;
  }
</style>