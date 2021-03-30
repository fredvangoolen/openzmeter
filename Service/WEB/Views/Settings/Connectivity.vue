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
      <bluetooth :settings.sync="Modules.Bluetooth.Settings" v-if="View=='Bluetooth'" @close="Update"></bluetooth>
      <wifi :settings.sync="Modules.Wifi.Settings" v-if="View=='Wifi'" @close="Update"></wifi>
      <ethernet :settings.sync="Modules.Ethernet.Settings" v-if="View=='Ethernet'" @close="Update"></ethernet>
      <ntp :settings.sync="Modules.NTP.Settings" v-if="View=='NTP'" @close="Update"></ntp>
      <telegram :settings.sync="Modules.Telegram.Settings" v-if="View=='Telegram'" @close="Update"></telegram>
      <modbus-tcp :settings.sync="Modules.ModbusTCP.Settings" v-if="View=='ModbusTCP'" @close="Update"></modbus-tcp>
      <vpn :settings.sync="Modules.OpenVPN.Settings" v-if="View=='OpenVPN'" @close="Update"></vpn>
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
  import Vpn from './Connectivity/OpenVPN.vue';
  export default {
    data: function() {
      return { View: null,
               Modules: {
                 'Bluetooth': { Module: 'Bluetooth connection', Settings: null, Method: this.LoadBluetooth },
                 'Wifi': { Module: 'Wifi connection', Settings: null, Method: this.LoadWIFI },
                 'Ethernet': { Module: 'Ethernet', Settings: null, Method: this.LoadEthernet },
                 'NTP': { Module: 'NTP synchronization', Settings: null, Method: this.LoadNTP },
                 'Telegram': { Module: 'Telegram Bot', Settings: null, Method: this.LoadTelegram },
                 'ModbusTCP': { Module: 'ModbusTCP', Settings: null, Method: this.LoadModbusTCP },
                 'OpenVPN': { Module: 'OpenVPN', Settings: null, Method: this.LoadOpenVPN }
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
      this.Update();
    },
    methods: {
      Update: function() {
        this.View = null;
        this.LoadBluetooth(false);
        this.LoadWIFI(false);
        this.LoadEthernet(false);
        this.LoadNTP(false);
        this.LoadTelegram(false);
        this.LoadModbusTCP(false);
        this.LoadOpenVPN(false);
      },
      EnabledStatus: function(value) {
        return (value === true) ? '&#9635;' : '&#9633;';
      },
      LoadOpenVPN: function(show) {
        this.Post('getVPN', { Full: show }).then(
          (result) => {
            this.Modules.OpenVPN.Settings = result.body;
            if(show) this.View = 'OpenVPN';
          },
          (fail) => { }
        );
      },
      LoadBluetooth: function(show) {
        this.Post('getBluetooth', { }).then(
          (result) => {
            this.Modules.Bluetooth.Settings = result;
            if(show) this.View = 'Bluetooth';
          },
          (fail) => { }
        );
      },
      LoadWIFI: function(show) {
        this.Post('getWIFI', { Full: show }).then(
          (result) => {
            this.Modules.Wifi.Settings = result;
            if(show) this.View = 'Wifi';
          },
          (fail) => { }
        );
      },
      LoadEthernet: function(show) {
        this.Post('getEthernet', {  }).then(
          (result) => {
            this.Modules.Ethernet.Settings = result;
            if(show) this.View = 'Ethernet';
          },
          (fail) => { }
        );
      },
      LoadNTP: function(show) {
        this.Post('getNTP', { }).then(
          (result) => {
            this.Modules.NTP.Settings = result;
            if(show) this.View = 'NTP';
          },
          (fail) => { }
        );
      },
      LoadTelegram: function(show) {
        this.Post('getTelegram', { }).then(
          (result) => {
            this.Modules.Telegram.Settings = result;
            if(show) this.View = 'Telegram';
          },
          (fail) => { }
        );
      },
      LoadModbusTCP: function(show) {
        this.Post('getModbusTCP', { }).then(
          (result) => {
            this.Modules.ModbusTCP.Settings = result;
            if(show) this.View = 'ModbusTCP';
          },
          (fail) => { }
        );
      },
    },
    components: { Bluetooth, Wifi, Ethernet, Ntp, ModbusTcp, Telegram, Vuetable, Vpn }
  }
</script>

<style scoped>
  textarea {
    resize: none;
    font-family: 'Fira Code';
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