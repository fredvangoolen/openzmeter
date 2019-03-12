<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="huge-small pull-left">Tariffs</div>
      <button type="button" class="pull-right btn btn-default btn-xs" data-toggle="tooltip" title="Add tariff" @click="Add"><img src="Imgs/plus.png"> Add</button>
    </div>
    <div class="panel-body">
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" style="width:100%">
            <button class="btn btn-xs btn-default" @click="Edit(props.rowData.RateID, $event)">Edit</button>
            <button class="btn btn-xs btn-danger" @click="Delete(props.rowData.RateID, $event)">Delete</button>
          </div>
        </template>
      </vuetable>
    </div>
    <tariffs-edit ref="EditDialog"></tariffs-edit>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import moment from 'JS/moment.js';
  import TariffsEdit from './TariffsEdit.vue';
  export default {
    data: function() {
      return {
        Columns: [
          {name: 'CountryCode', title: 'Country'},   
          {name: 'Name', title: 'Name'},
          {name: 'StartDate', title: 'Start date', callback: 'DateFormat'},
          {name: 'EndDate', title: 'End date', callback: 'DateFormat'}, 
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    methods: {
      DateFormat: function(val) {
        return moment(val).format('DD/MM/YYYY');
      },
      GetTariffs: function() {
        $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) {
            var TableData = { data: result };
            this.$refs.table.setData(TableData);
          }.bind(this)
        });
      },
      Edit: function(id, evt) {
        $(evt.target).blur();
        this.$refs.EditDialog.Show(id);
      },
      Add: function() {
        this.$refs.EditDialog.Show(null);
      },
      Delete: function(id, evt) {
        $(evt.target).blur();
        $.ajax({ url: "delTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ RateID: id }), success: GetTariffs });
      }
    },
    mounted: function() {
      this.GetTariffs();
    },
    components: { Vuetable, TariffsEdit }
  }
</script>

<style scoped>
  .table >>> th:nth-child(1) {
    width: 60px;
    text-align: center;
  }
  .table >>> td:nth-child(1) {
    text-align: center;
  }
  .table >>> th:nth-child(3), .table >>> th:nth-child(4)  {
    width: 150px;
    text-align: center;
  }
  .table >>> td:nth-child(3), .table >>> td:nth-child(4) {
    text-align: center;
  }
  .table >>> th:nth-child(5) {
    width: 100px;
  }
  .table >>> td:nth-child(5) {
    text-align: right;
  }
</style>