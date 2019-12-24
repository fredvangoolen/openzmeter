<template>
  <div v-if="analyzer != null">
    <div class="row">
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <span class="header-text">Energy distribution</span>
            <div class="header-buttons clearfix">
              <color-bar :width="140" :height="10" :range="Range" units="Wh"></color-bar>
              <toolbar-years storage="StatsYear" v-model="Year" :analyzer="analyzer" aggreg="1H"></toolbar-years>
              <toolbar-tools ref="tools" :options="['EXPORT']" v-on:ExportClick="ExportClick"></toolbar-tools>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <calendar :data="Data" :year="Year" v-on:range="Range = $event"></calendar>
          </div>
        </div>
      </div>
      <div class="col-md-12 col-lg-6" v-if="!ShowSpinner"><months :data="Data"></months></div>
      <div class="col-md-12 col-lg-6" v-if="!ShowSpinner"><day :data="Data"></day></div>
    </div>
  </div>
  <div v-else-if="analyzer == null">
    <div class="alert alert-danger text-center">
      <p>No selected analyzer</p>
    </div>  
  </div>
</template>

<script>
  import Calendar from './Calendar.vue';
  import Months from './Months.vue';
  import Day from './Day.vue';
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { Range: null, Year: null, Data: null, ShowSpinner: true };
    },
    watch: {
      Year: function(val) {
        this.ShowSpinner = true;
        this.Data = null;
        var To = moment().year(val).endOf('year').valueOf();
        var Period = To - moment().year(val).startOf('year').valueOf();
        this.$http.post("getPrices", { Period: Period, To: To, Aggreg: '1H', LastSample: 0, Series: ['ACTIVE_POWER'], Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.ShowSpinner = false;
            this.Data = result.data;
          }
        );
      }
    },
    methods: {
      ExportClick: function() {
        var Data = [];
        Data.push(['Daily stats']);
        Data = this.$refs.calendar.GetData(Data);
        Data.push([]);
        Data.push(['Monthly stats']);
        Data = this.$refs.months.GetData(Data);
        Data.push([]);
        Data.push(['Weekday stats']);
        Data = this.$refs.day.GetData(Data);
        this.$refs.tools.SaveCVS('Distribution.csv', Data);
      }
    },
    components: { Calendar, Months, Day }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>