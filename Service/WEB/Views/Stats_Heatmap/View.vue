<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy stats-headmap">
          <div class="panel-heading">
            <span class="header-text">Energy Heatmap</span>
            <div class="header-buttons clearfix">
              <color-bar :width="140" :height="10" :range="Range" units="Wh"></color-bar>
              <toolbar-aggregation storage="HeatmapAggreg" :options="['15M', '1H']" v-model="Aggreg"></toolbar-aggregation>
              <toolbar-years :analyzer="analyzer" v-model="Year" aggreg="15M" storage="StatsYear"></toolbar-years>
              <toolbar-tools :options="['EXPORT']" ref="tools" v-on:ExportClick="ExportClick"></toolbar-tools>
            </div>
          </div>
          <div class="panel-body" style="min-height: 300px">
            <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
            <heatmap-table :data="Data" :aggreg="Aggreg" v-on:range="Range = $event"></heatmap-table>
          </div>
        </div>
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
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  import HeatmapTable from './HeatmapTable.vue';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { Aggreg: null, Range: null, Data: null, Year: null, ShowSpinner: true };
    },
    watch: {
      Year: function(val) {
        this.ShowSpinner = true;
        this.Data = null;
        var To = moment().year(val).endOf('year').valueOf();
        var Period = To - moment().year(val).startOf('year').valueOf();
        this.$http.post("getSeries", { Period: Period, To: To, Aggreg: '15M', LastSample: 0, Series: ['ACTIVE_POWER'], Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.ShowSpinner = false;
            this.Data = result.data;
          }
        );
      }
    },
    methods: {
      ExportClick: function() {
        this.$refs.tools.SaveCVS('Heatmap.csv', this.$refs.table.GetData());
      }
    },
    components: { HeatmapTable }
  }
</script>

<style>
  .stats-headmap .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>