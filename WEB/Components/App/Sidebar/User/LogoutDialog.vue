<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">User</h4></div>
        </div>
        <div class="modal-body">
          <p>The current session has expired due to inactivity. Please re-login</p>
          <p><a href="/Login">Login</a></p>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    data() {
      return { ShowDialog: false };
    },
    methods: {
      show: function() {
        this.ShowDialog = true;
        this.$nextTick(function() {
          $(this.$el).modal('show');
        });
      }
    },
    mounted: function() {
      this.$root.$el.append(this.$el);
    },
    destroyed: function() {
      this.$root.$el.removeChild(this.$el);
    }
  }
</script>

<style>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
  .modal-dialog p {
    text-align: center;
  }
  .modal-dialog a {
    font-size: 24px;
  }
</style>