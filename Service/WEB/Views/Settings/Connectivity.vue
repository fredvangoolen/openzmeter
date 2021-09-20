<template>
  <q-card>
    <q-toolbar dense class="bg-grey-9 text-white q-py-none">
      <q-toolbar-title>Connectivity</q-toolbar-title>
    </q-toolbar>
    <q-table :loading="ShowSpinner" :hide-bottom="true" dense :columns="Columns" row-key="module" :data="Modules" class="q-ma-xs">
      <template v-slot:body-cell-enabled="props">
        <q-td :props="props">
          <q-checkbox dense disable v-model="props.row.Enabled"></q-checkbox>
        </q-td>
      </template>
      <template v-slot:body-cell-actions="props">
        <q-td :props="props">
          <q-btn size="sm" :disable="!props.row.Avaliable" @click="Configure(props.row)">Configure</q-btn>
        </q-td>
      </template>
    </q-table>
    <bluetooth :settings="Settings" v-if="View=='Bluetooth'" @close="Update"></bluetooth>
    <wifi :settings="Settings" v-if="View=='Wifi'" @close="Update"></wifi>
    <ethernet :settings="Settings" v-if="View=='Ethernet'" @close="Update"></ethernet>
    <ntp :settings="Settings" v-if="View=='NTP'" @close="Update"></ntp>
    <telegram :settings="Settings" v-if="View=='Telegram'" @close="Update"></telegram>
    <modbus-tcp :settings="Settings" v-if="View=='ModbusTCP'" @close="Update"></modbus-tcp>
    <vpn :settings="Settings" v-if="View=='OpenVPN'" @close="Update"></vpn>
    <updates :settings="Settings" v-if="View=='Updates'" @close="Update"></updates>
  </q-card>
</template>

<script>
  import Bluetooth from './Connectivity/Bluetooth.vue';
  import Wifi from './Connectivity/WIFI.vue';
  import Ethernet from './Connectivity/Ethernet.vue';
  import Ntp from './Connectivity/NTP.vue';
  import ModbusTcp from './Connectivity/ModbusTCP.vue';
  import Telegram from './Connectivity/Telegram.vue';
  import Vpn from './Connectivity/OpenVPN.vue';
  import Updates from './Connectivity/Updates.vue'
  export default {
    data: function() {
      return { View: null, Settings: null,
               Modules: [
                 { name: 'Bluetooth', Title: 'Bluetooth connection', Enabled: false, Avaliable: false, Method: 'getBluetooth' },
                 { name: 'Wifi', Title: 'Wifi connection', Enabled: false, Avaliable: false, Method: 'getWIFI' },
                 { name: 'Ethernet', Title: 'Ethernet', Enabled: false, Avaliable: false, Method: 'getEthernet' },
                 { name: 'NTP', Title: 'NTP synchronization', Enabled: false, Avaliable: false, Method: 'getNTP' },
                 { name: 'Telegram', Title: 'Telegram Bot', Enabled: false, Avaliable: false, Method: 'getTelegram' },
                 { name: 'ModbusTCP', Title: 'ModbusTCP', Enabled: false, Avaliable: false, Method: 'getModbusTCP' },
                 { name: 'OpenVPN', Title: 'OpenVPN', Enabled: false, Avaliable: false, Method: 'getVPN' },
                 { name: 'Updates', Title: 'Automatic updates', Enabled: false, Avaliable: false, Method: 'getUpdates' }
               ],
               Columns: [
                 { name: 'module', label: 'Module', field: row => row.Title, align: 'left' },
                 { name: 'enabled', label: 'Enabled', align: 'center' },
                 { name: 'actions', label: '' }
               ],
             };
    },
    created: function() {
      this.Update();
    },
    methods: {
      Update: function() {
        this.View = null;
        for(var n in this.Modules) this.LoadModule(this.Modules[n], false);
      },
      EnabledStatus: function(value) {
        return (value === true) ? '&#9635;' : '&#9633;';
      },
      LoadModule: function(module, full) {
        this.Post(module.Method, { Full: full }).then(
          (result) => {
            module.Enabled = result.Enabled;
            module.Avaliable = true;
            if(full) {
              this.Settings = result;
              this.View = module.Name;
            }
          },
          (fail) => { module.Avaliable = false; }
        );
      }
    },
    components: { Bluetooth, Wifi, Ethernet, Ntp, ModbusTcp, Telegram, Vpn, Updates }
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