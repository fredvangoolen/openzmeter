<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <div class="header-text">Energy consumption</div>
            <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
              <toolbar-aggregation storage="AggregationEnergy" v-model="Aggreg" :options="['15M', '1H']"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range" :analyzer="analyzer"></toolbar-period>
              <div class="btn-group">
                <button-export file="Energy" :rows="Export"></button-export>
                <button-expand v-model="Expanded"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <div class="row">
              <counters class="col-md-5 col-lg-4 col-xl-3" :analyzer="analyzer" :height.sync="Height" :data="TransferGraph.Data"></counters>
              <div class="col-md-7 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!Expanded" class="panel-body">
                  <spinner v-if="TransferGraph.Data==null"></spinner>
                  <energy :analyzer="analyzer" :height="Expanded?DialogHeight:Height-10" :data="TransferGraph.Data" :periods="TransferGraph.Periods" @range="Range=$event"></energy>
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
        <graphs ref="g0" serie="VOLTAGE" :analyzer="analyzer" :data="Transfer3S.Data" :paused.sync="Paused3S"></graphs>
      </div>
      <div class="col-lg-6 col-md-12">
        <graphs ref="g1" serie="CURRENT" :analyzer="analyzer" :data="Transfer3S.Data" :paused.sync="Paused3S"></graphs>
      </div>
      <div v-bind:class="{'col-lg-6': (analyzer.NominalFrequency > 0.0)}" class="col-md-12">
        <graphs ref="g2" serie="ACTIVE_POWER" :analyzer="analyzer" :data="Transfer3S.Data" :paused.sync="Paused3S"></graphs>
      </div>
      <div class="col-lg-6 col-md-12" v-if="analyzer.NominalFrequency > 0.0">
        <graphs ref="g3" serie="FREQUENCY" :analyzer="analyzer" :data="Transfer3S.Data" :paused.sync="Paused3S"></graphs>
      </div>
    </div>
    <div class="row" v-if="analyzer.NominalFrequency > 0.0">
      <div class="col-lg-12">
        <variables :analyzer="analyzer" :data="Transfer1M.Data" :dataNow="TransferNow.Data"></variables>
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
  import Energy from './Energy.vue';
  import Counters from './Counters.vue';
  import Graphs from './GraphsPanel.vue';
  import Variables from './Variables.vue';
  import 'JS/dygraph-synchronizer.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { analyzer: { type: Object } },
    data () {
      return { Aggreg: null, Range: null, Height: 200, Paused3S: false, Expanded: false, DialogHeight: 200,
               TransferNow: this.CreateSeriesTransfer(this, ['VOLTAGE_THD', 'ACTIVE_POWER', 'REACTIVE_POWER', 'CURRENT_THD', 'POWER_FACTOR', 'PHI']),
               Transfer3S: this.CreateSeriesTransfer(this, ['VOLTAGE', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY']),
               Transfer1M: this.CreateSeriesTransfer(this,  ['VOLTAGE_THD', 'ACTIVE_POWER', 'REACTIVE_POWER', 'CURRENT_THD', 'POWER_FACTOR', 'PHI']),
               TransferGraph: this.CreateSeriesTransfer(this, ['ACTIVE_POWER', 'EXPENSE', 'PERIOD_COLOR', 'PERIOD_NUMBER', 'BILLS'])
      };
    },
    created: function() {
      this.TransferNow.UpdateNow(this.analyzer);
      this.Transfer3S.Update(this.analyzer, '3S', 'LastHours');
      this.Transfer1M.Update(this.analyzer, '1M', 'LastDay');
      this.TransferGraph.UpdatePrices(this.analyzer, this.Aggreg, this.Range);
    },
    beforeDestroy: function() {
      this.TransferNow.Destroy();
      this.Transfer3S.Destroy();
      this.Transfer1M.Destroy();
      this.TransferGraph.Destroy();
    },
    mounted: function() {
      this.BindGraphs();
    },
    watch: {
      analyzer: function() {
        this.TransferNow.UpdateNow(this.analyzer);
        this.Transfer3S.Update(this.analyzer, '3S', 'LastHours');
        this.Transfer1M.Update(this.analyzer, '1M', 'LastDay');
        this.TransferGraph.UpdatePrices(this.analyzer, this.Aggreg, this.Range);
        this.BindGraphs();
      },
      Aggreg: function(val, prev) {
        if(prev != null) this.TransferGraph.UpdatePrices(this.analyzer, this.Aggreg, this.Range);
      },
      Range: function() {
        this.TransferGraph.UpdatePrices(this.analyzer, this.Aggreg, this.Range);
      }
    },
    methods: {
      BindGraphs: function() {
        if(this.analyzer != null) setTimeout(() => {
          var Graphs = [];
          if(this.$refs.g0 != null) Graphs.push(this.$refs.g0.$refs.graph.Graph);
          if(this.$refs.g1 != null) Graphs.push(this.$refs.g1.$refs.graph.Graph);
          if(this.$refs.g2 != null) Graphs.push(this.$refs.g2.$refs.graph.Graph);
          if(this.$refs.g3 != null) Graphs.push(this.$refs.g3.$refs.graph.Graph);
          Dygraph.synchronize(Graphs, { range: false, zoom: false });
        }, 500);
      },
      Export: function() {
        if(this.TransferGraph.Data == null) return [];
        var Div = (this.TransferGraph.Data.Aggreg == '1H') ? 1 : 4;
        var Data = [['Sampletime', 'Active Energy', 'Period name', 'Period expense']];
        for(var i = 0; i < this.TransferGraph.Data.Sampletime.length; i++) Data.push([this.TransferGraph.Data.Sampletime[i], this.Round(this.TransferGraph.Data.Active_Power[i] / Div, 3), 'P' + this.TransferGraph.Data.Period_Number[i], this.Round(this.TransferGraph.Data.Expense[i], 3)]);
        return Object.seal(Data);
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