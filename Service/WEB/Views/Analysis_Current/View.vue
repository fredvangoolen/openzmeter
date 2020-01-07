<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-current">
          <div class="panel-heading">
            <div class="header-text">Current</div>
            <div class="header-buttons clearfix" ref="Toolbar">
              <toolbar-aggregation storage="Aggregation" :options="['3S', '1M', '10M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <!--toolbar-phases storage="CurrentViewPhases" :current="true" :multi="true" v-model="Phases"></toolbar-phases-->
              <!--toolbar-tools variable="To" :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools-->
            </div>
          </div>
          <div class="panel-body">
            <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" :height.sync="Height"></counters>
              <current class="col-md-6 col-lg-8 col-xl-9" :phases="Phases" :analyzer="analyzer" :data="GraphData"></current>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <detail-tabs :tabs="['Waveform', 'Harmonics', 'Evolution', 'Advanced', 'Parameters']" storage="CurrentDetailTab" v-model="ActiveTab"></detail-tabs>
        <waveform v-if="ActiveTab=='Waveform'" :data="NowData" :analyzer="analyzer" :paused.sync="Paused" :phases.sync="PhasesMulti"></waveform>
        <!--harmonics v-if="ActiveTab=='Harmonics'"></harmonics>
        <evolution v-if="ActiveTab=='Evolution'"></evolution>
        <advanced v-if="ActiveTab=='Advanced'"></advanced>
        <parameters v-if="ActiveTab=='Parameters'"></parameters-->
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
  import Current from './Current.vue';
  import Counters from './Counters.vue';
  import Advanced from './Advanced.vue';
  import Evolution from './Evolution.vue';
  import Harmonics from './Harmonics.vue';
  import Parameters from './Parameters.vue';
  import Waveform from './Waveform.vue';
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
        if(this.analyzer != null) this.$http.post('getSeriesNow', { Series: ['CURRENT', 'VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'VOLTAGE_HARMONICS', 'CURRENT_HARMONICS', 'VOLTAGE_FFT', 'CURRENT_FFT', 'POWER_FACTOR', 'CURRENT_THD', 'PHI'], Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.NowData = (result.status == 200) ? Object.freeze(result.data) : null;
            this.NowTimer = setTimeout(() => { this.UpdateNowValues(); }, 2000);
          }
        );
      },
      UpdateCubismValues: function() {

      },
      UpdateGraphValues: function() {

      },
    },
    components: { Current, Counters, Waveform, Advanced, Evolution, Harmonics, Parameters }
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