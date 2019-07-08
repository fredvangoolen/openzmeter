<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">Connection lost</h4></div>
        </div>
        <div class="modal-body">
          <p>Connection to the server is lost, please check your internet connection.</p>
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
      Logout: function() {
        window.location.href = 'Logout';
      },
      show: function() {
        this.ShowDialog = true;
        this.$nextTick(function() {
          $(this.$el).modal('show');
        });
      },
      hide: function() {
        $(this.$el).modal('hide');
        this.$nextTick(function() {
          this.ShowDialog = false;
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

<style scoped>
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