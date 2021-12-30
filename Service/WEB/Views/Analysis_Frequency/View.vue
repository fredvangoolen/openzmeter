<template>
  <q-page>
    <q-card>
      <q-toolbar class="bg-green-6 text-white q-pr-none">
        <q-toolbar-title>Frequency</q-toolbar-title>
        <toolbar-aggreg storage="Frequency.Aggregation" :options="['3S', '1M', '10M', '1H']" v-model="Aggreg" />
        <q-btn stretch dense flat icon="menu">
          <q-menu>
            <q-list dense>
              <q-item clickable v-close-popup>Export CSV</q-item>
            </q-list>
          </q-menu>
        </q-btn>
      </q-toolbar>
      <q-card-section class="q-pa-none">
      <div class="col-lg-12">
            <portal to="fs-toolbar" :disabled="!Expanded" class="header-buttons">

              <toolbar-period storage="Period" v-model="Range"></toolbar-period>
              <div class="btn-group">
                <button-export :rows="Export" file="Frequency"></button-export>
                <button-expand v-model="Expanded"></button-expand>
              </div>
            </portal>

          <div class="panel-body">
            <div class="row">
              <counters class="col-md-6 col-lg-4 col-xl-3" :dataNow="TransferNow.LastData" :dataGraph="TransferGraph.Data" :height.sync="Height"></counters>
              <div class="col-md-6 col-lg-8 col-xl-9">
                <portal to="fs-body" :disabled="!Expanded" class="panel-body">
                  <spinner v-if="TransferGraph.Data==null"></spinner>
                  <!--frequency :data="TransferGraph.Data" :height="Expanded?DialogHeight:Height" @range="Range=$event"></frequency-->
                </portal>
              </div>
            </div>
          </div>

      </div>
      </q-card-section>
    </q-card>
    <dlg-fs v-if="Expanded" title="Frequency" :height.sync="DialogHeight"></dlg-fs>
  </q-page>
</template>

<script>
  import Frequency from './Frequency.vue';
  import Counters from './Counters.vue';
  export default {
    data: function() {
      return { Aggreg: null, Range: null, Height: 300, Expanded: false, DialogHeight: 300,
               TransferGraph: this.SeriesTransfer(['Frequency']),
               TransferNow: this.SeriesTransfer(['Frequency']),
      };
    },
    created: function() {
      this.TransferNow.Update(this.analyzer, '3S', 'LastSamples');
      this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
    },
    beforeDestroy: function() {
      this.TransferNow.Destroy();
      this.TransferGraph.Destroy();
    },
    watch: {
      analyzer: function() {
        this.TransferNow.Update(this.analyzer, '3S', 'LastSamples');
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range)
      },
      Aggreg: function(val, prev) {
        if(prev != null) this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      },
      Range: function() {
        this.TransferGraph.Update(this.analyzer, this.Aggreg, this.Range);
      },
    },
    methods: {
      Export: function() {
        if(this.TransferGraph.Data == null) return [];
        var Rows = [['Time', 'Frequency']];
        var TimeCol = this.TransferGraph.Data.Headers.indexOf('Time');
        var ValueCol = this.TransferGraph.Data.Headers.indexOf('Frequency');
        for(var n = 0; n < this.TransferGraph.Data.Rows.length; n++) Rows.push([this.analyzer.FormatDateTime(this.TransferGraph.Data.Rows[n][TimeCol]), this.Round(this.TransferGraph.Data.Rows[n][ValueCol], 3)]);
        return Object.seal(Rows);
      }
    },
    components: { Frequency, Counters }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>
