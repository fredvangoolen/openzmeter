<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Wifi settings</h4></div>
      <div class="pull-right btn-group" v-if="Saving==null">
        <button type="button" class="btn btn-default btn-xs" v-if="View=='Edit'" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save changes and close"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <div class="row" v-if="View=='Edit' && Saving==null">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enable</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Config.Enabled"></toggle>
      </div>
      <div class="col-xs-12"><hr style="margin-top: 5px; margin-bottom: 5px; "/></div>
    </div>
    <div class="row" v-if="View=='Edit'">
      <div class="col-xs-12" v-if="Saving==null">
        <label class="control-label">SSID</label>
        <div class="input-group">
          <input type="text" class="form-control" v-model="Config.ESSID" autocomplete="off" autocapitalize="off" autocorrect="off">
          <span class="input-group-btn">
            <button type="button" class="btn btn-default" @click="Scan"><img :src="require('Imgs/magnifier.png')" alt="Scan networks"></button>
          </span>
        </div>
      </div>
      <div class="col-md-3 col-sm-6" v-if="Saving==null">
        <label class="control-label">Mode</label>
        <option-picker :options="[{name: 'NONE', value: 'NONE'}, {name: 'WEP', value: 'WEP'}, {name: 'WPA', value: 'WPA'}, {name: 'WPA2', value: 'WPA2'}]" v-model="Config.KeyMode"></option-picker>
      </div>
      <div class="col-md-3 col-sm-6" v-if="Saving==null">
        <label class="control-label">IP Mode</label>
        <option-picker :options="[{name: 'DHCP', value: 'DHCP'}, {name: 'STATIC', value: 'STATIC'}]" v-model="Config.IPMode"></option-picker>
      </div>
      <div class="col-md-6 col-sm-12" v-if="Saving==null">
        <label class="control-label">ESSID Key</label>
        <password :disabled="Config.KeyMode=='NONE'" v-model="Config.Key"></password>
      </div>
      <div class="col-md-12" v-if="Config.IPMode=='STATIC' && Saving==null">
        <label class="control-label">IP address</label>
        <input type="text" class="form-control" :disabled="Config.IPMode=='DHCP'" v-model="Config.IP">
      </div>
      <div class="col-md-6" v-if="Config.IPMode=='STATIC' && Saving==null">
        <label class="control-label">Network mask</label>
        <input type="text" class="form-control" v-model="Config.Netmask">
      </div>
      <div class="col-md-6" v-if="Config.IPMode=='STATIC' && Saving==null">
        <label class="control-label">Gateway address</label>
        <input type="text" class="form-control" v-model="Config.Gateway">
      </div>
      <div class="col-md-6" v-if="Config.IPMode=='STATIC' && Saving==null">
        <label class="control-label">Primary DNS address</label>
        <input type="text" class="form-control" v-model="Config.DNS[0]">
      </div>
      <div class="col-md-6" v-if="Config.IPMode=='STATIC' && Saving==null">
        <label class="control-label">Secondary DNS address</label>
        <input type="text" class="form-control" v-model="Config.DNS[1]">
      </div>
      <div class="col-xs-12">
        <label class="control-label">Details</label>
        <textarea rows="4" class="form-control" disabled :value="Details"></textarea>
      </div>
      <div class="col-xs-12">
        <label class="control-label">System log messages</label>
        <textarea rows="10" class="form-control" disabled :value="SysMessages"></textarea>
      </div>
    </div>
    <div v-if="View=='Scan'">
      <vuetable ref="table" :fields="Columns" :api-mode="false" :data="WifiList" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-success btn-xs" @click="Select(props.rowData)">Select</button>
          </div>
        </template>
        <template slot="signal" slot-scope="props">
          <progress-bar :value="props.rowData.Quality"></progress-bar>
        </template>
      </vuetable>
    </div>
  </dlg>
</template>

<script>
  import Vuetable from 'vuetable-2';
  export default {
    props: { settings: Object },
    data() {
      return { View: 'Edit', WifiList: [], Timer: null, ShowSpinner: false, Saving: null, Config: null, Details: '', SysMessages: '',
        Columns: [
          {name: 'ESSID', title: 'Name'},
          {name: 'Channel', title: 'Channel'},
          {name: '__slot:signal', title: 'Signal'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    created: function() {
      this.Config = Object.assign({}, this.settings);
      this.Refresh();
    },
    methods: {
      Refresh: function() {
        if(this.Timer != null) clearTimeout(this.Timer);
        Promise.all([this.Post('getWIFI', { Full: true }), this.Post('getSystem', { Series: ['SystemLog']})]).then(
          (result) => {
            this.Details = 'Device IP address   : ' + result[0].Status.IP + ' (' + result[0].Status.IPMode + ')\nDNS servers         : ' + result[0].Status.DNS.join(', ') + '\nDefault gateway     : ' + result[0].Status.Gateway + '\nCurrent WIFI channel: ' + result[0].Status.Channel + ' (Signal: ' + result[0].Status.Quality + '%)';
            this.SysMessages = result[1].SystemLog;
            this.Timer = setTimeout(this.Refresh, 5000);
            if(this.Saving != null) {
              if(result[0].Status.Connected == true) {
                this.$toast.open({type:'info', message: "Connected to WIFI '" + result[0].ESSID + "'", duration: 3000});
                setTimeout(this.Back(), 1000);
              } else {
                this.Saving++;
                if(this.Saving == 5) {
                  this.$toast.open({type:'error', message: "Can not connect to WIFI", duration: 3000});
                  this.Saving = null;
                }
              }
            }
          },
          (fail) => { this.Timer = setTimeout(this.Refresh, 5000); }
        );
      },
      Scan: function() {
        if(this.View != 'Scan') this.ShowSpinner = true;
        this.View = 'Scan';
        if(this.Timer != null) clearTimeout(this.Timer);
        this.Post('scanWIFI', { }).then(
          (result) => {
            this.WifiList = result;
            this.ShowSpinner = false;
            this.Timer = setTimeout(this.Scan, 5000);
          },
          (fail) => { this.Timer = setTimeout(this.Scan, 5000); }
        );
      },
      Back: function() {
        if(this.Timer != null) clearTimeout(this.Timer);
        if(this.View == 'Scan') {
          this.View = 'Edit';
          this.Refresh();
        } else {
          this.$emit('close');
        }
      },
      Select: function(row) {
        if(this.Timer != null) clearTimeout(this.Timer);
        this.Refresh();
        this.Config.ESSID = row.ESSID;
        this.View = 'Edit';
      },
      Save: function() {
        this.ShowSpinner = true;
        this.Post('setWIFI', this.Config).then(
          (result) => {
            this.ShowSpinner = false;
            this.Saving = 0;
          },
          (fail) => {
            this.ShowSpinner = false;
          }
        );
      }
    },
    components: { Vuetable }
  }
</script>

<style scoped>
  textarea {
    resize: none;
    font-family: 'Fira Code';
    font-size: 10px;
    white-space: pre;
  }
  div >>> .progress {
    height: 12px;
    background-color: #FFFFFF;
    border:1px solid #BBB;
    margin-bottom: 0px;
  }
  .table >>> th:nth-child(2) {
    width: 60px;
    text-align: center;
  }
  .table >>> th:nth-child(3) {
    text-align: center;
  }
  .table >>> td:nth-child(2) {
    text-align: center;
  }
  .table >>> th:nth-child(4) {
    width: 60px;
  }
</style>