<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop='false' @close="$emit('close')">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <component :is="Frame" ref="frame" :params="ChildParams"></component>
    </div>
  </div>
</template>

<script>
    import AnalyzerNotFound from './AnalyzerNotFound.vue';
    import AnalyzerTable from './AnalyzerTable.vue';
    import AnalyzerConfigure from './AnalyzerConfigure.vue';
    import AnalyzerTariffs from './AnalyzerTariffs';
  export default {
    props: ['analyzers'],
    data() {
      return { ChildParams: null, Frame: null };
    },
    methods: {
      show(frame, params) {
        $(this.$el).modal('show');
        this.Frame = frame;
        this.ChildParams = params;
//        this.$refs.frame.SetData(params);
      }
    },
    mounted: function() {
      $(this.$el).modal('show');
      if(this.analyzers.length === 0) {
        this.Frame = 'AnalyzerNotFound';
      } else {
        this.Frame = 'AnalyzerTable';
      }
    },
    components: { AnalyzerNotFound, AnalyzerTariffs, AnalyzerConfigure, AnalyzerTable }
  }
</script>

<style>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
</style>