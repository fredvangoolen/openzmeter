<template>
  <div class="panel panel-other" v-show="Enabled">
    <div class="panel-heading clearfix">
      <div class="huge-small pull-left">Tariffs</div>
      <div class="pull-right input-group input-group-xs" style="width:200px">
        <input type="text" class="form-control" placeholder="Filter..." disabled>
        <span class="input-group-btn">
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Add tariff" @click="Add"><img :src="require('Imgs/plus.png')"> Add</button>
        </span>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" :data="Tariffs" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" >
            <button class="btn btn-xs btn-default" @click="Edit(props.rowData)">Edit</button>
            <button class="btn btn-xs btn-danger" @click="Delete(props.rowData)">Delete</button>
          </div>
        </template>
      </vuetable>
    </div>
    <tariff v-if="ShowDialog" :tariff="Tariff" @close="LoadTariffs"></tariff>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import moment from 'moment';
  import Tariff from './Tariffs/Tariff.vue';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data: function() {
      return { ShowSpinner: true, Tariffs: [], ShowDialog: false, Filter: '', Enabled: false, Tariff: null,
        Columns: [
          { name: 'TimeZone', title: 'Time zone' },
          { name: 'Name', title: 'Name' },
          { name: 'StartDate', title: 'Start date' },
          { name: 'EndDate', title: 'End date' },
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
        this.$http.post('getTariffs', { Limit: 10, Offset: 0, Filter: this.Filter }).then(
          (result) => {
            this.Enabled = true;
            this.Tariffs = result.data;
            this.ShowSpinner = false;
            for(var n in this.Tariffs) {
              this.Tariffs[n].StartDate = moment.tz(this.Tariffs[n].StartDate, this.Tariffs[n].TimeZone).format('DD/MM/YYYY');
              this.Tariffs[n].EndDate   = moment.tz(this.Tariffs[n].EndDate,   this.Tariffs[n].TimeZone).format('DD/MM/YYYY');
            }
          },
          (fail) => {}
        );
      },
      Add: function() {
        this.Tariff = null;
        this.ShowDialog = true;
      },
      Edit: function(tariff) {
        this.ShowSpinner = true;
        this.$http.post('getTariff', { TariffID: tariff.TariffID }).then(
          (result) => {
            this.Tariff = result.data;
            this.ShowDialog = true;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      Delete: function(tariff) {
        if(confirm("Delete '" + tariff.Name + "' tariff?")){
          this.$http.post('delTariff', { TariffID: tariff.TariffID }).then(
            (result) => {
              this.LoadTariffs();
            },
            (fail) => {}
          );
        }
      }
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    components: { Vuetable, Tariff }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1) {
      width: 60px;
      text-align: center;
    }
    .table >>> td:nth-child(1) {
      text-align: center;
    }
    .table >>> th:nth-child(3), .table >>> th:nth-child(4)  {
      width: 150px;
      text-align: center;
    }
    .table >>> td:nth-child(3), .table >>> td:nth-child(4) {
      text-align: center;
    }
    .table >>> th:nth-child(5) {
      width: 100px;
    }
    .table >>> td:nth-child(5) {
      text-align: right;
    }
  }
</style>