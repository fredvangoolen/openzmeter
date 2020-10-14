<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-current">
          <div class="panel-heading">
            <div class="header-text">Current</div>
            <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
              <toolbar-aggregation storage="Aggregation" :options="['3S', '1M', '10M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range" :analyzer="analyzer"></toolbar-period>
              <toolbar-phases storage="ViewPhases" :analyzer="analyzer" :current="true" :multi="true" v-model="Phases"></toolbar-phases>
              <div class="btn-group">
                <button-export file="Current" :rows="Export"></button-export>
                <button-expand v-model="Expanded"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" :analyzer="analyzer" :data1M="Transfer1M.LastData" :data3S="Transfer3S.LastData" :dataGraph="TransferGraph.Data" :height.sync="Height"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!Expanded" class="panel-body">
                  <spinner v-if="TransferGraph.Data==null"></spinner>
                  <current :phases="Phases" :analyzer="analyzer" :data="TransferGraph.Data" :height="Expanded?DialogHeight:Height" @range="Range=$event"></current>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <detail-tabs v-if="analyzer.NominalFrequency>0" :tabs="['Waveform', 'Harmonics', 'Evolution', 'Advanced', 'Parameters']" storage="Current.Details.Tabs.AC" v-model="ActiveTab"></detail-tabs>
        <detail-tabs v-if="analyzer.NominalFrequency==0" :tabs="['Waveform', 'Advanced']" storage="Current.Details.Tabs.DC" v-model="ActiveTab"></detail-tabs>
        <waveform v-if="ActiveTab=='Waveform'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></waveform>
        <harmonics v-if="ActiveTab=='Harmonics'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></harmonics>
        <evolution v-if="ActiveTab=='Evolution'" :data="TransferCubism.Data" :analyzer="analyzer"></evolution>
        <advanced v-if="ActiveTab=='Advanced'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></advanced>
        <parameters v-if="ActiveTab=='Parameters'" :data="TransferCubism.Data" :dataNow="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></parameters>
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
      return { ActiveTab: null, Aggreg: null, Range: null, Phases: null, Height: 300, Paused: false, Expanded: false, DialogHeight: 300,
               TransferGraph: this.SeriesTransfer(['Current']),
               TransferNow: this.NowTransfer(['Current', 'VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'Voltage_Harmonics_Complex', 'Current_Harmonics_Complex', 'Voltage_FFT', 'Current_FFT', 'POWER_FACTOR', 'CURRENT_THD', 'PHI']),
               TransferCubism: this.SeriesTransfer(['CURRENT_HARMONICS', 'POWER_FACTOR', 'CURRENT_THD', 'PHI']),
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
        var Rows = [['Time'].concat(this.analyzer.GetSeriesNames('Current', this.analyzer.CurrentChannels))];
        var TimeCol = this.TransferGraph.Data.Headers.indexOf('Time');
        var CurrentCol = this.TransferGraph.Data.Headers.indexOf('Current');
        for(var n = 0; n < this.TransferGraph.Data.Rows.length; n++) {
          var Row = [this.analyzer.FormatDateTime(this.TransferGraph.Data.Rows[n][TimeCol])];
          for(var i = 0; i < this.analyzer.CurrentChannels; i++) Row.push(this.Round(this.TransferGraph.Data.Rows[n][CurrentCol][i], 3));
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