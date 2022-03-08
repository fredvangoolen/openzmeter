<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Automatic updates</h4></div>
      <div class="pull-right btn-group">
        <z-button @click="Save" :image="require('Imgs/diskette.png')" tooltip="Save changes and close" />
        <z-button @click="$emit('close')" :image="require('Imgs/arrow_undo.png')" tooltip="Discard" />
      </div>
    </template>
    <z-row>
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Config.Enabled"></toggle>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 5px; margin-bottom: 5px; "/></div>
    </z-row>
    <z-row>
      <div class="col-sm-12">
        <label class="control-label">Update URL</label>
          <div class="input-group input-group-xs">
          <input type="text" class="form-control" v-model="Config.URL">
          <span class="input-group-addon">/{{ Config.Arch }}/{{ Config.Branch }}</span>
        </div>
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
        this.Post('setUpdates', this.Config).then(
          (result) => { this.$emit('close'); },
          (fail) => { this.ShowSpinner = false; }
        );
      }
    },
    components: {  }
  }
</script>

<style scoped>
</style>