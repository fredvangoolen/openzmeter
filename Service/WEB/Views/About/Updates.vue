<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Current version: {{ (Version==null) ? 'Wait...' : Version }}</div>
      <div class="header-buttons">
        <div class="btn-group">
          <button type="button" class="btn btn-success btn-xs" @click="Upgrade">Upgrade selected</button>
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Update" @click="Update(true)"><img :src="require('Imgs/update.png')"> Check updates</button>
        </div>
      </div>
    </div>
    <div class="panel-body" style="min-height: 100px">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px" v-if="Updates==null">
        Click 'Check  updates' button. This operation may take up to a minute. After finish all upgradable packages are shown and can be upgraded.
      </div>
      <vuetable ref="table" :data="Updates" :fields="Columns" :api-mode="false" class="table small table-striped"></vuetable>
    </div>
      <dlg :loading="false" v-if="ShowDialog">
        <template v-slot:header>
          <div class="pull-left"><h4 class="modal-title">Upgrading packages...</h4></div>
          <div class="pull-right btn-group">
            <button type="button" class="btn btn-default btn-xs" v-if="Done==true" @click="Update(false)"><img :src="require('Imgs/arrow_undo.png')" alt="Close"></button>
          </div>
        </template>
        <div class="alert alert-success text-center" style="margin-bottom: 5px; padding: 5px" v-if="Status==true">All selected packages upgraded correctly</div>
        <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Status==false">Upgrade failed, see log messages</div>
        <div class="row">
          <div class="col-xs-12">
            <textarea ref="log" rows="20" class="form-control" readonly="true" v-model="Log"></textarea>
          </div>
        </div>
      </dlg>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  import { SSE } from 'sse.js';
  export default {
    data: function() {
      return { ShowDialog: false, ShowSpinner: false, Version: null, Updates: null, Done: false, Status: null, Log: '',
        Columns: [
          { name: '__checkbox' },
          { name: 'Package', title: 'Package' },
          { name: 'From', title: 'From' },
          { name: 'To', title: 'To' }
        ]
      };
    },
    created: function() {
      this.CheckVersion();
    },
    methods: {
      CheckVersion: function() {
        this.$http.post('getVersion', {}).then(
          (result) => { this.Version = result.data.Version; },
          (fail) => { this.Version = 'Not avaliable'; }
        );
      },
      Update: function(force) {
        this.ShowDialog = false;
        this.ShowSpinner = true;
        this.$http.post('getUpdates', { Force: force }).then(
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
    components: { vuetable }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
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
</style>