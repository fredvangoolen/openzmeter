<template>
  <q-card>
    <q-toolbar class="bg-grey-9 text-white">
      <q-toolbar-title>Tariffs</q-toolbar-title>
      <q-input dense debounce="300" label="Search" color="grey-3" label-color="white" v-model="Filter" input-class="text-white"  class="q-mr-sm">
         <template v-slot:prepend>
            <q-icon name="search" color="white"></q-icon>
          </template>
      </q-input>
      <q-btn  color="positive" @click="Add">Add</q-btn>
    </q-toolbar>
    <q-table :loading="ShowSpinner" dense :columns="ColumnsTariffs" :data="Tariffs" :filter="Filter" row-key="user" class="q-ma-xs" title="Template definitions">
      <template v-slot:body-cell-actions="props">
        <q-td :props="props">
          <q-btn-group>
            <q-btn size="sm" @click="Edit(props.row)">Edit</q-btn>
            <q-separator vertical></q-separator>
            <q-btn size="sm" @click="Duplicate(props.row)">Duplicate</q-btn>
            <q-separator vertical></q-separator>
            <q-btn size="sm" @click="Delete(props.row)">Delete</q-btn>
          </q-btn-group>
        </q-td>
      </template>
    </q-table>
    <q-table :loading="ShowSpinner" dense :hide-bottom="true" :columns="ColumnsModules" :data="Modules" row-key="name" class="q-ma-xs" title="Price fetch modules">
      <template v-slot:body-cell-actions="props">
        <q-td :props="props">
          <q-btn size="sm" @click="Configure(props.row)">Configure</q-btn>
        </q-td>
      </template>
    </q-table>
    <tariff v-if="ShowDialog" :tariff="Tariff" :source="SourceTariff" @close="LoadTariffs"></tariff>
    <esp-esios v-if="Module!=null && Module.Name=='ESP_ESIOS'" :settings="Module" @close="Module=null"></esp-esios>
  </q-card>
</template>

<script>
  import moment from 'moment';
  import Tariff from './Tariffs/Tariff.vue';
  import EspEsios from './Tariffs/EspEsios.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data: function() {
      return { ShowSpinner: true, Tariffs: [], Filter: '', ShowDialog: false, Enabled: false, Tariff: null, SourceTariff: null, Module: null,
        Modules: [
          { Name: 'ESP_ESIOS', Description: 'Fetch hourly prices from spanish ESIOS (Sistema de informacion del operador del sistema)' }
        ],
        ColumnsModules: [
          { name: 'Name', label: 'Name', field: row => row.Name, align: 'left' },
          { name: 'Description', label: 'Description', field: row => row.Description, align: 'left' },
          { name: 'actions', label: '' }
        ],
        ColumnsTariffs: [
          { name: 'User', label: 'Owner', field: row => row.User, align: 'left' },
          { name: 'Name', label: 'Name', field: row => row.Name, align: 'left' },
          { name: 'actions', label: '' }
        ]
      };
    },
    created: function() {
      this.LoadTariffs();
    },
    methods: {
      LoadTariffs: function() {
        this.ShowDialog = false;
        this.ShowSpinner = true;
        this.Post('getTariffs', { }).then(
          (result) => {
            this.Enabled = true;
            this.Tariffs = result;
            this.ShowSpinner = false;
            for(var n in this.Tariffs) {
              this.Tariffs[n].StartDate = moment.tz(this.Tariffs[n].StartDate, this.Tariffs[n].TimeZone).format('DD/MM/YYYY');
              this.Tariffs[n].EndDate   = moment.tz(this.Tariffs[n].EndDate,   this.Tariffs[n].TimeZone).format('DD/MM/YYYY');
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

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1) {
      width: 100px;
      text-align: center;
    }
    .table >>> td:nth-child(1) {
      text-align: center;
    }
    .table >>> th:nth-child(3) {
      width: 120px;
    }
    .table >>> td:nth-child(3) {
      text-align: right;
    }
  }
</style>