<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <slot name="header"></slot>
        </div>
        <div class="modal-body">
          <spinner v-if="loading"></spinner>
          <slot></slot>          
        </div>
      </div>
    </div>
  </div>
</template>
  
<script>
  export default {
    name: 'Dlg',
    props: {loading: {type: Boolean, default: false}},
    data: function() {
      return { };
    },
    mounted: function() {
      this.$root.$el.append(this.$el);
      this.$nextTick(() => {
        $(this.$el).modal('show');
      });
    }
  }
</script>

<style scoped>
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
