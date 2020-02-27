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
                <button-expand v-model="ExpandedActive"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <spinner v-if="TransferGraph.Data==null"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="ACTIVE_POWER" :analyzer="analyzer" :data3S="Transfer3S.LastData" :data1M="Transfer1M.LastData" :dataGraph="TransferGraph.Data" :height.sync="ActiveHeight"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedActive" class="panel-body">
                  <power serie="Active_Power" :analyzer="analyzer" :data="TransferGraph.Data" :height="ActiveHeight" @range="Range=$event"></power>
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
                <button-expand v-model="ExpandedReactive"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <spinner v-if="TransferGraph.Data==null"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="REACTIVE_POWER" :analyzer="analyzer" :data3S="Transfer3S.LastData" :data1M="Transfer1M.LastData" :dataGraph="TransferGraph.Data" :height.sync="ReactiveHeight"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedReactive" class="panel-body">
                  <power serie="Reactive_Power" :analyzer="analyzer" :data="TransferGraph.Data" :height="ReactiveHeight" :range.sync="Range"></power>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <detail-tabs :tabs="['Waveform', 'Harmonics', 'Evolution', 'Distribution', 'Parameters', 'Lissajou']" storage="PowerDetailTab" v-model="ActiveTab"></detail-tabs>
        <waveform v-if="ActiveTab=='Waveform'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></waveform>
        <harmonics v-if="ActiveTab=='Harmonics'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></harmonics>
        <evolution v-if="ActiveTab=='Evolution'" :data="TransferCubism.Data" :analyzer="analyzer"></evolution>
        <distribution v-if="ActiveTab=='Distribution'" :analyzer="analyzer"></distribution>
        <parameters v-if="ActiveTab=='Parameters'" :data="TransferCubism.Data" :dataNow="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></parameters>
        <lissajou v-if="ActiveTab=='Lissajou'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></lissajou>
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
  import Lissajou from './Lissajou.vue';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { ActiveTab: null, Aggreg: null, Range: null, ActiveHeight: 300, ReactiveHeight: 300, ExpandedActive: false, ExpandedReactive: false, DialogHeight: 300, Paused: false,
               TransferNow: this.CreateSeriesTransfer(this, ['VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'POWER_HARMONICS', 'CURRENT', 'ACTIVE_POWER', 'REACTIVE_POWER', 'APPARENT_POWER', 'POWER_FACTOR', 'PHI']),
               Transfer3S: this.CreateSeriesTransfer(this, ['ACTIVE_POWER', 'REACTIVE_POWER']),
               Transfer1M: this.CreateSeriesTransfer(this, ['ACTIVE_POWER', 'REACTIVE_POWER']),
               TransferGraph: this.CreateSeriesTransfer(this, ['VOLTAGE']),
               TransferCubism: this.CreateSeriesTransfer(this, ['POWER_HARMONICS', 'ACTIVE_POWER', 'REACTIVE_POWER', 'VOLTAGE', 'CURRENT', 'POWER_FACTOR', 'PHI'])
      };
    },
    created: function() {
      this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      this.TransferNow.UpdateNow(this.analyzer);
      this.TransferCubism.Update(this.analyzer, '1M', 'LastDay');
      this.Transfer1M.Update(this.analyzer, '1M', 'LastSamples');
      this.Transfer3S.Update(this.analyzer, '3S', 'LastSamples');
    },
    beforeDestroy: function() {
      this.TransferNow.Destroy();
      this.Transfer1M.Destroy();
      this.Transfer3S.Destroy();
      this.TransferGraph.Destroy();
      this.TransferCubism.Destroy();
    },
    watch: {
      analyzer: function() { 
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
        this.TransferNow.UpdateNow(this.analyzer);
        this.TransferCubism.Update(this.analyzer, '1M', 'LastDay');
        this.Transfer1M.Update(this.analyzer, '1M', 'LastSamples');
        this.Transfer3S.Update(this.analyzer, '3S', 'LastSamples');
      },
      Aggreg: function() { 
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      },
      Range: function(val, prev) { 
        if(prev != null) this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      }
    },
    computed: {
      Export: function() {
        return [];
      }
    },
    methods: {
//      UpdateNowValues: function(clear) {
//        if(clear == true) this.DataNow = null;
//        this.ControlNow = this.CancelTransfer(this.ControlNow);
//        if(this.analyzer == null) return;
//        this.ControlNow.CancelFunc = this.analyzer.GetSeriesNow(['VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'POWER_HARMONICS', 'CURRENT', 'ACTIVE_POWER', 'REACTIVE_POWER', 'APPARENT_POWER', 'POWER_FACTOR', 'PHI'],
//          (working) => {},
//          (result) => { if(this.Paused == false) this.DataNow = Object.seal(result); },
//          (timeout) => { this.ControlNow.Timer = setTimeout(() => { this.UpdateNowValues(false); }, timeout); }
//        );
//      },
//      Update3SValues: function(clear) {
//        if(clear == true) this.Data3S = null;
//        this.Control3S = this.CancelTransfer(this.Control3S);
//        if(this.analyzer == null) return;
//        this.Control3S.CancelFunc = this.analyzer.GetSeriesLast('3S', ['ACTIVE_POWER', 'REACTIVE_POWER'],
//          (working) => {},
//          (result) => { this.Data3S = Object.seal(result); },
//          (timeout) => { this.Control3S.Timer = setTimeout(() => { this.Update3SValues(false); }, timeout); }
//        );
//      },
//      Update1MValues: function(clear) {
//        if(clear == true) this.Data1M = null;
//        this.Control1M = this.CancelTransfer(this.Control1M);
//        if(this.analyzer == null) return;
//        this.Control1M.CancelFunc = this.analyzer.GetSeriesLast('1M', ['ACTIVE_POWER', 'REACTIVE_POWER'],
//          (working) => {},
//          (result) => { this.Data1M = Object.seal(result); },
//          (timeout) => { this.Control1M.Timer = setTimeout(() => { this.Update1MValues(false); }, timeout); }
//        );
//      },
//      UpdateCubismValues: function(clear) {
//        if(clear == true) this.DataCubism = null;
//        this.ControlCubism = this.CancelTransfer(this.ControlCubism);
//        if(this.analyzer == null) return;
//        this.ControlCubism.CancelFunc = this.analyzer.GetSeriesRange('1M', 'LastDay', this.CubismData, ['POWER_HARMONICS', 'ACTIVE_POWER', 'REACTIVE_POWER', 'VOLTAGE', 'CURRENT', 'POWER_FACTOR', 'PHI'],
//          (working) => {},
//          (result) => { this.DataCubism = Object.seal(result); },
//          (timeout) => { this.ControlCubism.Timer = setTimeout( () => { this.UpdateCubismValues(false); }, timeout); }
//        );
//      },
//      UpdateGraphValues: function(clear) {
//        if(clear == true) this.DataGraph = null;
//        this.ControlGraph = this.CancelTransfer(this.ControlGraph);
//        if((this.Aggreg == null) || (this.Range == null) || (this.analyzer == null)) return;
//        this.ControlGraph.CancelFunc = this.analyzer.GetPricesRange(this.Aggreg, this.Range, this.DataGraph,
//          (working) => { this.ShowSpinner = working; },
//          (result) => { this.DataGraph = Object.seal(result); },
//          (timeout) => { this.ControlGraph.Timer = setTimeout( () => { this.UpdateGraphValues(false); }, timeout); }
//        );
//      }
    },
    components: { Power, Counters, Distribution, Waveform, Harmonics, Evolution, Parameters, Lissajou }
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