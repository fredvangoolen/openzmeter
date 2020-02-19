<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Current version: {{ (Version==null) ? 'Wait...' : Version }}</div>
      <div class="header-buttons">
        <div class="btn-group">
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Update" @click="Update(true)"><img :src="require('Imgs/update.png')"> Check updates</button>
        </div>
      </div>
    </div>
    <div class="panel-body" style="min-height: 100px">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px" v-if="Updates==null">
        Click 'Check  updates' button. This operation may take up to a minute. After finish all upgradable packages are shown and can be upgraded.
      </div>
      <vuetable :data="Updates" :fields="Columns" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" @click="Upgrade(props.rowData.Package)">Upgrade</button>
          </div>
        </template>
      </vuetable>
    </div>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  export default {
    data: function() {
      return { ShowSpinner: false, Version: null, Updates: null,
        Columns: [
          {name: 'Package', title: 'Package'},
          {name: 'From', title: 'From'},
          {name: 'To', title: 'To'},
          { name: '__slot:actions', title: '' }
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
        this.ShowSpinner = true;
        this.$http.post('getUpdates', { Force: force }).then(
          (result) => {
            this.Updates = result.data;
            this.ShowSpinner = false;
          },
          (fail) => { this.ShowSpinner = false; }
        );
      },
      Upgrade: function(pkg) {
        this.ShowSpinner = true;
        this.$http.post('setUpgrade', { Package: pkg }).then(
          (result) => { this.Update(false); },
          (fail) => { this.Version = 'Not avaliable'; }
        );
      }
    },
    components: { vuetable }
  }
</script>

<style scoped>
  .table >>> th:nth-child(2) {
    text-align: center;
  }
  .table >>> td:nth-child(2) {
    text-align: center;
  }
  .table >>> th:nth-child(3) {
    text-align: center;
  }
  .table >>> td:nth-child(3) {
    text-align: center;
  }
  .table >>> th:nth-child(4) {
    width: 100px;
  }
  .table >>> td:nth-child(4) {
    text-align: right;
  }
</style>