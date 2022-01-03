<template>
  <q-page>
    <q-card>
      <q-toolbar class="bg-indigo-9 text-white q-pr-none">
        <q-toolbar-title>Energy distribution</q-toolbar-title>
        <toolbar-year storage="Stats_Distribution.Year" aggreg="1H" v-model="Year" />
        <q-separator dark vertical />
        <toolbar-export file="Distribution" :rows="Export" />
      </q-toolbar>
      <q-linear-progress v-if="Data.Progress != null" color="indigo-9" :value="Data.Progress" class="q-ma-none" />

      <q-card-section class="bg-grey-3 q-pa-xs text-right">
        <color-bar :width="140" :height="10" :range="Range" units="Wh"></color-bar>
      </q-card-section>
      <q-separator />
      <q-card-section>
        <calendar :data="Data" :year="Year" :range.sync="Range" @update:export="CalendarData=$event"></calendar>
      </q-card-section>
    </q-card>
    <div class="row" v-if="Data.Progress==null">
      <q-card class="col-12 col-lg-6">
      <!--months :data="Data"></months-->
      </q-card>
      <q-card class="col-12 col-lg-6">
        <!--week-day :data="Data"></week-day-->
      </q-card>
    </div>
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
      return { Data: this.Analyzer.Queries.Suscribe('Costs', ['Active_Energy', 'Period'], 600000),
               Year: null, Range: [0, 1], CalendarData: [], Max: 0 };
    },
    unmounted: function() {
      this.Data.Stop();
    },
    watch: {
      Year: function(val) {
        let YearDate = moment.tz(this.Analyzer.TimeZone).year(val);
        this.Data.SetRange([YearDate.startOf('year').valueOf(), YearDate.endOf('year').valueOf()]);
      },
    },
    computed: {
      YearsList: function() {
        return this.Years(this.Analyzer.Ranges.Aggreg1H).reverse();
      }
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