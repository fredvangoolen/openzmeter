<template>
  <q-page class="q-gutter-sm">
    <q-card>
      <q-toolbar class="bg-indigo-9 text-white">
        <q-toolbar-title>Energy consumption</q-toolbar-title>
            <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
              <toolbar-aggregation storage="AggregationEnergy" v-model="Aggreg" :options="['15M', '1H']"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <div class="btn-group">
                <button-export file="Energy" :rows="Export"></button-export>
                <button-expand v-model="Expanded"></button-expand>
              </div>
            </portal>
      </q-toolbar>

            <div class="row">
              <counters class="col-md-5 col-lg-4 col-xl-3" :height.sync="Height" :data="TransferGraph.Data"></counters>
              <div class="col-md-7 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!Expanded" class="panel-body">
                  <spinner v-if="TransferGraph.Data==null"></spinner>
                  <!--energy :height="Expanded?DialogHeight:Height-10" :data="TransferGraph.Data" @range="Range=$event"></energy-->
                </portal>
              </div>
            </div>


    </q-card>
    <!--dlg-fs v-if="Expanded" title="Energy consumption" :height.sync="DialogHeight"></dlg-fs-->

    <div class="row">
      <div class="col-lg-6 col-md-12">
        <!--graphs ref="g0" serie="Voltage" :data="Transfer3S.Data" :last="Transfer3S.LastData" :paused.sync="Paused3S"></graphs-->
      </div>
      <div class="col-lg-6 col-md-12">
        <!--graphs ref="g1" serie="Current" :data="Transfer3S.Data" :last="Transfer3S.LastData" :paused.sync="Paused3S"></graphs-->
      </div>
      <div :class="{'col-lg-6': (Analyzer.NominalFrequency > 0.0)}" class="col-md-12">
        <!--graphs ref="g2" serie="Active_Power" :data="Transfer3S.Data" :last="Transfer3S.LastData" :paused.sync="Paused3S"></graphs-->
      </div>
      <div class="col-lg-6 col-md-12" v-if="Analyzer.NominalFrequency > 0.0">
        <!--graphs ref="g3" serie="Frequency" :data="Transfer3S.Data" :last="Transfer3S.LastData" :paused.sync="Paused3S"></graphs-->
      </div>
    </div>
    <div class="row" v-if="Analyzer.NominalFrequency > 0.0">
      <div class="col-lg-12">
        <!--variables :data="Transfer1M.Data" :dataNow="TransferNow.Data"></variables-->
      </div>
    </div>
  </q-page>
</template>

<script>
  import Energy from './Energy.vue';
  import Counters from './Counters.vue';
  import Graphs from './GraphsPanel.vue';
  import Variables from './Variables.vue';
  import 'JS/dygraph-synchronizer.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    data: function() {
      return { Aggreg: null, Range: null, Height: 200, Paused3S: false, Expanded: false, DialogHeight: 200,
               TransferNow: this.NowTransfer(['Voltage_THD', 'Active_Power', 'Reactive_Power', 'Current_THD', 'Power_Factor', 'Phi']),
               Transfer3S: this.SeriesTransfer(['Voltage', 'Current', 'Active_Power', 'Frequency']),
               Transfer1M: this.SeriesTransfer(['Voltage_THD', 'Active_Power', 'Reactive_Power', 'Current_THD', 'Power_Factor', 'Phi']),
               TransferGraph: this.PriceTransfer(['Active_Energy', 'Cost', 'Color', 'Period', 'Bill'])
      };
    },
    created: function() {
      this.TransferNow.Update(this.analyzer);
      this.Transfer3S.Update(this.analyzer, '3S', 'LastHours');
      this.Transfer1M.Update(this.analyzer, '1M', 'LastDay');
      this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
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
        this.TransferNow.Update(this.analyzer);
        this.Transfer3S.Update(this.analyzer, '3S', 'LastHours');
        this.Transfer1M.Update(this.analyzer, '1M', 'LastDay');
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
        this.BindGraphs();
      },
      Aggreg: function(val, prev) {
        if(prev != null) this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      },
      Range: function() {
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
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
        var Data = [['Time', 'Active Energy', 'Period expense']];
        var TimeCol = this.TransferGraph.Data.Headers.indexOf('Time');
        var ActiveCol = this.TransferGraph.Data.Headers.indexOf('Active_Energy');
        var ExpenseCol = this.TransferGraph.Data.Headers.indexOf('Cost');
        for(var i = 0; i < this.TransferGraph.Data.Rows.length; i++) {
          if(this.TransferGraph.Data.Rows[i][ActiveCol] == null) continue;
          Data.push([this.analyzer.FormatDateTime(this.TransferGraph.Data.Rows[i][TimeCol]), this.Round(this.TransferGraph.Data.Rows[i][ActiveCol], 3), this.Round(this.TransferGraph.Data.Rows[i][ExpenseCol], 3)]);
        }
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