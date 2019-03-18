<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Tariffs</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" id="btnAnalyzersTariffsAdd"><img src="Imgs/plus.png" alt="New tariff"></button>
        <button type="button" class="btn btn-default btn-xs" id="btnAnalyzersTariffsBack"><img src="Imgs/arrow_undo.png" alt="Discard"></button>
      </div>
    </div>
    <div class="modal-body">
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" v-on:click="Edit(props.rowData)">Edit</button>
            <button type="button" class="btn btn-danger btn-xs" v-on:click="Edit(props.rowData)">Delete</button>
          </div>
        </template>
      </vuetable>                  
    </div>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  import moment from 'JS/moment.js';
  export default {
    data() {
      return { Show: true,
        Columns: [
          {name: 'Name', title: 'Name'},
          {name: 'StartDate', title: 'Start date', callback: 'DateFormat'},
          {name: 'EndDate', title: 'End date', callback: 'DateFormat'}, 
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    methods: {
      Load(id) {
        this.Show = true;
        $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: id.ID }), 
          success: function(result) {
            var TableData = { data: result };
            this.$refs.table.setData(TableData);
          }.bind(this)
        });
      },
      DateFormat: function(val) {
        return moment(val).format('DD/MM/YYYY');
      },
    },
    components: { vuetable }
  }
</script>
  
<style>

</style>