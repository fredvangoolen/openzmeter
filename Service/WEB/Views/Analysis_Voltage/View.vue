<template>
  <q-page>
    <q-card>
      <voltage @update:range="Range=$event"/>
      <q-toolbar class="bg-grey-9 text-white">
        <q-tabs v-model="ActiveTab" shrink stretch active-bg-color="primary" align="left">
          <q-tab name="waveform" label="Waveform" />
          <q-tab name="harmonics" label="Harmonics" v-if="Analyzer.NominalFrequency>0" />
          <q-tab name="evolution" label="Evolution" v-if="Analyzer.NominalFrequency>0" />
          <q-tab name="advanced" label="Advanced" />
          <q-tab name="phasors" label="Phasors" v-if="Analyzer.NominalFrequency>0" />
          <q-tab name="parameters" label="Parameters" v-if="Analyzer.NominalFrequency>0" />
          <q-tab name="events" label="Events" v-if="Analyzer.NominalFrequency>0" />
        </q-tabs>
      </q-toolbar>
      <q-tab-panels v-model="ActiveTab">
        <waveform name="waveform" :data="TransferNow.Data" v-model:paused="Paused" />
        <harmonics name="harmonics" :data="TransferNow.Data" v-model:paused="Paused" />
        <evolution name="evolution" :data="TransferCubism.Data" />
        <advanced name="advanced" :data="TransferNow.Data" v-model:paused="Paused" />
        <parameters name="parameters" :data="TransferCubism.Data" :dataNow="TransferNow.Data" v-model:paused="Paused" />
        <phasors name="phasors" :data="TransferNow.Data" v-model:paused="Paused" />
        <events name="events" :events="TransferEvents.Data" @eventschanged="UpdateEvents" :pagesize="10" />
      </q-tab-panels>
    </q-card>
  </q-page>
</template>

<script>
  import Voltage from './Voltage.vue';
  import Advanced from './Advanced.vue';
  import Evolution from './Evolution.vue';
  import Harmonics from './Harmonics.vue';
  import Parameters from './Parameters.vue';
  import Waveform from './Waveform.vue';
  import Events from '../Events_Disturbances/Events.vue';
  import Phasors from './Phasors.vue';
  export default {
    data: function() {
      return { ActiveTab: 'waveform', Range: null, Paused: false,

               TransferNow: this.NowTransfer(['Voltage', 'Current', 'Voltage_Samples', 'Voltage_Phase', 'Current_Samples', 'Voltage_Harmonics_Complex', 'Current_Harmonics_Complex', 'Voltage_FFT', 'Current_FFT', 'Power_Factor', 'Voltage_THD', 'Phi', 'Unbalance']),
               TransferCubism: this.SeriesTransfer(['Voltage_Harmonics', 'Power_Factor', 'Voltage_THD', 'Phi', 'Unbalance']),
               Transfer10M: this.SeriesTransfer(['Voltage']),
               TransferEvents: this.EventsTransfer()
      };
    },
    created: function() {

//      this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
//      this.TransferNow.Update(this.analyzer);
//      this.Transfer10M.Update(this.analyzer, '10M', 'LastSamples');
//      this.TransferCubism.Update(this.analyzer, '1M', 'LastDay');
//      this.TransferEvents.Update(this.analyzer, this.Range);
    },
    beforeDestroy: function() {
//      this.TransferGraph.Destroy();
//      this.TransferNow.Destroy();
//      this.Transfer10M.Destroy();
//      this.TransferCubism.Destroy();
//      this.TransferEvents.Destroy();
    },
    watch: {
      analyzer: function(val) {
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
        this.TransferNow.Update(this.analyzer);
        this.TransferEvents.Update(this.analyzer, this.Range);
        this.Transfer10M.Update(this.analyzer, '10M', 'LastSamples');
        this.TransferCubism.Update(this.analyzer, '1M', 'LastDay');
      },
      Aggreg: function() {
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      },
      Range: function(val, prev) {
        if(prev != null) this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
        this.TransferEvents.Update(this.analyzer, this.Range);
      },
      Paused: function() {
        if(this.Paused == true) {
          this.TransferNow.Destroy();
        } else {
          this.TransferNow.Update();
        }
      }
    },
    methods: {
      UpdateEvents: function() {
        this.TransferEvents.Data = null;
        this.TransferEvents.Update(this.analyzer, this.Range);
      }
    },
    components: { Voltage, Waveform, Advanced, Events, Evolution, Harmonics, Parameters, Phasors }
  }
</script>