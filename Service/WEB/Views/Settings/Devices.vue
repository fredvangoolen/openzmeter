<template>
  <div class="panel panel-other">
    <div class="panel-heading">
      <div class="huge-small header-text">Device manager</div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label" style="line-height: 22px">Device export</label>
          <toggle class="float-right" onText="Yes" offText="No" v-model="Config.EnableClient"></toggle>
          <div class="help">Export hardware devices to remote server. Only phisical devices are exported. This allow access your devices from public nectwork without expose then.</div>
        </div>
        <div class="col-xs-12 col-xl-6">
          <label class="control-label">Remote URL</label>
          <input type="text" class="form-control" v-model="Config.Server">
        </div>
        <div class="col-xs-12 col-xl-3">
          <label class="control-label">User name</label>
          <input type="text" class="form-control" v-model="Config.User">
        </div>
        <div class="col-xs-12 col-xl-3">
          <label class="control-label">Password</label>
          <input type="password" class="form-control" v-model="Config.Pass">
        </div>
        <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
        <div class="col-xs-12">
          <label class="control-label" style="line-height: 22px">Device import</label>
          <toggle class="float-right" onText="Yes" offText="No" v-model="Config.EnableServer"></toggle>
          <div class="help">Allow other devices to connect and acts as gateway for then. Remote devices can be acceded as they are local ones.</div>
        </div>
        <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
      </div>
      <vuetable ref="table" :fields="Columns" :data="Devices" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" >
            <button class="btn btn-xs btn-default" @click="Configure(props.rowData)">Configure</button>
          </div>
        </template>
      </vuetable>
    </div>
    <dlg v-if="SettingsVisible" :loading="SettingsLoading">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Device &raquo; {{ Settings.Serial }}</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" v-on:click="Save"><img :src="require('Imgs/diskette.png')"></button>
          <button type="button" class="btn btn-default btn-xs" v-on:click="SettingsVisible = false"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
      </template>
      <component :is="Driver" v-model="Settings" :error="Error"></component>
    </dlg>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import Spinner from 'Components/Spinner.vue';
  import DeviceVirtual from './Devices/Virtual.vue';
  import DeviceV1 from './Devices/V1.vue';
  import DeviceV2 from './Devices/V2.vue';
  import DeviceRemote from './Devices/Remote.vue';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data() {
      return { Config: { Devices: [] }, ShowSpinner: true, Driver: null, SettingsVisible: false, SettingsLoading: false, Settings: null, Error: '',
        Config: {Enabled: true},
        Columns: [
          {name: 'Driver', title: 'Driver'},
          {name: 'Name', title: 'Name'},
          {name: 'Serial', title: 'Serial'},
          {name: 'Analyzers', title: 'Analyzers'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    methods: {
      LoadTable: function () {
        this.ShowSpinner = true;
        this.$http.post('getDevices', {}).then(
          (result) => {
            this.Config = {};
            this.Config.Devices = result.data;
            this.Config.EnableClient = true;
            this.Config.EnableServer = true;
            this.Config.User = 'admin';
            this.Config.Pass = 'admin';
            this.Config.Server = 'http://openzmeter.com/myozm';
//            this.Devices = result.data;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      Configure: function (id) {
        this.ShowSpinner = true;
        if(id.Driver === 'Virtual') this.Driver = 'DeviceVirtual';
        if(id.Driver === 'V1') this.Driver = 'DeviceV1';
        if(id.Driver === 'V2') this.Driver = 'DeviceV2';
        if(id.Driver === 'Remote') this.Driver = 'DeviceRemote';
        this.$http.post('getDevice', { Serial: id.Serial } ).then(
          (result) => {
            this.Settings = result.data;
            this.Settings.Serial = id.Serial;
            this.SettingsVisible = true;
            this.ShowSpinner = false;
          },
          (fail) => {
            this.ShowSpinner = false;
          }
        );
      },
      Save: function () {
        this.SettingsLoading = true;
        this.$http.post('setDevice', this.Settings).then(
          (result) => {
            this.SettingsLoading = false;
            this.SettingsVisible = false;
            this.LoadTable();
          },
          (fail) => {
            this.SettingsLoading = false;
            this.Settings.Error = 'Failed to save device settings!';
            setTimeout(() => { this.Settings.Error = null; }, 1000);
          }
        );
      }
    },
    mounted: function () {
      this.LoadTable();
    },
    updated: function () {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    components: {Vuetable, DeviceVirtual, DeviceV1, DeviceV2, DeviceRemote}
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1) {
      width: 60px;
      text-align: center;
    }
    .table >>> td:nth-child(1) {
      text-align: center;
    }
    .table >>> th:nth-child(3) {
      width: 250px;
      text-align: center;
    }
    .table >>> td:nth-child(3) {
      text-align: center;
    }
    .table >>> th:nth-child(4) {
      width: 20px;
      text-align: center;
    }
    .table >>> td:nth-child(4) {
      text-align: center;
    }
    .table >>> th:nth-child(5) {
      width: 90px;
      text-align: center;
    }
    .table >>> td:nth-child(5) {
      text-align: center;
    }
    .table >>> th:nth-child(6) {
      width: 120px;
    }
  }
  .help {
    font-size: 80%;
  }
</style>