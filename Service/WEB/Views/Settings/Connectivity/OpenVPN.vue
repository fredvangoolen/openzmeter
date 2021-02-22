<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">OpenVPN</h4></div>
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
      </div>
      <div class="col-xs-12"><hr style="margin-top: 5px; margin-bottom: 5px;"/></div>
      <div class="col-xs-12">
        <label class="control-label">Config file (OpenVPN with embedded certificates)</label>
        <textarea rows="20" class="form-control" v-model="Config.Config"></textarea>
      </div>
      <div class="col-xs-12">
        <label class="control-label">Details</label>
        <textarea rows="2" class="form-control" disabled :value="Config.Status"></textarea>
      </div>
    </div>
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
    beforeDestroy: function() {
      if(this.Timer != null) clearTimeout(this.Timer);
      this.Timer = null;
    },
    methods: {
      Save: function(value) {
        this.ShowSpinner = true;
        this.$http.post('setVPN', { Enabled: this.Config.Enabled, Config: this.Config.Config }).then(
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