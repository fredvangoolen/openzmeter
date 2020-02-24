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
              <div class="btn-group">
                <button-export file="Distribution" :rows="CalendarRows"></button-export>
              </div>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner"></spinner>
            <calendar :data="Data" :analyzer="analyzer" :year="Year" :range.sync="Range" :export.sync="CalendarRows"></calendar>
          </div>
        </div>
      </div>
      <div class="col-md-12 col-lg-6" v-if="!ShowSpinner">
        <months :data="Data" :analyzer="analyzer"></months>
      </div>
      <div class="col-md-12 col-lg-6" v-if="!ShowSpinner">
        <week-day :data="Data" :analyzer="analyzer"></week-day>
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
  import Calendar from './Calendar.vue';
  import Months from './Months.vue';
  import WeekDay from './WeekDay.vue';
  import moment from 'moment';
  import 'moment-timezone';
  export default {
    props: { analyzer: { type: Object } },
    data: function() {
      return { Range: [0, 1], Year: null, Data: null, ShowSpinner: true, CalendarRows: [] };
    },
    watch: {
      Year: function(val) {
        this.ShowSpinner = true;
        this.Data = {};
        var To = moment.tz(this.analyzer.TimeZone).year(val).endOf('year').valueOf();
        var From = moment.tz(this.analyzer.TimeZone).year(val).startOf('year').valueOf();
        this.$http.post("getPrices", { From: From, To: To, Aggreg: '1H', Series: ['ACTIVE_POWER'], Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.ShowSpinner = false;
            this.Data = result.data;
          }
        );
      }
    },
    components: { Calendar, Months, WeekDay }
  }
</script>

<style scoped>
  .header-text {
    font-size: 20px;
    line-height: 24px;
  }
</style>