<template>
  <div class="modal" tabindex="-1" role="dialog" data-backdrop='false'>
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <edit-main v-if="Frame === 'Main'"></edit-main>




    </div>
  </div>
</template>

<script>
  import EditMain from './TariffsEditMain.vue';
  export default {
    data: function() {
      return { Frame: 'Main' };
    },
    mounted: function() {
      $(this.$el).modal('show');
      this.$root.$el.append(this.$el);
    },
    destroyed: function() {
      this.$el.parentNode.removeChild(this.$el);
    },
    components: { EditMain }
  }
</script>

<style scoped>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
    margin-top: 70px;
  }
</style> 
 