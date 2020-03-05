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
              <div class="btn-group">
                <button-export file="Heatmap" :rows="Export"></button-export>
              </div>
            </div>
          </div>
          <div class="panel-body" style="min-height: 300px">
            <spinner :class="{ 'hide': !ShowSpinner }"></spinner>
            <heatmap-table :data="Data" :aggreg="Aggreg" :analyzer="analyzer" :range.sync="Range" @update:export="ExportData=$event"></heatmap-table>
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
    props: { analyzer: Object },
    data: function() {
      return { Aggreg: '1H', Range: [0, 1], Data: {}, Year: 0, ShowSpinner: true, ExportData: null };
    },
    watch: {
      Year: function(val) {
        this.ShowSpinner = true;
        this.Data = null;
        var To = moment.tz(this.analyzer.TimeZone).year(val).endOf('year').valueOf();
        var From = moment.tz(this.analyzer.TimeZone).year(val).startOf('year').valueOf();
        this.$http.post('getSeries', { From: From, To: To, Aggreg: '15M', Series: ['ACTIVE_POWER'], Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.ShowSpinner = false;
            this.Data = Object.seal(result.data);
          },
          (fail) => {}
        );
      }
    },
    methods: {
      Export: function() {
        return this.ExportData;
      }
    },
    components: { HeatmapTable }
  }
</script>

<style>
  .stats-headmap .header-text {
    font-size: 20px;
    line-height: 24px;
  }
</style>