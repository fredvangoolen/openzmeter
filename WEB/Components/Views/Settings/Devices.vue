<template>
  <div class="panel panel-other">
    <div class="panel-heading">
      <div class="huge-small header-text">Input devices</div>
    </div>
    <div class="panel-body">
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" style="width:100%">
            <button class="btn btn-xs btn-danger" v-if="props.rowData.Status === 'RUNNING'" @click="Stop(props.rowData.Serial, $event)">Stop</button>
            <button class="btn btn-xs btn-success" v-if="props.rowData.Status !== 'RUNNING'" @click="Start(props.rowData.Serial, $event)">Stop</button>
            <button class="btn btn-xs btn-default" @click="Configure(props.rowData.Serial, $event)">Configure</button>
          </div>
        </template>
      </vuetable>
    </div>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  export default {
    data() {
      return { Devices: [], Table: null, 
        Columns: [
          {name: 'Driver', title: 'Driver'},
          {name: 'Name', title: 'Name'},
          {name: 'Serial', title: 'Serial'}, 
          {name: 'Analyzers', title: 'Analyzers'}, 
          {name: 'Status', title: 'Status'}, 
          {name: '__slot:actions', title: ''}
        ] 
      };
    },
    methods: {
      Stop: function(id, evt) {
        $(evt.target).blur();
        //alert(id);
      },
      Start: function(id, evt) {
        $(evt.target).blur();
        //alert(id);        
      },
      Configure: function(id, evt) {
        $(evt.target).blur();
        //alert(id);        
      }
    },
    mounted: function() {
      $.ajax({ url: "getDevices", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
        success: function(result) { 
          var TableData = { data: result };
          this.$refs.table.setData(TableData); 
        }.bind(this)
      });
    },
    components: { Vuetable }
  }
</script>

<style>
  thead tr th:nth-child(1) {
    width: 60px;
    text-align: center;
  }
  tr td:nth-child(1) {
    text-align: center;
  }
  thead tr th:nth-child(3) {
    width: 250px;
    text-align: center;
  }
  tr td:nth-child(3) {
    text-align: center;
  }
  thead tr th:nth-child(4) {
    width: 20px;
    text-align: center;
  }
  tr td:nth-child(4) {
    text-align: center;
  }
  thead tr th:nth-child(5) {
    width: 90px;
    text-align: center;
  }
  tr td:nth-child(5) {
    text-align: center;
  }
  thead tr th:nth-child(6) {
    width: 120px;
  }
  tr td:nth-child(6) {
    text-align: center;
  }
</style>