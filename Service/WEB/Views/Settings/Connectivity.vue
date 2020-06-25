<template>
  <div class="panel panel-other" v-show="Rows.length>0">
    <div class="panel-heading">
      <div class="huge-small header-text">Connectivity</div>
    </div>
    <div class="panel-body">
      <vuetable ref="table" :fields="Columns" :data="Rows" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" >
            <button class="btn btn-xs btn-default" @click="props.rowData.Method(true)">Configure</button>
          </div>
        </template>
      </vuetable>
      <bluetooth :settings.sync="Modules.Bluetooth.Settings" v-if="View=='Bluetooth'" @close="View=null"></bluetooth>
      <wifi :settings.sync="Modules.Wifi.Settings" v-if="View=='Wifi'" @close="View=null"></wifi>
      <Ethernet :settings.sync="Modules.Ethernet.Settings" v-if="View=='Ethernet'" @close="View=null"></Ethernet>
      <ntp :settings.sync="Modules.NTP.Settings" v-if="View=='NTP'" @close="View=null"></ntp>
      <telegram :settings.sync="Modules.Telegram.Settings" v-if="View=='Telegram'" @close="View=null"></telegram>
      <modbus-tcp :settings.sync="Modules.ModbusTCP.Settings" v-if="View=='ModbusTCP'" @close="View=null"></modbus-tcp>
    </div>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import Bluetooth from './Connectivity/Bluetooth.vue';
  import Wifi from './Connectivity/WIFI.vue';
  import Ethernet from './Connectivity/Ethernet.vue';
  import Ntp from './Connectivity/NTP.vue';
  import ModbusTcp from './Connectivity/ModbusTCP.vue';
  import Telegram from './Connectivity/Telegram.vue';
  export default {
    data: function() {
      return { View: null,
               Modules: {
                 'Bluetooth': { Module: 'Bluetooth connection', Settings: null, Method: this.LoadBluetooth },
                 'Wifi': { Module: 'Wifi connection', Settings: null, Method: this.LoadWIFI },
                 'Ethernet': { Module: 'Ethernet', Settings: null, Method: this.LoadEthernet },
                 'NTP': { Module: 'NTP synchronization', Settings: null, Method: this.LoadNTP },
                 'Telegram': { Module: 'Telegram Bot', Settings: null, Method: this.LoadTelegram },
                 'ModbusTCP': { Module: 'ModbusTCP', Settings: null, Method: this.LoadModbusTCP }
               },
               Columns: [
                 { name: 'Module', title: 'Module' },
                 { name: 'Settings.Enabled', title: 'Enabled', callback: this.EnabledStatus},
                 { name: '__slot:actions', title: '' }
               ],
             };
    },
    computed: {
      Rows: function() {
        var Rows = [];
        for(var n in this.Modules) {
          if(this.Modules[n].Settings != null) Rows.push(Object.assign({ Name: n }, this.Modules[n]));
        }
        return Rows;
      }
    },
    created: function() {
      this.LoadBluetooth(false);
      this.LoadWIFI(false);
      this.LoadEthernet(false);
      this.LoadNTP(false);
      this.LoadTelegram(false);
      this.LoadModbusTCP(false);
    },
    methods: {
      EnabledStatus: function(value) { return (value === true) ? '&#9635;' : '&#9633;'; },
      LoadBluetooth: function(show) {
        this.$http.post('getBluetooth', { }).then(
          (result) => {
            this.Modules.Bluetooth.Settings = result.body;
            if(show) this.View = 'Bluetooth';
          },
          (fail) => { }
        );
      },
      LoadWIFI: function(show) {
        this.$http.post('getWIFI', { Type: "CURRENT" }).then(
          (result) => {
            this.Modules.Wifi.Settings = result.body;
            if(show) this.View = 'Wifi';
          },
          (fail) => { }
        );
      },
      LoadEthernet: function(show) {
        this.$http.post('getEthernet', { Type: "CURRENT" }).then(
          (result) => {
            this.Modules.Ethernet.Settings = result.body;
            if(show) this.View = 'Ethernet';
          },
          (fail) => { }
        );
      },
      LoadNTP: function(show) {
        this.$http.post('getNTP', { }).then(
          (result) => {
            this.Modules.NTP.Settings = result.body;
            if(show) this.View = 'NTP';
          },
          (fail) => { }
        );
      },
      LoadTelegram: function(show) {
        this.$http.post('getTelegram', { }).then(
          (result) => {
            this.Modules.Telegram.Settings = result.body;
            if(show) this.View = 'Telegram';
          },
          (fail) => { }
        );
      },
      LoadModbusTCP: function(show) {
        this.$http.post('getModbusTCP', { }).then(
          (result) => {
            this.Modules.ModbusTCP.Settings = result.body;
            if(show) this.View = 'ModbusTCP';
          },
          (fail) => { }
        );
      },
    },
    components: { Bluetooth, Wifi, Ethernet, Ntp, ModbusTcp, Telegram, Vuetable }
  }
</script>

<style scoped>
  textarea {
    resize: none;
    font-family: monospace;
    font-size: 12px;
  }
  .table >>> th:nth-child(2) {
    width: 60px;
    text-align: center;
  }
  .table >>> td:nth-child(2) {
    text-align: center;
  }
  .table >>> th:nth-child(3) {
    width: 80px;
    text-align: center;
  }
  .table >>> td:nth-child(3) {
    text-align: center;
  }
</style>