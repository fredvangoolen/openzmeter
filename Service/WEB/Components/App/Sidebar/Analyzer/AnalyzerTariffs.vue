<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Tariffs</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: ShowSpinner }" @click="Add"><img :src="require('Imgs/plus.png')" alt="Add tariff"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </div>
    <div class="modal-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != '' && !ShowSpinner">{{ Error }}</div>
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" v-on:click="Edit(props.rowData)">Edit</button>
            <button type="button" class="btn btn-danger btn-xs" v-on:click="Delete(props.rowData)">Delete</button>
          </div>
        </template>
      </vuetable>                  
    </div>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  import moment from 'JS/moment.js';
  import Spinner from 'Components/Spinner.vue';
  export default {
    data() {
      return { Show: false, ShowSpinner: true, Error: '',
        Columns: [
          {name: 'Name', title: 'Name'},
          {name: 'StartDate', title: 'Start date', callback: 'DateFormat'},
          {name: 'EndDate', title: 'End date', callback: 'DateFormat'}, 
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    created: function() {
      this.Analizer = null;
    },
    methods: {
      DateFormat: function(val) {
        return moment(val).format('DD/MM/YYYY');
      },
      Load(analizer) {
        this.Show = true;
        this.ShowSpinner = true;
        this.Analyzer = analizer;
        $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: analizer.ID }),
          success: function(result) {
            var TableData = { data: result };
            this.$refs.table.setData(TableData);
            this.ShowSpinner = false;
            this.Error = '';
            for(var n = 1; n < result.length; n++) {
              if(result[n].StartDate <= result[n - 1].EndDate) {
                this.Error = 'Assigned tariffs overlaps, please check date ranges';
                return;
              }
            }
          }.bind(this),
          complete: function() {
            this.ShowSpinner = false;
          }.bind(this)
        });
      },
      Edit: function(row) {
        this.Show = false;
        this.$parent.$refs.Tariff.Load(this.Analyzer, row.RateID);
      },
      Delete: function(row) {
        if(confirm("Delete '"+row.Name+"' tariff?")){
          this.ShowSpinner = true;
          $.ajax({ url: "delTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ RateID: row.RateID, Analyzer: this.Analyzer.ID }), success: function() { this.Load(this.Analyzer); }.bind(this) });
        }
      },
      Add: function() {
        this.Show = false;
        this.$parent.$refs.Tariff.Load(this.Analyzer, null);
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Table.Load();
      }
    },
    components: { vuetable, Spinner }
  }
</script>
  
<style scoped>
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
</style>