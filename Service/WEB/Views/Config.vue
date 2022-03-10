<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" data-keyboard="false" ref="dialog">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <div class="text-center"><h4 class="modal-title">Welcome to openZmeter</h4></div>
        </div>
        <div class="modal-body">
          <spinner v-if="ShowSpinner"></spinner>
          <div v-if="View=='Intro'">
            <z-row>
              <div class="col-xs-12">
                <p>
                  Welcome to your <strong>openzmeter</strong> startup wizard.
                </p>
                <p style="text-align: justify">
                  Follow the next steps to configure the network connection. At the end, your device will be connected and you will be able to access all the functions available from any browser.
                </p>
              </div>
            </z-row>
            <hr style="margin: 5px 0px 5px 0px"/>
            <z-row>
              <div class="col-xs-4"><button type="button" @click="FullSite" class="btn btn-default btn-xs btn-block">Go to full site</button></div>
              <div class="col-xs-4"></div>
              <div class="col-xs-4"><button type="button" @click="Scan" class="btn btn-success btn-xs btn-block">Next</button></div>
            </z-row>
          </div>
          <div v-if="View=='Scan'">
            <z-row>
              <div class="col-xs-12">
                Select one of the detected networks from the list below, if you cannot see your network make sure it is in range. If you want to modify a previous configuration or your network does not appear in the list, you can skip this step.</div>
            </z-row>
            <z-row>
              <div class="col-xs-12">
                <vuetable ref="table" :fields="WifiColumns" :api-mode="false" :data="WifiList" class="table small table-striped">
                  <template slot="actions" slot-scope="props">
                    <z-button-group>
                      <button type="button" class="btn btn-success btn-xs" @click="Settings(props.rowData.ESSID)">Select</button>
                    </z-button-group>
                  </template>
                  <template slot="signal" slot-scope="props">
                    <progress-bar :value="props.rowData.Quality"></progress-bar>
                  </template>
                </vuetable>
              </div>
            </z-row>
            <hr style="margin: 5px 0px 5px 0px"/>
            <z-row>
              <div class="col-xs-4"><button type="button" @click="FullSite" class="btn btn-default btn-xs btn-block">Go to full site</button></div>
              <div class="col-xs-4"><button type="button" @click="Intro" class="btn btn-default btn-xs btn-block">Previous</button></div>
              <div class="col-xs-4"><button type="button" @click="Settings(null)" class="btn btn-success btn-xs btn-block">Next</button></div>
            </z-row>
          </div>
          <div v-if="View=='Settings'">
            <z-row>
              <div class="col-xs-12">
                <label class="control-label">NTP server</label>
                <input-text v-model="NTPConfig.Server"></input-text>
              </div>
              <div class="col-xs-12">
                <label class="control-label">SSID</label>
                <input-text v-model="WifiConfig.ESSID"></input-text>
              </div>
              <div class="col-xs-6">
                <label class="control-label">Mode</label>
                <option-picker :options="[{name: 'NONE', value: 'NONE'}, {name: 'WEP', value: 'WEP'}, {name: 'WPA', value: 'WPA'}, {name: 'WPA2', value: 'WPA2'}]" v-model="WifiConfig.KeyMode"></option-picker>
              </div>
              <div class="col-xs-6">
                <label class="control-label">IP Mode</label>
                <option-picker :options="[{name: 'DHCP', value: 'DHCP'}, {name: 'STATIC', value: 'STATIC'}]" v-model="WifiConfig.IPMode"></option-picker>
              </div>
              <div class="col-xs-12">
                <label class="control-label">ESSID Key</label>
                <password :disabled="WifiConfig.KeyMode=='NONE'" v-model="WifiConfig.Key"></password>
              </div>
              <div class="col-xs-12" v-if="WifiConfig.IPMode=='STATIC'">
                <label class="control-label">IP address</label>
                <input-text :disabled="WifiConfig.IPMode=='DHCP'" v-model="WifiConfig.IP"></input-text>
              </div>
              <div class="col-xs-6" v-if="WifiConfig.IPMode=='STATIC' ">
                <label class="control-label">Network mask</label>
                <input-text v-model="WifiConfig.Netmask"></input-text>
              </div>
              <div class="col-xs-6" v-if="WifiConfig.IPMode=='STATIC'">
                <label class="control-label">Gateway address</label>
                <input-text v-model="WifiConfig.Gateway"></input-text>
              </div>
              <div class="col-xs-6" v-if="WifiConfig.IPMode=='STATIC'">
                <label class="control-label">Primary DNS address</label>
                <input-text v-model="WifiConfig.DNS[0]"></input-text>
              </div>
              <div class="col-xs-6" v-if="WifiConfig.IPMode=='STATIC'">
                <label class="control-label">Secondary DNS address</label>
                <input-text v-model="WifiConfig.DNS[1]"></input-text>
              </div>
            </z-row>
            <hr style="margin: 5px 0px 5px 0px"/>
            <z-row>
              <div class="col-xs-4"><button type="button" @click="FullSite" class="btn btn-default btn-xs btn-block">Go to full site</button></div>
              <div class="col-xs-4"><button type="button" @click="Scan" class="btn btn-default btn-xs btn-block">Previous</button></div>
              <div class="col-xs-4"><button type="button" @click="Save" class="btn btn-success btn-xs btn-block">Next</button></div>
            </z-row>
          </div>
          <div v-if="View=='Save'">
            <z-row>
              <div class="col-xs-12">
                <p>
                  Your settings have been saved and the changes are being applied.
                </p>
                <p>
                  If the device has connected successfully, you can close this wizard and access the device with the indicated network address.
                </p>
              </div>
            </z-row>
            <z-row>
              <div class="col-xs-12">
                <textarea rows="10" class="form-control" disabled :value="Details"></textarea>
              </div>
            </z-row>
            <hr style="margin: 5px 0px 5px 0px"/>
            <z-row>
              <div class="col-xs-4"><button type="button" @click="FullSite" class="btn btn-default btn-xs btn-block">Go to full site</button></div>
              <div class="col-xs-4"><button type="button" @click="Settings(null)" class="btn btn-default btn-xs btn-block">Previous</button></div>
            </z-row>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import moment from 'moment';
  import 'moment-timezone';
  export default {
    props: { user: Object },
    data() {
      return { ShowSpinner: false, View: 'Intro', WifiList: [], Timer: null, WifiConfig: null, NTPConfig: null, Details: null,
        WifiColumns: [
          {name: 'ESSID', title: 'Name'},
          {name: 'Channel', title: 'Channel'},
          {name: '__slot:signal', title: 'Signal'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    mounted: function() {
      this.$nextTick(() => {
        $(this.$refs.dialog).modal('show');
      });
    },
    beforeDestroy: function() {
      if(this.Timer != null) clearTimeout(this.Timer);
    },
    created: function() {
      if(window.LocalConnection != true) this.$router.replace('Dashboard');
    },
    methods: {
      FullSite: function() {
        this.ShowSpinner = true;
        if(this.Timer != null) clearTimeout(this.Timer);
        this.Post('getUser', { User: 'admin' }).then(
          (result) => {
            this.ShowSpinner = false;
            this.$emit('update:user', result);
            this.$router.replace('Dashboard');
          },
          (fail) => {
            this.$toast.open({ type:'error', message: "Failed to login<br>Invalid user name or password", duration: 3000});
            this.ShowSpinner = false;
          }
        );
      },
      Intro: function() {
        if(this.Timer != null) clearTimeout(this.Timer);
        this.View = 'Intro';
      },
      Scan: function() {
        if(this.Timer != null) clearTimeout(this.Timer);
        if(this.View != 'Scan') this.ShowSpinner = true;
        this.Post('scanWIFI', { }).then(
          (result) => {
            this.View = 'Scan';
            this.WifiList = result;
            this.ShowSpinner = false;
            this.Timer = setTimeout(this.Scan, 5000);
          },
          (fail) => { this.Timer = setTimeout(this.Scan, 5000); }
        );
      },
      Settings: function(sel) {
        if(this.Timer != null) clearTimeout(this.Timer);
        Promise.all([this.Post('getWIFI', { Full: true }), this.Post('getNTP', { Full: true })]).then(
          (result) => {
            this.WifiConfig = result[0];
            this.WifiConfig.Enabled = true;
            this.NTPConfig = result[1];
            this.NTPConfig.Enabled = true;
            if(sel != null) {
              this.WifiConfig.ESSID = sel;
            }
            this.View = 'Settings';
          }
        );
      },
      Save: function() {
        if(this.Timer != null) clearTimeout(this.Timer);
        if(this.View != 'Save') {
          this.ShowSpinner = true;
          Promise.all([this.Post('setNTP', this.NTPConfig), this.Post('setWIFI', this.WifiConfig)]).then(
            (result) => {
              this.ShowSpinner = false;
              this.View = 'Save';
              this.Timer = setTimeout(this.Save, 5000);
            },
            (fail) => { this.Timer = setTimeout(this.Save, 5000); }
          );
        } else {
          Promise.all([this.Post('getWIFI', { Full: true }), this.Post('getNTP', { Full: true })]).then(
            (result) => {
              this.Details  = 'Device IP address   : ' + result[0].Status.IP + ' (' + result[0].Status.IPMode + ')\n';
              this.Details += 'DNS servers         : ' + result[0].Status.DNS.join(', ') + '\n';
              this.Details += 'Default gateway     : ' + result[0].Status.Gateway + '\n';
              this.Details += 'Current WIFI channel: ' + result[0].Status.Channel + ' (Signal: ' + result[0].Status.Quality + '%)\n\n';
              this.Details += 'Local timezone      : ' + moment.tz.zone(moment.tz.guess()).name + '\n';
              this.Details += 'Time                : ' + moment(result[1].Time).format('DD/MM/YYYY HH:mm:ss');
              this.Timer = setTimeout(this.Save, 5000);
            }
          );
        }
      }
    }
  }
</script>

<style scoped>
  .help {
    font-size: 80%;
  }
  textarea {
    resize: none;
    font-family: 'Fira Code';
    font-size: 12px;
  }
  .modal-header {
    padding: 4px 5px 2px 5px;
  }
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
  .table >>> th:nth-child(4) {
    width: 60px;
  }
</style>