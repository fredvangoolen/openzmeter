<template>
  <div class="panel panel-other" v-show="Rows.length>0">
    <div class="panel-heading">
      <div class="huge-small header-text">Connectivity</div>
    </div>
    <div class="panel-body">
      <vuetable ref="table" :fields="Columns" :data="Rows" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" >
            <button class="btn btn-xs btn-default" @click="View=props.rowData.Name">Configure</button>
          </div>
        </template>
      </vuetable>
      <bluetooth :settings.sync="Modules.Bluetooth.Settings" v-if="View=='Bluetooth'" @close="View=null"></bluetooth>
      <wifi :settings.sync="Modules.Wifi.Settings" v-if="View=='Wifi'" @close="View=null"></wifi>
      <ntp :settings.sync="Modules.NTP.Settings" v-if="View=='NTP'" @close="View=null"></ntp>
      <telegram :settings.sync="Modules.Telegram.Settings" v-if="View=='Telegram'" @close="View=null"></telegram>
    </div>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import Bluetooth from './Connectivity/Bluetooth.vue';
  import Wifi from './Connectivity/WIFI.vue';
  import Lan from './Connectivity/LAN.vue';
  import Ntp from './Connectivity/NTP.vue';
  import Modbus from './Connectivity/MODBus.vue';
  import Telegram from './Connectivity/Telegram.vue';
  export default {
    data: function() {
      return { View: null,
               Modules: {
                 'Bluetooth': { Module: 'Bluetooth connection', Settings: null },
                 'Wifi': { Module: 'Wifi connection', Settings: null },
                 'NTP': { Module: 'NTP synchronization', Settings: null },
                 'Telegram': { Module: 'Telegram Bot', Settings: null }
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
      this.$http.post('getBluetooth', { }).then(
        (result) => { this.Modules.Bluetooth.Settings = result.body; },
        (fail) => { }
      );
      this.$http.post('getWIFI', { Type: "CURRENT" }).then(
        (result) => { this.Modules.Wifi.Settings = result.body; },
        (fail) => { }
      );
      this.$http.post('getNTP', { }).then(
        (result) => { this.Modules.NTP.Settings = result.body; },
        (fail) => { }
      );
      this.Modules.Telegram.Settings = { Enabled: true, Token: 'dsasdasdfsdfsdfsdfsdfsdf'};
    },
    methods: {
      EnabledStatus: function(value) { return (value === true) ? '&#9635;' : '&#9633;'; },
    },
    components: { Bluetooth, Wifi, Lan, Ntp, Modbus, Telegram, Vuetable }
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
