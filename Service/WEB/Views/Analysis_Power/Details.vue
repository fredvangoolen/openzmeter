<template>
  <div>
    <detail-tabs :tabs="['Waveform', 'Harmonics', 'Evolution', 'Distribution', 'Parameters']" variable="PowerDetailTab" v-on:TabChange="TabChange"></detail-tabs>
    <waveform ref="waveform" v-bind:class="{ hide: (ActiveTab != 'Waveform') }"></waveform>
    <harmonics ref="harmonics" v-bind:class="{ hide: (ActiveTab != 'Harmonics') }"></harmonics>
    <evolution ref="evolution" v-bind:class="{ hide: (ActiveTab != 'Evolution') }"></evolution>
    <distribution ref="distribution" v-bind:class="{ hide: (ActiveTab != 'Distribution') }"></distribution>
    <parameters ref="parameters" v-bind:class="{ hide: (ActiveTab != 'Parameters') }"></parameters>
  </div>
</template>

<script>
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  import Parameters from './Parameters.vue';
  import Waveform from './Waveform.vue';
  import Harmonics from './Harmonics.vue';
  import Evolution from './Evolution.vue';
  import Distribution from './Distribution.vue';
  export default {
    data () {
      return { ActiveTab: '' };
    },
    created: function() {
      this.Ajax = null;
      this.LastSample = 0;
//      this.CurrentMultiplier = 1.0;
      this.UpdateGraphs1MTimer = null;
      this.UpdateGraphsNowTimer = null;
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      if(this.$root.SelectedAnalyzer !== null) {
        this.UpdateGraphs1M();
        this.UpdateGraphsNow();
      }
      this.TabChange();
    },
    beforeDestroy: function() {
      if(this.UpdateGraphs1MTimer !== null) clearTimeout(this.UpdateGraphs1MTimer);
      if(this.UpdateGraphsNowTimer !== null) clearTimeout(this.UpdateGraphsNowTimer);
    },
    methods: {
      PlayStatusChanged: function(stat) {
        if(stat === true) {
          this.Ajax.abort();
          clearInterval(this.UpdateGraphsNowTimer);
        } else {
          this.UpdateGraphsNow();
        }
        this.$refs.waveform.$refs.tools.SetPlayStatus(stat);
      },
      TabChange(tab, name) {
        if(name !== undefined) this.ActiveTab = name;
        if(this.ActiveTab === 'Waveform') this.$refs.waveform.Redraw();
        if(this.ActiveTab === 'Harmonics') this.$refs.harmonics.Redraw();
        if(this.ActiveTab === 'Evolution') this.$refs.evolution.Redraw();
        if(this.ActiveTab === 'Distribution') this.$refs.distribution.Redraw();
        if(this.ActiveTab === 'Parameters') this.$refs.parameters.Redraw();
      },
      PhasesChangedSingle: function(phase) {
        this.$refs.parameters.SetPhases(phase);
        this.$refs.evolution.SetPhases(phase);
      },
      PhasesChangedMulti: function(phases) {
        this.$refs.waveform.SetPhases(phases);
        this.$refs.harmonics.SetPhases(phases);
      },      
      SetAnalyzer() {
        this.LastSample = 0;
        this.UpdateGraphs1M();
        this.UpdateGraphsNow();
      },
      UpdateGraphs1M() {
        if(this.$root.SelectedAnalyzer === null) return;
        this.Ajax = $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '1M', Period: 86400000, To: 0, LastSample: this.LastSample, Series: ['POWER_HARMONICS', 'ACTIVE_POWER', 'REACTIVE_POWER', 'VOLTAGE', 'CURRENT', 'POWER_FACTOR', 'PHI'], Analyzer: this.$root.SelectedAnalyzer.Analyzer }),
          success: function(result) {
            result['Apparent_Power'] = [];        
            for(var n = 0; n < result['Sampletime'].length; n++) result['Apparent_Power'].push(result['Voltage'][n].map( function(val, index) { return val * result['Current'][n][index]; }));
            this.$refs.evolution.SetData(result);
            this.$refs.parameters.SetData(result);
            this.LastSample = oZm.LastSampletime(result, this.LastSample);  
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphs1MTimer !== null) clearTimeout(this.UpdateGraphs1MTimer);
            this.UpdateGraphs1MTimer = setTimeout(this.UpdateGraphs1M, 30000);
          }.bind(this)
        });
      },
      UpdateGraphsNow() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'POWER_HARMONICS', 'CURRENT', 'ACTIVE_POWER', 'REACTIVE_POWER', 'APPARENT_POWER', 'POWER_FACTOR', 'PHI'], Analyzer: this.$root.SelectedAnalyzer.Analyzer }), 
          success: function(result) {
            this.$refs.waveform.SetData(result);
            this.$refs.harmonics.SetData(result);
            this.$refs.distribution.SetData(result);
            this.$refs.parameters.SetDataNow(result);
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphsNowTimer !== null) clearTimeout(this.UpdateGraphsNowTimer);
            this.UpdateGraphsNowTimer = setTimeout(this.UpdateGraphsNow, 1000);
          }.bind(this)
        });
      }
    },
    components: { Distribution, Waveform, Harmonics, Evolution, Parameters }
  }
</script>

<style scoped>
  .panel-other {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
</style>