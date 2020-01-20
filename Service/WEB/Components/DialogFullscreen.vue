<template>
  <portal to="dialogs">
    <div class="modal" tabindex="1" role="dialog" data-backdrop="false" data-keyboard="false" ref="dialog">
      <div class="modal-backdrop in"></div>
      <div class="modal-dialog">
        <div class="modal-content" ref="body">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">{{ title }}</h4></div>
            <portal-target name="fs-toolbar" class="pull-right clearfix"></portal-target>
          </div>
          <div class="modal-body">
            <portal-target name="fs-body"></portal-target>
          </div>
        </div>
      </div>
    </div>
  </portal>
</template>

<script>
  export default {
    props: { title: String, height: Number },
    data: function() {
      return { };
    },
    methods: {
      ResizeHandler: function() {
        this.$emit('update:height', $(this.$refs.body).height() - 60);
      }
    },
    beforeDestroy: function() {
      $(window).off('resize', this.ResizeHandler);
    },
    mounted: function() {
      $(window).on('resize', this.ResizeHandler);
      this.$nextTick(() => {
        $(this.$refs.dialog).modal('show');

        window.dispatchEvent(new Event('resize'));
      });
    }
  }
</script>

<style scoped>
  .modal-header {
    padding: 3px 5px;
  }
  .pull-right {
    margin-top: 1px;
  }
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
    width: 100%;
    height: 100%;
    padding: 0px;
    margin: 0px !important;
  }
  @media only screen and (max-width: 768px) {
  .modal-dialog {
     margin-top:120px !important;
    }
  }
  .modal-content {
    height: 100%;
  }
  @media (min-width: 640px) {
    .modal-dialog {
      padding: 10px;
    }
  }
</style>
