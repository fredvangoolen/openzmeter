<template>
  <div id="app">
    <div id="DisableDiv" class="overlay"></div>
    <topbar ref="indicator_bar"></topbar>
    <sidebar ref="sidebar"></sidebar>
    <router-view id="contents"></router-view>
  </div>
</template>

<script>
  import Topbar from './Topbar.vue';
  import Sidebar from './Sidebar.vue';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  import Graph from 'Components/Graph.vue';
  export default {
    data: function() {
      return { };
    },
    methods: {
      UpdateIndicators1() {
        if(this.UpdateIndicatorsTimer1) clearInterval(this.UpdateIndicatorsTimer1);
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['VOLTAGE', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY', 'SYS_STATS'], Analyzer: this.$root.SelectedAnalyzer.Analyzer }), 
          success: function(result) {
            this.$refs.indicator_bar.setValue(result);
          }.bind(this),
          complete: function() {
            this.UpdateIndicatorsTimer1 = setTimeout(this.UpdateIndicators1, 2000);
          }.bind(this), 
          error: function() {
            this.$refs.indicator_bar.setValue(null);
          }.bind(this)
        });
      },
      UpdateIndicators2() {
        if(this.UpdateIndicatorsTimer2) clearInterval(this.UpdateIndicatorsTimer2);
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['ACTIVE_POWER'], LastSample: 0, Analyzer: this.$root.SelectedAnalyzer.Analyzer, Aggreg: '15M', To: moment().endOf('day').valueOf(), Period: 86400000 }), 
          success: function(result) {
            var Energy;
            if(result['Active_Power'].length > 0) Energy = Array(result['Active_Power'][0].length).fill(0);
            for(var n = 0; n < result['Active_Power'].length; n++) {
              for(var p = 0; p < result['Active_Power'][n].length; p++) {
                if(result['Active_Power'][n][p] === null) continue;
                Energy[p] = Energy[p] + result['Active_Power'][n][p] / 4.0;
              }
            }
            this.$refs.indicator_bar.setValue({ Active_Energy: Energy });
          }.bind(this),
          complete: function() {
            this.UpdateIndicatorsTimer2 = setTimeout(this.UpdateIndicators2, 1000000);
          }.bind(this), 
          error: function() {
            this.$refs.indicator_bar.setValue(null);
          }.bind(this)
        });
      },
      SetAnalyzer(analyzer) {
        this.UpdateIndicators1();
        this.UpdateIndicators2();
      }
    },
    created: function() {
      this.UpdateIndicatorsTimer1 = null;
      this.UpdateIndicatorsTimer2 = null;
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
  .modal-dialog.full-screen-dialog{
    width: 100%; 
    height: 100%; 
    padding: 0px; 
    margin: 0px !important;
  }
  @media (min-width: 640px) {
    #contents {
      top: 56px;
    }
    .modal-dialog.full-screen-dialog{
      padding: 10px; 
    }
  }
  @media (min-width: 1280px) {
    #contents {
      left: 256px;
    }
  }
</style>
