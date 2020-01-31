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
              <div class="btn-group">
              </div>
              <!--toolbar-tools :options="['EXPORT', 'MAXIMIZE']" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" ref="tools"></toolbar-tools-->
            </div>
          </div>
          <div class="panel-body">
            <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
            <div class="row">
              <counters class="col-md-5 col-lg-4 col-xl-3" :analyzer="analyzer" :height.sync="Height" :data="DataGraph"></counters>
              <energy class="col-md-7 col-lg-8 col-xl-9" :analyzer="analyzer" :height="Height" :data="DataGraph"></energy>
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
      <div v-bind:class="{'col-lg-6': (analyzer.NominalFrequency > 0.0)}" class="col-md-12">
        <graphs serie="ACTIVE_POWER" :analyzer="analyzer" :data="Data3S"></graphs>
      </div>
      <div class="col-lg-6 col-md-12" v-if="analyzer.NominalFrequency > 0.0">
        <graphs serie="FREQUENCY" :analyzer="analyzer" :data="Data3S"></graphs>
      </div>
    </div>
    <div class="row" v-if="analyzer.NominalFreq > 0.0">
      <div class="col-lg-12"><variables :analyzer="analyzer" :data="Data1M"></variables></div>
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
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { analyzer: { type: Object } },
    data () {
      return { Aggreg: null, Range: null, ShowSpinner: false, Height: 200,
               DataNow: null, ControlNow: null, Data3S: null, Control3S: null, Data1M: null, Control1M: null, DataGraph: null, ControlGraph: null };
    },
    watch: {
      analyzer: function() { this.UpdateNowValues(); this.Update3SValues(); this.Update1MValues(); this.UpdateGraphValues(); },
      Aggreg: function(val, prev) { if(prev != null) this.UpdateGraphValues(); },
      Range: function() { this.UpdateGraphValues(); }
    },
    computed: {
      ExportData: function() {
        return [];
      }
    },
    created: function() {
      this.UpdateNowValues();
      this.Update3SValues();
      this.Update1MValues();
      this.UpdateGraphValues();
    },
    beforeDestroy: function() {
      this.CancelTransfer(this.ControlNow);
      this.CancelTransfer(this.Control3S);
      this.CancelTransfer(this.Control1M);
      this.CancelTransfer(this.ControlGraph);
    },
    methods: {
      UpdateNowValues: function() {
        this.ControlNow = this.CancelTransfer(this.ControlNow);
        if(this.analyzer == null) return;
        this.ControlNow.CancelFunc = this.analyzer.GetSeriesNow(['VOLTAGE_THD', 'ACTIVE_POWER', 'REACTIVE_POWER', 'CURRENT_THD', 'POWER_FACTOR', 'PHI'],
          (working) => {},
          (result) => {this.DataNow = Object.freeze(result); },
          (timeout) => {this.ControlNow.Timer = setTimeout(() => { this.UpdateNowValues(); }, timeout); }
        );
      },
      Update3SValues: function() {
        this.Control3S = this.CancelTransfer(this.Control3S);
        if(this.analyzer == null) return;
        var Now = moment().tz(this.analyzer.TimeZone).valueOf();
        this.Control3S.CancelFunc = this.analyzer.GetSeriesRange('3S', [Now - 7200000, Now], this.Data3S, ['VOLTAGE', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY'],
          (working) => {},
          (result) => { this.Data3S = Object.freeze(result); },
          (timeout) => { this.Control3S.Timer = setTimeout( () => { this.Update3SValues(); }, timeout); }
        );
      },
      Update1MValues: function() {
        this.Control1M = this.CancelTransfer(this.Control1M);
        if(this.analyzer == null) return;
        this.Control1M.CancelFunc = this.analyzer.GetSeriesRange('1M', 'LastDay', this.Data1M, ['VOLTAGE_THD', 'ACTIVE_POWER', 'REACTIVE_POWER', 'CURRENT_THD', 'POWER_FACTOR', 'PHI'],
          (working) => {},
          (result) => { this.Data1M = Object.freeze(result); },
          (timeout) => { this.Control1M.Timer = setTimeout( () => { this.Update1MValues(); }, timeout); }
        );
      },
      UpdateCounters: function() {
        return;
//        if(this.CounterTimer != null) clearTimeout(this.CounterTimer);
//        if(this.analyzer == null) return;
//        this.analyzer.GetSeriesLast('3S', ['ACTIVE_POWER', 'REACTIVE_POWER'],
//          (working) => {},
//          (result) => { this.Data3S = result; },
//          (timeout) => {
//            this.analyzer.GetSeriesLast('1M', ['ACTIVE_POWER', 'REACTIVE_POWER'],
//              (working) => {},
//              (result) => { this.Data1M = result; },
//              (timeout) => { this.CounterTimer = setTimeout(() => { this.UpdateCounters(); }, 3000); }
//            );
//          }
//        );
      },
      UpdateGraphValues: function() {
        this.ControlGraph = this.CancelTransfer(this.ControlGraph);
        if((this.Aggreg == null) || (this.Range == null) || (this.analyzer == null)) return;
        this.ControlGraph.CancelFunc = this.analyzer.GetPricesRange(this.Aggreg, this.Range, this.DataGraph,
          (working) => { this.ShowSpinner = working; },
          (result) => { this.DataGraph = Object.freeze(result); },
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
