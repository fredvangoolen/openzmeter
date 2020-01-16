<template>
  <div v-if="Config!=null">
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">WIFI connection</label>
      <div class="input-group">
        <span class="input-group-addon input-group-addon-toogle">
          <toggle v-model="Config.Enabled" v-on:change="Changes=true"></toggle>
        </span>
        <input type="text" class="form-control" disabled :value="Config.ESSID">
        <span class="input-group-btn">
          <button class="btn btn-default" type="button" v-on:click="Configure"><img :src="require('Imgs/network_wireless_edit.png')"></button>
          <button class="btn btn-default" v-if="Changes" v-on:click="Save" data-toggle="tooltip" title="Save changes"><img :src="require('Imgs/diskette.png')"></button>
        </span>
      </div>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Details</label>
      <textarea rows="4" class="form-control" disabled :value="Details"></textarea>
    </div>
    <wifi-dialog v-if="Settings!=null" :settings="Settings" @close="Settings=null"></wifi-dialog>
  </div>
</template>

<script>
  import WifiDialog from './WIFIEdit.vue';
  export default {
    data: function() {
      return { Config: null, Changes: false, Settings: null, Timer: null };
    },
    computed: {
      Details: function() {
        if(this.Config.Connected == true) {
          return 'Device IP address   : ' + this.Config.IP + ' (' + this.Config.IPMode + ')\nDNS servers         : ' + this.Config.DNS.join(', ') + '\nDefault gateway     : ' + this.Config.Gateway + '\nCurrent WIFI channel: ' + this.Config.Channel;
        } else {
          return 'Not connected';
        }
      }
    },
    created: function() {
      this.$http.post('getWIFI', { Type: "CURRENT" }).then(
        (result) => {
          this.Config = result.data;
          this.$emit('enabled');
          this.Refresh();
        },
        (fail) => { }
      );
    },
    beforeDestroy: function() {
      if(this.Timer != null) clearTimeout(this.Timer);
      this.Config = null;
    },
    methods: {
      Save: function(value) {
        this.$http.post('setWIFI', {Enabled: this.Config.Enabled}).then(
          (result) => { this.Changes = false; },
          (fail) => {}
        );
      },
      Refresh: function() {
        if(this.Timer != null) clearTimeout(this.Timer);
        this.$http.post('getWIFI', { Type: "CURRENT" }).then(
          (result) => {
            if(this.Config == null) return;
            this.Config = result.data;
            this.Timer = setTimeout(this.Refresh, 5000);
          },
          (fail) => { }
        );
      },
      Configure() {
        this.$http.post('getWIFI', { Type: 'CONFIG'}).then(
          (result) => { this.Settings = result.data; },
          (fail) => { }
        );
      }
    },
    components: { WifiDialog }
  }
</script>

<style scoped>
  textarea {
    resize: none;
    font-family: monospace;
    font-size: 12px;
  }
</style>