<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><active-power></active-power></div>
      <div class="col-lg-12"><reactive-power></reactive-power></div>
      <div class="col-lg-12"><details-power></details-power></div>
    </div>
  </div>
</template>

<script>
  import ActivePower from './ActivePower.vue';
  import ReactivePower from './ReactivePower.vue';
  import DetailsPower from './DetailsPower.vue';
  export default {
    data () {
      return { };
    },
    created: function() {
      this.LastSample = 0;
    },
    mounted: function() {
      window.dispatchEvent(new Event('resize'));
      this.UpdateGraph();
      this.NowValue();
    },
    beforeDestroy: function() {
      clearInterval(this.Timer);
      clearInterval(this.NowTimer);
      $(window).off('resize', this.ResizeHandler);
    },
    methods: {
      SetAnalyzer: function() {
        this.LastSample = 0;
        this.UpdateGraph();
        this.NowValue();
      },
      NowValue: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['ACTIVE_POWER', 'REACTIVE_POWER'], Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
//            this.CurrentValue = ((result['Frequency'] === null) ? 'No data' : oZm.Round3(result['Frequency']) + '<em>Hz</em>');
          }.bind(this),
          complete: function() {
            clearTimeout(this.NowTimer);
            this.NowTimer = setTimeout(this.NowValue.bind(this), 2000);
          }.bind(this), 
          error: function() {
//            this.CurrentValue = 'No data';
          }.bind(this)
        });
      },
    },
    components: { ActivePower, ReactivePower, DetailsPower }
  }
</script>