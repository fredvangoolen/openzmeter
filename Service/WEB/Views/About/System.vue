<template>
  <div>
    <!--spinner v-if="System.Loading"></spinner-->
    <div class="row q-col-gutter-sm q-pa-sm" v-if="System.Data.Status!=null">
      <div class="col" v-if="System.Data.Status.CPU!=null">
        <q-card class="full-height q-pa-xs" flat bordered>
          <q-item>
            <q-item-section avatar><img :src="require('Imgs/chip.png')"></q-item-section>
            <q-item-section><q-item-title class="text-h6">CPU</q-item-title></q-item-section>
          </q-item>
          <q-separator />
          <div>
            <div class="card-content"> {{ Round(System.Data.Status.CPU.Usage, 1) }} %</div>
            <div v-if="System.Data.Status.CPU.Count!=null">Total of {{ System.Data.Status.CPU.Count }} cores<span v-if="System.Data.Status.CPU.Frequency!=null"> at {{ System.Data.Status.CPU.Frequency }} MHz</span></div>
          </div>
        </q-card>
      </div>
      <div class="col" v-if="System.Data.Status.RAM!=null">
        <q-card class="full-height q-pa-xs" flat bordered>
          <q-item>
            <q-item-section avatar><img :src="require('Imgs/ddr_memory.png')"></q-item-section>
            <q-item-section><q-item-title class="text-h6">RAM</q-item-title></q-item-section>
          </q-item>
          <q-separator />
          <div>
            <div class="card-content"> {{ Round((System.Data.Status.RAM.Total - (System.Data.Status.RAM.Free + System.Data.Status.RAM.Cached + System.Data.Status.RAM.Buffers)) / System.Data.Status.RAM.Total * 100.0, 1) }} %</div>
            <div>Total memory {{ Round(System.Data.Status.RAM.Total / 1024 / 1024, 2) }} MB</div>
          </div>
        </q-card>
      </div>
      <div class="col" v-if="System.Data.Status.Storage!=null">
        <q-card class="full-height q-pa-xs" flat bordered>
          <q-item>
            <q-item-section avatar><img :src="require('Imgs/storage.png')"></q-item-section>
            <q-item-section><q-item-title class="text-h6">Storage</q-item-title></q-item-section>
          </q-item>
          <q-separator />
          <div>
            <div class="card-content"> {{ Round((System.Data.Status.Storage.Total - System.Data.Status.Storage.Free) / System.Data.Status.Storage.Total * 100, 1) }} %</div>
            <div>Total space {{ Round(System.Data.Status.Storage.Total / 1024 / 1024, 1) }} MB</div>
          </div>
        </q-card>
      </div>
      <div class="col" v-if="System.Data.Status.Temperature!=null">
        <q-card class="full-height q-pa-xs" flat bordered>
          <q-item>
            <q-item-section avatar><img :src="require('Imgs/temperature_normal.png')"></q-item-section>
            <q-item-section><q-item-title class="text-h6">Temperature</q-item-title></q-item-section>
          </q-item>
          <q-separator />
          <div>
            <div class="card-title"></div>
            <div class="card-content"> {{ Round(System.Data.Status.Temperature, 1) }} ºC</div>
          </div>
        </q-card>
      </div>
    </div>
    <q-separator />
    <q-toolbar class="bg-grey-9 text-white q-pr-none">
      <q-tabs stretch v-model="ActiveTab" active-bg-color="primary">
        <q-tab name="system" label="System Log" />
        <q-tab name="service" label="Service Log" />
      </q-tabs>
      <div>{{ (System.Data.Timestamp==null) ? 'Loading data...' : 'Last update at ' + FormatDate(System.Data.Timestamp) }}</div>
      <q-space></q-space>
      <q-btn stretch flat size="sm"><div>Level<br><strong>Warnings</strong></div></q-btn>
      <!--div class="header-buttons">
        <toolbar-options storage="SystemOptions" :options="['Debug', 'Warning', 'Error', 'Info']" v-model="Options"></toolbar-options>
        <toolbar-pager :pageSize="20" :input="Messages" :output.sync="FilterMessages" :filtercols="['Message', 'Date']"></toolbar-pager>
      </div-->
    </q-toolbar>
    <q-separator />
    <q-tab-panels v-model="ActiveTab">
      <q-tab-panel name="system" class="q-pa-sm">
        <textarea rows="10" class="form-control" disabled :value="System.Data.SystemLog"></textarea>
      </q-tab-panel>
      <q-tab-panel name="service" class="q-pa-sm">
        <q-table :hide-bottom="true" flat dense :columns="Columns" row-key="module" :rows="Modules" class="q-ma-xs" />
      </q-tab-panel>
    </q-tab-panels>


  </div>
</template>

<script>
//  import moment from 'moment';
  export default {
    data: function() {
      return { ActiveTab: 'system', AllMessages: [], Messages: [], FilterMessages: [], Options: {}, LastUpdate: null, Status: {},
               System: this.APIUpdate(this.API.GetSystem, ['Status', 'Log', 'SystemLog']),
               Columns: [
                 { name: 'Type', title: 'Type', callback: this.FormatType },
                 { name: 'Date', title: 'Date', callback: this.FormatDate },
                 { name: 'Message', title: 'Message' }
               ]
             };
    },
    unmounted: function() {
      this.System.Stop();
    },
    watch: {
      AllMessages: function() { this.Filter(); },
      Options: function() { this.Filter(); }
    },
    methods: {
      FormatDate: function(date) {
        return moment(date).format('DD/MM/YYYY - HH:mm:ss');
      },
      FormatType: function(val) {
        if(val == 'WARNING') return '<span style="color: #f0ad4e">' + val + '</span>';
        if(val == 'INFO') return '<span style="color: #318ce7">' + val + '</span>';
        if(val == 'ERROR') return '<span style="color: #d9534f">' + val + '</span>';
        if(val == 'DEBUG') return '<span style="color: #dddddd">' + val + '</span>';
      },
      Filter: function() {
        this.Messages = [];
        for(var n = 0; n < this.AllMessages.length; n++) {
          if((this.AllMessages[n].Type == 'DEBUG') && (this.Options.Debug == false)) continue;
          if((this.AllMessages[n].Type == 'WARNING') && (this.Options.Warning == false)) continue;
          if((this.AllMessages[n].Type == 'INFO') && (this.Options.Info == false)) continue;
          if((this.AllMessages[n].Type == 'ERROR') && (this.Options.Error == false)) continue;
          this.Messages.push(this.AllMessages[n]);
        }
      },

    }
  }
</script>

<style scoped>
  textarea {
    resize: none;
    font-family: 'Fira Code';
    font-size: 10px;
    white-space: pre;
  }
</style>