<template>
  <q-card v-show="Tariffs.Loading == false">
    <q-toolbar class="bg-grey-9 text-white">
      <q-toolbar-title>Tariffs</q-toolbar-title>
      <q-tabs stretch v-model="Tab" active-bg-color="primary">
        <q-tab name="templates" label="Templates" />
        <q-tab name="modules" label="Price modules" />
      </q-tabs>
    </q-toolbar>
    <q-card-section v-if="Tab=='templates'" class="q-pa-none">
      <q-toolbar class="bg-grey-3 q-pa-none">
        <q-space />
        <q-input dense debounce="300" label="Search" v-model="Filter" class="q-mr-sm">
          <template v-slot:prepend><q-icon :name="mdiMagnify" /></template>
        </q-input>
        <q-btn stretch unelevated color="primary" @click="Add">Add</q-btn>
      </q-toolbar>
      <q-separator />
      <q-table flat dense :columns="ColumnsTariffs" :rows="Tariffs.Rows" :filter="Filter" row-key="user" class="q-ma-xs">
        <template v-slot:no-data="data">
          <div class="full-width row flex-center q-gutter-sm">{{ data.message }}</div>
        </template>
        <template v-slot:body-cell-actions="props">
          <q-td :props="props">
            <q-btn-dropdown size="sm" split unelevated color="primary" @click="Edit(props.row)" label="Edit">
              <q-list dense>
                <q-item clickable @click="Duplicate(props.row)">Duplicate</q-item>
                <q-item clickable @click="Delete(props.row)">Delete</q-item>
              </q-list>
            </q-btn-dropdown>
          </q-td>
        </template>
      </q-table>
    </q-card-section>
    <q-card-section v-if="Tab=='modules'" class="q-pa-none">
      <q-table dense :hide-bottom="true" :columns="ColumnsModules" :rows="Modules" row-key="name" class="q-ma-xs">
        <template v-slot:no-data="data">
          <div class="full-width row flex-center q-gutter-sm">{{ data.message }}</div>
        </template>
        <template v-slot:body-cell-actions="props">
          <q-td :props="props">
            <q-btn size="sm" unelevated color="primary" @click="Configure(props.row)" label="Configure" />
          </q-td>
        </template>
      </q-table>
    </q-card-section>
    <tariff v-if="ShowDialog" :tariff="Tariff" :source="SourceTariff" @close="LoadTariffs"></tariff>
    <esp-esios v-if="Module!=null && Module.Name=='ESP_ESIOS'" :settings="Module" @close="Module=null"></esp-esios>
    <q-inner-loading :showing="ShowSpinner"><q-spinner-dots size="50px" color="orange-10" /></q-inner-loading>
  </q-card>
</template>

<script>
  import { mdiMagnify } from '@quasar/extras/mdi-v6';
  import Tariff from './Tariffs/Tariff.vue';
  import EspEsios from './Tariffs/EspEsios.vue';
  export default {
    data: function() {
      return { ShowSpinner: false, Filter: '', ShowDialog: false, Tariff: null, SourceTariff: null, Module: null, Tab: 'templates', Tariffs: this.APIList(this.API.GetTariffs),
        Modules: [
          { Name: 'ESP_ESIOS', Description: 'Fetch hourly prices from spanish ESIOS (Sistema de informacion del operador del sistema)' }
        ],
        ColumnsModules: [
          { name: 'Name', label: 'Name', field: row => row.Name, align: 'left' },
          { name: 'Description', label: 'Description', field: row => row.Description, align: 'left' },
          { name: 'actions', label: '' }
        ],
        ColumnsTariffs: [
          { name: 'User', label: 'Owner', field: row => row.User, align: 'left', style: 'width: 100px' },
          { name: 'Name', label: 'Name', field: row => row.Name, align: 'left' },
          { name: 'actions', label: '' }
        ]
      };
    },
    created: function() {
      this.mdiMagnify = mdiMagnify;
    },
    watch: {
      'Tariffs.Loading': function(val) {
        this.$emit('done');
      }
    },
    mounted: function() {
      this.$emit('queries', 1);
    },
    unmounted: function() {
      this.Tariffs.Stop();
    },
    methods: {
      LoadTariffs: function() {
        this.ShowDialog = false;
        this.ShowSpinner = true;
        this.Post('getTariffs', { }).then(
          (result) => {
            var NameCol = result[0].indexOf("Name");
            var UserCol = result[0].indexOf("User");
            this.Enabled = true;
            this.Tariffs = [];
            this.ShowSpinner = false;
            for(var n = 1; n < result.length; n++) {
              this.Tariffs.push({ Name: result[n][NameCol], User: result[n][UserCol]});
            }
          },
          (fail) => { }
        );
      },
      Add: function() {
        this.SourceTariff = null;
        this.Tariff = null;
        this.ShowDialog = true;
      },
      Duplicate: function(tariff) {
        this.Tariff = null;
        this.ShowSpinner = true;
        this.Post('getTariff', { Name: tariff.Name }).then(
          (result) => {
            this.SourceTariff = result;
            this.ShowDialog = true;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      Edit: function(tariff) {
        this.SourceTariff = null;
        this.ShowSpinner = true;
        this.Post('getTariff', { Name: tariff.Name }).then(
          (result) => {
            this.Tariff = result;
            this.ShowDialog = true;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      Delete: function(tariff) {
        if(confirm("Delete '" + tariff.Name + "' tariff?")) {
          this.Post('delTariff', { Name: tariff.Name }).then(
            (result) => {
              this.LoadTariffs();
            },
            (fail) => {}
          );
        }
      },
      Configure: function(module) {
        if(module.Name == 'ESP_ESIOS') {
          this.Post('getESIOS', { }).then(
            (result) => { this.Module = result; this.Module.Name = module.Name; },
            (fail) => { }
          );
        }
      }
    },
    components: { Tariff, EspEsios }
  }
</script>