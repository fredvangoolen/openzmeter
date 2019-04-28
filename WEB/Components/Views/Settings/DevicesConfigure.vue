<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">Device &raquo; {{ Serial }}</h4></div>
          <div class="pull-right btn-group">
            <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')"></button>
            <button type="button" class="btn btn-default btn-xs" @click="Close"><img :src="require('Imgs/arrow_undo.png')"></button>
          </div>
        </div>
        <div class="modal-body">
          <spinner v-if="ShowSpinner"></spinner>
          <device-virtual v-if="Driver == 'Virtual'" ref="options" :settings="Settings"></device-virtual>
          <device1 v-if="Driver == 'V1.01'" ref="options" :settings="Settings"></device1>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import DeviceVirtual from './DeviceVirtual.vue';
  import Device1 from './Device1.vue';
  
  export default {
    data() {
      return { ShowSpinner: false, ShowDialog: false, Serial: '', Driver: '', Settings: {} };
    },
    methods: {
      Load: function(dev) {
        this.Serial = dev.Serial;
        this.ShowDialog = true;
        $.ajax({ url: "getDevice", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Serial: dev.Serial }),
              success: function(result) {
                this.Settings = result;
                this.$nextTick(function() {
                  $(this.$el).modal('show');
                });
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
    components: { DeviceVirtual }
  }
</script>

<style scoped>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
</style>