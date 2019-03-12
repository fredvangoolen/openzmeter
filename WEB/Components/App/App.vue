<template>
  <div id="app">
    <div id="DisableDiv" class="overlay"></div>
    <topbar ref="indicator_bar"></topbar>
    <sidebar></sidebar>
    <router-view id="contents"></router-view>
  </div>
</template>

<script>
import Topbar from './Topbar/Topbar.vue';
import Sidebar from './Sidebar/Sidebar.vue';
export default {
  data: function() {
    return { UpdateIndicatorsTimer: null };
  },
  methods: {
    UpdateIndicators() {
      if(this.$root.SelectedAnalyzer === null) return;
      $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['VOLTAGE', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY'], Analyzer: this.$root.SelectedAnalyzer.ID }), 
        success: function(result) {
          this.$refs.indicator_bar.setValue(result);
        }.bind(this),
        complete: function() {
          if(this.UpdateIndicatorsTimer !== null) clearTimeout(this.UpdateIndicatorsTimer);
          this.UpdateIndicatorsTimer = setTimeout(this.UpdateIndicators, 2000);
        }.bind(this), 
        error: function() {
          this.$refs.indicator_bar.setValue(null);
        }.bind(this)
      });
    },
    SetAnalyzer() {
      this.UpdateIndicators();  
    }
  },
  created: function() {
    this.$root.SelectedAnalyzer = null;
    this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
  },
  components: { Topbar, Sidebar }
}
</script>

<style>
  #contents {
    position: fixed;
    z-index: 0;
    padding: 10px;
    overflow-x: hidden;
    overflow-y: auto;
    left: 0px;
    top: 116px;
    bottom: 0px;
    right: 0px;
  }
  @media (min-width: 768px) {
    #contents {
      top: 56px;
    }
  }
  @media (min-width: 1024px) {
    #contents {
      left: 256px;
    }
  }
</style>
