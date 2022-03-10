<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Bluetooth connection</h4></div>
      <div class="pull-right btn-group">
        <div class="pull-right btn-group">
          <z-button tooltip="Save" @click="Save" :image="require('Imgs/diskette.png')" />
          <z-button tooltip="Back" @click="$emit('close')" :image="require('Imgs/arrow_undo.png')" />
        </div>
      </div>
    </template>
    <z-row>
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Config.Enabled"></toggle>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 5px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12">
        <label class="control-label">Details</label>
        <textarea rows="3" class="form-control" disabled>
  Device name : {{ Config.Name }}
  Device IP   : 10.0.0.1
  Clients IP  : 10.0.0.2 - 10.0.0.254</textarea>
      </div>
    </z-row>
  </dlg>
</template>

<script>
  export default {
    props: { settings: Object },
    data: function() {
      return { ShowSpinner: false, Config: null };
    },
    created: function() {
      this.Config = Object.assign({}, this.settings);
    },
    methods: {
      Save: function() {
        this.ShowSpinner = true;
        this.Post('setBluetooth', { Enabled: this.Config.Enabled }).then(
          (result) => { this.$emit('close'); },
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