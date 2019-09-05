<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; ACLs</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="Add tariff"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </div>
    <div class="modal-body">
      <spinner v-if="ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped">
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
  import 'CSS/table-responsive.css';
  import Spinner from 'Components/Spinner.vue';
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'JS/moment.js';
  export default {
    data() {
      return { Show: false, ShowSpinner: true,
        Columns: [
          {name: 'UserName', title: 'User Name'},
          {name: 'View', title: 'View', callback: 'DateFormat'},
          {name: ''},
          {name: 'EndDate', title: 'End date', callback: 'DateFormat'}, 
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    created: function() {
      this.Serial = null;
    },
    methods: {
      DateFormat: function(val) {
        return moment(val).format('DD/MM/YYYY');
      },
      Load(id) {
        this.Show = true;
        this.Serial = id;
        this.ShowSpinner = true;
        $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: id }), 
          success: function(result) {
            this.$refs.table.setData({ data: result });
            this.ShowSpinner = false;
          }.bind(this)
        });
      },
      Edit: function(row) {
        this.Show = false;
        this.$parent.$refs.Tariff.Load(this.Serial, row.RateID);
      },
      Delete: function(row) {
        //Elimino
        if(confirm("Delete '"+this.serial+"' device?")){
          this.Load(this.Serial);        
        }
      },
      Add: function() {
        this.Show = false;
        this.$parent.$refs.Tariff.Load(this.Serial, null);
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Table.Load();
      }
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    components: { vuetable }
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