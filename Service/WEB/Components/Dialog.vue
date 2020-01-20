<template>
  <portal to="dialogs">
    <div class="modal" tabindex="1" role="dialog" data-backdrop="false" data-keyboard="false" ref="dialog">
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
          <div class="panel-footer" v-if="$slots['footer']">
            <slot name="footer"></slot>
          </div>
        </div>
      </div>
    </div>
  </portal>
</template>

<script>
  export default {
    props: { loading: Boolean },
    data: function() {
      return { };
    },

    mounted: function() {
//      this.$root.$el.append(this.$el);
      this.$nextTick(() => {
        $(this.$refs.dialog).modal('show');
      });
    }
  }
</script>

<style scoped>
  .modal-header {
    padding: 3px 5px;
  }
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
