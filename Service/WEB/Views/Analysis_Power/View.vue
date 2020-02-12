<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-power">
          <div class="panel-heading">
            <div class="huge header-text">Active Power</div>
            <div class="header-buttons clearfix" ref="ToolbarActive">
              <toolbar-aggregation storage="AggregationEnergy" :options="['15M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <div class="btn-group">
                <button-export file="Power" :rows="[]"></button-export>
                <button-expand :expanded.sync="ExpandedActive"></button-expand>
              </div>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="ACTIVE_POWER" :analyzer="analyzer" :data3S="Data3S" :data1M="Data1M" :dataGraph="GraphData" :height.sync="ActiveHeight"></counters>
              <power class="col-md-6 col-lg-8 col-xl-9" serie="ACTIVE_POWER" :analyzer="analyzer" :data="GraphData" :height="ActiveHeight"></power>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12">
        <div class="panel panel-reactive-power">
          <div class="panel-heading">
            <div class="header-text">Fund. Reactive Power</div>
            <div class="header-buttons clearfix" ref="ToolbarReactive">
            <div class="btn-group">
              <button-export file="Power" :rows="[]"></button-export>
              <button-expand :expanded.sync="ExpandedActive"></button-expand>
            </div>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="REACTIVE_POWER" :analyzer="analyzer" :data3S="Data3S" :data1M="Data1M" :dataGraph="GraphData" :height.sync="ReactiveHeight"></counters>
              <power class="col-md-6 col-lg-8 col-xl-9" serie="REACTIVE_POWER" :analyzer="analyzer" :data="GraphData" :height="ReactiveHeight"></power>
            </div>
          </div>
        </div>
      </div>
    </div>
    <div class="col-lg-12">
      <detail-tabs :tabs="['Waveform', 'Harmonics', 'Evolution', 'Distribution', 'Parameters']" storage="PowerDetailTab" v-model="ActiveTab"></detail-tabs>
      <waveform v-if="ActiveTab=='Waveform'" :analyzer="analyzer"></waveform>
      <harmonics v-if="ActiveTab=='Harmonics'" :analyzer="analyzer"></harmonics>
      <evolution v-if="ActiveTab=='Evolution'" :analyzer="analyzer"></evolution>
      <distribution v-if="ActiveTab=='Distribution'" :analyzer="analyzer"></distribution>
      <parameters v-if="ActiveTab=='Parameters'" :analyzer="analyzer"></parameters>
    </div>
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
      return { Aggreg: null, ShowSpinner: true, ActiveTab: null,
               Data3S: null, Data1M: null, GraphData: null, NowData: null, CubismData: null,
               CounterTimer: null, NowTimer: null, CubismTimer: null, GraphTimer: null,
               Range: null, ActiveHeight: 300, ReactiveHeight: 300 };
    },
    watch: {
      analyzer: function() { this.UpdateNowValues(); this.UpdateCounters(); this.UpdateCubism(); this.UpdateGraphValues(); },
      Aggreg: function() {  this.UpdateGraphValues(); },
      Range: function() { this.UpdateGraphValues(); }
    },
    computed: {
      ExportData: function() {
        return [];
      }
    },
    created: function() {
      this.UpdateNowValues();
      this.UpdateGraphValues();
      this.UpdateCounters();
      this.UpdateCubism();
    },
    beforeDestroy: function() {
      if(this.NowTimer != null) clearTimeout(this.NowTimer);
      if(this.CounterTimer != null) clearTimeout(this.CounterTimer);
      if(this.GraphTimer != null) clearTimeout(this.GraphTimer);
      if(this.CubismTimer != null) clearTimeout(this.CubismTimer);
    },
    methods: {
      UpdateCubism: function() {
        if(this.CubismTimer != null) clearTimeout(this.CubismTimer);
        if(this.analyzer == null) return;
        this.analyzer.GetSeriesRange('1M', 'Day', this.CubismData, ['POWER_HARMONICS', 'ACTIVE_POWER', 'REACTIVE_POWER', 'VOLTAGE', 'CURRENT', 'POWER_FACTOR', 'PHI'],
          (working) => {},
          (result) => { this.CubismData = Object.freeze(result); },
          (timeout) => { if(timeout != null) this.CubismTimer = setTimeout( () => { this.UpdateCubism(); }, timeout); }
        );
      },
      UpdateNowValues: function() {
        if(this.NowTimer != null) clearTimeout(this.NowTimer);
        if(this.analyzer == null) return;
        this.$http.post('getSeriesNow', { Series: ['VOLTAGE_SAMPLES', 'CURRENT_SAMPLES', 'POWER_HARMONICS', 'CURRENT', 'ACTIVE_POWER', 'REACTIVE_POWER', 'APPARENT_POWER', 'POWER_FACTOR', 'PHI'], Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.NowData = (result.status == 200) ? Object.freeze(result.data) : null;
            this.NowTimer = setTimeout(() => { this.UpdateNowValues(); }, 2000);
          }
        );
      },
      UpdateCounters: function() {
        if(this.CounterTimer != null) clearTimeout(this.CounterTimer);
        if(this.analyzer == null) return;
        this.analyzer.GetSeriesLast('3S', ['ACTIVE_POWER', 'REACTIVE_POWER'],
          (working) => {},
          (result) => { this.Data3S = result; },
          (timeout) => {
            this.analyzer.GetSeriesLast('1M', ['ACTIVE_POWER', 'REACTIVE_POWER'],
              (working) => {},
              (result) => { this.Data1M = result; },
              (timeout) => { this.CounterTimer = setTimeout(() => { this.UpdateCounters(); }, 3000); }
            );
          }
        );
      },
      UpdateGraphValues: function() {
        if(this.GraphTimer != null) clearTimeout(this.GraphTimer);
        if((this.Aggreg == null) || (this.Range == null) || (this.analyzer == null)) return;
        this.analyzer.GetPricesRange(this.Aggreg, this.Range, this.GraphData,
          (working) => { this.ShowSpinner = working; },
          (result) => { this.GraphData = Object.freeze(result); },
          (timeout) => { if(timeout != null) this.GraphTimer = setTimeout( () => { this.UpdateGraphValues(); }, timeout); }
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
</style>