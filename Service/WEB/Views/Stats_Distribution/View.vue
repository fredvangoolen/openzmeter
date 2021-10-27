<template>
  <q-page>
    <q-card>
      <q-toolbar class="bg-indigo-9 text-white q-pr-none">
        <q-toolbar-title>Energy distribution</q-toolbar-title>
        <q-btn stretch flat size="sm"><div>Year<br><strong>sdf</strong></div></q-btn>
        <q-separator dark vertical />
        <q-btn stretch flat icon="menu">
          <q-menu>
            <q-list dense>
              <q-item clickable v-close-popup>Reload</q-item>
              <q-item clickable v-close-popup>Export</q-item>
            </q-list>
          </q-menu>
        </q-btn>
      </q-toolbar>
      <q-card-section class="bg-grey-3 q-pa-xs">
        <color-bar :width="140" :height="10" :range="Range" units="Wh"></color-bar>
      </q-card-section>
      <q-separator />
                    <div class="header-buttons clearfix">
              <color-bar :width="140" :height="10" :range="Range" units="Wh"></color-bar>
              <toolbar-years storage="StatsYear" v-model="Year" aggreg="1H"></toolbar-years>
              <div class="btn-group">
                <button-export file="Distribution" :rows="Export"></button-export>
              </div>
            </div>


        <q-card-section>
            <calendar :data="Data" :year="Year" :range.sync="Range" @update:export="CalendarData=$event"></calendar>
        </q-card-section>
        <q-inner-loading :showing="ShowSpinner"><q-spinner-dots size="50px" color="orange-10" /></q-inner-loading>
      </q-card>
      <q-card class="col-12 col-lg-6" v-if="!ShowSpinner">
        <months :data="Data"></months>
      </q-card>
      <q-card class="col-12 col-lg-6" v-if="!ShowSpinner">
        <week-day :data="Data"></week-day>
    </q-card>
  </q-page>
</template>

<script>
  import Calendar from './Calendar.vue';
  import Months from './Months.vue';
  import WeekDay from './WeekDay.vue';
  import moment from 'moment';
  import 'moment-timezone';
  export default {
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
        let Count = 0;
        let Blocks = [];
        if((this.Year == null) || (this.analyzer == null)) return;
        let GetMonth = (month) => {
          let To = moment.tz(this.Analyzer.TimeZone).year(this.Year).month(month).endOf('month').valueOf();
          let From = moment.tz(this.Analyzer.TimeZone).year(this.Year).month(month).startOf('month').valueOf();
          this.Post('getCosts', { From: From, To: To, Series: ['Active_Energy', 'Period'], Analyzer: this.Analyzer.Analyzer, Group: 0 }).then(
            (result) => {
              Blocks[month] = (month == 0) ? result : result.slice(1);
              Count++;
              this.Progress = (Count / 12.0) * 100.0;
              if(Count == 12) {
                let Data = [];
                for(let n = 0; n < Count; n++) Data = Data.concat(Blocks[n]);
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