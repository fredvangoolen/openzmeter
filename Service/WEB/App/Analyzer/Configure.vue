<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Configuration</h4></div>
      <div class="pull-right btn-group">
        <z-button :disabled="ShowSpinner" @click="Save" :image="require('Imgs/diskette.png')" tooltip="Save" />
        <z-button @click="$emit('close', null)" :image="require('Imgs/arrow_undo.png')" tooltip="Discard" />
      </div>
    </template>
    <z-row>
      <div class="col-xs-9">
        <label class="control-label">Serial number</label>
        <input-text :disabled="true" :value="Analyzer.Analyzer"></input-text>
      </div>
      <div class="col-xs-3">
        <label class="control-label">Mode</label>
        <input-text :disabled="true" :value="Analyzer.Mode"></input-text>
      </div>
      <div class="col-xs-12 col-sm-7">
        <label class="control-label">Display name</label>
        <input-text v-model="Analyzer.Name"></input-text>
      </div>
      <div class="col-xs-12 col-sm-5">
        <label class="control-label">Time zone</label>
        <option-picker :options="TimeZones" v-model="Analyzer.TimeZone"></option-picker>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Nominal voltage</label>
        <input-spin prepend="V" :list="[{name: '120 V (L-N)', value: 120}, {name: '240 V (L-L)', value: 240}, {name: '230 V (L-N)', value: 230}]" :decimals="1" :min="10" :max="10000" :increment="1" v-model="Analyzer.NominalVoltage"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Nominal frequency</label>
        <option-picker :options="[{name: 'DC', value: 0}, {name: '50 Hz', value: 50}, {name: '60 Hz', value: 60}]" v-model="Analyzer.NominalFrequency"></option-picker>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Min current</label>
        <input-spin prepend="A" :decimals="1" :min="0" :max="500" :increment="1" v-model="Analyzer.MinCurrent"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Max current</label>
        <input-spin prepend="A" :decimals="1" :min="0.1" :max="500" :increment="1" v-model="Analyzer.MaxCurrent"></input-spin>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <z-row>
        <div class="col-xs-12">
          <label class="control-label" style="line-height: 22px">Phase names</label>
        </div>
        <div class="col-xs-3">
          <input-text prepend="L1" v-model="Analyzer.PhaseNames[0]"></input-text>
        </div>
        <div class="col-xs-3">
          <input-text prepend="L2" :disabled="Analyzer.PhaseNames.length<2" v-model="Analyzer.PhaseNames[1]"></input-text>
        </div>
        <div class="col-xs-3">
          <input-text prepend="L3" :disabled="Analyzer.PhaseNames.length<3" v-model="Analyzer.PhaseNames[2]"></input-text>
        </div>
        <div class="col-xs-3">
          <input-text prepend="N" :disabled="Analyzer.NeutralName==null" v-model="Analyzer.NeutralName"></input-text>
        </div>
      </z-row>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable 200ms</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Analyzer.Save200MS"></toggle>
        <div class="help">This function allow access all parameters obtained from 200ms blocks. This aggregation is not saved to disk and is only retained for an hour. All parameters can be browsed from 'Analysis/200MS'</div>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12">
        <table>
          <tr><td><strong>Tag name</strong></td><td class="text-right"><strong>Value</strong></td></tr>
          <tr v-for="(tag, idx) in Tags" :key="idx">
            <td>
              <div class="input-group input-group-xs">
                <span class="input-group-btn"><button class="btn btn-default" type="button" @click="Tags.splice(idx, 1)"><img :src="require('Imgs/cross.png')"></button></span>
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
              <button type="button" class="btn btn-default btn-xs btn-block" @click="Tags.push({ Name: 'Unnamed tag', Value: 'none'})"><img :src="require('Imgs/plus.png')"> New tag</button>
            </td>
          </tr>
        </table>
      </div>
    </z-row>
  </dlg>
</template>

<script>
  import moment from 'moment';
  export default {
    props: { analyzer: Object },
    data() {
      return { ShowSpinner: false, Analyzer: null, Tags: [],
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
      this.Tags = [];
      for(var prop in this.Analyzer.Tags) this.Tags.push({Name: prop, Value: this.Analyzer.Tags[prop]})
    },
    methods: {
      Save: function() {
        this.ShowSpinner = true;
        for(var prop in this.Tags) this.Analyzer.Tags[this.Tags[prop].Name] = this.Tags[prop].Value;
        this.Post('setAnalyzer', this.Analyzer).then(
          () => { this.$emit('close', this.Analyzer); },
          (fail) => {
            this.ShowSpinner = false;
            if(fail != null) this.$toast.open({type:'error', message: "Failed to save analyzer config<br>" + fail.message, duration: 3000});
          }
        );
      }
    }
  }
</script>

<style scoped>
  table, .input-group {
    width: 100%;
  }
  table tr td {
    padding: 1px 3px;
  }
  .help {
    font-size: 80%;
  }
</style>