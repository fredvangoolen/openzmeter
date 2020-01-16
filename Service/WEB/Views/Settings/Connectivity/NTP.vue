<template>
  <div v-if="Config!=null">
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">NTP synchronization</label>
      <div class="input-group">
        <span class="input-group-addon input-group-addon-toogle">
          <toggle v-model="Config.Enabled" @change="Changes=true"></toggle>
        </span>
        <input type="text" class="form-control" v-model="Config.Server" :disabled="!Config.Enabled">
        <span class="input-group-btn" v-if="Changes">
          <button class="btn btn-default" @click="Save" data-toggle="tooltip" title="Save changes (If set to disabled, current client date is taken)"><img :src="require('Imgs/diskette.png')"></button>
        </span>
      </div>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Details</label>
      <textarea rows="2" class="form-control" disabled :value="Details"></textarea>
    </div>
  </div>
</template>

<script>
  import moment from 'moment';
  import 'moment-timezone';
  export default {
    data: function() {
      return { Config: null, Changes: false, Timer: null, TmpConfig: null };
    },
    computed: {
      Details: function() {
        return 'Local timezone : ' + moment.tz.zone(moment.tz.guess()).name + '\nTime           : ' + moment(this.TmpConfig.Time).format('DD/MM/YYYY HH:mm:ss');
      }
    },
    created: function() {
      this.$http.post('getNTP', { }).then(
        (result) => {
          this.Config = result.data;
          this.TmpConfig = result.data;
          this.$emit('enabled');
          this.Timer = setTimeout(this.Refresh, 1000);
        },
        (fail) => { }
      );
    },
    beforeDestroy: function() {
      if(this.Timer != null) clearTimeout(this.Timer);
      this.Config = null;
    },
    methods: {
      Refresh: function() {
        if(this.Timer != null) clearTimeout(this.Timer);
        this.$http.post('getNTP', { }).then(
          (result) => {
            if(this.Config == null) return;
            this.TmpConfig = result.data;
            this.Timer = setTimeout(this.Refresh, 5000);
          },
          (fail) => { }
        );
      },
      Save: function(value) {
        this.$http.post('setNTP', { Enabled: this.Config.Enabled, Server: this.Config.Server, Time: moment().valueOf() }).then(
          (result) => { this.Changes = false; },
          () => {}
        );
      }
    }
  }
</script>

<style scoped>
  textarea {
    resize: none;
    font-family: monospace;
    font-size: 12px;
  }
</style>