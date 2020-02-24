<template>
  <div class="panel panel-other" v-show="Enabled">
    <div class="panel-heading">
      <div class="huge-small header-text">Connectivity</div>
    </div>
    <div class="panel-body">
      <spinner v-if="Loading"></spinner>
<<<<<<< HEAD
      <div class="row" v-if="Bluetooth != null">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Bluetooth connection</label>
          <div class="input-group">
            <span class="input-group-addon input-group-addon-toogle">
              <toggle v-model="Bluetooth.Enabled" v-on:change="BTChanges = true"></toggle>
            </span>
            <input type="text" class="form-control" disabled v-model="Bluetooth.Name">
            <span class="input-group-btn" v-if="BTChanges">
              <button class="btn btn-default" v-on:click="SaveChangesBT" data-toggle="tooltip" title="Save changes"><img :src="require('Imgs/diskette.png')"></button>
            </span>
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Details</label>
          <textarea rows="2" class="form-control" disabled>
Device IP  : 10.0.0.1 
Clients IP : 10.0.0.2 - 10.0.0.254</textarea>
        </div>
      </div>
      <div class="row" v-if="WIFI != null">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">WIFI connection</label>
          <div class="input-group">
            <span class="input-group-addon input-group-addon-toogle">
              <toggle v-model="WIFI.Enabled" v-on:change="WIFIChanges = true"></toggle>
            </span>
            <input type="text" class="form-control" disabled :value="WIFI.ESSID">
            <span class="input-group-btn">
              <button class="btn btn-default" type="button" v-on:click="ConfigWIFI"><img :src="require('Imgs/network_wireless_edit.png')"></button>
              <button class="btn btn-default" v-if="WIFIChanges" v-on:click="SaveChangesWIFI" data-toggle="tooltip" title="Save changes"><img :src="require('Imgs/diskette.png')"></button>
            </span>
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Details</label>
          <textarea rows="4" class="form-control" disabled>{{ WIFIDetails }}</textarea>
        </div>
      </div>
      <wifi-dialog v-if="WifiVisible" :loading="WifiLoading" v-model="WifiSettings"></wifi-dialog>
      <div class="row" v-if="NTP != null">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">NTP synchronization</label>
          <div class="input-group">
            <span class="input-group-addon input-group-addon-toogle">
              <toggle v-model="NTP.Enabled" v-on:change="NTPChanges = true"></toggle>
            </span>
            <input type="text" class="form-control" v-model="NTP.Server" :disabled="!NTP.Enabled" v-on:change="NTPChanges = true">
            <span class="input-group-btn" v-if="NTPChanges">
              <button class="btn btn-default" v-on:click="SaveChangesNTP" data-toggle="tooltip" title="Save changes (If set to disabled, current client date is taken)"><img :src="require('Imgs/diskette.png')"></button>
            </span>
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Details</label>
          <textarea rows="2" class="form-control" disabled>{{ NTPDetails }}</textarea>
        </div>
      </div>
      <div class="row" v-if="ModBus != null">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Modbus/TCP</label>
          <div class="input-group">
            <span class="input-group-addon input-group-addon-toogle">
              <toggle v-model="ModBus.Enabled" v-on:change="ModbusChanges"></toggle>
            </span>
            <input type="text" class="form-control">
            <span class="input-group-btn" v-if="ModBusChanges">
              <button class="btn btn-default" v-on:click="SaveChangesModBus" data-toggle="tooltip" title="Save changes"><img :src="require('Imgs/diskette.png')"></button>
            </span>
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Details</label>
          <textarea rows="2" class="form-control" disabled></textarea>
        </div>
      </div>
=======
      <bluetooth class="row" @enabled="Enabled=true"></bluetooth>
      <wifi class="row" @enabled="Enabled=true"></wifi>
      <ntp class="row" @enabled="Enabled=true"></ntp>
>>>>>>> web_cleanup
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
