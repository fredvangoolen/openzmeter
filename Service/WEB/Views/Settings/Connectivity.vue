<template>
  <div class="panel panel-other" v-show="Enabled">
    <div class="panel-heading">
      <div class="huge-small header-text">Connectivity</div>
    </div>
    <div class="panel-body">
      <spinner v-if="Loading"></spinner>
      <bluetooth class="row" @enabled="Enabled=true"></bluetooth>
      <wifi class="row" @enabled="Enabled=true"></wifi>
      <ntp class="row" @enabled="Enabled=true"></ntp>
    </div>
  </div>
</template>

<script>
  import Bluetooth from './Connectivity/Bluetooth.vue';
  import Wifi from './Connectivity/WIFI.vue';
  import Lan from './Connectivity/LAN.vue';
  import Ntp from './Connectivity/NTP.vue';
  import Modbus from './Connectivity/MODBus.vue';
  import Telegram from './Connectivity/Telegram.vue';


  import Toggle from 'Components/Toggle.vue';
  import Spinner from 'Components/Spinner.vue';

  export default {
    data: function() {
      return { Enabled: false,



                Modbus_Support: false, ModbusChange: false, ModbusSettings: {},
                Telegram_Support: false, TelegramChange: false, TelegramSettings: {},
                Loading: false, NTPEnabled: false,
                LAN: null };
    },
    components: { Bluetooth, Wifi, Lan, Ntp, Modbus, Telegram }
  }
</script>

<style scoped>
  textarea {
    resize: none;
    font-family: monospace;
    font-size: 12px;
  }
</style>
