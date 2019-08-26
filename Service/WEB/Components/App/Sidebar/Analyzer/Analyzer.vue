<template>
  <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" data-keyboard="false">
    <div class="modal-backdrop in"></div>
    <div class="modal-dialog">
      <div class="modal-content" v-if="View=='List'">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">Analyzers</h4></div>
          <div class="pull-right">
            <button type="button" class="btn btn-default btn-xs" v-on:click="Close"><img :src="require('Imgs/arrow_undo.png')"></button>
          </div>
        </div>
        <div class="modal-body">
          <spinner v-if="ShowSpinner"></spinner>
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
                  <li><a v-on:click="Alerts(props.rowData)" v-if="false">Alerts</a></li>
                  <!-- li role="separator" class="divider"></li>
                  <li><a v-on:click="Maintenance(props.rowData)">Maintenance</a></li -->
                </ul>
              </div>
            </template>
          </vuetable>
        </div>
      </div>
      <analyzer-configure v-if="View=='Configure'" :analyzer="Analyzer"></analyzer-configure>
      <analyzer-rates v-if="View=='Rates'" :analyzer="Analyzer"></analyzer-rates>
      
      
      
      
      
      <!--
      
      
      
      <!--analyzer-acls ref="ACLs"></analyzer-acls>
      <analyzer-acl ref="ACL"></analyzer-acl-->
      <!--analyzer-maintenance ref="Maintenance"></analyzer-maintenance-->
    </div>
  </div>
</template>

<script>
  import Vue from 'vue';
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import Spinner from 'Components/Spinner.vue';
  import * as oZm from 'JS/openZmeter.js';
  import AnalyzerConfigure from './AnalyzerConfigure.vue';
  import AnalyzerRates from './AnalyzerRates';
//  import AnalyzerAcls from './AnalyzerACLs.vue';
//  import AnalyzerAcl from './AnalyzerACL.vue';
//  import AnalyzerMaintenance from './AnalyzerMaintenance.vue';
  export default {
    data() {
      return { ShowDialog: false, ShowSpinner: false, Analyzers: [], View: 'List', Analyzer: null,
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
      GetAnalyzers() {
        this.ShowSpinner = true;
        $.ajax({ url: "getAnalyzers", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: (result) => {
            this.Analyzers = result;
            this.$refs.table.setData({ data: result });
            this.ShowSpinner = false;
          }
        });
      },
      OnlineStatus: function(value) {
        return (value === true) ? '&#9635;' : '&#9633;';
      },
      Show: function() {
        this.ShowDialog = true;
        this.$nextTick(() => {     
          this.$root.$el.append(this.$el);
          $(this.$el).modal('show');
          this.GetAnalyzers();
        });
      },
      Close: function() {
        $(this.$el).modal('hide');
        this.ShowDialog = false;
        this.$root.$el.removeChild(this.$el);
      },
      CurrentlySelected: function(analyzer) {
        if(this.$root.SelectedAnalyzer === null) return false;
        if(analyzer.Analyzer === this.$root.SelectedAnalyzer.Analyzer) return true;
        return false;
      },
      Select: function(analyzer) {
        this.ShowSpinner = true;
        $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: analyzer.Analyzer }),
          success: (result) => { 
            this.$root.SelectedAnalyzer = result;
            this.$root.$emit('AnalyzerChanged', result);
            this.Close();
            localStorage.setItem('SelAnalyzer', analyzer.Analyzer);
          },
          complete: () => {
            this.ShowSpinner = false;
          }
        });
      },
      Configure: function(analyzer) {
        this.ShowSpinner = true;
        $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: analyzer.Analyzer }), 
          success: (result) => { 
            this.Analyzer = result;
            this.View = 'Configure'; 
          },
          completed: () => {
            this.ShowSpinner = false;
          }
        });
      },
      ACL: function(id) {
        alert("ACL not yet implemented"); 
      },
      Rates: function(analyzer) {
        this.ShowSpinner = true;
        $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: analyzer.Analyzer }), 
          success: (result) => {
            this.Analyzer = result;
            this.View = 'Rates';
          },
          error: () => {
            this.ShowSpinner = false;
          }
        });
      },
      Alerts: function(id) {
        alert("Alerts not yet implemented");
      },
      Maintenance: function(analyzer) {
        this.Show = false;
        this.$parent.$refs.Maintenance.Load(analyzer);
      }
    },
    components: { Spinner, Vuetable, AnalyzerConfigure, AnalyzerRates }
//    components: { AnalyzerNotFound, AnalyzerTable, AnalyzerRate, AnalyzerAcls, AnalyzerAcl, AnalyzerMaintenance }
  }
</script>

<style scoped>
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
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
</style>