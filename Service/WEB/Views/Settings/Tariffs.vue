<template>
  <div class="panel panel-other" v-show="Enabled">
    <div class="panel-heading clearfix">
      <div class="huge-small pull-left">Tariffs</div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Add tariff" @click="Add"><img :src="require('Imgs/plus.png')"> Add</button>
      </div>
      <toolbar-pager class="pull-right" :pageSize="10" :input="Tariffs" :output.sync="FilterTariffs" :filtercols="['TimeZone', 'Name', 'StartDate', 'EndDate']"></toolbar-pager>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="row">
        <div class="col-sm-12">
          <label class="control-label">Template definitions</label>
          <vuetable :fields="ColumnsTariffs" :api-mode="false" :data="FilterTariffs" class="table small table-striped">
            <template slot="actions" slot-scope="props">
              <div class="btn-group">
                <button class="btn btn-xs btn-default" @click="Edit(props.rowData)">Edit</button>
                <button class="btn btn-xs btn-danger" @click="Delete(props.rowData)">Delete</button>
              </div>
            </template>
          </vuetable>
        </div>
        <div class="col-sm-12">
          <label class="control-label">Price fetch modules</label>
          <vuetable :fields="ColumnsModules" :api-mode="false" :data="Modules" class="table small table-striped modules">
            <template slot="actions" slot-scope="props">
              <div class="btn-group">
                <button class="btn btn-xs btn-default" @click="Configure(props.rowData)">Configure</button>
              </div>
            </template>
          </vuetable>
        </div>
      </div>
    </div>
    <tariff v-if="ShowDialog" :tariff="Tariff" @close="LoadTariffs"></tariff>
    <esp-esios v-if="Module!=null && Module.Name=='ESP_ESIOS'" :settings="Module" @close="Module=null"></esp-esios>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import moment from 'moment';
  import Tariff from './Tariffs/Tariff.vue';
  import EspEsios from './Tariffs/EspEsios.vue';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data: function() {
      return { ShowSpinner: true, Tariffs: [], FilterTariffs: [], ShowDialog: false, Enabled: false, Tariff: null, Module: null,
        Modules: [
          { Name: 'ESP_ESIOS', Description: 'Fetch hourly prices from spanish ESIOS (Sistema de informacion del operador del sistema)' }
        ],
        ColumnsModules: [
          { name: 'Name', title: 'Name' },
          { name: 'Description', title: 'Description' },
          { name: '__slot:actions', title: '' }
        ],
        ColumnsTariffs: [
          { name: 'UserName', title: 'Owner' },
          { name: 'Name', title: 'Name' },
          { name: '__slot:actions', title: '' }
        ]
      };
    },
    created: function() {
      this.LoadTariffs();
    },
    watch: {
      Filter: function() {
        this.LoadTariffs();
      }
    },
    methods: {
      LoadTariffs: function() {
        this.ShowDialog = false;
        this.ShowSpinner = true;
        this.$http.post('getTariffs', { }).then(
          (result) => {
            this.Enabled = true;
            this.Tariffs = result.data;
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
        this.Tariff = null;
        this.ShowDialog = true;
      },
      Edit: function(tariff) {
        this.ShowSpinner = true;
        this.$http.post('getTariff', { Name: tariff.Name }).then(
          (result) => {
            this.Tariff = result.body;
            this.ShowDialog = true;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      Delete: function(tariff) {
        if(confirm("Delete '" + tariff.Name + "' tariff?")) {
          this.$http.post('delTariff', { Name: tariff.Name }).then(
            (result) => {
              this.LoadTariffs();
            },
            (fail) => {}
          );
        }
      },
      Configure: function(module) {
        if(module.Name == 'ESP_ESIOS') {
          this.$http.post('getESIOS', { Type: "CURRENT" }).then(
            (result) => { this.Module = result.body; this.Module.Name = module.Name; },
            (fail) => { }
          );
        }
      }
    },
//    updated: function() {
//      oZm.createCaptionsForResponsiveTable(this.$refs.table);
//    },
    components: { Vuetable, Tariff, EspEsios }
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
      width: 100px;
    }
    .table >>> td:nth-child(3) {
      text-align: right;
    }
    .table.modules >>> th:nth-child(3) {
      width: 80px;
    }
    .table.modules >>> td:nth-child(3) {
      text-align: right;
    }
  }
</style>