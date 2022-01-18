<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="Rate==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Rates</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save configured rates"></button>
          <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="Add rate"></button>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close', analyzer.Analyzer)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
        </div>
      </template>
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px">All dates/times are in analyzer timezone.</div>
      <vuetable key="rates" :fields="Columns" :data="Analyzer.Rates" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" @click="Edit(props.rowIndex)">Edit</button>
            <button type="button" class="btn btn-danger btn-xs" @click="Delete(props.rowIndex)">Delete</button>
          </div>
        </template>
      </vuetable>
    </dlg>
    <analyzer-rate v-if="Rate!=null" :analyzer="analyzer" :rate="Rate" @update="Update($event)"></analyzer-rate>
  </div>
</template>

<script>
  import AnalyzerRate from './Rate.vue';
//  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object },
    data() {
      return { ShowSpinner: false, Rates: [], Rate: null,
        Columns: [
          { name: 'Group', title: 'Type', callback: this.FormatGroup },
          { name: 'Name', title: 'Name' },
          { name: 'StartDate', title: 'Start date' },
          { name: 'EndDate', title: 'End date' },
          { name: '__slot:actions', title: '' }
        ]
      };
    },
    created: function() {
      this.Analyzer = Object.assign({}, this.analyzer);
    },
    methods: {
      UUID: function() {
        return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) { var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8); return v.toString(16); });
      },
      FormatGroup: function(val) {
        return (val == 0) ? 'Contracted' : 'Group ' + val;
      },
      Update: function(rate) {
        this.Analyzer.Rates[this.Rate] = rate;
        this.Rate = null;
      },
      Add: function() {
        this.Analyzer.Rates.push({ Name: 'Unname rate', RateID: this.UUID(), Description: '', Group: 0, StartDate: moment().format('DD/MM/YYYY'), EndDate: moment().format('DD/MM/YYYY'), Module: 'NONE', ContractedPower: 0, PowerCost: '0', EnergyCost: '0', Taxes: [], Billing: [] });
        this.Rate = this.Analyzer.Rates.length - 1;
      },
      Edit: function(row) {
        this.Rate = row;
      },
      Delete: function(row) {
        if(confirm("Delete '" + this.Analyzer.Rates[row].Name + "' rate?") == false) return;
        this.Analyzer.Rates.splice(row, 1);
      },
      Save: function() {
        this.ShowSpinner = true;
        this.Post('setAnalyzer', this.Analyzer).then(
          () => { this.$emit('close', this.Analyzer); },
          (fail) => {
            this.ShowSpinner = false;
            if(fail != null) this.$toast.open({type:'error', message: "Failed to save analyzer config<br>" + fail.message, duration: 3000});
          }
        );
      }
    },
    components: { AnalyzerRate }
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