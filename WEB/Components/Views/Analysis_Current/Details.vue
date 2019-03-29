<template>
  <div>
    <detail-tabs :tabs="['Waveform', 'Harmonics', 'Evolution', 'Advanced', 'Parameters']" variable="CurrentDetailTab" v-on:TabChange="TabChange"></detail-tabs>
    <waveform ref="waveform" v-bind:class="{ hide: (ActiveTab != 'Waveform') }"></waveform>
    <harmonics ref="harmonics" v-bind:class="{ hide: (ActiveTab != 'Harmonics') }"></harmonics>
    <evolution ref="evolution" v-bind:class="{ hide: (ActiveTab != 'Evolution') }"></evolution>
    <advanced ref="advanced" v-bind:class="{ hide: (ActiveTab != 'Advanced') }"></advanced>
    <parameters ref="parameters" v-bind:class="{ hide: (ActiveTab != 'Parameters') }"></parameters>
  </div>
</template>

<script>
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  import Spinner from 'Components/Spinner.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarOptions from 'Components/ToolbarOptions.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Advanced from './Advanced.vue';
  import Evolution from './Evolution.vue';
  import Harmonics from './Harmonics.vue';
  import Parameters from './Parameters.vue';
  import Waveform from './Waveform.vue';
  import DetailTabs from 'Components/Tabs.vue';
  export default {
    data() {
      return { ActiveTab: '', Paused: false };
    },
    created: function() {
      this.LastSample = 0;
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
        this.Paused = stat;
        if(this.Paused === true) 
          clearInterval(this.UpdateGraphsNowTimer);
        else
          this.UpdateGraphsNow();
        this.$refs.waveform.$refs.tools.SetPlayStatus(stat);
        this.$refs.harmonics.$refs.tools.SetPlayStatus(stat);
        this.$refs.advanced.$refs.tools.SetPlayStatus(stat);
      },
      TabChange(tab, name) {
        if(name !== undefined) this.ActiveTab = name;
        if(this.ActiveTab === 'Waveform') this.$refs.waveform.Redraw();
        if(this.ActiveTab === 'Harmonics') this.$refs.harmonics.Redraw();
        if(this.ActiveTab === 'Evolution') this.$refs.evolution.Redraw();
        if(this.ActiveTab === 'Advanced') this.$refs.advanced.Redraw();
        if(this.ActiveTab === 'Parameters') this.$refs.parameters.Redraw();
      },
      PhasesChangedMulti: function(phases) {
        this.$refs.waveform.SetPhases(phases);
        this.$refs.harmonics.SetPhases(phases);
        this.$refs.advanced.SetPhases(phases);
      },
      PhasesChangedSingle: function(phase) {
        this.$refs.evolution.SetPhases(phase);
        this.$refs.parameters.SetPhases(phase);
      },
      SetAnalyzer() {
        this.Time = 'Loading data...';
        this.LastSample = 0;
        this.UpdateGraphs1M();
        this.UpdateGraphsNow();
      },
      UpdateGraphs1M() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '1M', Period: 86400000, To: 0, LastSample: this.LastSample, Series: ['CURRENT_HARMONICS', 'POWER_FACTOR', 'CURRENT_THD', 'PHI'], Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
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
        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'VOLTAGE_HARMONICS', 'CURRENT_HARMONICS', 'VOLTAGE_FFT', 'CURRENT_FFT', 'POWER_FACTOR', 'CURRENT_THD', 'PHI'], Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            this.$refs.waveform.SetData(result);
            this.$refs.harmonics.SetData(result);
            this.$refs.advanced.SetData(result);
            this.$refs.parameters.SetDataNow(result);
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphsNowTimer !== null) clearTimeout(this.UpdateGraphsNowTimer);
            this.UpdateGraphsNowTimer = setTimeout(this.UpdateGraphsNow, 1000);
          }.bind(this)
        });
      }
    },
    components: { Spinner, DetailTabs, Waveform, Harmonics, Evolution, Parameters, Advanced, ToolbarPhases, ToolbarOptions, ToolbarTools }
  }
</script>