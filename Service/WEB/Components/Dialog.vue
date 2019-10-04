<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <slot name="header"></slot>
        </div>
        <div class="modal-body">
          <spinner v-if="ShowSpinner"></spinner>
          <slot></slot>          
        </div>
      </div>
    </div>
  </div>
</template>
  
<script>
  import Spinner from 'Components/Spinner.vue';
  export default {
    name: 'Dlg',
    data: function() {
      return { ShowDialog: false, ShowSpinner: false };
    },
    methods: {
      Show: function() {
        this.ShowDialog = true;
        this.$nextTick(() => {
          $(this.$el).modal('show');
        });
      },
      SpinnerState: function(val) {
        this.ShowSpinner = val;
      },
      Close: function() {
        $(this.$el).modal('hide');
        this.ShowDialog = false;
      },
    },
    mounted: function() {
      this.$root.$el.append(this.$el);
    },
    destroyed: function() {
      this.$root.$el.removeChild(this.$el);
    },
    components: { Spinner }
  }
</script>

<style>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
  @media only screen and (max-width: 768px) {
  .modal-dialog {
    margin-top:120px !important;
  }
}
</style>
