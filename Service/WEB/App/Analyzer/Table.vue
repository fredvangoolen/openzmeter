<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="View==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Analyzers</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
        <toolbar-pager class="pull-right" :pageSize="10" :input="Analyzers" :output.sync="FilterAnalyzers" :filtercols="['Name', 'Analyzer']"></toolbar-pager>
      </template>
      <div class="alert alert-danger text-center" v-if="Analyzers.length == 0" style="margin-bottom: 5px; padding: 5px">
        <p>There are no analyzers available.</p>
        <p>If you are an administrator, check device settings, otherwise request permission.</p>
      </div>
      <vuetable key="analyzers" :fields="Columns" :data="FilterAnalyzers" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-success btn-xs" :disabled="((analyzer != null) && (analyzer.Analyzer == props.rowData.Analyzer))" @click="Select(props.rowData.Analyzer, true)">Select</button>
            <button type="button" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown"><span class="caret"></span></button>
            <ul class="dropdown-menu pull-right">
              <li><a @click="Alarms(props.rowData.Analyzer)">Alarms and events</a></li>
              <li><a @click="Configure(props.rowData.Analyzer)">Configure</a></li>
              <li><a @click="Permissions(props.rowData.Analyzer)">Permissions</a></li>
              <li><a @click="Rates(props.rowData.Analyzer)">Rates</a></li>
              <li><a @click="Remote(props.rowData.Analyzer)">Remote access</a></li>
              <li role="separator" class="divider"></li>
              <li><a @click="Maintenance(props.rowData.Analyzer)">Maintenance</a></li>
            </ul>
          </div>
        </template>
      </vuetable>
    </dlg>
    <analyzer-configure v-if="View=='Configure'" :analyzer="Analyzer" @close="CloseSelect($event)"></analyzer-configure>
    <analyzer-alarms v-if="View=='Alarms'" :analyzer="Analyzer" @close="CloseSelect($event)"></analyzer-alarms>
    <analyzer-permissions v-if="View=='Permissions'" :analyzer="Analyzer" @close="CloseSelect($event)"></analyzer-permissions>
    <analyzer-remote v-if="View=='Remote'" :analyzer="Analyzer" @close="CloseSelect($event)"></analyzer-remote>
    <analyzer-rates v-if="View=='Rates'" :analyzer="Analyzer" @close="CloseSelect($event)"></analyzer-rates>
    <analyzer-maintenance v-if="View=='Maintenance'" :analyzer="Analyzer" @close="CloseSelect($event)"></analyzer-maintenance>
  </div>
</template>

<script>
  import AnalyzerConfigure from './Configure.vue';
  import AnalyzerPermissions from './Permissions.vue';
  import AnalyzerRates from './Rates.vue';
  import AnalyzerMaintenance from './Maintenance.vue';
  import AnalyzerAlarms from './Alarms.vue';
  import AnalyzerRemote from './Remote.vue';
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object },
    data() {
      return { View: null, Analyzers: [], FilterAnalyzers: [], ShowSpinner: false, Analyzer: null, Filter: '',
        Columns: [
          { name: 'Online', title: 'Online', callback: this.OnlineStatus },
          { name: 'Name', title: 'Description' },
          { name: 'Analyzer', title: 'Serial' },
          { name: '__slot:actions', title: '' }
        ]
      };
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    mounted: function() {
      this.GetAnalyzers();
    },
    methods: {
      GetAnalyzers: function() {
        this.ShowSpinner = true;
        this.$http.post("getAnalyzers", {}).then(
          (result) => {
            this.Analyzers = result.data;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      OnlineStatus: function(value, close) {
        return (value === true) ? '&#9635;' : '&#9633;';
      },
      Select: function(analyzer, close) {
        this.$emit('update:analyzer', null);
        this.ShowSpinner = true;
        localStorage.setItem('Analyzer', analyzer);
        this.$http.post('getAnalyzer', { Analyzer: analyzer }).then(
          (result) => {
            result.data.Analyzer = analyzer;
            this.$emit('update:analyzer', result.data);
            this.ShowSpinner = false;
            if(close == true) this.$emit('close');
          },
          (fail) => {}
        );
      },
      CloseSelect: function(analyzer) {
        this.View = null;
        if(analyzer == null) return;
        this.GetAnalyzers();
        if(analyzer == this.analyzer.Analyzer) this.Select(analyzer, false);
      },
      Configure: function(analyzer) {
        this.ShowSpinner = true;
        this.$http.post('getAnalyzer', { Analyzer: analyzer }).then(
          (result) => {
            result.data.Analyzer = analyzer;
            this.Analyzer = result.data;
            this.View = 'Configure';
            this.ShowSpinner = false;
          }
        );
      },
      Rates: function(analyzer) {
        this.ShowSpinner = true;
        this.$http.post('getAnalyzer', { Analyzer: analyzer }).then(
          (result) => {
            result.data.Analyzer = analyzer;
            this.Analyzer = result.data;
            this.View = 'Rates';
            this.ShowSpinner = false;
          }
        );
      },
      Alarms: function(analyzer) {
        this.ShowSpinner = true;
        this.$http.post('getAnalyzer', { Analyzer: analyzer }).then(
          (result) => {
            result.data.Analyzer = analyzer;
            this.Analyzer = result.data;
            this.View = 'Alarms';
            this.ShowSpinner = false;
          }
        );
      },
      Remote: function(analyzer) {
        this.ShowSpinner = true;
        this.$http.post('getAnalyzer', { Analyzer: analyzer }).then(
          (result) => {
            result.data.Analyzer = analyzer;
            this.Analyzer = result.data;
            this.View = 'Remote';
            this.ShowSpinner = false;
          }
        );
      },
      Permissions: function(analyzer) {
        this.ShowSpinner = true;
        this.$http.post('getAnalyzer', { Analyzer: analyzer }).then(
          (result) => {
            result.data.Analyzer = analyzer;
            this.Analyzer = result.data;
            this.View = 'Permissions';
            this.ShowSpinner = false;
          }
        );
      },
      Maintenance: function(analyzer) {
        this.ShowSpinner = true;
        this.$http.post('getAnalyzer', { Analyzer: analyzer }).then(
          (result) => {
            result.data.Analyzer = analyzer;
            this.Analyzer = result.data;
            this.View = 'Maintenance';
            this.ShowSpinner = false;
          }
        );
      }
    },
    components: { Vuetable, AnalyzerRemote, AnalyzerConfigure, AnalyzerRates, AnalyzerMaintenance, AnalyzerAlarms, AnalyzerPermissions }
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
      font-size: 14px;
    }
    .table >>> th:nth-child(3) {
      width: 180px;
      text-align: center;
    }
    .table >>> td:nth-child(3) {
      text-align: center;
    }
    .table >>> th:nth-child(4) {
      width: 90px;
    }
  }
</style>