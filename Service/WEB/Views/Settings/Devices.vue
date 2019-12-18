<template>
  <div class="panel panel-other">
    <div class="panel-heading">
      <div class="huge-small header-text">Input devices</div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" >
            <button class="btn btn-xs btn-danger" v-if="props.rowData.Status === 'RUNNING'" @click="Stop(props.rowData.Serial, $event)">Stop</button>
            <button class="btn btn-xs btn-success" v-if="props.rowData.Status !== 'RUNNING'" @click="Start(props.rowData.Serial, $event)">Start</button>
            <button class="btn btn-xs btn-default" @click="Configure(props.rowData, $event)">Configure</button>
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
      <component :is="Driver" v-model="Settings"></component>
    </dlg>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import Spinner from 'Components/Spinner.vue';
  import DeviceVirtual from './DeviceVirtual.vue';
  import DeviceV1 from './DeviceV1.vue';
  import DeviceV2 from './DeviceV2.vue';
  import DeviceRemote from './DeviceRemote.vue';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data() {
      return {Devices: [], ShowSpinner: true, Driver: null, SettingsVisible: false, SettingsLoading: false, Settings: null,
        Columns: [
          {name: 'Driver', title: 'Driver'},
          {name: 'Name', title: 'Name'},
          {name: 'Serial', title: 'Serial'},
          {name: 'Analyzers', title: 'Analyzers'},
          {name: 'Status', title: 'Status'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    methods: {
      LoadTable: function () {
        this.ShowSpinner = true;
        $.ajax({url: "getDevices", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({}),
          success: function (result) {
            var TableData = {data: result};
            this.$refs.table.setData(TableData);
            this.ShowSpinner = false;
          }.bind(this)
        });
      },
      Stop: function (id, evt) {
        $(evt.target).blur();
        this.ShowSpinner = true;
        $.ajax({url: "stopDevice", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({Serial: id}),
          success: (result) => {
            this.LoadTable();
            this.$root.$children[0].$refs.sidebar.$refs.analyzer.CheckAnalyzer();
          },
          complete: () => {
            this.ShowSpinner = false;
          }
        });
      },
      Start: function (id, evt) {
        $(evt.target).blur();
        this.ShowSpinner = true;
        $.ajax({url: "startDevice", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({Serial: id}),
          success: (result) => {
            this.LoadTable();
            this.$root.$children[0].$refs.sidebar.$refs.analyzer.CheckAnalyzer();
          },
          complete: () => {
            this.ShowSpinner = false;
          }
        });
      },
      Configure: function (id, evt) {
        $(evt.target).blur();
        this.ShowSpinner = true;
        if(id.Driver === 'Virtual') this.Driver = 'DeviceVirtual';
        if(id.Driver === 'V1') this.Driver = 'DeviceV1';
        if(id.Driver === 'V2.01') this.Driver = 'DeviceV2';
        if(id.Driver === 'Remote') this.Driver = 'DeviceRemote';
        $.ajax({url: "getDevice", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({Serial: id.Serial}),
          success: (result) => {
            this.Settings = result;
            this.Settings.Serial = id.Serial;
            this.SettingsVisible = true;
          },
          complete: () => {
            this.ShowSpinner = false;
          }
        });
      },
      Save: function () {
        this.SettingsLoading = true;
        $.ajax({url: "setDevice", timeout: 20000, type: "POST", contentType: 'application/json', data: JSON.stringify(this.Settings),
          success: (result) => {
            this.SettingsLoading = false;
            this.SettingsVisible = false;
            this.LoadTable();
          },
          error: () => {
            this.Settings.Error = 'Failed to save device settings!';
            setTimeout(() => { this.Settings.Error = null; }, 1000);
          }
        });
      }
    },
    mounted: function () {
      this.LoadTable();
    },
    updated: function () {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    components: {Vuetable, Spinner, DeviceVirtual, DeviceV1, DeviceV2, DeviceRemote}
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
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
  .checkbox {
    margin-top: 1px;
    margin-bottom: 0px;
    margin-right: 4px;
  }
  .modal-body {
    min-height: 100px;
  }
</style>