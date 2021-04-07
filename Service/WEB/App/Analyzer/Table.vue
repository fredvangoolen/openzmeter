<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="View==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Analyzers</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
        <toolbar-pager class="pull-right" :pageSize="10" :input="Analyzers" :output.sync="FilterAnalyzers" :filtercols="['Name', 'Analyzer']"></toolbar-pager>
      </template>
      <div class="alert alert-danger text-center" v-if="Analyzers.length == 0" style="margin-bottom: 5px; padding: 5px">
        <p>There are no analyzers available.</p>
        <p>If you are an administrator, check device settings, otherwise request permission.</p>
      </div>
      <vuetable key="analyzers" :fields="Columns" :data="FilterAnalyzers" :api-mode="false" class="table small table-striped">
        <template slot="state" slot-scope="props">
          <img :src="require('Imgs/bullet_green.png')" v-if="props.rowData.State=='ONLINE'">
          <img :src="require('Imgs/bullet_yellow.png')" v-if="props.rowData.State=='SYNCING'">
          <img :src="require('Imgs/bullet_red.png')" v-if="props.rowData.State=='OFFLINE'">
        </template>
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-success btn-xs" :disabled="((analyzer != null) && (analyzer.Analyzer == props.rowData.Analyzer))" @click="Select(props.rowData.Analyzer, true)">Select</button>
            <button type="button" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown"><span class="caret"></span></button>
            <ul class="dropdown-menu pull-right">
              <li><a @click="LoadView('Alarms', props.rowData.Analyzer)">Alarms and events</a></li>
              <li><a @click="LoadView('Configure', props.rowData.Analyzer)">Configure</a></li>
              <li><a @click="LoadView('Permissions', props.rowData.Analyzer)">Permissions</a></li>
              <li><a @click="LoadView('Rates', props.rowData.Analyzer)">Rates</a></li>
              <li><a @click="LoadView('Remote', props.rowData.Analyzer)">Remote access</a></li>
              <li role="separator" class="divider"></li>
              <li><a @click="LoadView('Maintenance', props.rowData.Analyzer)">Maintenance</a></li>
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
  import CBOR from 'cbor-js';
  export default {
    props: { analyzer: Object },
    data() {
      return { View: null, Analyzers: [], FilterAnalyzers: [], ShowSpinner: true, Analyzer: null, Filter: '', Timer: null,
        Columns: [
          { name: '__slot:state', title: 'Online' },
          { name: 'User', title: 'Owner' },
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
        if(this.Timer != null) clearTimeout(this.Timer);
        this.Post("getAnalyzers", {}).then(
          (result) => {
            this.Analyzers = [];
            for(var i = 1; i < result.length; i++) this.Analyzers.push({ Analyzer: result[i][0], User: result[i][1], Name: result[i][2], State: result[i][3], Read: result[i][4], Write: result[i][5], Config: result[i][6] });
            this.ShowSpinner = false;
            this.Timer = setTimeout(this.GetAnalyzers, 5000);
          },
          (fail) => { this.Timer = setTimeout(this.GetAnalyzers, 5000); }
        );
      },
      Select: function(analyzer, close) {
        this.$emit('update:analyzer', null);
        this.ShowSpinner = true;
        localStorage.setItem('Analyzer', analyzer);
        this.Post('getAnalyzer', { Analyzer: analyzer }).then(
          (result) => {
            if(this.Timer != null) clearTimeout(this.Timer);
            result.Analyzer = analyzer;
            this.$emit('update:analyzer', result);
            this.ShowSpinner = false;
            if(close == true) this.$emit('close');
          }
        );
      },
      CloseSelect: function(analyzer) {
        this.View = null;
        this.GetAnalyzers();
        if(analyzer == this.analyzer.Analyzer) this.Select(analyzer, false);
      },
      LoadView: function(view, analyzer) {
        this.ShowSpinner = true;
        this.Post('getAnalyzer', { Analyzer: analyzer }).then(
          (result) => {
            if(this.Timer != null) clearTimeout(this.Timer);
            result.Analyzer = analyzer;
            this.Analyzer = result;
            this.View = view;
            this.ShowSpinner = false;
          }
        );
      },
      Back: function() {
        if(this.Timer != null) clearTimeout(this.Timer);
        this.$emit('close');
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
    .table >>> th:nth-child(4) {
      width: 180px;
      text-align: center;
    }
    .table >>> td:nth-child(4) {
      text-align: center;
    }
    .table >>> th:nth-child(5) {
      width: 90px;
    }
  }
</style>