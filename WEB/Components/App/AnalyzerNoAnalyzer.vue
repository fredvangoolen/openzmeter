<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop='false'>
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">Analyzers</h4></div>
          <div class="pull-right">
            <button type="button" class="btn btn-default btn-xs" data-dismiss="modal"><img src="Imgs/arrow_undo.png"></button>
          </div>
        </div>
        <div class="modal-body">
          <p>There are no analyzers available.</p>
          <p>If you are an administrator, check in the 'Settings' section that there are capture devices, and that these are configured.</p>
          <p>Otherwise check with the owner of the desired analyzer for permission.</p>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    data() {
      return {};
    },
    methods: {
      show() { 
        $(this.$el).modal('show');
      }
    }
  }
</script>

<style>
  .modal-dialog {
    z-index: 1100;
    color: #333;
  }
</style>
