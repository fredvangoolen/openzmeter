<template>
  <dlg ref="Dialog">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Rates</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-on:click="Add"><img :src="require('Imgs/plus.png')" alt="Add rate"></button>
        <button type="button" class="btn btn-default btn-xs" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != ''">{{ Error }}</div>
    <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error == ''">All dates/times are in analyzer timezone.</div>
    <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped">
      <template slot="actions" slot-scope="props">
        <div class="btn-group">
          <button type="button" class="btn btn-default btn-xs" v-on:click="Edit(props.rowData)">Edit</button>
          <button type="button" class="btn btn-danger btn-xs" v-on:click="Delete(props.rowData)">Delete</button>
        </div>
      </template>
    </vuetable>           
  </dlg>
</template>

<script>
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import moment from 'JS/moment.js';
  import Dlg from 'Components/Dialog.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data() {
      return { Error: '', Analyzer: null, Tariffs: null,
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
    methods: {
      Show: function(analyzer) {
        this.Analyzer = analyzer;
        this.$refs.Dialog.Show();
        this.GetRates();
      },
      GetRates: function() {
        this.$refs.Dialog.ShowSpinner = true;
        $.ajax({ url: "getRates", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: this.Analyzer.Analyzer }),
          success: (result) => {
            this.$refs.table.setData({ data: result });
            this.Error = '';
            for(var n = 1; n < result.length; n++) {
              if(result[n].StartDate > result[n - 1].EndDate) continue;
              this.Error = 'Assigned rates overlaps, please check date ranges';
              return;
            }
          },
          complete: () => {
            this.$refs.Dialog.ShowSpinner = false;
          }
        });
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
        return moment.tz(val, this.Analyzer.TimeZone).format('DD/MM/YYYY');
      },
      Edit: function(row) {
        this.GetTariffs((tariffs) => {
          this.$refs.Dialog.ShowSpinner = true;
          $.ajax({ url: "getRate", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: this.Analyzer.Analyzer, RateID: row.RateID }), 
            success: (result) => {
              this.$refs.Dialog.Close();
              this.$parent.$refs.Rate.Show(this.Analyzer, tariffs, result);
            },
            complete: () => {
              this.$refs.Dialog.ShowSpinner = false;              
            }
          });
        });
      },        
      Delete: function(row) {
        if(confirm("Delete '"+row.Name+"' rate?")) {
          this.$refs.Dialog.ShowSpinner = true;
          $.ajax({ url: "delRate", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ RateID: row.RateID, Analyzer: this.Analyzer.Analyzer }), 
            success: () => { 
              this.GetRates();
            },
            Error: () => {
              this.$refs.Dialog.ShowSpinner = false;
            }
          });
        }
      },
      Add: function() {
        this.GetTariffs((tariffs) => {
          this.$refs.Dialog.Close();
          this.$parent.$refs.Rate.Show(this.Analyzer, tariffs, null);
        });
      },
      Back: function() {
        this.$refs.Dialog.Close();
        this.$parent.$refs.Dialog.Show();
      }
    },
    components: { vuetable, Dlg }
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