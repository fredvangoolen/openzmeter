<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Telegram Bot</h4></div>
      <div class="pull-right btn-group">
        <div class="pull-right btn-group">
          <z-button tooltip="Save and close" @click="Save" :image="require('Imgs/diskette.png')" />
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
        <label class="control-label">Token</label>
        <input type="text" class="form-control" v-model="Config.Token">
      </div>
      <div class="col-xs-12">
        <label class="control-label">Status</label>
        <textarea rows="3" class="form-control" disabled :value="Details"></textarea>
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
    computed: {
      Details: function() {
        if(this.Config.Status.ID == null) return 'Status : Disconected';
        return 'Status   : Connected\nUserName : ' + this.Config.Status.UserName + '(' + this.Config.Status.ID + ')\nName     : ' + this.Config.Status.FirstName + ' ' + this.Config.Status.LastName;
      }
    },
    methods: {
      Save: function() {
        this.ShowSpinner = true;
        this.Post('setTelegram', this.Config).then(
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
    font-family: 'Fira Code';
    font-size: 12px;
  }
</style>