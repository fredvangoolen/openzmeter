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
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null" v-html="Error"></div>
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
    <analyzer-rate v-if="View=='Rate'" :analyzer="analyzer" :rate="Rate" @close="GetRates"></analyzer-rate>
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
      return { Error: null, ShowSpinner: false, View: null, Rates: [], Rate: null,
        Columns: [
          { name: 'Group', title: 'Type', callback: this.FormatGroup },
          { name: 'Name', title: 'Name' },
          { name: 'StartDate', title: 'Start date' },
          { name: 'EndDate', title: 'End date' },
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
      FormatGroup: function(val) {
        return (val == 0) ? 'Contracted' : 'Group ' + val;
      },
      GetRates: function() {
        this.View = null;
        this.ShowSpinner = true;
        if(this.analyzer == null) return;
        this.$http.post('getRates', { Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.Rates = result.data;
            this.ShowSpinner = false;
          },
          (fail) => {}
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
            this.Rate = result.data;
            this.Rate.RateID = row.RateID;
            this.View = 'Rate';
          },
          (fail) => {
            this.ShowSpinner = false;
          }
        );
      },
      Delete: function(row) {
        if(confirm("Delete '" + row.Name + "' rate?")) {
          this.ShowSpinner = true;
          this.$http.post('delRate', { Analyzer: this.analyzer.Analyzer, RateID: row.RateID }).then(
            (result) => { this.GetRates(); },
            (fail) => { this.ShowSpinner = false; }
          );
        }
      }
    },
    components: { AnalyzerRate, vuetable }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1) {
      width: 62px;
      text-align: center;
    }
    .table >>> td:nth-child(1) {
      text-align: center;
    }
    .table >>> th:nth-child(3), .table >>> th:nth-child(4) {
      width: 80px;
      text-align: center;
    }
    .table >>> td:nth-child(3), .table >>> td:nth-child(4) {
      text-align: center;
    }
    .table >>> th:nth-child(5) {
      width: 94px;
    }
    .table >>> td:nth-child(5) {
      text-align: right;
    }
  }
</style>