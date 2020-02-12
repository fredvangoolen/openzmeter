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
              <toolbar-phases storage="ViewPhases" :analyzer="analyzer" :current="false" :multi="true" v-model="Phases"></toolbar-phases>
              <div class="btn-group">
                <button-export file="Voltage" :rows="[]"></button-export>
                <button-expand :expanded.sync="Expanded"></button-expand>
              </div>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" :analyzer="analyzer" :data10M="Data10M" :dataGraph="DataGraph" :height.sync="Height"></counters>
              <voltage class="col-md-6 col-lg-8 col-xl-9" :phases="Phases" :analyzer="analyzer" :data="DataGraph" :height="Height"></voltage>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <detail-tabs :tabs="['Waveform', 'Harmonics', 'Evolution', 'Advanced', 'Phasors', 'Parameters', 'Events']" storage="VoltageDetailTab" v-model="ActiveTab"></detail-tabs>
          <waveform v-if="ActiveTab=='Waveform'" :data="DataNow" :analyzer="analyzer" :paused.sync="Paused"></waveform>
          <harmonics v-if="ActiveTab=='Harmonics'" :data="DataNow" :analyzer="analyzer" :paused.sync="Paused"></harmonics>
          <evolution v-if="ActiveTab=='Evolution'" :data="DataCubism" :analyzer="analyzer"></evolution>
          <advanced v-if="ActiveTab=='Advanced'" :data="DataNow" :analyzer="analyzer" :paused.sync="Paused"></advanced>
          <parameters v-if="ActiveTab=='Parameters'" :data="DataCubism" :dataNow="DataNow" :analyzer="analyzer" :paused.sync="Paused"></parameters>
          <phasors v-if="ActiveTab=='Phasors'" :data="DataNow" :analyzer="analyzer" :paused.sync="Paused"></phasors>
          <events v-if="ActiveTab=='Events'" :analyzer="analyzer" :range="Range"></events>
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
      return { ShowSpinner: false, ActiveTab: '', Aggreg: null, Range: null, Phases: null, Height: 300, Paused: false, Expanded: false,
               DataGraph: null, ControlGraph: null, DataNow: null, ControlNow: null, DataCubism: null, ControlCubism: null, Data10M: null, Control10M: null };
    },
    created: function() {
      this.UpdateNowValues();
      this.Update10MValues();
      this.UpdateGraphValues();
      this.UpdateCubismValues();
    },
    beforeDestroy: function() {
      this.CancelTransfer(this.ControlGraph);
      this.CancelTransfer(this.ControlNow);
      this.CancelTransfer(this.Control10M);
      this.CancelTransfer(this.ControlCubism);
    },
    watch: {
      analyzer: function(val) {
        this.UpdateNowValues();
        this.Update10MValues();
        this.UpdateGraphValues();
        this.UpdateCubismValues();
      },
      Aggreg: function() {  this.UpdateGraphValues(); },
      Range: function() { this.UpdateGraphValues(); }
    },
    methods: {
      UpdateNowValues: function() {
        this.ControlNow = this.CancelTransfer(this.ControlNow);
        if(this.analyzer == null) return;
        this.ControlNow.CancelFunc = this.analyzer.GetSeriesNow(['VOLTAGE', 'CURRENT', 'VOLTAGE_SAMPLES', 'VOLTAGE_PHASE', 'CURRENT_SAMPLES', 'VOLTAGE_HARMONICS', 'CURRENT_HARMONICS', 'VOLTAGE_FFT', 'CURRENT_FFT', 'POWER_FACTOR', 'VOLTAGE_THD', 'PHI'],
          (working) => {},
          (result) => { if(this.Paused == false) this.DataNow = Object.freeze(result); },
          (timeout) => { this.ControlNow.Timer = setTimeout(() => { this.UpdateNowValues(); }, timeout); }
        );
      },
      Update10MValues: function() {
        this.Control10M = this.CancelTransfer(this.Control10M);
        if(this.analyzer == null) return;
        this.Control10M.CancelFunc = this.analyzer.GetSeriesLast('10M', ['VOLTAGE'],
          (working) => {},
          (result) => { this.Data10M = Object.freeze(result); },
          (timeout) => { this.Control10M.Timer = setTimeout(() => { this.Update10MValues(); }, timeout); }
        );
      },
      UpdateCubismValues: function() {
        this.ControlCubism = this.CancelTransfer(this.ControlCubism);
        if(this.analyzer == null) return;
        this.ControlCubism.CancelFunc = this.analyzer.GetSeriesRange('1M', 'LastDay', this.DataCubism, ['VOLTAGE_HARMONICS', 'POWER_FACTOR', 'VOLTAGE_THD', 'PHI'],
          (working) => {},
          (result) => { this.DataCubism = Object.freeze(result); },
          (timeout) => { this.ControlCubism.Timer = setTimeout( () => { this.UpdateCubismValues(); }, timeout); }
        );
      },
      UpdateGraphValues: function() {
        this.ControlGraph = this.CancelTransfer(this.ControlGraph);
        if((this.Aggreg == null) || (this.Phases == null) || (this.Range == null) || (this.analyzer == null)) return;
        this.ControlGraph.CancelFunc = this.analyzer.GetSeriesRange(this.Aggreg, this.Range, this.DataGraph, ['VOLTAGE'],
          (working) => { this.ShowSpinner = working; },
          (result) => { this.DataGraph = Object.freeze(result); },
          (timeout) => { this.ControlGraph.Timer = setTimeout( () => { this.UpdateGraphValues(); }, timeout); }
        );
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