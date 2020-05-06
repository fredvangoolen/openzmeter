<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">
        <span>From {{ analyzer.FormatDate(bill.From) }} to {{ analyzer.FormatDate(bill.To) }} ({{ bill.Name }})</span>
        <div v-if="bill.Complete==false" class="title-warning"><img :src="require('Imgs/exclamation.png')">Bill is not yet complete, partial data is shown</div>
      </div>
      <div class="float-right"><strong style="font-size: 16px" v-html="Round(bill.Total, 2, true) + ' ' + bill.Currency"></strong></div>
    </div>
    <div class="panel-body">
      <div class="row">
        <div class="col-md-12 col-lg-6">
          <div v-for="(p, idx) in bill.Periods" :key="idx" class="main-div">
            <table class="main-table">
              <tr><td class="border-bottom">Period P{{ idx + 1 }}
                <div class="float-right"><strong v-html="Round(p.reduce((sum, i) => { return sum + ((i.Type != 'INFO') ? i.Value : 0.0); }, 0.0), 2, true) + ' ' + bill.Currency"></strong></div>
                </td></tr>
              <tr v-if="p.reduce((sum, i) => { return sum + i.Value; }, 0.0)!=0"><td>
                <vuetable :fields="ColumnsInfo" :api-mode="false" v-show="p.filter((v)=>v.Type=='INFO').length>0" :data="p.filter((v) => v.Type == 'INFO')" class="table-info table small table-striped"></vuetable>
                <vuetable :fields="ColumnsCost" :api-mode="false" v-show="p.filter((v)=>v.Type!='INFO').length>0" :data="p.filter((v) => v.Type != 'INFO')" class="table-cost table small table-striped"></vuetable>
              </td></tr>
            </table>
          </div>
        </div>
        <div class="col-md-12 col-lg-6">
          <table class="main-table" v-show="bill.General.length>0">
            <tr><td class="border-bottom">General
              <div class="float-right"><strong v-html="Round(bill.General.reduce((sum, i) => { return sum + ((i.Type != 'INFO') ? i.Value : 0.0); }, 0.0), 2, true) + ' ' + bill.Currency"></strong></div>
            </td></tr>
            <tr><td>
              <vuetable :fields="ColumnsInfo" :api-mode="false" v-show="bill.General.filter((v)=>v.Type=='INFO').length>0" :data="bill.General.filter((v) => v.Type == 'INFO')" class="table-info table small table-striped"></vuetable>
              <vuetable :fields="ColumnsCost" :api-mode="false" v-show="bill.General.filter((v)=>v.Type!='INFO').length>0" :data="bill.General.filter((v) => v.Type != 'INFO')" class="table-cost table small table-striped"></vuetable>
            </td></tr>
          </table>
          <table class="main-table" v-show="bill.Taxes.length>0">
            <tr><td class="border-bottom">Tax and rents</td></tr>
            <tr><td>
              <vuetable :fields="ColumnsTax" :api-mode="false" :data="bill.Taxes" class="table-info table small table-striped"></vuetable>
            </td></tr>
          </table>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import vuetable from 'vuetable-2';
  export default {
    props: { bill: Object, analyzer: Object },
    data: function() {
      return { Group: false,
        ColumnsCost: [
          { name: 'Description', title: 'Concept' },
          { name: 'Type', title: 'Term', callback: this.TermFormat },
          { name: 'Value', title: 'Cost', callback: this.CurrencyFormat }
        ],
        ColumnsInfo: [
          { name: 'Description', title: 'Concept' },
          { name: 'Value', title: 'Value', callback: this.ValueFormat }
        ],
        ColumnsTax: [
          { name: 'Description', title: 'Concept' },
          { name: 'Value', title: 'Value', callback: this.CurrencyFormat }
        ],
      };
    },
    methods: {
      ValueFormat: function(val) {
        return this.Round(val, 2);
      },
      CurrencyFormat: function(val) {
        return this.Round(val, 2) + this.bill.Currency;
      },
      TermFormat: function(val) {
        return (val == 'COST_ENERGY') ? 'Energy' : (val == 'COST_POWER') ? 'Power' : 'General';
      }
    },
    components: { vuetable }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
  .panel-other {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
  }
  .main-table {
    border: 1px solid #ccc;
  }
  .main-div {
    margin-bottom: 5px;
  }
  .main-table:not(:first-child) {
    margin: 5px 0px;
  }
  .main-table .border-bottom {
    border-bottom: 1px solid #ccc;
    background: #eee;
    font-weight: bold;
    padding: 3px;
  }
  table {
    width: 100%;
  }
  .table-cost >>> th:nth-child(2) {
    width: 80px;
    text-align: center;
  }
  .table-cost >>> td:nth-child(2) {
    text-align: center;
  }
  .table-cost >>> th:nth-child(3), .table-info >>> th:nth-child(2) {
    width: 80px;
    text-align: right;
  }
  .table-cost >>> td:nth-child(3), .table-info >>> td:nth-child(2) {
    text-align: right;
  }
</style>