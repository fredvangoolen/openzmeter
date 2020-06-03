<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-power">
          <div class="panel-heading">
            <div class="huge header-text">Active Power</div>
            <portal to="fs-toolbar" :disabled="!ExpandedActive" class="header-buttons">
              <toolbar-aggregation storage="AggregationEnergy" :options="['15M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range" :analyzer="analyzer"></toolbar-period>
              <div class="btn-group">
                <button type="button" class="btn btn-default btn-xs" :class="{ active: ShowContracted }" @click="ShowContracted=!ShowContracted" data-toggle="tooltip" title="Scale includes contracted power limit"><img :src="require('Imgs/tag_blue.png')"></button>
                <button-export file="Power" :rows="Export"></button-export>
                <button-expand v-model="ExpandedActive"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <spinner v-if="TransferGraph.Data==null"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="Active_Power" :analyzer="analyzer" :data3S="Transfer3S.LastData" :data1M="Transfer1M.LastData" :dataGraph="TransferGraph.Data" :height.sync="ActiveHeight"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedActive" class="panel-body">
                  <power serie="Active_Power" :analyzer="analyzer" :data="TransferGraph.Data" :showContracted="ShowContracted" :periods="TransferGraph.Periods" :height="ExpandedActive?DialogHeight:ActiveHeight" @range="Range=$event"></power>
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
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="Reactive_Power" :analyzer="analyzer" :data3S="Transfer3S.LastData" :data1M="Transfer1M.LastData" :dataGraph="TransferGraph.Data" :height.sync="ReactiveHeight"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedReactive" class="panel-body">
                  <power serie="Reactive_Power" :analyzer="analyzer" :data="TransferGraph.Data" :periods="TransferGraph.Periods" :height="ExpandedReactive?DialogHeight:ReactiveHeight" @range="Range=$event"></power>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <div class="panel panel-reactive-power">
          <div class="panel-heading">
            <div class="header-text">Apparent Power</div>
            <portal to="fs-toolbar" :disabled="!ExpandedApparent" class="header-buttons">
              <div class="btn-group">
                <button-export file="Power" :rows="Export"></button-export>
                <button-expand v-model="ExpandedApparent"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <spinner v-if="TransferGraph.Data==null"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="Apparent_Power" :analyzer="analyzer" :data3S="Transfer3S.LastData" :data1M="Transfer1M.LastData" :dataGraph="TransferGraph.Data" :height.sync="ReactiveHeight"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedApparent" class="panel-body">
                  <power serie="Apparent_Power" :analyzer="analyzer" :data="TransferGraph.Data" :periods="TransferGraph.Periods" :height="ExpandedApparent?DialogHeight:AppaerentHeight" @range="Range=$event"></power>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <detail-tabs :tabs="['Waveform', 'Harmonics', 'GA', 'Evolution', 'Distribution', 'Parameters', 'Lissajou']" storage="PowerDetailTab" v-model="ActiveTab"></detail-tabs>
        <waveform v-if="ActiveTab=='Waveform'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></waveform>
        <harmonics v-if="ActiveTab=='Harmonics'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></harmonics>
        <ga v-if="ActiveTab=='GA'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></harmonics>
        <evolution v-if="ActiveTab=='Evolution'" :data="TransferCubism.Data" :analyzer="analyzer"></evolution>
        <distribution v-if="ActiveTab=='Distribution'" :analyzer="analyzer"></distribution>
        <parameters v-if="ActiveTab=='Parameters'" :data="TransferCubism.Data" :dataNow="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></parameters>
        <lissajou v-if="ActiveTab=='Lissajou'" :data="TransferNow.Data" :analyzer="analyzer" :paused.sync="Paused"></lissajou>
      </div>
    </div>
    <dlg-fs v-if="ExpandedActive" title="Active Power" :height.sync="DialogHeight"></dlg-fs>
    <dlg-fs v-if="ExpandedReactive" title="Reactive Power" :height.sync="DialogHeight"></dlg-fs>
    <dlg-fs v-if="ExpandedApparent" title="Apparent Power" :height.sync="DialogHeight"></dlg-fs>
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
  import Ga from './GA.vue';
  import Evolution from './Evolution.vue';
  import Distribution from './Distribution.vue';
  import Lissajou from './Lissajou.vue';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { ActiveTab: null, Aggreg: null, Range: null, ActiveHeight: 300, ReactiveHeight: 300, ExpandedActive: false, ExpandedReactive: false, DialogHeight: 300, Paused: false, ShowContracted: true,
               ExpandedApparent: false, AppaerentHeight: 300,
               TransferNow: this.CreateSeriesTransfer(this, ['VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'POWER_HARMONICS', 'CURRENT', 'ACTIVE_POWER', 'REACTIVE_POWER', 'APPARENT_POWER', 'POWER_FACTOR', 'PHI']),
               Transfer3S: this.CreateSeriesTransfer(this, ['ACTIVE_POWER', 'REACTIVE_POWER']),
               Transfer1M: this.CreateSeriesTransfer(this, ['ACTIVE_POWER', 'REACTIVE_POWER']),
               TransferGraph: this.CreateSeriesTransfer(this, ['ACTIVE_POWER', 'REACTIVE_POWER', 'PERIOD_POWER_CONTRACTED', 'PERIOD_NUMBER', 'PERIOD_COLOR', 'BILLS']),
               TransferCubism: this.CreateSeriesTransfer(this, ['POWER_HARMONICS', 'ACTIVE_POWER', 'REACTIVE_POWER', 'VOLTAGE', 'CURRENT', 'POWER_FACTOR', 'PHI'])
      };
    },
    created: function() {
      this.TransferGraph.AggregFail = () => { this.Aggreg = null };
      this.TransferGraph.UpdatePrices(this.analyzer, this.Aggreg, this.Range, false);
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
        this.TransferGraph.UpdatePrices(this.analyzer, this.Aggreg, this.Range, false);
        this.TransferNow.UpdateNow(this.analyzer);
        this.TransferCubism.Update(this.analyzer, '1M', 'LastDay');
        this.Transfer1M.Update(this.analyzer, '1M', 'LastSamples');
        this.Transfer3S.Update(this.analyzer, '3S', 'LastSamples');
      },
      Aggreg: function() {
        this.TransferGraph.UpdatePrices(this.analyzer, this.Aggreg, this.Range, false);
      },
      Range: function(val, prev) {
        if(prev != null) this.TransferGraph.UpdatePrices(this.analyzer, this.Aggreg, this.Range, false);
      },
      Paused: function() {
        if(this.Paused == true) {
          this.TransferNow.Destroy();
        } else {
          this.TransferNow.UpdateNow(this.analyzer);
        }
      }
    },
    methods: {
      Export: function() {
        var Data = [['Time', 'Period', 'Contracted', 'Active power', 'Reactive power']];
        if(this.TransferGraph.Data != null) {
          for(var n in this.TransferGraph.Data.Sampletime) Data.push([this.TransferGraph.Data.Sampletime[n], 'P' + this.TransferGraph.Data.Period_Number[n], this.Round(this.TransferGraph.Data.Period_Power[n], 3), this.Round(this.TransferGraph.Data.Active_Power[n], 3), this.Round(this.TransferGraph.Data.Reactive_Power[n], 3)]);
        }
        return Object.seal(Data);
      }
    },
    components: { Power, Counters, Distribution, Waveform, Harmonics, Ga, Evolution, Parameters, Lissajou }
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