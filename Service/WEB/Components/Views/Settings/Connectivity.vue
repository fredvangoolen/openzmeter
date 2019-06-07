<template>
  <div class="panel panel-other" v-if="Bluetooth_Support || WIFI_Support || NTP_Support">
    <div class="panel-heading">
      <div class="huge-small header-text">Connectivity</div>
      <div class="header-buttons">
        <div class="btn-group">
          <button type="button" class="btn btn-default btn-xs" v-if="BTChanges || WIFIChanges || NTPChanges" v-on:click="SaveChanges" data-toggle="tooltip" title="Save changes"><img :src="require('Imgs/diskette.png')"> Save</button>
        </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="Loading"></spinner>
      <div class="row" v-bind:style="{ display: Bluetooth_Support ? 'block' : 'none'}">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Bluetooth connection</label>
          <div class="input-group">
            <span class="input-group-addon input-group-addon-toogle">
              <toggle ref="EnableBT" v-on:Changed="EnableBTChange"></toggle>
            </span>
            <input type="text" class="form-control" disabled :value="BTName">
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Details</label>
          <textarea rows="2" class="form-control" disabled>
Device IP  : 10.0.0.1 
Clients IP : 10.0.0.2 - 10.0.0.254</textarea>
        </div>
      </div>
      <div class="row" :style="{ display: WIFI_Support ? 'block' : 'none'}">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">WIFI connection</label>
          <div class="input-group">
            <span class="input-group-addon input-group-addon-toogle">
              <toggle ref="EnableWIFI" v-on:Changed="EnableWIFIChange"></toggle>
            </span>
            <input type="text" class="form-control" disabled :value="WIFIName">
            <span class="input-group-btn">
              <button class="btn btn-default" type="button" v-on:click="ConfigWIFI"><img :src="require('Imgs/network_wireless_edit.png')"></button>
            </span>
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Details</label>
          <textarea rows="4" class="form-control" disabled>{{ WIFIDetails }}</textarea>
        </div>
      </div>
      <wifi-dialog ref="dialog"></wifi-dialog>
      <div class="row">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">NTP synchronization</label>
          <div class="input-group">
            <span class="input-group-addon input-group-addon-toogle">
              <toggle ref="EnableNTP" v-on:Changed="EnableNTPChange"></toggle>
            </span>
            <input type="text" class="form-control" :value="NTP_Server">
            <input type='text' class="form-control" v-if="false"/>
            <span class="input-group-addon" v-if="false">
              <span class="glyphicon-calendar"></span>
            </span>  
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Details</label>
          <textarea rows="2" class="form-control" disabled>{{ NTPDetails }}</textarea>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Toggle from 'Components/Toggle.vue';
  import moment from 'JS/moment.js';
  import 'JS/moment-timezone.js';
  import Spinner from 'Components/Spinner.vue';
  import WifiDialog from './WIFIEdit.vue';
  export default {
    data: function() {
      return { BTName: '', Bluetooth_Support: false, BTChanges: false,
                WIFIName: '', WIFIDetails: '', WIFI_Support: false, WIFIChanges: false, WIFISettings: {},
                NTP_Support: false, NTP_Server: '', NTPDetails: '', NTPChanges: false,
                Loading: false };
    },
    created: function() {
      this.BTEnabled = false;
      this.WIFIEnabled = false;
      this.NTPEnabled = false;
      this.Timer = null;
      this.Timer2 = null;
    },
    mounted: function() {
      this.Load();
    },
    beforedestroy: function() {
      clearInterval(this.Timer);
      clearInterval(this.Timer2);
    },
    methods: {
      UpdateTime: function() {
        $.ajax({ url: "getNTP", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) { 
            this.NTPDetails = 'Local timezone : ' + moment.tz.zone(moment.tz.guess()).name + '\nTime           : ' + moment(result.Time).format('DD/MM/YYYY HH:mm:ss');
          }.bind(this),
          complete: function() { 
            setTimeout(this.UpdateTime, 500); 
          }.bind(this)
        });  
      },
      UpdateWifi: function() {
        $.ajax({ url: "getWIFI", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Type: "CURRENT" }), 
          success: function(result) { 
            if(result.Connected === true) {
              this.WIFIName = result.ESSID;
              this.WIFIDetails = 'Device IP address   : ' + result.IP + ' (' + result.IPMode + ')\nDNS servers         : ' + result.DNS.join(', ') + '\nDefault gateway     : ' + result.Gateway + '\nCurrent WIFI channel: ' + result.Channel;
            } else {
              this.WIFIName = '';
              this.WIFIDetails = 'Not connected';
            }
          }.bind(this),
          complete: function() { 
            setTimeout(this.UpdateWifi, 5000); 
          }.bind(this)
        });  
      },
      Load: function() {
        $.ajax({ url: "getBluetooth", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) { 
            this.Bluetooth_Support = true;
            this.BTEnabled = result.Enabled;
            this.BTName = result.Name;
            this.Loading = false;
            this.$nextTick(function() { 
              this.$refs.EnableBT.SetState(this.BTEnabled);
            });
          }.bind(this)
        });
        $.ajax({ url: "getWIFI", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Type: "CURRENT" }), 
          success: function(result) { 
            this.WIFI_Support = true;
            this.WIFIEnabled = result.Enabled;
            if(result.Connected === true) {
              this.WIFIName = result.ESSID;
              this.WIFIDetails = 'Device IP address   : ' + result.IP + ' (' + result.IPMode + ')\nDNS servers         : ' + result.DNS.join(', ') + '\nDefault gateway     : ' + result.Gateway + '\nCurrent WIFI channel: ' + result.Channel;
            } else {
              this.WIFIName = '';
              this.WIFIDetails = 'Not connected';
            }
            this.Loading = false;
            this.$nextTick(function() { 
              this.$refs.EnableWIFI.SetState(this.WIFIEnabled);
              setTimeout(this.UpdateWifi, 1000); 
            });
          }.bind(this)
        });
        $.ajax({ url: "getNTP", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) { 
            this.NTP_Support = true;
            this.NTPEnabled = result.Enabled;
            this.NTP_Server = result.Server;
            this.NTPDetails = 'Local timezone : ' + /*moment.tz.zone(moment.tz.guess()).name +*/ '\nTime           : ' + moment(result.Time).format('DD/MM/YYYY HH:mm:ss');
            this.Loading = false;
            this.$nextTick(function() { 
              this.$refs.EnableNTP.SetState(this.NTPEnabled); 
              setTimeout(this.UpdateTime, 1000); 
            });
          }.bind(this)
        });  
      },
      EnableBTChange: function(value) {
        this.BTEnabled = value;
        this.BTChanges = true;
      },
      EnableWIFIChange: function(value) {
        this.WIFIEnabled = value;
        this.WIFIChanges = true;
      },
      EnableNTPChange: function(value) {
        this.NTPEnabled = value;
        this.NTPChanges = true;
      },
      ConfigWIFI: function() {
        this.$refs.dialog.Show(); 
      },
      SetWifiConfig: function(config) {
        this.WIFIChanges = true;
        this.WIFISettings = config;
      },
      SaveChanges: function() {
        clearInterval(this.Timer);
        clearInterval(this.Timer2);
        if(this.BTChanges === true) {
          $.ajax({ url: "setBluetooth", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Enabled: this.BTEnabled }) });
          this.BTChanges = false;
        }
        if(this.WIFIChanges === true) {
          this.WIFISettings.Enabled = this.WIFIEnabled;
          $.ajax({ url: "setWIFI", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify(this.WIFISettings)});
          this.WIFIChanges = false;
          this.WIFISettings = {};
        }
      }
    },
    components: { Toggle, Spinner, WifiDialog }
  }
  
  
  
  
  
  
  
  $('#checkNTP').change(function() {
    if($(this).prop('checked')) {
      $('#inputCurrentTime > span').css("pointer-events", "none");
      $('#inputCurrentTime > input').attr("disabled", "true");
      $('#inputNTPServer').removeAttr("disabled");
    } else {
      $('#inputNTPServer').attr("disabled", "true");
      $('#inputCurrentTime > input').removeAttr("disabled");
      $('#inputCurrentTime > span').css("pointer-events", "inherit");
    }
  });
