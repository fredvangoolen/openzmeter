<template>
  <q-page>
    <q-card>
      <q-toolbar class="bg-indigo-9 text-white q-pr-none">
        <q-toolbar-title>Energy Heatmap</q-toolbar-title>
        <toolbar-select name="Aggregation" storage="Stats_Heatmap.Aggreg" :options="Aggregs" v-model="Aggreg" />
        <q-separator dark vertical />
        <toolbar-select name="Year" storage="Stats_Heatmap.Year" :options="Years" v-model="Year" />
        <q-separator dark vertical />
        <q-btn stretch dense flat icon="menu">
          <q-menu>
            <q-list dense>
              <q-item clickable v-close-popup>Reload</q-item>
              <q-item clickable v-close-popup>Export CSV</q-item>
              <q-item clickable v-close-popup>Export SVG</q-item>
            </q-list>
          </q-menu>
        </q-btn>
      </q-toolbar>
      <q-linear-progress stripe v-if="Data.Progress != null" color="indigo-9" :value="Data.Progress" class="q-ma-none" />
      <!--div style="height:300px"></div-->
      <!--q-card-section class="bg-grey-3 q-pa-xs">
        <color-bar :width="140" :height="10" :range="Range" units="Wh"></color-bar>
      </q-card-section>
      <q-separator /-->

      <heatmap-table :data="Data" :year="Year" @update:export="ExportData=$event" class="q-ma-xs" />
      <!--q-inner-loading :showing="!Loaded" v-if="Queries>0">
        <q-circular-progress show-value font-size="12px" :value="(Queries-Pending)/Queries*100" size="50px" :thickness="0.3" color="orange-10">{{ Math.trunc(((Queries-Pending)/Queries)*100, 0) }} %</q-circular-progress>
      </q-inner-loading-->
    </q-card>
  </q-page>
</template>

<script>
  import moment from 'moment';
  import  'moment-timezone';
  import HeatmapTable from './HeatmapTable.vue';
  export default {
    data: function() {
      return { Data: this.Analyzer.Queries.Suscribe('Series', ['Active_Energy']), Aggregs: this.Aggregations(['15M', '1H']),
               Aggreg: null, Year: null, ShowSpinner: true, Progress: 0, ExportData: null };
    },
    unmounted: function() {
      this.Data.Stop();
    },
    computed: {
      Years: function() {
        let Items = [];
        if(this.Analyzer.Analyzer == null) return;
        if(this.Analyzer.Ranges.Aggreg3S != null) {
          let StartYear = moment(this.Analyzer.Ranges.Aggreg3S.From).year();
          let EndYear = moment(this.Analyzer.Ranges.Aggreg3S.To).year();
          for(let year = StartYear; year <= EndYear; year++) Items.push({ value: year, name: String(year) });
          Items = Items.reverse();
        }
        return Items;
           /* if(this.Years.length == 0) this.Years.push(moment().year());
              var Stored = null;
              if(this.storage != null) {
                Stored = parseInt(localStorage.getItem(this.storage));
                if(isNaN(Stored) == true) Stored = null;
              }
              if(Years.length > 0) {
                if((Stored >= StartYear) && (Stored <= EndYear)) {
                  this.$emit('input', Stored);
                } else {
                  this.$emit('input', parseInt(Years[0]));
                }
              } else {
                this.$emit('input', null);
              }
            }*/

      }
    },
    watch: {
      Aggreg: function(val) {
        if(val == null) return;
        this.Data.SetAggreg(val);
      },
      Year: function(val) {
        if(val == null) return;
        let YearDate = moment.tz(this.Analyzer.TimeZone).year(val);
        this.Data.SetRange([YearDate.startOf('year').valueOf(), YearDate.endOf('year').valueOf()]);
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
          this.Post('getSeries', { From: From, To: To, Aggreg: '15M', Series: ['Active_Energy'], Analyzer: this.Analyzer.Analyzer }).then(
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
            },
            (fail) => {}
          );
        };
        for(let month = 0; month < 12; month++) GetMonth(month);
      },
      Export: function() {
        return this.ExportData;
      }
    },
    components: { HeatmapTable }
  }
</script>