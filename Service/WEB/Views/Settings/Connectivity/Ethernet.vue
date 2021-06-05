<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Ethernet settings</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save changes"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Config.Enabled"></toggle>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 5px; margin-bottom: 5px;"/></div>
      <div class="col-md-12">
        <label class="control-label">IP Mode</label>
        <option-picker :options="[{name: 'DHCP', value: 'DHCP'}, {name: 'STATIC', value: 'STATIC'}]" v-model="Config.IPMode"></option-picker>
      </div>
      <div class="col-md-12" v-if="Config.IPMode=='STATIC'">
        <label class="control-label">IP address</label>
        <input-text :disabled="Config.IPMode=='DHCP'" v-model="Config.IP"></input-text>
      </div>
      <div class="col-md-6" v-if="Config.IPMode=='STATIC'">
        <label class="control-label">Network mask</label>
        <input-text v-model="Config.Netmask"></input-text>
      </div>
      <div class="col-md-6" v-if="Config.IPMode=='STATIC'">
        <label class="control-label">Gateway address</label>
        <input-text v-model="Config.Gateway"></input-text>
      </div>
      <div class="col-md-6" v-if="Config.IPMode=='STATIC'">
        <label class="control-label">Primary DNS address</label>
        <input-text v-model="Config.DNS[0]"></input-text>
      </div>
      <div class="col-md-6" v-if="Config.IPMode=='STATIC'">
        <label class="control-label">Secondary DNS address</label>
        <input-text v-model="Config.DNS[1]"></input-text>
      </div>
      <div class="col-xs-12">
        <label class="control-label">Details</label>
        <textarea rows="4" class="form-control" disabled :value="Details"></textarea>
      </div>
    </div>
  </dlg>
</template>

<script>
  export default {
    props: { settings: Object },
    data() {
      return { Timer: null, ShowSpinner: false, Config: null, Details: '' };
    },
    created: function() {
      this.Config = Object.assign({}, this.settings);
      this.Timer = setTimeout(this.Refresh, 100);
    },
    beforeDestroy: function() {
      if(this.Timer != null) clearTimeout(this.Timer);
      if(this.ScanTimer != null) clearTimeout(this.ScanTimer);
      this.Timer = null;
    },
    methods: {
      Refresh: function() {
        if(this.Timer != null) clearTimeout(this.Timer);
        this.Post('getEthernet', { }).then(
          (result) => {
            if(this.Config == null) return;
            this.Details = 'Device IP address   : ' + result.Status.IP + ' (' + result.Status.IPMode + ')\nDNS servers         : ' + result.Status.DNS.join(', ') + '\nDefault gateway     : ' + result.Status.Gateway;
            this.Timer = setTimeout(this.Refresh, 5000);
          },
          (fail) => { }
        );
      },
      Back: function() {
        this.$emit('close');
      },
      Save: function() {
        this.ShowSpinner = true;
        this.Post('setEthernet', this.Config).then(
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
</style>