function AjaxGetTimeZones() {
  $('#checkNTP').bootstrapToggle();
  $.ajax({ url: "getTimeSettings", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), success: 
    function(result) {
      NTP_DateTimePicker = $('#inputCurrentTime').datetimepicker({ format: 'DD/MM/YYYY HH:mm:00', defaultDate: moment(result.Time), sideBySide: true });
      $('#inputCurrentTime > input').attr('value', moment(moment(result.Time)).format('DD/MM/YYYY HH:mm:ss'));
      $('#dropdownTimeZone span:first-child').text(result.TimeZone);
      $('#inputNTPServer').val(result.NTP_Server);
      if(result.NTP_Enabled === true) {
        $('#checkNTP').bootstrapToggle('on');
        $('#inputNTPServer').removeAttr("disabled");
      } else {
        $('#checkNTP').bootstrapToggle('off');
        $('#inputNTPServer').attr("disabled", "true");
      }
      result.TimeZones.sort();
      for(i = 0; i < result.TimeZones.length; i++) $('#dropdownListTimeZone').append('<li><a href="#">' + result.TimeZones[i] + '</a></li>');
      
      $("#dropdownListTimeZone li a").click(function(event) {
        event.preventDefault();
        $('#dropdownTimeZone span:first-child').text($(this).text());
     });
    }
  });
}
$('#btnNTPSave').click(function() {
  $('#btnNTPSave').attr("disabled");
  $.ajax({ url: "setTimeSettings", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', 
    data: JSON.stringify({ NTP_Enabled: $('#checkNTP').is(':checked'), NTP_Server: $('#inputNTPServer').val(), TimeZone: $('#dropdownTimeZone span:first-child').text(), Time: moment.tz($('#inputCurrentTime > input').val(), 'DD/MM/YYYY HH:mm:ss', $('#dropdownTimeZone span:first-child').text()).utc().valueOf() }), 
    success: function(result) {
      $('#dlgSave .modal-body > p').html("Settings changed correctly!");
      $('#dlgSave').modal('show'); 
    },
    error: function(result) {
      $('#dlgSave .modal-body > p').html("Failed to save settings!");
      $('#dlgSave').modal('show'); 
    },
    complete: function(result) {
      $('#btnNTPSave').removeAttr("disabled");    
    }
  });
});

</script>

<style scoped>
  textarea {
    resize: none;
    font-family: monospace;
    font-size: 12px;
  }
</style>