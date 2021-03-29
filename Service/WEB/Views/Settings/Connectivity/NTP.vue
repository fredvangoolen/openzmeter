<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">NTP synchronization</h4></div>
      <div class="pull-right btn-group">
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" title="Save" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
          <button type="button" class="btn btn-default btn-xs" title="Back" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
      </div>
    </template>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Config.Enabled"></toggle>
        <div class="help">If config is saved in disabled state localtime is send to openZmeter.</div>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 5px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12">
        <label class="control-label">Server</label>
        <input type="text" class="form-control" v-model="Config.Server">
      </div>
      <div class="col-xs-12">
        <label class="control-label">Details</label>
        <textarea rows="2" class="form-control" disabled :value="Details"></textarea>
      </div>
    </div>
  </dlg>
</template>

<script>
  import moment from 'moment';
  import 'moment-timezone';
  export default {
    props: { settings: Object },
    data: function() {
      return { ShowSpinner: false, Config: null, Timer: null, Details: '' };
    },
    created: function() {
      this.Config = Object.assign({}, this.settings);
      this.Timer = setTimeout(this.Refresh, 100);
    },
    beforeDestroy: function() {
      if(this.Timer != null) clearTimeout(this.Timer);
      this.Timer = null;
    },
    methods: {
      Refresh: function() {
        clearTimeout(this.Timer);
        this.Post('getNTP', { }).then(
          (result) => {
            if(this.Timer == null) return;
            this.Details = 'Local timezone : ' + moment.tz.zone(moment.tz.guess()).name + '\nTime           : ' + moment(result.Time).format('DD/MM/YYYY HH:mm:ss');
            this.Timer = setTimeout(this.Refresh, 500);
          },
          (fail) => {
            if(this.Timer == null) return;
            this.Details = '';
            this.Timer = setTimeout(this.Refresh, 500);
          }
        );
      },
      Save: function(value) {
        this.ShowSpinner = true;
        this.Post('setNTP', { Enabled: this.Config.Enabled, Server: this.Config.Server, Time: moment().valueOf() }).then(
          (result) => {
            this.$emit('update:settings', this.Config);
            this.$emit('close');
          },
          (fail) => { this.ShowSpinner = false; }
        );
      }
    }
  }
</script>

<style scoped>
  textarea {
    resize: none;
    font-family: 'Fira Code';
    font-size: 12px;
  }
  .help {
    font-size: 80%;
  }
</style>