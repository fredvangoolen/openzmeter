<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="View==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Rates</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="Add rate"></button>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close', analyzer.Analyzer)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
        </div>
      </template>
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null">{{ Error }}</div>
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error==null">All dates/times are in analyzer timezone.</div>
      <vuetable key="rates" :fields="Columns" :data="Rates" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" @click="Edit(props.rowData)">Edit</button>
            <button type="button" class="btn btn-danger btn-xs" @click="Delete(props.rowData)">Delete</button>
          </div>
        </template>
      </vuetable>
    </dlg>
    <analyzer-rate v-if="View=='Rate'" @close="GetRates($event)" :rate="Rate" :analyzer="analyzer"></analyzer-rate>
  </div>
</template>

<script>
  import AnalyzerRate from './Rate.vue';
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object },
    data() {
      return { Error: null, Rates: null, ShowSpinner: false, View: null, Rate: null,
        Columns: [
          { name: 'Name', title: 'Name' },
          { name: 'StartDate', title: 'Start date', callback: this.DateFormat },
          { name: 'EndDate', title: 'End date', callback: this.DateFormat },
          { name: '__slot:actions', title: '' }
        ]
      };
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    created: function() {
      this.GetRates();
    },
    methods: {
      GetRates: function(update) {
        this.View = null;
        if(update == false) return;
        this.ShowSpinner = true;
        this.$http.post('getRates', { Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.Rates = result.data;
            this.Error = null;
            for(var n = 1; n < result.data.length; n++) {
              if(result.data[n].StartDate > result.data[n - 1].EndDate) continue;
              this.Error = 'Assigned rates overlaps, please check date ranges';
              break;
            }
            this.ShowSpinner = false;
          },
          (fail) => { }
        );
      },
      Add: function() {
        this.Rate = null;
        this.View = 'Rate';
      },
      Edit: function(row) {
        this.ShowSpinner = true;
        this.$http.post('getRate', { Analyzer: this.analyzer.Analyzer, RateID: row.RateID }).then(
          (result) => {
            this.Rate = result.body;
            this.View = 'Rate';
            this.ShowSpinner = false;
          },
          (fail) => { this.ShowSpinner = false; }
        );
      },
      Delete: function(row) {
        if(confirm("Delete '" + row.Name + "' rate?") == false) return;
        this.ShowSpinner = true;
        this.$http.post('delRate', { RateID: row.RateID, Analyzer: this.analyzer.Analyzer }).then(
          (result) => { this.GetRates(); },
          (fail) => { this.ShowSpinner = false; }
        );
      },
      GetTariffs: function(onComplete) {
        this.ShowSpinner = true;
        $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }),
          success: (result) => {
            onComplete(result);
          },
          complete: () => {
            this.ShowSpinner = false;
          }
        });
      },
      DateFormat: function(val) {
        return moment.tz(val, this.analyzer.TimeZone).format('DD/MM/YYYY');
      }
    },
    components: { AnalyzerRate, vuetable }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(2), .table >>> th:nth-child(3)  {
      width: 100px;
      text-align: center;
    }
    .table >>> td:nth-child(2), .table >>> td:nth-child(3) {
      text-align: center;
    }
    .table >>> th:nth-child(4) {
      width: 94px;
    }
    .table >>> td:nth-child(4) {
      text-align: right;
    }
  }
</style>