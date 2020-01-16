<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Wifi settings</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-if="Mode=='Edit'" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save changes"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <div v-if="Mode=='Scan'">
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
    <div v-if="Mode=='Edit'">
      <div class="row">
        <div class="col-md-12">
          <label class="control-label">SSID</label>
          <div class="input-group">
            <input type="text" class="form-control" v-model="Settings.ESSID" autocomplete="off" autocapitalize="off" autocorrect="off">
            <span class="input-group-btn">
              <button type="button" class="btn btn-default" @click="Scan"><img :src="require('Imgs/magnifier.png')" alt="Scan networks"></button>
            </span>
          </div>
        </div>
        <div class="col-md-3 col-sm-6">
          <label class="control-label">Mode</label>
          <option-picker :options="[{name: 'NONE', value: 'NONE'}, {name: 'WEP', value: 'WEP'}, {name: 'WPA', value: 'WPA'}, {name: 'WPA2', value: 'WPA2'}]" v-model="Settings.KeyMode" :big="true"></option-picker>
        </div>
        <div class="col-md-3 col-sm-6">
          <label class="control-label">IP Mode</label>
          <option-picker :options="[{name: 'DHCP', value: 'DHCP'}, {name: 'STATIC', value: 'STATIC'}]" v-model="Settings.IPMode" :big="true"></option-picker>
        </div>
        <div class="col-md-6 col-sm-12">
          <label class="control-label">ESSID Key</label>
          <input type='password' class="form-control" :disabled="Settings.KeyMode=='NONE'" v-model="Settings.Key"/>
        </div>
        <div class="col-md-12" v-if="Settings.IPMode=='STATIC'">
          <label class="control-label">IP address</label>
          <input type="text" class="form-control" :disabled="Settings.IPMode=='DHCP'" v-model="Settings.IP">
        </div>
        <div class="col-md-6" v-if="Settings.IPMode=='STATIC'">
          <label class="control-label">Network mask</label>
          <input type="text" class="form-control" v-model="Settings.Netmask">
        </div>
        <div class="col-md-6" v-if="Settings.IPMode=='STATIC'">
          <label class="control-label">Gateway address</label>
          <input type="text" class="form-control" v-model="Settings.Gateway">
        </div>
        <div class="col-md-6" v-if="Settings.IPMode=='STATIC'">
          <label class="control-label">Primary DNS address</label>
          <input type="text" class="form-control" v-model="Settings.DNS[0]">
        </div>
        <div class="col-md-6" v-if="Settings.IPMode=='STATIC'">
          <label class="control-label">Secondary DNS address</label>
          <input type="text" class="form-control" v-model="Settings.DNS[1]">
        </div>
      </div>
    </div>
  </dlg>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { settings: Object },
    data() {
      return { Mode: 'Edit', WifiList: [], ScanTimer: null, ShowSpinner: false, Settings: null,
        Columns: [
          {name: 'ESSID', title: 'Name'},
          {name: 'Channel', title: 'Channel'},
          {name: '__slot:signal', title: 'Signal'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    created: function() {
      this.Settings = Object.assign({}, this.settings);
      if(this.Settings.DNS == null) this.Settings.DNS = [];
    },
    methods: {
      Scan: function() {
        this.Mode = 'Scan';
        this.ShowSpinner = true;
        this.$http.post('scanWIFI', { }).then(
          (result) => {
            this.WifiList = result.data;
            this.ShowSpinner = false;
            this.ScanTimer = setTimeout(this.ReScan, 3000);
          },
          (fail) => { }
        );
      },
      Back: function() {
        if(this.Mode == 'Scan') {
          this.Mode = 'Edit';
          clearInterval(this.ScanTimer);
        } else {
          this.$emit('close');
        }
      },
      ReScan: function() {
        clearInterval(this.ScanTimer);
        this.$http.post('scanWIFI', { }).then(
          (result) => {
            this.WifiList = result;
            if(this.Mode == 'Scan') this.ScanTimer = setTimeout(this.ReScan, 3000);
          },
          (fail) => { }
        );
      },
      Select: function(row) {
        this.Mode = 'Edit';
        this.Settings.ESSID = row.ESSID;
      },
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post('setWIFI', this.Settings).then(
          (result) => {
            this.ShowSpinner = false;
            this.$emit('close');
          },
          (fail) => { this.ShowSpinner = false; }
        );
      }
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    components: { Vuetable }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
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
  }
</style>
