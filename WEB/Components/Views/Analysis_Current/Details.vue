<template>
  <div>
    <detail-tabs :tabs="['Waveform', 'Harmonics', 'Evolution', 'Advanced', 'Parameters']" variable="CurrentDetailTab" v-on:TabChange="TabChange"></detail-tabs>
    <div class="panel panel-other">
      <div class="panel-heading clearfix">
        <div class="header-text">{{ Time }}</div>
          <div id="LowWarning" class="title-warning hidden"><i class="fa fa-exclamation-triangle"></i> No current or current too low</div>
          <div class="header-buttons">
            <toolbar-phases></toolbar-phases>
            <toolbar-options :options="['1st Harmonic', 'Current', 'Log']"></toolbar-options>
            <toolbar-tools></toolbar-tools>
          </div>
        </div>
        <div class="panel-body">
          <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
          <waveform ref="waveform" :visible="(ActiveTab == 'Waveform')"></waveform>
          <harmonics ref="harmonics" :visible="(ActiveTab == 'Harmonics')"></harmonics>
          <evolution ref="evolution" v-bind:class="{ hide: (ActiveTab != 'Evolution') }"></evolution>
          <advanced ref="advanced" :visible="(ActiveTab == 'Advanced')"></advanced>
          <parameters ref="parameters" v-bind:class="{ hide: (ActiveTab != 'parameters') }"></parameters>
        </div>
    </div>
  </div>
</template>

<script>
  import moment from 'moment';
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
      return { ActiveTab: '', Time: 'Loading data...', LastSample: 0, UpdateGraphs1MTimer: null, UpdateGraphsNowTimer: null, ShowSpinner: true };
    },
    created: function() {
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      if(this.$root.SelectedAnalyzer !== null) {
        this.UpdateGraphs1M();
        this.UpdateGraphsNow();
      }
    },
    beforeDestroy: function() {
      if(this.UpdateGraphs1MTimer !== null) clearTimeout(this.UpdateGraphs1MTimer);
      if(this.UpdateGraphsNowTimer !== null) clearTimeout(this.UpdateGraphsNowTimer);
    },
    methods: {
      TabChange(tab, name) {
        this.ActiveTab = name;
      },
      SetAnalyzer() {
        this.Time = 'Loading data...';
        this.LastSample = 0;
        this.UpdateGraphs1M();
        this.UpdateGraphsNow();
      },
UpdateGraphs1M() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['VOLTAGE_SAMPLES', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY'], Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
//            this.$refs.evolution.SetData(result);
//            this.$refs.parameters.SetData(result);
    //            this.$refs.indicator_bar.setValue(result);
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphs1MTimer !== null) clearTimeout(this.UpdateGraphs1MTimer);
            this.UpdateGraphs1MTimer = setTimeout(this.UpdateGraphs1M, 30000);
          }.bind(this), 
          error: function() {
  //deshabilitar¿?
    //            this.$refs.indicator_bar.setValue(null);
          }.bind(this)
        });
      },
      UpdateGraphsNow() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'VOLTAGE_HARMONICS', 'CURRENT_HARMONICS', 'VOLTAGE_FFT', 'CURRENT_FFT'], Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            this.ShowSpinner = false;
            this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
            this.$refs.waveform.SetData(result);
            this.$refs.harmonics.SetData(result);
            this.$refs.advanced.SetData(result);
//  SetCubismNowLabel(PowerFactorChart, result.Power_Factor);
//  SetCubismNowLabel(THDChart, result.Voltage_THD); 
//  SetCubismNowLabel(PHIChart, result.Phi); 
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphsNowTimer !== null) clearTimeout(this.UpdateGraphsNowTimer);
            this.UpdateGraphsNowTimer = setTimeout(this.UpdateGraphsNow, 1000);
          }.bind(this), 
          error: function() {
//deshabilitar¿?
    //            this.$refs.indicator_bar.setValue(null);
          }.bind(this)
        });
      }
    },
    components: { Spinner, DetailTabs, Waveform, Harmonics, Evolution, Parameters, Advanced, ToolbarPhases, ToolbarOptions, ToolbarTools }
  }
</script>