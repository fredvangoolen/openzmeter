<template>
  <div class="row" v-if="Config!=null">
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Bluetooth connection</label>
      <div class="input-group">
        <span class="input-group-addon input-group-addon-toogle">
          <toggle v-model="Config.Enabled" v-on:change="Changes=true"></toggle>
        </span>
        <input type="text" class="form-control" disabled v-model="Config.Name">
        <span class="input-group-btn" v-if="Changes">
          <button class="btn btn-default" v-on:click="Save" data-toggle="tooltip" title="Save changes"><img :src="require('Imgs/diskette.png')"></button>
        </span>
      </div>
    </div>
    <div class="col-xs-12 col-sm-6">
      <label class="control-label">Details</label>
      <textarea rows="2" class="form-control" disabled>
Device IP  : 10.0.0.1
Clients IP : 10.0.0.2 - 10.0.0.254</textarea>
    </div>
  </div>
</template>

<script>
  export default {
    data: function() {
      return { Config: null, Changes: false };
    },
    created: function() {
      this.$http.post('getBluetooth', { }).then(
        (result) => {
          this.Config = result.data;
          this.$emit('enabled');
        },
        (fail) => {}
      );
    },
    methods: {
      Save: function() {
        $.ajax({ url: "setBluetooth", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Enabled: this.Bluetooth.Enabled }) });
        this.BTChanges = false;
      },

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