<template>
  <div class="panel panel-other" v-if="(Bluetooth != null) || (WIFI != null) || (NTP != null) || (Modbus != null) || (Telegram != null)">
    <div class="panel-heading">
      <div class="huge-small header-text">Connectivity</div>
    </div>
    <div class="panel-body">
      <spinner v-if="Loading"></spinner>
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
            <input type="text" class="form-control" v-model="NTP.Server" :disabled="!NTP.Enabled">
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
      <!--div class="row">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Modbus/TCP</label>
          <div class="input-group">
            <span class="input-group-addon input-group-addon-toogle">
              <toggle ref="EnableModbus" v-on:change="EnableModbusChange"></toggle>
            </span>
            <input type="text" class="form-control" ref="ntpserver" v-on:input="NTPServerChanged" :disabled="!NTPEnabled">
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Details</label>
          <textarea rows="2" class="form-control" disabled></textarea>
        </div>
      </div-->
    </div>
  </div>
</template>

<script>
  import Toggle from 'Components/Toggle.vue';
  import moment from 'moment';
  import 'moment-timezone';
  import Spinner from 'Components/Spinner.vue';
  import WifiDialog from './WIFIEdit.vue';
  export default {
    data: function() {
      return { Bluetooth: null, BTChanges: false,
                WIFI: false, WIFIChanges: false, WIFIDetails: '', WifiSettings: {}, WifiVisible: false, WifiLoading: false,
                NTP: false, NTPChanges: false, NTPDetails: '',
                
                Modbus_Support: false, ModbusChange: false, ModbusSettings: {},
                Telegram_Support: false, TelegramChange: false, TelegramSettings: {},
                Loading: false, NTPEnabled: false };
    },
    created: function() {
      this.BTEnabled = false;
      this.WIFIEnabled = false;
      this.NTPAjax = null;
      this.WifiAjax = null;
      this.Timer = null;
      this.Timer2 = null;
    },
    mounted: function() {
      this.Load();
    },
    beforeDestroy: function() {
      if(this.NTPAjax) this.NTPAjax.abort();
      if(this.WifiAjax) this.WifiAjax.abort();
      clearInterval(this.Timer);
      clearInterval(this.Timer2);
    },
    methods: {
      UpdateTime: function() {
        clearInterval(this.Timer2);
        if(this.NTPAjax) this.NTPAjax.abort();
        this.NTPAjax = $.ajax({ url: "getNTP", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: (result) => { 
            this.NTPDetails = 'Local timezone : ' + moment.tz.zone(moment.tz.guess()).name + '\nTime           : ' + moment(result.Time).format('DD/MM/YYYY HH:mm:ss');
          },
          complete: () => {
            this.Timer2 = setTimeout(this.UpdateTime, 500);
          }
        });  
      },
      UpdateWifi: function() {
        clearInterval(this.Timer);
        if(this.WifiAjax) this.WifiAjax.abort();
        this.WifiAjax = $.ajax({ url: "getWIFI", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Type: "CURRENT" }), 
          success: (result) => { 
            if(result.Connected === true) {
              this.WIFI = result;
              this.WIFIDetails = 'Device IP address   : ' + result.IP + ' (' + result.IPMode + ')\nDNS servers         : ' + result.DNS.join(', ') + '\nDefault gateway     : ' + result.Gateway + '\nCurrent WIFI channel: ' + result.Channel;
            } else {
              this.WIFIName = '';
              this.WIFIDetails = 'Not connected';
            }
          },
          complete: () => { 
            this.Timer = setTimeout(this.UpdateWifi, 5000); 
          }
        });  
      },
      Load: function() {
        $.ajax({ url: "getBluetooth", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: (result) => { 
            this.Bluetooth = result;
          }
        });
        $.ajax({ url: "getWIFI", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Type: "CURRENT" }), 
          success: (result) => { 
            this.WIFI = result;
            if(result.Connected === true) {
              this.WIFIDetails = 'Device IP address   : ' + result.IP + ' (' + result.IPMode + ')\nDNS servers         : ' + result.DNS.join(', ') + '\nDefault gateway     : ' + result.Gateway + '\nCurrent WIFI channel: ' + result.Channel;
            } else {
              this.WIFIDetails = 'Not connected';
            }
          },
          complete: () => {
            this.Timer = setTimeout(this.UpdateWifi, 1000); 
          }
        });
        $.ajax({ url: "getNTP", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: (result) => { 
            this.NTP = result;
            this.NTPDetails = 'Local timezone : ' + '\nTime           : ' + moment(result.Time).format('DD/MM/YYYY HH:mm:ss');
          }, 
          complete: () => {
            this.Timer2 = setTimeout(this.UpdateTime, 1000); 
          }
        });  
      },
      SaveChangesBT: function(value) {
        $.ajax({ url: "setBluetooth", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Enabled: this.Bluetooth.Enabled }) });
        this.BTChanges = false;
      },
      SaveChangesWIFI: function(value) {
        $.ajax({ url: "setWIFI", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({Enabled: this.WIFI.Enabled})});
        this.WIFIChanges = false;
      },
      SaveChangesNTP: function(value) {
        $.ajax({ url: "setNTP", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Enabled: this.NTP.Enabled, Server: this.NTP.Server, Time: moment().valueOf() }) });
        this.NTPChanges = false;      
      },
      ConfigWIFI: function() {
        this.WifiLoading = true;
        this.WifiVisible = true;
        $.ajax({ url: "getWIFI", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Type: 'CONFIG'}), 
          success: (result) => {
            if(result.DNS == undefined) result.DNS = [];    
            this.WifiSettings = result;
            this.WifiLoading = false;
          },
          error: () => {
            this.WifiVisible = false;
          }
        });
      }
    },
    components: { Toggle, Spinner, WifiDialog }
  }
</script>

<style scoped>
  textarea {
    resize: none;
    font-family: monospace;
    font-size: 12px;
  }
</style>
