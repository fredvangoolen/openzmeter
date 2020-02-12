<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <div class="huge header-text">Energy consumption</div>
            <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
              <toolbar-aggregation storage="AggregationEnergy" v-model="Aggreg" :options="['15M', '1H']"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <div class="btn-group">
                <button-export file="Energy" :rows="[]"></button-export>
                <button-expand :expanded.sync="Expanded"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
            <div class="row">
              <counters class="col-md-5 col-lg-4 col-xl-3" :analyzer="analyzer" :height.sync="Height" :data="DataGraph"></counters>
              <div class="col-md-7 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!Expanded" class="panel-body">
                  <energy :analyzer="analyzer" :height="Expanded?DialogHeight:Height-10" :data="DataGraph" :range.sync="Range"></energy>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
      <dlg-fs v-if="Expanded" title="Energy consumption" :height.sync="DialogHeight"></dlg-fs>
    </div>
    <div class="row">
      <div class="col-lg-6 col-md-12">
        <graphs serie="VOLTAGE" :analyzer="analyzer" :data="Data3S" :paused.sync="Paused3S"></graphs>
      </div>
      <div class="col-lg-6 col-md-12">
        <graphs serie="CURRENT" :analyzer="analyzer" :data="Data3S" :paused.sync="Paused3S"></graphs>
      </div>
      <div v-bind:class="{'col-lg-6': (analyzer.NominalFrequency > 0.0)}" class="col-md-12">
        <graphs serie="ACTIVE_POWER" :analyzer="analyzer" :data="Data3S" :paused.sync="Paused3S"></graphs>
      </div>
      <div class="col-lg-6 col-md-12" v-if="analyzer.NominalFrequency > 0.0">
        <graphs serie="FREQUENCY" :analyzer="analyzer" :data="Data3S" :paused.sync="Paused3S"></graphs>
      </div>
    </div>
    <div class="row" v-if="analyzer.NominalFrequency > 0.0">
      <div class="col-lg-12"><variables :analyzer="analyzer" :data="Data1M" :dataNow="DataNow"></variables></div>
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
      return { Aggreg: null, Range: null, ShowSpinner: false, Height: 200, Paused3S: false, Expanded: false, DialogHeight: 200,
               DataNow: null, ControlNow: null, Data3S: null, Control3S: null, Data1M: null, Control1M: null, DataGraph: null, ControlGraph: null };
    },
    watch: {
      analyzer: function() {
        this.DataNow = null;
        this.Data3S = null;
        this.Data1M = null;
        this.DataGraph = null;
        this.UpdateNowValues();
        this.Update3SValues();
        this.Update1MValues();
        this.UpdateGraphValues();
      },
      Aggreg: function(val, prev) {
        if(prev != null) this.UpdateGraphValues();
      },
      Range: function() { this.ShowSpinner= true; console.log(this.Range); this.UpdateGraphValues(); }
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
