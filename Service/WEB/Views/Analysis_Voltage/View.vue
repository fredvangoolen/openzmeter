<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-voltage">
          <div class="panel-heading">
            <div class="header-text">Voltage</div>
            <div class="header-buttons clearfix" ref="Toolbar">
              <toolbar-aggregation storage="Aggregation" :options="['3S', '1M', '10M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <!--toolbar-phases variable="VoltageViewPhases" type="VOLTAGE" :multi="true" v-on:PhasesChanged="PhasesChanged"></toolbar-phases>
              <toolbar-tools :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools-->
            </div>
          </div>
          <div class="panel-body">
            <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" :height.sync="Height"></counters>
              <voltage class="col-md-6 col-lg-8 col-xl-9" :phases="Phases" :analyzer="analyzer" :data="GraphData"></voltage>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <detail-tabs :tabs="['Waveform', 'Harmonics', 'Evolution', 'Advanced', 'Phasors', 'Parameters', 'Events']" storage="VoltageDetailTab" v-model="ActiveTab"></detail-tabs>
          <waveform v-if="ActiveTab=='Waveform'" :data="NowData" :analyzer="analyzer" :paused.sync="Paused" :phases.sync="PhasesMulti"></waveform>
          <!--harmonics v-if="ActiveTab=='Harmonics'" :data="NowData" :analyzer="analyzer" :paused.sync="Paused" :phases.sync="PhasesMulti"></harmonics>
          <evolution v-if="ActiveTab=='Evolution'" :data="CubismData" :analyzer="analyzer" :paused.sync="Paused" :phase.sync="PhaseSingle"></evolution-->
          <advanced v-if="ActiveTab=='Advanced'" :data="NowData" :analyzer="analyzer" :paused.sync="Paused" :phases.sync="PhasesMulti"></advanced>
          <!--parameters v-if="ActiveTab=='Parameters'" :data="CubismData" :analyzer="analyzer" :paused.sync="Paused" :phase.sync="PhaseSingle"></parameters>
          <phasors v-if="ActiveTab=='Phasors'" :data="NowData" :analyzer="analyzer" :paused.sync="Paused"></phasors>
          <events v-if="ActiveTab=='Events'" :analyzer="analyzer" :range="Range"></events-->
      </div>
    </div>
  </div>
  <div v-else-if="analyzer == null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>
  </div>
</template>

<script>
  import Voltage from './Voltage.vue';
  import Counters from './Counters.vue';
  import Advanced from './Advanced.vue';
  import Evolution from './Evolution.vue';
  import Harmonics from './Harmonics.vue';
  import Parameters from './Parameters.vue';
  import Waveform from './Waveform.vue';
  import Events from './Events.vue';
  import Phasors from './Phasors.vue';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { ShowSpinner: false, ActiveTab: '', Aggreg: '', Range: null, GraphData: {}, GraphTimer: null, Phases: [], Height: 300,
               NowData: {}, NowTimer: null, CubismData: {}, CubismTimer: null, PhasesMulti: [], PhaseSingle: 0, Paused: false };
    },
    created: function() {
      this.UpdateNowValues();
      this.UpdateGraphValues();
      this.UpdateCubismValues();
    },
    beforeDestroy: function() {
      if(this.NowTimer != null) clearTimeout(this.NowTimer);
      if(this.GraphTimer != null) clearTimeout(this.GraphTimer);
      if(this.CubismTimer != null) clearTimeout(this.CubismTimer);
    },
    watch: {
      analyzer: function(val) {
        this.UpdateNowValues();
        this.UpdateGraphValues();
        this.UpdateCubismValues();
      }
    },
    methods: {
      UpdateNowValues: function() {
        if(this.NowTimer != null) clearTimeout(this.NowTimer);
        if(this.analyzer != null) this.$http.post('getSeriesNow', { Series: ['VOLTAGE', 'CURRENT', 'VOLTAGE_SAMPLES', 'VOLTAGE_PHASE', 'CURRENT_SAMPLES', 'VOLTAGE_HARMONICS', 'CURRENT_HARMONICS', 'VOLTAGE_FFT', 'CURRENT_FFT', 'POWER_FACTOR', 'VOLTAGE_THD', 'PHI'], Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.NowData = (result.status == 200) ? Object.freeze(result.data) : null;
            this.NowTimer = setTimeout(() => { this.UpdateNowValues(); }, 2000);
          }
        );
      },
      UpdateCubismValues: function() {

      },
      UpdateGraphValues: function() {
        return;
        if(this.$root.SelectedAnalyzer === null) return;
        this.Ajax = $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '1M', Period: 86400000, To: 0, LastSample: this.LastSample, Series: ['VOLTAGE_HARMONICS', 'POWER_FACTOR', 'VOLTAGE_THD', 'PHI'], Analyzer: this.$root.SelectedAnalyzer.Analyzer }),
          success: function(result) {
            this.$refs.evolution.SetData(result);
            this.$refs.parameters.SetData(result);
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphs1MTimer !== null) clearTimeout(this.UpdateGraphs1MTimer);
            this.UpdateGraphs1MTimer = setTimeout(this.UpdateGraphValues, 30000);
          }.bind(this)
        });
      },
    },
    components: { Voltage, Counters, Waveform, Advanced, Events, Evolution, Harmonics, Parameters, Phasors }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
  .panel-other {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
</style>