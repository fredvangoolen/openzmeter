<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">Wifi settings</h4></div>
          <div class="pull-right btn-group">
            <button type="button" class="btn btn-default btn-xs" v-if="Mode=='Edit'" v-bind:class="{ disabled: (Error != '') }" v-on:click="Scan"><img :src="require('Imgs/magnifier.png')" alt="Scan networks"> Scan</button>
            <button type="button" class="btn btn-default btn-xs" v-if="Mode=='Edit'" v-bind:class="{ disabled: (Error != '') }" v-on:click="Save"><img :src="require('Imgs/diskette.png')" alt="Save changes"></button>
            <button type="button" class="btn btn-default btn-xs" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
          </div>
        </div>
        <div class="modal-body" v-if="Mode=='Scan'">
          <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
            <template slot="actions" slot-scope="props">
              <div class="btn-group">
                <button type="button" class="btn btn-success btn-xs" v-on:click="Select(props.rowData)">Select</button>
              </div>
            </template>
            <template slot="signal" slot-scope="props">
              <progress-bar :value="props.rowData.Quality"></progress-bar>
            </template>
          </vuetable>
        </div>
        <div class="modal-body" v-if="Mode=='Edit'">
          <spinner v-if="Loading"></spinner>
          <div class="row">
            <div class="col-md-12">
              <label class="control-label">SSID</label>
              <input type="text" class="form-control" ref="SSID" :value="Config.ESSID">
            </div>
            <div class="col-md-3 col-sm-6">
              <label class="control-label">Mode</label>
              <option-picker :options="['NONE', 'WEP', 'WPA', 'WPA2']" :selected="Config.KeyMode" ref="KeyType" v-on:OptionChanged="SetKeyType" :big="true"></option-picker>
            </div>
            <div class="col-md-9 col-sm-6">
              <label class="control-label">Password</label>
              <input type='password' class="form-control" ref="Password" :disabled="Config.KeyMode=='NONE'" :value="Config.Key"/>
            </div>
            <div class="col-md-12">
              <label class="control-label">IP address</label>
              <div class="input-group">
                <span class="input-group-addon input-group-addon-toogle">
                  <toggle ref="EnableDHCP" v-on:Changed="DHCPChanged" onText="DHCP" offText="STATIC"></toggle>
                </span>
                <input type="text" class="form-control" ref="IP" :disabled="Config.IPMode==='DHCP'" :value="Config.IP">
              </div>
            </div>
            <div class="col-md-6" v-if="Config.IPMode!=='DHCP'">
              <label class="control-label">Network mask</label>
              <input type="text" class="form-control" ref="Mask" :value="Config.Netmask">
            </div>
            <div class="col-md-6" v-if="Config.IPMode!=='DHCP'">
              <label class="control-label">Gateway address</label>
              <input type="text" class="form-control" ref="GW" :value="Config.Gateway">
            </div>
            <div class="col-md-6" v-if="Config.IPMode!=='DHCP'">
              <label class="control-label">Primary DNS address</label>
              <input type="text" class="form-control" ref="DNS1" :value="((Config.DNS != undefined) && (Config.DNS[0] != undefined)) ? Config.DNS[0] : ''">
            </div>
            <div class="col-md-6" v-if="Config.IPMode!=='DHCP'">
              <label class="control-label">Secondary DNS address</label>
              <input type="text" class="form-control" ref="DNS2" :value="((Config.DNS != undefined) && (Config.DNS[1] != undefined)) ? Config.DNS[1] : ''">
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Toggle from 'Components/Toggle.vue';
  import Vuetable from 'vuetable-2';
  import OptionPicker from 'Components/OptionPicker.vue';
  import Spinner from 'Components/Spinner.vue';
  import ProgressBar from 'Components/ProgressBar.vue';
  export default {
    data() {
      return { Error: '', ShowDialog: false, Loading: true, Mode: 'Edit', Config: {},
      Columns: [
          {name: 'ESSID', title: 'Name'},
          {name: 'Channel', title: 'Channel'}, 
          {name: '__slot:signal', title: 'Signal'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    methods: {
      Show: function() {
        this.Mode = 'Edit';
        this.ShowDialog = true;
        this.$nextTick(function() {     
          $(this.$el).modal('show');
          $.ajax({ url: "getWIFI", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Type: 'CONFIG'}), 
            success: function(result) {
              this.Config = result;
              this.$refs.KeyType.SetOption(result.KeyMode);
              this.$refs.EnableDHCP.SetState(result.IPMode === 'DHCP');
              this.Loading = false;
            }.bind(this)
          });
        });
      },
      Back: function() {
        if(this.Mode === 'Scan') {
          this.Mode = 'Edit';
        } else {
          $(this.$el).modal('hide');
          this.ShowDialog = false;
        }
      },
      Scan: function() {
        this.Mode = 'Scan';
        $.ajax({ url: "scanWIFI", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) {
            var TableData = { data: result };
            this.$refs.table.setData(TableData);
          }.bind(this)
        });
      },
      Select: function(row) {
        this.Mode = 'Edit';
        this.$nextTick(function() {
          this.Config.ESSID = row.ESSID;
        });
      },
      Save: function() {
        this.Config.ESSID = this.$refs.SSID.value;
        this.Config.Key = this.$refs.Password.value;
        if(this.Config.IPMode !== 'DHCP') {
          this.Config.IP = this.$refs.IP.value;
          this.Config.Netmask = this.$refs.Mask.value;
          this.Config.Gateway = this.$refs.GW.value;
          this.Config.DNS = [this.$refs.DNS1.value, this.$refs.DNS2.value];
        }
        this.$parent.SetWifiConfig(this.Config);
        this.Back();
      },
      SetKeyType: function(value) {
        this.Config.KeyMode = value;
      },
      DHCPChanged: function(value) {
        this.Config.IPMode = value===true ? 'DHCP' : 'STATIC';
      }
    },
    mounted: function() {
      this.$root.$el.append(this.$el);
    },
    destroyed: function() {
      this.$root.$el.removeChild(this.$el);
    },
    components: { Toggle, OptionPicker, Spinner, Vuetable, ProgressBar }
  }
</script>

<style scoped>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
  .modal-body >>> .progress {
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
