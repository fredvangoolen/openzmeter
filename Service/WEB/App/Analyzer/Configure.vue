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
        <input type="text" class="form-control" disabled="" :value="analyzer.Analyzer">
      </div>
      <div class="col-xs-3">
        <label class="control-label">Mode</label>
        <input type="text" class="form-control" disabled="" :value="analyzer.Mode">
      </div>
      <div class="col-xs-12 col-sm-7">
        <label class="control-label">Display name</label>
        <input type="text" class="form-control" v-model="analyzer.Name" required>
      </div>
      <div class="col-xs-12 col-sm-5">
        <label class="control-label">Time zone</label>
        <option-picker :options="TimeZones" v-model="analyzer.TimeZone" :big="true"></option-picker>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Nominal voltage</label>
        <input-spin prepend="V" :list="[{name: '120 V (L-N)', value: 120}, {name: '240 V (L-L)', value: 240}, {name: '230 V (L-N)', value: 230}]" :decimals="1" :min="10" :max="10000" :increment="1" v-model="analyzer.NominalVoltage"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Nominal frequency</label>
        <input-spin prepend="Hz" :list="[{name: 'DC', value: 0}, {name: '50 Hz', value: 50}, {name: '60 Hz', value: 60}]" :decimals="1" :min="0" :max="500" :increment="1" v-model="analyzer.NominalFreq"></input-spin>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Min current</label>
        <input-spin prepend="A" :decimals="1" :min="0" :max="500" :increment="1" v-model="analyzer.MinCurrent"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Max current</label>
        <input-spin prepend="A" :decimals="1" :min="0.1" :max="500" :increment="1" v-model="analyzer.MaxCurrent"></input-spin>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12">
        <table>
          <tr><td><strong>Tag name</strong></td><td class="text-right"><strong>Value</strong></td></tr>
          <tr v-for="(tag, idx) in Tags" :key="idx">
            <td>
              <div class="input-group input-group-xs">
                <span class="input-group-btn"><button class="btn btn-default" type="button" @click="Tags.splice(idx, 1)"><img :src="require('Imgs/cross.png')"></button></span>
                <input type="text" class="form-control" v-model="tag.name">
              </div>
            </td>
            <td>
              <div class="input-group input-group-xs">
                <input type="text" class="form-control rounded" v-model="tag.value">
              </div>
            </td>
          </tr>
          <tr>
            <td colspan="2"><button type="button" class="btn btn-default btn-xs" style="width:100%" v-on:click="Tags.push({ name: 'Unnamed tag', Value: 'none'})"><img :src="require('Imgs/plus.png')"> New tag</button></td>
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
      return { ShowSpinner: false, Error: '', Tags: [{name: 'bliu', value: 'blau'}],
      Columns: [
          {name: 'Name', title: 'Name'},
          {name: '__slot:power', title: 'Power mode'},
          {name: '__slot:reactive', title: 'Reactive mode'},
          {name: '__slot:color', title: 'Colour'}
        ]
      }
      ;
    },
    computed: {
      TimeZones: function() {
        return moment.tz.names().map((val) => { return { name: val, value: val }; });
      }
    },
    mounted: function() {
      this.View = 'Basic';
    },
    methods: {
      GetLocation: function() {
        navigator.geolocation.getCurrentPosition((val) => {
          this.value.GeoPosition = [val.coords.latitude, val.coords.longitude];
        });
      },
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post('setAnalyzer', this.analyzer).then(
          (result) => {
            if(result.status == 200) {
              this.$http.post('getAnalyzer', { Analyzer: this.analyzer.Analyzer }).then(
                (result) => {
                  this.ShowSpinner = false;
                  if(result.status == 200) this.$emit('close', result.data);
                }
              );
            } else {
              this.ShowSpinner = false;
              this.Error = 'Failed to save analyzer!';
              setTimeout(() => { this.Error = ''; }, 5000);
            }
          },
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