<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">Wifi settings</h4></div>
          <div class="pull-right btn-group">
            <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Error != '') }" @click="Save"><img :src="require('Imgs/magnifier.png')" alt="Scan networks"> Scan</button>
            <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Error != '') }" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save changes"></button>
            <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
          </div>
        </div>
        <div class="modal-body">
          <spinner v-if="Loading"></spinner>
          <div class="row">
            <div class="col-md-12">
              <label class="control-label">SSID</label>
              <input type="text" class="form-control" ref="SSID">                             
            </div>
            <div class="col-md-3 col-sm-6">
              <label class="control-label">Mode</label>
              <option-picker :options="['OPEN', 'WEP', 'WPA', 'WPA2']" selected="NONE" ref="KeyType" v-on:OptionChanged="SetKeyType" :big="true"></option-picker>
            </div>
            <div class="col-md-9 col-sm-6">
              <label class="control-label">Password</label>
              <input type='password' class="form-control" ref="Password" :disabled="KeyMode=='NONE'"/>
            </div>
            <div class="col-md-12">
              <label class="control-label">IP address</label>
              <div class="input-group">
                <span class="input-group-addon input-group-addon-toogle">
                  <toggle ref="EnableDHCP" v-on:Changed="DHCPChanged" onText="DHCP" offText="STATIC"></toggle>
                </span>
                <input type="text" class="form-control" ref="IP" :disabled="DHCPActive">
              </div>
            </div>
            <div class="col-md-6" v-if="!DHCPActive">
              <label class="control-label">Network mask</label>
              <input type="text" class="form-control" ref="Mask">
            </div>
            <div class="col-md-6" v-if="!DHCPActive">
              <label class="control-label">Gateway address</label>
              <input type="text" class="form-control" ref="GW">
            </div>
            <div class="col-md-6" v-if="!DHCPActive">
              <label class="control-label">Primary DNS address</label>
              <input type="text" class="form-control" ref="DNS1">
            </div>
            <div class="col-md-6" v-if="!DHCPActive">
              <label class="control-label">Secondary DNS address</label>
              <input type="text" class="form-control" ref="DNS2">
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Toggle from 'Components/Toggle.vue';
  import OptionPicker from 'Components/OptionPicker.vue';
  import Spinner from 'Components/Spinner.vue';
  export default {
    data() {
      return { Error: '', ShowDialog: false, Loading: true, DHCPActive: true, KeyMode: 'NONE' };
    },
    created: function() {
      this.Config = null;
    },
    methods: {
      Show: function() {
        this.ShowDialog = true;
        this.$nextTick(function() {     
          $(this.$el).modal('show');
          $.ajax({ url: "getWIFI", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Type: 'CONFIG'}), 
            success: function(result) {
              this.Config = result;
              this.$refs.SSID.value = result.ESSID;
              this.HasKey = (result.KeyMode !== 'NONE');
              this.$refs.Password.value = result.Key;
              this.DHCPActive = (result.IPMode === 'DHCP');
              this.KeyMode = result.KeyMode;
              this.$refs.KeyType.SetOption(this.KeyMode);
              this.$refs.EnableDHCP.SetState(this.DHCPActive);
              if(this.DHCPActive === false) {
                this.$nextTick(function() { 
                  if(result.IP != undefined) this.$refs.IP.value = result.IP;
                  if(result.Netmask != undefined) this.$refs.Mask.value = result.Netmask;
                  if(result.Gateway != undefined) this.$refs.GW.value = result.Gateway;
                  if(result.DNS != undefined) {
                    if(result.DNS[0] != undefined) this.$refs.DNS1.value = result.DNS[0];
                    if(result.DNS[1] != undefined) this.$refs.DNS2.value = result.DNS[1];
                  }
                });
              }
              this.Loading = false;
            }.bind(this)
          });
        });
      },
      Back: function() {
        $(this.$el).modal('hide');
        this.ShowDialog = false;
      },
      Save: function() {
        this.Config.ESSID = this.$refs.SSID.value;
        this.Config.Key = this.$refs.Password.value;
        this.Config.IPMode = (this.DHCPActive === true) ? "DHCP" : "STATIC";
        if(this.DHCPActive === false) {
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
        this.DHCPActive = value;
      }
    },
    mounted: function() {
      this.$root.$el.append(this.$el);
    },
    destroyed: function() {
      this.$root.$el.removeChild(this.$el);
    },
    components: { Toggle, OptionPicker, Spinner }
  }
</script>

<style scoped>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
</style>
