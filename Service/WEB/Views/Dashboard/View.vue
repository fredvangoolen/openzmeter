<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <div class="huge header-text">Energy consumption</div>
            <div class="header-buttons clearfix" ref="Toolbar">
              <toolbar-aggregation storage="AggregationEnergy" v-model="Aggreg" :options="['15M', '1H']"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <!--toolbar-tools :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools-->
            </div>
          </div>
          <div class="panel-body">
            <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
            <div class="row">
              <counters class="col-md-5 col-lg-4 col-xl-3"></counters>
              <energy class="col-md-7 col-lg-8 col-xl-9" :analyzer="analyzer"></energy>
            </div>
          </div>
        </div>
      </div>
    </div>
    <div class="row">
      <div class="col-lg-6 col-md-12">
        <graphs serie="VOLTAGE" :analyzer="analyzer" :data="Data3S"></graphs>
      </div>
      <div class="col-lg-6 col-md-12">
        <graphs serie="CURRENT" :analyzer="analyzer" :data="Data3S"></graphs>
      </div>
      <div v-bind:class="{'col-lg-6': (analyzer.NominalFreq > 0.0)}" class="col-md-12">
        <graphs serie="ACTIVE_POWER" :analyzer="analyzer" :data="Data3S"></graphs>
      </div>
      <div class="col-lg-6 col-md-12" v-if="analyzer.NominalFreq > 0.0">
        <graphs serie="FREQUENCY" :analyzer="analyzer" :data="Data3S"></graphs>
      </div>
    </div>
    <div class="row" v-if="analyzer.NominalFreq > 0.0">
      <div class="col-lg-12"><variables :analyzer="analyzer"></variables></div>
    </div>
  </div>
  <div v-else-if="analyzer == null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>
  </div>
</template>

<script>
  import Energy from './Energy.vue';
  import Counters from './Counters.vue';
  import Graphs from './GraphsPanel.vue';
  import Variables from './Variables.vue';
  import moment from 'moment';
  import 'JS/dygraph-synchronizer.js';
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { analyzer: { type: Object } },
    data () {
      return { Aggreg: null, Range: null, ShowSpinner: false, Data3S: null, Timer3S: null };
    },
    watch: {
      analyzer: function() { this.Update3S(); this.UpdateNowValues(); this.UpdateCounters(); this.UpdateCubism(); this.UpdateGraphValues(); },
      Aggreg: function() {  this.UpdateGraphValues(); },
      Range: function() { this.UpdateGraphValues(); }
    },
    computed: {
      ExportData: function() {
        return [];
      }
    },
    created: function() {
      this.Update3S();
//      this.UpdateNowValues();
//      this.UpdateGraphValues();
//      this.UpdateCounters();
//      this.UpdateCubism();
    },
    beforeDestroy: function() {
      if(this.NowTimer != null) clearTimeout(this.NowTimer);
      if(this.CounterTimer != null) clearTimeout(this.CounterTimer);
      if(this.GraphTimer != null) clearTimeout(this.GraphTimer);
      if(this.CubismTimer != null) clearTimeout(this.CubismTimer);
    },
    methods: {
      Update3S: function() {
        if(this.Timer3S != null) clearTimeout(this.Timer3S);
        if(this.analyzer == null) return;
        var Now = moment().tz(this.analyzer.TimeZone).valueOf();
        this.analyzer.GetSeriesRange('3S', [Now - 7200000, Now], this.Data3S, ['VOLTAGE', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY'],
          (working) => {},
          (result) => { this.Data3S = Object.freeze(result); },
          (timeout) => { if(timeout != null) this.Timer3S = setTimeout( () => { this.Update3S(); }, timeout); }
        );

      },
      UpdateCubism: function() {
        return;
        if(this.CubismTimer != null) clearTimeout(this.CubismTimer);
        if(this.analyzer == null) return;
        this.analyzer.GetSeriesRange('1M', 'Day', this.CubismData, ['POWER_HARMONICS', 'ACTIVE_POWER', 'REACTIVE_POWER', 'VOLTAGE', 'CURRENT', 'POWER_FACTOR', 'PHI'],
          (working) => {},
          (result) => { this.CubismData = Object.freeze(result); },
          (timeout) => { if(timeout != null) this.CubismTimer = setTimeout( () => { this.UpdateCubism(); }, timeout); }
        );
      },
      UpdateNowValues: function() {
        return;
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
        return;
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
        return;
        if(this.GraphTimer != null) clearTimeout(this.GraphTimer);
        if((this.Aggreg == null) || (this.Range == null) || (this.analyzer == null)) return;
        this.analyzer.GetPricesRange(this.Aggreg, this.Range, this.GraphData,
          (working) => { this.ShowSpinner = working; },
          (result) => { this.GraphData = Object.freeze(result); },
          (timeout) => { if(timeout != null) this.GraphTimer = setTimeout( () => { this.UpdateGraphValues(); }, timeout); }
        );
      }
    },

    components: { Energy, Counters, Graphs, Variables }
  }
</script>

<style scoped>
  .header-text {
    font-size: 20px;
    line-height: 24px;
  }
</style>
