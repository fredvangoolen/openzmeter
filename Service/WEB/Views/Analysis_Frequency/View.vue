<template>
  <div v-if="analyzer!=null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-frequency">
          <div class="panel-heading">
            <div class="huge header-text">Frequency</div>
            <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">
              <toolbar-aggregation storage="Aggregation" :options="['3S', '1M', '10M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <div class="btn-group">
                <button-export :rows="Export" file="Frequency"></button-export>
                <button-expand :expanded.sync="Expanded"></button-expand>
              </div>
            </portal>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" :analyzer="analyzer" :dataNow="DataNow" :dataGraph="DataGraph" :height.sync="Height"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!Expanded" class="panel-body">
                  <frequency :analyzer="analyzer" :data="DataGraph" :height="Expanded?DialogHeight:Height" @range="Range=$event"></frequency>
                </portal>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
    <dlg-fs v-if="Expanded" title="Frequency" :height.sync="DialogHeight"></dlg-fs>
  </div>
  <div v-else-if="analyzer == null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>
  </div>
</template>

<script>
  import Frequency from './Frequency.vue';
  import Counters from './Counters.vue';
  export default {
    props: { analyzer: Object },
    data: function() {
      return { Aggreg: null, ShowSpinner: true, Range: null, Range2:null, Height: 300, Expanded: false, DialogHeight: 300,
               DataNow: null, ControlNow: null, DataGraph: null, ControlGraph: null };
      },
    watch: {
      analyzer: function() { this.UpdateNowValues(); this.UpdateGraphValues(); },
      Aggreg: function() {  this.UpdateGraphValues(); },
      Range: function() { this.UpdateGraphValues(); },
      Range2: function(val) {
        console.log(val);
      }
    },
    computed: {
      Export: function() {
        if(this.DataGraph == null) return [];
        var Rows = [['Time', 'Frequency']];
        for(var n in this.DataGraph.Sampletime) Rows.push([this.DataGraph.Sampletime[n], this.DataGraph.Frequency[n]]);
        return Rows;
      }
    },
    created: function() {
      this.UpdateNowValues();
      this.UpdateGraphValues();
    },
    beforeDestroy: function() {
      this.CancelTransfer(this.ControlNow);
      this.CancelTransfer(this.ControlGraph);
    },
    methods: {
      UpdateNowValues: function() {
        this.ControlNow = this.CancelTransfer(this.ControlNow);
        if(this.analyzer == null) return;
        this.ControlNow.CancelFunc = this.analyzer.GetSeriesLast('3S', ['FREQUENCY'],
          (working) => {},
          (result) => { this.DataNow = result; },
          (timeout) => { this.ControlNow.Timer = setTimeout( () => { this.UpdateNowValues(); }, timeout); }
        );
      },
      UpdateGraphValues: function() {
        this.ControlGraph = this.CancelTransfer(this.ControlGraph);
        if((this.Aggreg == null) || (this.Range == null) || (this.analyzer == null)) return;
        this.ControlGraph.CancelFunc = this.analyzer.GetSeriesRange(this.Aggreg, this.Range, this.DataGraph, ['FREQUENCY'],
          (working) => { this.ShowSpinner = working; },
          (result) => { this.DataGraph = Object.freeze(result); },
          (timeout) => { this.ControlGraph.Timer = setTimeout( () => { this.UpdateGraphValues(); }, timeout); }
        );
      }
    },
    components: { Frequency, Counters }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
