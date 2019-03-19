<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">User</h4></div>
          <div class="pull-right">
            <button type="button" class="btn btn-default btn-xs" @click="close"><img src="Imgs/arrow_undo.png"></button>
          </div>
        </div>
        <div class="modal-body">
          <p>The current session has expired due to inactivity. Please re-login.</p>
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
      close() {
        window.location.href = 'Logout';
      }
    },
    mounted: function() {
      $(this.$el).modal('show');
    }
  }
</script>

<style>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
</style>