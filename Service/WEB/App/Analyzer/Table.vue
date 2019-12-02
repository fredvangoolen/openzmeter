<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers</h4></div>
      <div class="pull-right">
        <button type="button" class="btn btn-default btn-xs" v-on:click="$parent.View = null"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" v-if="Analyzers.length == 0" style="margin-bottom: 5px; padding: 5px">
      <p>There are no analyzers available.</p>
      <p>If you are an administrator, check device settings, otherwise request permission.</p>
    </div>
    <vuetable ref="table" :fields="Columns" :data="Analyzers" :api-mode="false" class="table small table-striped">
      <template slot="actions" slot-scope="props">
        <div class="btn-group">
          <button type="button" class="btn btn-success btn-xs" v-bind:disabled="CurrentAnalyzer(props.rowData)" v-on:click="Select(props.rowData)">Select</button>
          <button type="button" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown"><span class="caret"></span></button>
          <ul class="dropdown-menu pull-right">
            <li><a v-on:click="Configure(props.rowData)">Configure</a></li>
            <li><a v-on:click="ACL(props.rowData)" v-if="false">ACL</a></li>
            <li><a v-on:click="Rates(props.rowData)">Rates</a></li>
            <li><a v-on:click="Alarms(props.rowData)">Alarms</a></li>
            <li role="separator" class="divider"></li>
            <li><a v-on:click="Maintenance(props.rowData)">Maintenance</a></li>
          </ul>
        </div>
      </template>
    </vuetable>
  </dlg>
</template>

<script>
  import Vue from 'vue';
  import Vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data() {
      return { Analyzers: [], Analyzer: null, ShowSpinner: false,
        Columns: [
          {name: 'Online', title: 'Online', callback: this.OnlineStatus},
          {name: 'Name', title: 'Description'}, 
          {name: 'Analyzer', title: 'Serial'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    mounted: function() {
      this.ShowSpinner = true;
      $.ajax({ url: "getAnalyzers", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
        success: (result) => {
          this.Analyzers = result;
          this.ShowSpinner = false;
        }
      });
    },
    methods: {
      OnlineStatus: function(value) {
        return (value === true) ? '&#9635;' : '&#9633;';
      },
      CurrentAnalyzer: function(analyzer) {
        if(this.$root.SelectedAnalyzer === null) return false;
        if(this.$root.SelectedAnalyzer.Analyzer === analyzer.Analyzer) return true;
        return false;
      },
      Select: function(analyzer) {
        this.ShowSpinner = true;
        $.ajax({ url: "getAnalyzer", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: analyzer.Analyzer }),
          success: (result) => { 
            this.$root.SelectedAnalyzer = result;
            this.$root.$emit('AnalyzerChanged', result);
            localStorage.setItem('SelAnalyzer', result.Analyzer);    
            this.$parent.View = null;            
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
            this.$parent.Analyzer = result;
            this.$parent.View = null;
            this.$nextTick(() => { this.$parent.View = 'Configure'; } );
          },
          complete: () => {
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
            this.$parent.Analyzer = result;
            this.$parent.View = null;
            this.$nextTick(() => { this.$parent.View = 'Rates'; } );
          },
          complete: () => {
            this.ShowSpinner = false;
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
    components: { Vuetable }
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