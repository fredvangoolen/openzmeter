<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="View==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Rates</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save rates config"></button>
          <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="Add rate"></button>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close', Analyzer.Analyzer)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
        </div>
      </template>
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null">{{ Error }}</div>
      <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error==null">All dates/times are in analyzer timezone.</div>
      <vuetable key="rates" :fields="Columns" :data="Analyzer.Rates" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" @click="Edit(props.rowIndex)">Edit</button>
            <button type="button" class="btn btn-danger btn-xs" @click="Delete(props.rowIndex)">Delete</button>
          </div>
        </template>
      </vuetable>
    </dlg>
    <analyzer-rate v-if="View=='Rate'" @endEdit="EndEdit" :analyzer="Analyzer" :index="Index"></analyzer-rate>
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
      return { Error: null, ShowSpinner: false, View: null, Index: null, Analyzer: null,
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
      this.Analyzer = Object.assign({}, this.analyzer);
    },
    methods: {
      EndEdit: function(index, value) {
        this.Analyzer.Rates[index] = value;
        this.View = null;
      },
      Add: function() {
        this.Analyzer.Rates.push({ Name: 'Unname rate', StartDate: 0, EndDate: 0, Template: null, Config: { Periods:[], Taxs: [], Billing: [] } });
        this.Index = this.Analyzer.Rates.length - 1;
        this.View = 'Rate';
      },
      Edit: function(row) {
        this.Index = row;
        this.View = 'Rate';
      },
      Delete: function(row) {
        if(confirm("Delete '" + this.Analyzer.Rate[row].Name + "' rate?")) this.Analyzer.Rates.splice(row, 1);
      },
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post('setAnalyzer', this.Analyzer).then(
          (result) => { this.$emit('close', result.data); },
          (fail) => {
            this.ShowSpinner = false;
            this.Error = 'Failed to save analyzer!';
            setTimeout(() => { this.Error = null; }, 5000);
          }
        );
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