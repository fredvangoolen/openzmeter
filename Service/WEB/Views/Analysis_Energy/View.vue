<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <div class="huge header-text">Active Energy</div>
            <portal to="fs-toolbar" :disabled="!ExpandedActive" class="header-buttons">
              <toolbar-aggregation storage="AggregationEnergy" :options="['15M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range" :analyzer="analyzer"></toolbar-period>
              <div class="btn-group">
                <button-export file="Energy" :rows="Export"></button-export>
                <button-expand v-model="ExpandedActive"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="Active_Energy" :analyzer="analyzer" :data15M="Transfer15M.LastData" :data1H="Transfer1H.LastData" :dataGraph="TransferGraph.Data" :height.sync="ActiveHeight"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedActive" class="panel-body">
                  <spinner v-if="TransferGraph.Data==null"></spinner>
                  <energy serie="Active_Energy" :analyzer="analyzer" :data="TransferGraph.Data" :periods="TransferGraph.Periods" :bills="TransferGraph.Bills" :height="ExpandedActive?DialogHeight:ActiveHeight" @range="Range=$event"></energy>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="col-lg-12" v-if="analyzer.NominalFrequency!=0">
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
              <counters class="col-md-6 col-lg-4 col-xl-3" serie="Reactive_Energy" :analyzer="analyzer" :data15M="Transfer15M.LastData" :data1H="Transfer1H.LastData" :dataGraph="TransferGraph.Data" :height.sync="ReactiveHeight"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!ExpandedReactive" class="panel-body">
                  <spinner v-if="TransferGraph.Data==null"></spinner>
                  <energy serie="Reactive_Energy" :analyzer="analyzer" :data="TransferGraph.Data" :periods="TransferGraph.Periods" :bills="TransferGraph.Bills" :height="ExpandedReactive?DialogHeight:ReactiveHeight" @range="Range=$event"></energy>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
    <dlg-fs v-if="ExpandedActive" title="Active Energy" :height.sync="DialogHeight"></dlg-fs>
    <dlg-fs v-if="ExpandedReactive" title="Reactive Energy" :height.sync="DialogHeight"></dlg-fs>
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
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { Aggreg: null, Range: null, ActiveHeight: 300, ReactiveHeight: 300, ExpandedActive: false, ExpandedReactive: false, DialogHeight: 300,
               Transfer15M: this.CreateSeriesTransfer(this, ['ACTIVE_POWER', 'REACTIVE_POWER']),
               Transfer1H: this.CreateSeriesTransfer(this, ['ACTIVE_POWER', 'REACTIVE_POWER']),
               TransferGraph: this.PriceTransfer(['ACTIVE_ENERGY', 'REACTIVE_ENERGY', 'COLOR', 'PERIOD', 'BILL'])
      };
    },
    created: function() {
      this.TransferGraph.AggregFail = () => { this.Aggreg = null };
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
        this.Transfer1H.Update(this.analyzer, '1M', 'LastDay');
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
        var Data = [['Time', 'Period', 'Active energy', 'Reactive energy']];
        if(this.TransferGraph.Data != null) {
          var Div = (this.TransferGraph.Data.Aggreg == '1H') ? 1 : 4;
          for(var n in this.TransferGraph.Data.Sampletime) Data.push([this.TransferGraph.Data.Sampletime[n], 'P' + this.TransferGraph.Data.Period_Number[n], this.Round(this.TransferGraph.Data.Active_Power[n] / Div, 3), this.Round(this.TransferGraph.Data.Reactive_Power[n] / Div, 3)]);
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