<template>
  <q-page>
    <q-card>
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <div class="huge header-text">Active Energy</div>
            <portal to="fs-toolbar" :disabled="!ExpandedActive" class="header-buttons">
              <toolbar-select storage="Energy.Aggregation" :options="Aggregations(['15M', '1H'])" v-model="Aggreg" />
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <div class="btn-group">
                <button-export file="Energy" :rows="Export"></button-export>
                <button-expand v-model="ExpandedActive"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <div class="row">
              <!--counters class="col-md-6 col-lg-4 col-xl-3" serie="Active_Energy" :data15M="Transfer15M.LastData" :data1H="Transfer1H.LastData" :dataGraph="TransferGraph.Data" :height.sync="ActiveHeight"></counters-->
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedActive" class="panel-body">
                  <spinner v-if="TransferGraph.Data==null"></spinner>
                  <!--energy serie="Active_Energy" :data="TransferGraph.Data" :height="ExpandedActive?DialogHeight:ActiveHeight" @range="Range=$event"></energy-->
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12" v-if="Analyzer.NominalFrequency!=0">
        <div class="panel panel-reactive-energy">
          <div class="panel-heading">
            <div class="header-text">Fund. Reactive Energy</div>
            <portal to="fs-toolbar" :disabled="!ExpandedReactive" class="header-buttons">
              <div class="btn-group">
                <button-export file="Energy" :rows="Export"></button-export>
                <button-expand v-model="ExpandedReactive"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <div class="row">
              <!--counters class="col-md-6 col-lg-4 col-xl-3" serie="Reactive_Energy" :data15M="Transfer15M.LastData" :data1H="Transfer1H.LastData" :dataGraph="TransferGraph.Data" :height.sync="ReactiveHeight"></counters-->
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedReactive" class="panel-body">
                  <spinner v-if="TransferGraph.Data==null"></spinner>
                  <!--energy serie="Reactive_Energy" :data="TransferGraph.Data" :height="ExpandedReactive?DialogHeight:ReactiveHeight" @range="Range=$event"></energy-->
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
    </q-card>
    <dlg-fs v-if="ExpandedActive" title="Active Energy" :height.sync="DialogHeight"></dlg-fs>
    <dlg-fs v-if="ExpandedReactive" title="Reactive Energy" :height.sync="DialogHeight"></dlg-fs>
  </q-page>
</template>

<script>
  import Energy from './Energy.vue';
  import Counters from './Counters.vue';
  export default {
    data: function() {
      return { Aggreg: null, Range: null, ActiveHeight: 300, ReactiveHeight: 300, ExpandedActive: false, ExpandedReactive: false, DialogHeight: 300,
               Transfer15M: this.SeriesTransfer(['Active_Energy', 'Reactive_Energy']),
               Transfer1H: this.SeriesTransfer(['Active_Energy', 'Reactive_Energy']),
               TransferGraph: this.PriceTransfer(['Active_Energy', 'Reactive_Energy', 'Color', 'Period', 'Bill'])
      };
    },
    created: function() {
      this.Transfer15M.Update(this.analyzer, '15M', 'LastSamples');
      this.Transfer1H.Update(this.analyzer, '1H', 'LastSamples');
      this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
    },
    beforeDestroy: function() {
      this.Transfer15M.Destroy();
      this.Transfer1H.Destroy();
      this.TransferGraph.Destroy();
    },
    watch: {
      analyzer: function() {
        this.Transfer15M.Update(this.analyzer, '15M', 'LastHours');
        this.Transfer1H.Update(this.analyzer, '1H', 'LastDay');
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      },
      Aggreg: function(val, prev) {
        if(prev != null) this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      },
      Range: function() {
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      }
    },
    methods: {
      Export: function() {
        if(this.TransferGraph.Data == null) return [];
        var Data = [['Time', 'Active energy', 'Reactive energy']];
        var TimeCol = this.TransferGraph.Data.Headers.indexOf('Time');
        var ActiveCol = this.TransferGraph.Data.Headers.indexOf('Active_Energy');
        var ReactiveCol = this.TransferGraph.Data.Headers.indexOf('Reactive_Energy');
        for(var i = 0; i < this.TransferGraph.Data.Rows.length; i++) {
          if(this.TransferGraph.Data.Rows[i][ActiveCol] == null) continue;
          Data.push([this.analyzer.FormatDateTime(this.TransferGraph.Data.Rows[i][TimeCol]), this.Round(this.TransferGraph.Data.Rows[i][ActiveCol], 3), this.Round(this.TransferGraph.Data.Rows[i][ReactiveCol], 3)]);
        }
        return Object.seal(Data);
      }
    },
    components: { Energy, Counters }
  }
</script>

<style scoped>
  .header-text {
    font-size: 20px;
    line-height: 24px;
  }
</style>