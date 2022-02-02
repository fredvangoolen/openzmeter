<template>
  <q-page>
    <q-card>
      <q-toolbar class="bg-indigo-9 text-white q-pr-none">
        <q-toolbar-title>Energy Heatmap</q-toolbar-title>
        <q-separator dark vertical v-if="Data.Progress!=null" />
        <toolbar-aggreg storage="Stats_Heatmap.Aggreg" :options="['15M', '1H']" v-model="Aggreg" />
        <q-separator dark vertical />
        <toolbar-year storage="Stats_Heatmap.Year" aggreg="15M" v-model="Year" />
        <q-separator dark vertical />
        <toolbar-export file="Heatmap" :rows="Export" />
      </q-toolbar>
      <q-linear-progress v-if="Data.Progress != null" color="indigo-9" :value="Data.Progress" class="q-ma-none" />
      <q-card-section class="bg-grey-3 q-pa-xs text-right">
        <color-bar :width="Math.max(140, Width * 0.2)" :height="10" :range="[-Max, Max]" units="Wh" />
      </q-card-section>
      <q-separator />
      <div class="q-ma-sm">
        <heatmap-table :width="Width" :data="Data" v-model:max="Max" :year="Year" />
        <q-resize-observer @resize="onResize" />
      </div>
    </q-card>
  </q-page>
</template>

<script>
  import HeatmapTable from './HeatmapTable.vue';
  export default {
    data: function() {
      return { Data: this.Analyzer.Queries.Suscribe('Series', ['Active_Energy'], 60000),
               Aggreg: null, Year: null, ExportData: null, Width: 300, Max: 0 };
    },
    unmounted: function() {
      this.Data.Stop();
    },
    watch: {
      Aggreg: function(val) {
        this.Data.SetAggreg(val);
      },
      Year: function(val) {
        let YearDate = this.$dayjs().tz(this.Analyzer.TimeZone).year(val);
        this.Data.SetRange([YearDate.startOf('year').valueOf(), YearDate.endOf('year').valueOf()]);
      }
    },
    methods: {
      onResize: function(evt) {
        this.Width = evt.width;
      },
      Export: function() {
        return this.ExportData;
      }
    },
    components: { HeatmapTable }
  }
</script>