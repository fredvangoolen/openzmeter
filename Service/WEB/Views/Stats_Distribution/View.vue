<template>
  <div v-if="analyzer != null">
    <z-row>
      <div class="col-lg-12">
        <div class="panel panel-active-energy">
          <div class="panel-heading">
            <span class="header-text">Energy distribution</span>
            <div class="header-buttons clearfix">
              <color-bar :width="140" :height="10" :range="Range" units="Wh"></color-bar>
              <toolbar-years storage="StatsYear" v-model="Year" :analyzer="analyzer" aggreg="1H"></toolbar-years>
              <z-button-group>
                <button-export file="Distribution" :rows="Export"></button-export>
              </z-button-group>
            </div>
          </div>
          <div class="panel-body">
            <spinner v-if="ShowSpinner" :progress="Progress"></spinner>
            <calendar :data="Data" :analyzer="analyzer" :year="Year" :range.sync="Range" @update:export="CalendarData=$event"></calendar>
          </div>
        </div>
      </div>
      <div class="col-md-12 col-lg-6" v-if="!ShowSpinner">
        <months :data="Data" :analyzer="analyzer"></months>
      </div>
      <div class="col-md-12 col-lg-6" v-if="!ShowSpinner">
        <week-day :data="Data" :analyzer="analyzer"></week-day>
      </div>
    </z-row>
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
      return { Range: [0, 1], Year: null, Data: null, ShowSpinner: true, Progress: 0, CalendarData: [] };
    },
    watch: {
      Year: function() { this.Update(); },
      analyzer: function() { this.Update(); }
    },
    methods: {
      Update: function() {
        this.ShowSpinner = true;
        this.Progress = 0;
        this.Data = null;
        var Count = 0;
        var Blocks = [];
        if((this.Year == null) || (this.analyzer == null)) return;
        var GetMonth = (month) => {
          var To = moment.tz(this.analyzer.TimeZone).year(this.Year).month(month).endOf('month').valueOf();
          var From = moment.tz(this.analyzer.TimeZone).year(this.Year).month(month).startOf('month').valueOf();
          this.Post('getCosts', { From: From, To: To, Series: ['Active_Energy', 'Period'], Analyzer: this.analyzer.Analyzer, Group: 0 }).then(
            (result) => {
              Blocks[month] = (month == 0) ? result : result.slice(1);
              Count++;
              this.Progress = (Count / 12.0) * 100.0;
              if(Count == 12) {
                var Data = [];
                for(var n = 0; n < Count; n++) Data = Data.concat(Blocks[n]);
                this.Data = Object.seal(Data);
                this.ShowSpinner = false;
              }
            }
          );
        };
        for(var month = 0; month < 12; month++) GetMonth(month);
      },
      Export: function() {
        return this.CalendarData;
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