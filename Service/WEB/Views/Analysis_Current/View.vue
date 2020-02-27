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
              <toolbar-phases storage="ViewPhases" :analyzer="analyzer" :current="true" :multi="true" v-model="Phases"></toolbar-phases>
              <div class="btn-group">
                <button-export file="Current" :rows="[]"></button-export>
                <button-expand :expanded.sync="Expanded"></button-expand>
              </div>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="TransferGraph.Data==null"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" :analyzer="analyzer" :data1M="Transfer1M.LastData" :data3S="Transfer3S.LastData" :dataGraph="TransferGraph.Data" :height.sync="Height"></counters>
              <current class="col-md-6 col-lg-8 col-xl-9" :phases="Phases" :analyzer="analyzer" :data="TransferGraph.Data" :height="Height" @range="Range=$event"></current>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <detail-tabs :tabs="['Waveform', 'Harmonics', 'Evolution', 'Advanced', 'Parameters']" storage="CurrentDetailTab" v-model="ActiveTab"></detail-tabs>
        <waveform v-if="ActiveTab=='Waveform'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></waveform>
        <harmonics v-if="ActiveTab=='Harmonics'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></harmonics>
        <evolution v-if="ActiveTab=='Evolution'" :data="TransferCubism.Data" :analyzer="analyzer"></evolution>
        <advanced v-if="ActiveTab=='Advanced'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></advanced>
        <parameters v-if="ActiveTab=='Parameters'" :data="TransferCubism.Data" :dataNow="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></parameters>
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
      return { ActiveTab: '', Aggreg: '', Range: null, Phases: null, Height: 300, Paused: false, Expanded: false,
               TransferGraph: this.CreateSeriesTransfer(this, ['CURRENT']),
               TransferNow: this.CreateSeriesTransfer(this, ['CURRENT', 'VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'VOLTAGE_HARMONICS', 'CURRENT_HARMONICS', 'VOLTAGE_FFT', 'CURRENT_FFT', 'POWER_FACTOR', 'CURRENT_THD', 'PHI']),
               TransferCubism: this.CreateSeriesTransfer(this, ['CURRENT_HARMONICS', 'POWER_FACTOR', 'CURRENT_THD', 'PHI']),
               Transfer1M: this.CreateSeriesTransfer(this, ['CURRENT']),
               Transfer3S: this.CreateSeriesTransfer(this, ['CURRENT']) 
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
      this.TransferGraph.Destroy();
      this.TransferNow.Destroy();
      this.TransferCubism.Destroy();
      this.Transfer1M.Destroy();
      this.Transfer3S.Destroy();
    },
    watch: {
      analyzer: function(val) {
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
//    methods: {
//      UpdateNowValues: function() {
//        this.ControlNow = this.CancelTransfer(this.ControlNow);
//        if(this.analyzer == null) return;
//        this.ControlNow.CancelFunc = this.analyzer.GetSeriesNow(['CURRENT', 'VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'VOLTAGE_HARMONICS', 'CURRENT_HARMONICS', 'VOLTAGE_FFT', 'CURRENT_FFT', 'POWER_FACTOR', 'CURRENT_THD', 'PHI'],
//          (working) => {},
//          (result) => { this.DataNow = Object.freeze(result); },
//          (timeout) => { this.ControlNow.Timer = setTimeout(() => { this.UpdateNowValues(); }, timeout); }
//        );
//      },
//      Update1MValues: function() {
//        this.Control1M = this.CancelTransfer(this.Control1M);
//        if(this.analyzer == null) return;
//        this.Control1M.CancelFunc = this.analyzer.GetSeriesLast('1M', ['CURRENT'],
//          (working) => {},
//          (result) => { this.Data1M = Object.freeze(result); },
//          (timeout) => { this.Control1M.Timer = setTimeout(() => { this.Update1MValues(); }, timeout); }
//        );
//      },
//      Update3SValues: function() {
//        this.Control3S = this.CancelTransfer(this.Control3S);
//        if(this.analyzer == null) return;
//        this.Control3S.CancelFunc = this.analyzer.GetSeriesLast('3S', ['CURRENT'],
//          (working) => {},
//          (result) => { this.Data3S = Object.freeze(result); },
//          (timeout) => { this.Control3S.Timer = setTimeout(() => { this.Update3SValues(); }, timeout); }
//        );
//      },
//      UpdateCubismValues: function() {
//        this.ControlCubism = this.CancelTransfer(this.ControlCubism);
//        if(this.analyzer == null) return;
//        this.ControlCubism.CancelFunc = this.analyzer.GetSeriesRange('1M', 'LastDay', this.DataCubism, ['CURRENT_HARMONICS', 'POWER_FACTOR', 'CURRENT_THD', 'PHI'],
//          (working) => {},
//          (result) => { this.DataCubism = Object.freeze(result); },
//          (timeout) => { this.ControlCubism.Timer = setTimeout( () => { this.UpdateCubismValues(); }, timeout); }
//        );
//      },
//      UpdateGraphValues: function() {
//        this.ControlGraph = this.CancelTransfer(this.ControlGraph);
//        if((this.Aggreg == null) || (this.Phases == null) || (this.Range == null) || (this.analyzer == null)) return;
//        this.ControlGraph.CancelFunc = this.analyzer.GetSeriesRange(this.Aggreg, this.Range, this.DataGraph, ['CURRENT'],
//          (working) => { this.ShowSpinner = working; },
//          (result) => { this.DataGraph = Object.freeze(result); },
//          (timeout) => { this.ControlGraph.Timer = setTimeout( () => { this.UpdateGraphValues(); }, timeout); }
//        );
//      },
//    },
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