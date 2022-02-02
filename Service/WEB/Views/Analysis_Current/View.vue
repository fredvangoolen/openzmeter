<template>
  <q-page>
    <q-card>
      <current />
      <q-toolbar class="bg-grey-9 text-white">
        <q-tabs v-model="ActiveTab" shrink stretch active-bg-color="primary" align="left">
          <q-tab name="waveform" label="Waveform" />
          <q-tab name="harmonics" label="Harmonics" v-if="Analyzer.NominalFrequency>0" />
          <q-tab name="evolution" label="Evolution" v-if="Analyzer.NominalFrequency>0" />
          <q-tab name="advanced" label="Advanced" />
          <q-tab name="parameters" label="Parameters" v-if="Analyzer.NominalFrequency>0" />
        </q-tabs>
      </q-toolbar>
      <q-tab-panels v-model="ActiveTab">
        <waveform name="waveform" :data="TransferNow.Data" :paused.sync="Paused"></waveform>
        <harmonics name="harmonics" :data="TransferNow.Data" :paused.sync="Paused"></harmonics>
        <evolution name="evolution" :data="TransferCubism.Data"></evolution>
        <advanced name="advanced" :data="TransferNow.Data" :paused.sync="Paused"></advanced>
        <parameters name="parameters" :data="TransferCubism.Data" :dataNow="TransferNow.Data" :paused.sync="Paused"></parameters>
      </q-tab-panels>
    </q-card>

  </q-page>
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
    data: function() {
      return { ActiveTab: null,
               TransferGraph: this.SeriesTransfer(['Current']),
               TransferNow: this.NowTransfer(['Current', 'Voltage_Samples', 'Current_Samples', 'Voltage_Harmonics_Complex', 'Current_Harmonics_Complex', 'Voltage_FFT', 'Current_FFT', 'Power_Factor', 'Current_THD', 'Phi']),
               TransferCubism: this.SeriesTransfer(['Current_Harmonics', 'Power_Factor', 'Current_THD', 'Phi']),
               Transfer1M: this.SeriesTransfer(['Current']),
               Transfer3S: this.SeriesTransfer(['Current'])
      };
    },
    created: function() {
      this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      this.TransferNow.Update(this.analyzer);
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
        this.TransferNow.Update(this.analyzer);
        this.TransferCubism.Update(this.analyzer, '1M', 'LastDay');
        this.Transfer1M.Update(this.analyzer, '1M', 'LastSamples');
        this.Transfer3S.Update(this.analyzer, '3S', 'LastSamples');
      },
      Aggreg: function() {
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      },
      Range: function(val, prev) {
        if(prev != null) this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      },
      Paused: function() {
        if(this.Paused == true) {
          this.TransferNow.Destroy();
        } else {
          this.TransferNow.Update(this.analyzer);
        }
      }
    },
    methods: {
      Export: function() {
        if(this.TransferGraph.Data == null) return [];
        var Rows = [['Time'].concat(this.analyzer.SerieNames('Current'))];
        var TimeCol = this.TransferGraph.Data.Headers.indexOf('Time');
        var CurrentCol = this.TransferGraph.Data.Headers.indexOf('Current');
        for(var n = 0; n < this.TransferGraph.Data.Rows.length; n++) {
          var Row = [this.analyzer.FormatDateTime(this.TransferGraph.Data.Rows[n][TimeCol])];
          for(var i = 0; i < this.TransferGraph.Data.Rows[n][CurrentCol].length; i++) Row.push(this.Round(this.TransferGraph.Data.Rows[n][CurrentCol][i], 3));
          Rows.push(Row);
        }
        return Object.seal(Rows);
      }
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