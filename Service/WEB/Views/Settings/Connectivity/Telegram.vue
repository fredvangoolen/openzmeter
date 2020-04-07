<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Telegram Bot</h4></div>
      <div class="pull-right btn-group">
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Save" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Back" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
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
        <label class="control-label">Token</label>
        <input type="text" class="form-control" v-model="Config.Token">
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
    methods: {
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post('setTelegram', this.Config).then(
          (result) => {
            this.$emit('update:settings', this.Config);
            this.$emit('close');
          },
          (fail) => {
            this.ShowSpinner = false;
          }
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