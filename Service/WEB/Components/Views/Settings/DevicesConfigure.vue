<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">Device &raquo; {{ Serial }}</h4></div>
          <div class="pull-right btn-group">
            <button type="button" class="btn btn-default btn-xs" v-on:click="Save"><img :src="require('Imgs/diskette.png')"></button>
            <button type="button" class="btn btn-default btn-xs" v-on:click="Close"><img :src="require('Imgs/arrow_undo.png')"></button>
          </div>
        </div>
        <div class="modal-body">
          <spinner v-if="ShowSpinner"></spinner>
          <device-virtual v-if="(ShowSpinner == false) && (Driver == 'Virtual')" ref="options" :settings="Settings"></device-virtual>
          <device1 v-if="(ShowSpinner == false) && (Driver == 'V1.01')" ref="options" :settings="Settings"></device1>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import DeviceVirtual from './DeviceVirtual.vue';
  import Device1 from './Device1.vue';
  import Toggle from 'Components/Toggle.vue';
  import Spinner from 'Components/Spinner.vue';
  export default {
    data() {
      return { ShowSpinner: true, ShowDialog: false, Serial: '', Driver: '', Settings: {} };
    },
    methods: {
      Load: function(dev) {
        this.Serial = dev.Serial;
        this.ShowDialog = true;
        this.Driver = dev.Driver;
        this.$nextTick(function() {
          $(this.$el).modal('show');
        });
        $.ajax({ url: "getDevice", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Serial: dev.Serial }),
          success: function(result) {
            this.ShowSpinner = false;
            this.Settings = result;
          }.bind(this)
        });
      },
      Close: function() {
        $(this.$el).modal('hide');
        this.ShowDialog = false;
      },
      Save: function() {
        
      }
    },
    mounted: function() {
      this.$root.$el.append(this.$el);
    },
    destroyed: function() {
      this.$root.$el.removeChild(this.$el);
    },
    components: { Toggle, DeviceVirtual, Device1, Spinner }
  }
</script>

<style scoped>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
  .checkbox {
    margin-top: 1px;
    margin-bottom: 0px;
    margin-right: 4px;
  }
  .modal-body {
    min-height: 100px;
  }
</style>