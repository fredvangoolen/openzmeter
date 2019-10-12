<template>
  <dlg ref="Dialog">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers</h4></div>
      <div class="pull-right">
        <button type="button" class="btn btn-default btn-xs" v-on:click="Close"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" v-if="Analyzers.length == 0" style="margin-bottom: 5px; padding: 5px">
      <p>There are no analyzers available.</p>
      <p>If you are an administrator, check device settings, otherwise request permission.</p>
    </div>
    <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped">
      <template slot="actions" slot-scope="props">
        <div class="btn-group">
          <button type="button" class="btn btn-success btn-xs" :disabled="CurrentlySelected(props.rowData)" v-on:click="Select(props.rowData)">Select</button>
          <button type="button" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown"><span class="caret"></span></button>
          <ul class="dropdown-menu pull-right">
            <li><a v-on:click="Configure(props.rowData)" v-if="true">Configure</a></li>
            <li><a v-on:click="ACL(props.rowData)" v-if="false">ACL</a></li>
            <li><a v-on:click="Rates(props.rowData)">Rates</a></li>
            <li><a v-on:click="Alarms(props.rowData)">Alarms</a></li>
            <li role="separator" class="divider"></li>
            <li><a v-on:click="Maintenance(props.rowData)">Maintenance</a></li>
          </ul>
        </div>
      </template>
    </vuetable>

      
      
      <!--analyzer-acls ref="ACLs"></analyzer-acls>
      <analyzer-acl ref="ACL"></analyzer-acl-->

  </dlg>
</template>

<script>
  import Vue from 'vue';
  import Dlg from 'Components/Dialog.vue';
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  
//  import AnalyzerAcls from './AnalyzerACLs.vue';
//  import AnalyzerAcl from './AnalyzerACL.vue';
  export default {
    data() {
      return { Analyzers: [], Analyzer: null,
        Columns: [
          {name: 'Online', title: 'Online', callback: 'OnlineStatus'},
          {name: 'Name', title: 'Description'}, 
          {name: 'Analyzer', title: 'Serial'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    methods: {
      OnlineStatus: function(value) {
        return (value === true) ? '&#9635;' : '&#9633;';
      },
      Show: function() {
        this.$refs.Dialog.Show();
        this.$refs.Dialog.ShowSpinner = true;
        $.ajax({ url: "getAnalyzers", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: (result) => {
            this.Analyzers = result;
            this.$refs.table.setData({ data: result });
            this.$refs.Dialog.ShowSpinner = false;
          }
        });
      },
      Close: function() {
        this.$refs.Dialog.Close();
      },
      CurrentlySelected: function(analyzer) {
        if(this.$root.SelectedAnalyzer === null) return false;
        if(analyzer.Analyzer === this.$root.SelectedAnalyzer.Analyzer) return true;
        return false;
      },
      Select: function(analyzer) {
        this.$refs.Dialog.ShowSpinner = true;
        $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: analyzer.Analyzer }),
          success: (result) => { 
            this.$root.SelectedAnalyzer = result;
            this.$root.$emit('AnalyzerChanged', result);
            this.Close();
            localStorage.setItem('SelAnalyzer', analyzer.Analyzer);
          },
          complete: () => {
            this.$refs.Dialog.ShowSpinner = false;
          }
        });
      },
      Configure: function(analyzer) {
        this.$refs.Dialog.ShowSpinner = true;
        $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: analyzer.Analyzer }), 
          success: (result) => {
            this.$refs.Dialog.Close();
            this.$parent.$refs.Configure.Show(result);
          },
          complete: () => {
            this.$refs.Dialog.ShowSpinner = false;
          }
        });
      },
      ACL: function(id) {
        alert("ACL not yet implemented"); 
      },
      Rates: function(analyzer) {
        this.$refs.Dialog.ShowSpinner = true;
        $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: analyzer.Analyzer }), 
          success: (result) => {
            this.$refs.Dialog.Close();
            this.$parent.$refs.Rates.Show(result);
          },
          complete: () => {
            this.$refs.Dialog.ShowSpinner = false;
          }
        });
      },
      Alarms: function(analyzer) {
        this.$refs.Dialog.ShowSpinner = true;
        $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: analyzer.Analyzer }), 
          success: (result) => {
            this.$refs.Dialog.Close();
            this.$parent.$refs.Alarms.Show(result);
          },
          completed: () => {
            this.$refs.Dialog.ShowSpinner = false;
          }
        });
      },
      Maintenance: function(analyzer) {
        this.$refs.Dialog.ShowSpinner = true;
        $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: analyzer.Analyzer }), 
          success: (result) => {
            this.$refs.Dialog.Close();
            this.$parent.$refs.Maintenance.Show(result);
          },
          complete: () => {
            this.$refs.Dialog.ShowSpinner = false;
          }
        });
      }
    },
    components: { Dlg, Spinner, Vuetable }
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