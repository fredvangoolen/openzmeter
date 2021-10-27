<template>
  <q-page>
    <q-card>
      <q-toolbar class="bg-indigo-9 text-white q-pr-none">
        <q-toolbar-title>Energy Heatmap</q-toolbar-title>
        <toolbar-select name="Aggregation" storage="Stats_Heatmap.Aggreg" :options="Aggregations" :values="['15M', '1H']" v-model="Aggreg" />
        <q-separator dark vertical />
        <q-btn stretch flat size="sm"><div>Year<br><strong>2021</strong></div></q-btn>
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
      <q-card-section class="bg-grey-3 q-pa-xs">
        <color-bar :width="140" :height="10" :range="Range" units="Wh"></color-bar>
      </q-card-section>
      <q-separator />
      <heatmap-table :data="Data" :aggreg="Aggreg" :range.sync="Range" @update:export="ExportData=$event" class="q-ma-xs" />
      <q-inner-loading :showing="!Loaded" v-if="Queries>0">
        <q-circular-progress show-value font-size="12px" :value="(Queries-Pending)/Queries*100" size="50px" :thickness="0.3" color="orange-10">{{ Math.trunc(((Queries-Pending)/Queries)*100, 0) }} %</q-circular-progress>
      </q-inner-loading>
    </q-card>
  </q-page>
</template>

<script>
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  import HeatmapTable from './HeatmapTable.vue';
  export default {
    data: function() {
      return { Aggreg: '1H', Range: [0, 1], Data: null, Year: 0, ShowSpinner: true, Progress: 0, ExportData: null };
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