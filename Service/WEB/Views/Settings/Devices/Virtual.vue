<template>
  <dlg :loading="Loading">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Device &raquo; {{ Settings.Serial }}</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', false)"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null">{{ Error }}</div>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enabled</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Settings.Enabled"></toggle>
        <hr style="margin: 5px 0px 5px 0px"/>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Voltage</label>
        <input-spin :min="10" :max="350" :big="true" v-model="Settings.Voltage" :increment="10" :decimals="2" prepend="V"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Frequency</label>
        <input-spin :min="0" :max="100" :big="true" v-model="Settings.Frequency" :increment="1" :decimals="2" prepend="Hz"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Current</label>
        <input-spin :min="0.1" :max="1000" :big="true" v-model="Settings.Current" :increment="0.1" :decimals="2" prepend="A"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Current phase</label>
        <input-spin :min="-180" :max="180" :big="true" v-model="Settings.CurrentPhase" :increment="0.1" :decimals="2" prepend="°"></input-spin>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Mode</label>
        <option-picker :options="[ {name: '1PHASE', value: '1PHASE'}, {name: '3PHASE', value: '3PHASE'}, {name: '3PHASE+N', value: '3PHASE+N'} ]" v-model="Settings.Mode" :big="true"></option-picker>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">Samplerate</label>
        <input-spin :min="10000" :max="1000000" :big="true" v-model="Settings.SampleRate" :increment="100" :decimals="0" prepend="Hz"></input-spin>
      </div>
    </div>
  </dlg>
</template>

<script>
  export default {
    props: { settings: Object },
    data() {
      return { Loading: false, Error: null, Settings: null };
    },
    created: function() {
      this.Settings = Object.assign({}, this.settings);
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
      }
    }
  }
</script>