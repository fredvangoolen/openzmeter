<template>
  <z-panel title="Connectivity" color="#474544">
    <vuetable ref="table" :fields="Columns" :data="Modules" :api-mode="false" class="table small table-striped">
      <template slot="actions" slot-scope="props">
        <z-button :disabled="!props.rowData.Avaliable" @click="LoadModule(props.rowData, true)" caption="Configure" />
      </template>
    </vuetable>
    <bluetooth :settings="Settings" v-if="View=='Bluetooth'" @close="Update"></bluetooth>
    <wifi :settings="Settings" v-if="View=='Wifi'" @close="Update"></wifi>
    <ethernet :settings="Settings" v-if="View=='Ethernet'" @close="Update"></ethernet>
    <ntp :settings="Settings" v-if="View=='NTP'" @close="Update"></ntp>
    <telegram :settings="Settings" v-if="View=='Telegram'" @close="Update"></telegram>
    <modbus-tcp :settings="Settings" v-if="View=='ModbusTCP'" @close="Update"></modbus-tcp>
    <vpn :settings="Settings" v-if="View=='OpenVPN'" @close="Update"></vpn>
    <updates :settings="Settings" v-if="View=='Updates'" @close="Update"></updates>
  </z-panel>
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
                 { Name: 'Bluetooth', Title: 'Bluetooth connection', Enabled: false, Avaliable: false, Method: 'getBluetooth' },
                 { Name: 'Wifi', Title: 'Wifi connection', Enabled: false, Avaliable: false, Method: 'getWIFI' },
                 { Name: 'Ethernet', Title: 'Ethernet', Enabled: false, Avaliable: false, Method: 'getEthernet' },
                 { Name: 'NTP', Title: 'NTP synchronization', Enabled: false, Avaliable: false, Method: 'getNTP' },
                 { Name: 'Telegram', Title: 'Telegram Bot', Enabled: false, Avaliable: false, Method: 'getTelegram' },
                 { Name: 'ModbusTCP', Title: 'ModbusTCP', Enabled: false, Avaliable: false, Method: 'getModbusTCP' },
                 { Name: 'OpenVPN', Title: 'OpenVPN', Enabled: false, Avaliable: false, Method: 'getVPN' },
                 { Name: 'Updates', Title: 'Automatic updates', Enabled: false, Avaliable: false, Method: 'getUpdates' }
               ],
               Columns: [
                 { name: 'Title', title: 'Module' },
                 { name: 'Enabled', title: 'Enabled', callback: this.EnabledStatus },
                 { name: '__slot:actions', title: '' }
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