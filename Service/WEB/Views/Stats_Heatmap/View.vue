<template>
  <q-page>
    <q-card>
      <q-toolbar class="bg-indigo-9 text-white q-pr-none">
        <q-toolbar-title>Energy Heatmap</q-toolbar-title>
        <toolbar-aggreg storage="Stats_Heatmap.Aggreg" :options="['15M', '1H']" v-model="Aggreg" />
        <q-separator dark vertical />
        <toolbar-year storage="Stats_Heatmap.Year" aggreg="15M" v-model="Year" />
        <q-separator dark vertical />
        <q-btn stretch dense flat icon="mdiMenu">
          <q-menu>
            <q-list dense>
              <q-item clickable v-close-popup>Reload</q-item>
              <q-item clickable v-close-popup>Export CSV</q-item>
              <q-item clickable v-close-popup @click="ExportSVG">Export SVG</q-item>
            </q-list>
          </q-menu>
        </q-btn>
      </q-toolbar>
      <q-linear-progress v-if="Data.Progress != null" color="indigo-9" :value="Data.Progress" class="q-ma-none" />
      <q-card-section class="bg-grey-3 q-pa-xs text-right">
        <color-bar :width="Math.max(140, Width * 0.2)" :height="10" :range="[-Max, Max]" units="Wh"></color-bar>
      </q-card-section>
      <q-separator />
      <div class="q-ma-sm">
        <heatmap-table ref="map" :width="Width" :data="Data" v-model:max="Max" :year="Year" />
        <q-resize-observer @resize="onResize" />
      </div>
    </q-card>
  </q-page>
</template>

<script>
  import moment from 'moment';
  import  'moment-timezone';
  import HeatmapTable from './HeatmapTable.vue';
  export default {
    data: function() {
      return { Data: this.Analyzer.Queries.Suscribe('Series', ['Active_Energy'], 60000),
               Aggreg: null, Year: null, ShowSpinner: true, Progress: 0, ExportData: null, Width: 300, Max: 0 };
    },
    unmounted: function() {
      this.Data.Stop();
    },
    watch: {
      Aggreg: function(val, prev) {
        console.log("view", val, prev);
        this.Data.SetAggreg(val);
      },
      Year: function(val, prev) {
        let YearDate = moment.tz(this.Analyzer.TimeZone).year(val);
        this.Data.SetRange([YearDate.startOf('year').valueOf(), YearDate.endOf('year').valueOf()]);
      }
    },
    methods: {
      onResize: function(evt) {
        this.Width = evt.width;
      },
      ExportSVG: function() {
        this.SaveSvg(this.$refs.map.$refs.d3.childNodes[0], "Heatmap.svg");
      },
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