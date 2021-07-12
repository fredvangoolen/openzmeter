<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Automatic updates</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save changes and close"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Config.Enabled"></toggle>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 5px; margin-bottom: 5px; "/></div>
    </div>
    <div class="row">
      <div class="col-sm-12">
        <label class="control-label">Update URL</label>
        <input type="text" class="form-control" v-model="Config.URL">
      </div>
      <div class="col-xs-12">
        <label class="control-label">Details</label>
        <textarea rows="4" class="form-control" disabled :value="Details"></textarea>
      </div>
      <div class="col-xs-12">
        <label class="control-label">System log messages</label>
        <textarea rows="10" class="form-control" disabled :value="SysMessages"></textarea>
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
      Back: function() {

      },
      Save: function() {

      },
      CheckVersion: function() {
        this.Post('getVersion', {}).then(
          (result) => { /*this.Version = result.data.Version; */ },
          (fail) => { this.Version = 'Not avaliable'; }
        );
      },
      Update: function(force) {
        this.ShowDialog = false;
        this.ShowSpinner = true;
        this.Post('getUpdates', { Force: force }).then(
          (result) => {
            for(var n = 0; n < result.data.length; n++) result.data[n].id = result.data[n].Package;
            this.Updates = result.data;
            this.ShowSpinner = false;
          },
          (fail) => { this.ShowSpinner = false; }
        );
      },
      Upgrade: function() {
        if(this.$refs.table.selectedTo.length == 0) return;
        this.ShowSpinner = true;
        this.ShowDialog  = true;
        this.Status      = null;
        this.Log         = '';
        var Source       = new SSE('setUpgrade', {headers: {'Content-Type': 'application/json'}, payload: JSON.stringify({ Packages: this.$refs.table.selectedTo })});
        Source.addEventListener('status', (e) => {
          this.Log += e.data + '\n';
          this.$refs.log.scrollTop = this.$refs.log.scrollHeight;
          this.Status = (e.data.startsWith("OK") == true);
          this.ShowSpinner = false;
          this.Done = true;
        });
        Source.addEventListener('text', (e) => { this.Log += e.data + '\n'; this.$refs.log.scrollTop = this.$refs.log.scrollHeight; });
        Source.stream();
      }
    },
    components: {  }
  }
</script>

<style scoped>
  textarea {
    resize: none;
    font-family: 'Fira Code';
    font-size: 10px;
    white-space: pre;
  }



  .table >>> th:nth-child(3) {
    text-align: center;
  }
  .table >>> td:nth-child(3) {
    text-align: center;
  }
  .table >>> th:nth-child(4) {
    text-align: center;
  }
  .table >>> td:nth-child(4) {
    text-align: center;
  }
  textarea {
    resize: none;
    font-family: 'Fira Code';
    font-size: 11px;
    background-color: #111 !important;
    color: #ddd;
  }
  .panel-other {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
</style>