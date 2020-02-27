<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-voltage">
          <div class="panel-heading">
            <div class="header-text">Voltage</div>
            <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
              <toolbar-aggregation storage="Aggregation" :options="['3S', '1M', '10M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <toolbar-phases storage="ViewPhases" :analyzer="analyzer" :current="false" :multi="true" v-model="Phases"></toolbar-phases>
              <div class="btn-group">
                <button-export file="Voltage" :rows="[]"></button-export>
                <button-expand v-model="Expanded"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <spinner v-if="TransferGraph.Data==null"></spinner>            
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" :analyzer="analyzer" :data10M="Transfer10M.LastData" :dataGraph="TransferGraph.Data" :height.sync="Height"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!Expanded" class="panel-body">
                  <spinner v-if="Expanded && (TransferGraph.Data==null)"></spinner>
                  <voltage :phases="Phases" :analyzer="analyzer" :data="TransferGraph.Data" :height="Expanded?DialogHeight:Height" @range="Range=$event"></voltage>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <detail-tabs :tabs="['Waveform', 'Harmonics', 'Evolution', 'Advanced', 'Phasors', 'Parameters', 'Events']" storage="VoltageDetailTab" v-model="ActiveTab"></detail-tabs>
          <waveform v-if="ActiveTab=='Waveform'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></waveform>
          <harmonics v-if="ActiveTab=='Harmonics'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></harmonics>
          <evolution v-if="ActiveTab=='Evolution'" :data="TransferCubism.Data" :analyzer="analyzer"></evolution>
          <advanced v-if="ActiveTab=='Advanced'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></advanced>
          <parameters v-if="ActiveTab=='Parameters'" :data="TransferCubism.Data" :dataNow="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></parameters>
          <phasors v-if="ActiveTab=='Phasors'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></phasors>
          <events v-if="ActiveTab=='Events'" :analyzer="analyzer" :events="TransferEvents.Data"></events>
      </div>
    </div>
    <dlg-fs v-if="Expanded" title="Voltage" :height.sync="DialogHeight"></dlg-fs>
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
      return { ActiveTab: null, Aggreg: null, Range: null, Phases: null, Height: 300, Paused: false, Expanded: false, DialogHeight: 300,
               TransferGraph: this.CreateSeriesTransfer(this, ['VOLTAGE']),
               TransferNow: this.CreateSeriesTransfer(this, ['VOLTAGE', 'CURRENT', 'VOLTAGE_SAMPLES', 'VOLTAGE_PHASE', 'CURRENT_SAMPLES', 'VOLTAGE_HARMONICS', 'CURRENT_HARMONICS', 'VOLTAGE_FFT', 'CURRENT_FFT', 'POWER_FACTOR', 'VOLTAGE_THD', 'PHI']),
               TransferCubism: this.CreateSeriesTransfer(this, ['VOLTAGE_HARMONICS', 'POWER_FACTOR', 'VOLTAGE_THD', 'PHI']),
               Transfer10M: this.CreateSeriesTransfer(this, ['VOLTAGE']),
               TransferEvents: this.CreateSeriesTransfer(this)
      };
    },
    created: function() {
      this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      this.TransferNow.UpdateNow(this.analyzer);
      this.Transfer10M.Update(this.analyzer, '10M', 'LastSamples');
      this.TransferCubism.Update(this.analyzer, '1M', 'LastDay');
      this.TransferEvents.UpdateEvents(this.analyzer, this.Range);
    },
    beforeDestroy: function() {
      this.TransferGraph.Destroy();
      this.TransferNow.Destroy();
      this.Transfer10M.Destroy();
      this.TransferCubism.Destroy();
      this.TransferEvents.Destroy();
    },
    watch: {
      analyzer: function(val) {
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
        this.TransferNow.UpdateNow(this.analyzer);
        this.TransferEvents.UpdateEvents(this.analyzer, this.Range);        
        this.Transfer10M.Update(this.analyzer, '10M', 'LastSamples');
        this.TransferCubism.Update(this.analyzer, '1M', 'LastDay');
      },
      Aggreg: function() { 
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      },
      Range: function(val, prev) { 
        if(prev != null) this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
        this.TransferEvents.UpdateEvents(this.analyzer, this.Range);
      }
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