<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-power">
          <div class="panel-heading">
            <div class="huge header-text">Active Power</div>
            <portal to="fs-toolbar" :disabled="!ExpandedActive" class="header-buttons">
              <toolbar-aggregation storage="AggregationEnergy" :options="['15M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <div class="btn-group">
                <button-export file="Power" :rows="Export"></button-export>
                <button-expand :expanded.sync="ExpandedActive"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="ACTIVE_POWER" :analyzer="analyzer" :data3S="Data3S" :data1M="Data1M" :dataGraph="DataGraph" :height.sync="ActiveHeight"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedActive" class="panel-body">
                  <power serie="ACTIVE_POWER" :analyzer="analyzer" :data="DataGraph" :height="ActiveHeight"></power>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <div class="panel panel-reactive-power">
          <div class="panel-heading">
            <div class="header-text">Fund. Reactive Power</div>
            <portal to="fs-toolbar" :disabled="!ExpandedReactive" class="header-buttons">
              <div class="btn-group">
                <button-export file="Power" :rows="Export"></button-export>
                <button-expand :expanded.sync="ExpandedReactive"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="REACTIVE_POWER" :analyzer="analyzer" :data3S="Data3S" :data1M="Data1M" :dataGraph="DataGraph" :height.sync="ReactiveHeight"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedReactive" class="panel-body">
                  <power serie="REACTIVE_POWER" :analyzer="analyzer" :data="DataGraph" :height="ReactiveHeight"></power>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <detail-tabs :tabs="['Waveform', 'Harmonics', 'Evolution', 'Distribution', 'Parameters']" storage="PowerDetailTab" v-model="ActiveTab"></detail-tabs>
        <waveform v-if="ActiveTab=='Waveform'" :data="DataNow" :analyzer="analyzer" :paused.sync="Paused"></waveform>
        <harmonics v-if="ActiveTab=='Harmonics'" :data="DataNow" :analyzer="analyzer" :paused.sync="Paused"></harmonics>
        <evolution v-if="ActiveTab=='Evolution'" :data="DataCubism" :analyzer="analyzer"></evolution>
        <distribution v-if="ActiveTab=='Distribution'" :analyzer="analyzer"></distribution>
        <parameters v-if="ActiveTab=='Parameters'" :data="DataCubism" :dataNow="DataNow" :analyzer="analyzer" :paused.sync="Paused"></parameters>
      </div>
    </div>
    <dlg-fs v-if="ExpandedActive" title="Active Power" :height.sync="DialogHeight"></dlg-fs>
    <dlg-fs v-if="ExpandedReactive" title="Reactive Power" :height.sync="DialogHeight"></dlg-fs>
  </div>
  <div v-else-if="analyzer == null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>
  </div>
</template>

<script>
  import Power from './Power.vue';
  import Counters from './Counters.vue';
  import Parameters from './Parameters.vue';
  import Waveform from './Waveform.vue';
  import Harmonics from './Harmonics.vue';
  import Evolution from './Evolution.vue';
  import Distribution from './Distribution.vue';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { Aggreg: null, ShowSpinner: true, Range: null, ActiveHeight: 300, ReactiveHeight: 300, ExpandedActive: false, ExpandedReactive: false, DialogHeight: 300, ActiveTab: null, Paused: false,
               DataNow: null, ControlNow: null, Data3S: null, Control3S: null, Data1M: null, Control1M: null,
               DataGraph: null, ControlGraph: null, DataCubism: null, ControlCubism: null };
    },
    watch: {
      analyzer: function() { this.UpdateNowValues(); this.Update3SValues(); this.Update1MValues(); this.UpdateCubismValues(); this.UpdateGraphValues(); },
      Aggreg: function() {  this.UpdateGraphValues(); },
      Range: function() { this.UpdateGraphValues(); }
    },
    computed: {
      Export: function() {
        return [];
      }
    },
    created: function() {
      this.UpdateNowValues();
      this.Update3SValues();
      this.Update1MValues();
      this.UpdateCubismValues();
      this.UpdateGraphValues();
    },
    beforeDestroy: function() {
      this.CancelTransfer(this.ControlNow);
      this.CancelTransfer(this.Control3S);
      this.CancelTransfer(this.Control1M);
      this.CancelTransfer(this.ControlCubism);
      this.CancelTransfer(this.ControlGraph);
    },
    methods: {
      UpdateNowValues: function() {
        this.ControlNow = this.CancelTransfer(this.ControlNow);
        if(this.analyzer == null) return;
        this.ControlNow.CancelFunc = this.analyzer.GetSeriesNow(['VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'POWER_HARMONICS', 'CURRENT', 'ACTIVE_POWER', 'REACTIVE_POWER', 'APPARENT_POWER', 'POWER_FACTOR', 'PHI'],
          (working) => {},
          (result) => { if(this.Paused == false) this.DataNow = Object.freeze(result); },
          (timeout) => { this.ControlNow.Timer = setTimeout(() => { this.UpdateNowValues(); }, timeout); }
        );
      },
      Update3SValues: function() {
        this.Control3S = this.CancelTransfer(this.Control3S);
        if(this.analyzer == null) return;
        this.Control3S.CancelFunc = this.analyzer.GetSeriesLast('3S', ['ACTIVE_POWER', 'REACTIVE_POWER'],
          (working) => {},
          (result) => { this.Data3S = result; },
          (timeout) => { this.Control3S.Timer = setTimeout(() => { this.Update3SValues(); }, timeout); }
        );
      },
      Update1MValues: function() {
        this.Control1M = this.CancelTransfer(this.Control1M);
        if(this.analyzer == null) return;
        this.Control1M.CancelFunc = this.analyzer.GetSeriesLast('1M', ['ACTIVE_POWER', 'REACTIVE_POWER'],
          (working) => {},
          (result) => { this.Data1M = result; },
          (timeout) => { this.Control1M.Timer = setTimeout(() => { this.Update1MValues(); }, timeout); }
        );
      },
      UpdateCubismValues: function() {
        this.ControlCubism = this.CancelTransfer(this.ControlCubism);
        if(this.analyzer == null) return;
        this.ControlCubism.CancelFunc = this.analyzer.GetSeriesRange('1M', 'Day', this.CubismData, ['POWER_HARMONICS', 'ACTIVE_POWER', 'REACTIVE_POWER', 'VOLTAGE', 'CURRENT', 'POWER_FACTOR', 'PHI'],
          (working) => {},
          (result) => { this.CubismData = Object.freeze(result); },
          (timeout) => { this.ControlCubism.Timer = setTimeout( () => { this.UpdateCubismValues(); }, timeout); }
        );
      },
      UpdateGraphValues: function() {
        this.ControlGraph = this.CancelTransfer(this.ControlGraph);
        if((this.Aggreg == null) || (this.Range == null) || (this.analyzer == null)) return;
        this.ControlGraph.CancelFunc = this.analyzer.GetPricesRange(this.Aggreg, this.Range, this.DataGraph,
          (working) => { this.ShowSpinner = working; },
          (result) => { this.DataGraph = Object.freeze(result); },
          (timeout) => { this.ControlGraph.Timer = setTimeout( () => { this.UpdateGraphValues(); }, timeout); }
        );
      }
    },
    components: { Power, Counters, Distribution, Waveform, Harmonics, Evolution, Parameters }
  }
</script>

<style scoped>
  .header-text {
    font-size: 20px;
    line-height: 24px;
  }
  .panel-other {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
</style>