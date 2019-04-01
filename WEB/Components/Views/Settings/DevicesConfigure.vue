<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <device-virtual ref="Virtual"></device-virtual>
    </div>
  </div>
</template>

<script>
  import DeviceVirtual from './DevicesVirtual.vue';
  export default {
    data() {
      return { ShowDialog: false };
    },
    methods: {
      Show: function(dev) {
        this.ShowDialog = true;
//        this.Analyzers = analyzers;
        this.$nextTick(function() {     
          this.$refs.Virtual.Load(null);
//          if(analyzers.length === 0) {
//            this.$refs.NotFound.Load();
//          } else {
//            this.$refs.Table.Load();
//          }
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