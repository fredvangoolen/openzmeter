<template>
  <q-card>
    <q-toolbar dense class="bg-grey-9 text-white q-py-none">
      <q-toolbar-title>Connectivity</q-toolbar-title>
    </q-toolbar>
    <q-table :hide-bottom="true" dense :columns="Columns" row-key="module" :rows="Modules" class="q-ma-xs">
      <template v-slot:body-cell-enabled="props">
        <q-td :props="props">
          <q-checkbox dense disable v-model="props.row.Enabled"></q-checkbox>
        </q-td>
      </template>
      <template v-slot:body-cell-actions="props">
        <q-td :props="props">
          <q-btn size="sm" unelevated color="primary" :disable="!props.row.Avaliable" @click="Configure(props.row)" label="Configure"></q-btn>
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
    <q-inner-loading :showing="ShowSpinner"><q-spinner-dots size="50px" color="orange-10" /></q-inner-loading>
  </q-card>
</template>

<script>
  let Bluetooth = () => import('./Connectivity/Bluetooth.vue');
  let Wifi = () => import('./Connectivity/WIFI.vue');
  let Ethernet = () => import('./Connectivity/Ethernet.vue');
  let Ntp = () => import('./Connectivity/NTP.vue');
  let ModbusTcp = () => import('./Connectivity/ModbusTCP.vue');
  let Telegram = () => import('./Connectivity/Telegram.vue');
  let Vpn = () => import('./Connectivity/OpenVPN.vue');
  let Updates = () => import('./Connectivity/Updates.vue');
  export default {
    data: function() {
      return { ShowSpinner: false, View: null, Settings: null,
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
                 { name: 'module', label: 'Module', field: row => row.Title, align: 'left'},
                 { name: 'enabled', label: 'Enabled', align: 'center', style: 'width: 60px' },
                 { name: 'actions', label: '', style: 'width: 80px'  }
               ],
             };
    },
    created: function() {
      this.$emit('queries', this.Modules.length);
      this.Update();
    },
    methods: {
      Update: function() {
        this.View = null;
        this.ShowSpinner = true;
        this.Components = this.Modules.length;
        for(var n in this.Modules) this.LoadModule(this.Modules[n], false);
      },
      LoadModule: function(module, full) {
        this.Post(module.Method, { Full: full }).then(
          (result) => {
            module.Enabled = result.Enabled;
            module.Avaliable = true;
            this.Components--;
            if(this.Components == 0) this.ShowSpinner = false;
            this.$emit('done');
            if(full) {
              this.Settings = result;
              this.View = module.Name;
            }
          },
          () => {
            module.Avaliable = false;
            this.Components--;
            if(this.Components == 0) this.ShowSpinner = false;
            this.$emit('done');
          }
        );
      }
    },
    components: { Bluetooth, Wifi, Ethernet, Ntp, ModbusTcp, Telegram, Vpn, Updates }
  }
</script>