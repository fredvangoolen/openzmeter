<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">Wifi settings</h4></div>
          <div class="pull-right btn-group">
            <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Error != '') }" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save tariff"></button>
            <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
          </div>
        </div>
        <div class="modal-body">
          <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != ''">{{ Error }}</div>
          <div class="row">
            <div class="col-md-12">
              <label class="control-label">SSID</label>
              <input type="text" class="form-control" ref="ssid">                             
            </div>
            <div class="col-md-12 col-lg-6">
              <label class="control-label">Password</label>
              <input type="password" class="form-control" ref="password">
            </div>
            <div class="col-md-12 col-lg-6">
              <label class="control-label">Restrict to BSSID</label>
              <div class="input-group">
                <span class="input-group-addon input-group-addon-toogle">
                  <toggle type="checkbox" ref="restrict_bssid" data-size="small" data-onstyle="success"></toggle>
                </span>
                <input type="text" class="form-control" ref="bssid" placeholder="Blank for any BSSID">
              </div>
            </div>
            <div class="col-md-6 col-lg-3">
              <label class="control-label">Network type</label>
              <option-picker :options="['Open', 'WEP', 'WPA']" :big="true"></option-picker>
            </div>
            <div class="col-md-6 col-lg-3">
              <label class="control-label">Network address</label>
              <option-picker :options="['DHCP', 'Static']" :big="true"></option-picker>
            </div>
            <div class="col-md-12 col-lg-6">
              <label class="control-label">IP address</label>
              <input type="text" class="form-control" id="NetworkIP">
            </div>
            <div class="col-md-12 col-lg-6">
              <label class="control-label">Network mask</label>
              <input type="text" class="form-control" id="NetworkIP">
            </div>
            <div class="col-md-12 col-lg-6">
              <label class="control-label">Gateway address</label>
              <input type="text" class="form-control" id="NetworkGateway">
            </div>
            <div class="col-md-12 col-lg-6">
              <label class="control-label">Primary DNS address</label>
              <input type="text" class="form-control" id="NetworkDNS0">
            </div>
            <div class="col-md-12 col-lg-6">
              <label class="control-label">Secondary DNS address</label>
              <input type="text" class="form-control" id="NetworkDNS1">
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
  export default {
    data() {
      return { Error: '', ShowDialog: false };
    },
    methods: {
      Show: function(config) {
        this.ShowDialog = true;
        this.$nextTick(function() {     
          $(this.$el).modal('show');
        });
      },
      Close: function() {
        $(this.$el).modal('hide');
        this.ShowDialog = false;
      }
    },
    mounted: function() {
      this.$root.$el.append(this.$el);
    },
    destroyed: function() {
      this.$root.$el.removeChild(this.$el);
    },
    components: { Toggle, OptionPicker }
  }
</script>

<style scoped>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
</style>
