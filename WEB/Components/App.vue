<template>
  <div id="app">
    <!--BlockUI :message="msg" :html="html"></BlockUI-->
    <div id="DisableDiv" class="overlay"></div>
    <topbar ref="indicator_bar"></topbar>
    <sidebar></sidebar>
    <router-view id="contents"></router-view>
  </div>
</template>

<script>
import Topbar from './Topbar.vue';
import Sidebar from './Sidebar.vue';
export default {
  name: 'app',
  data () {
    return {
      Analyzer: null, UpdateIndicatorsTimer: null,
      
      msg: 'Welcome to Your Vue.js App',
      html: '<i class="fa fa-cog fa-spin fa-3x fa-fw"></i>'
    };
  },
  methods: {
    UpdateIndicators() {
      $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['VOLTAGE', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY'], Analyzer: 'VIRTUAL_1PHASE' }), 
        success: function(result) {
          this.$refs.indicator_bar.setValue(result);
        }.bind(this),
        complete: function() {
          if(this.UpdateIndicatorsTimer !== null) clearTimeout(this.UpdateIndicatorsTimer);
          this.UpdateIndicatorsTimer = setTimeout(this.UpdateIndicators, 5000);
        }.bind(this), 
        error: function() {
          this.$refs.indicator_bar.setValue(null);
        }.bind(this)
      });
    }
  },
//  beforeUpdate: function() { this.UpdateIndicators(); },
  mounted: function() { this.UpdateIndicators(); },
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
