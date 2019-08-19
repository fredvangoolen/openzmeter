<template>
  <div class="panel panel-other">
    <div class="panel-heading">
      <div class="huge-small header-text">Input devices</div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" >
            <button class="btn btn-xs btn-danger" v-if="props.rowData.Status === 'RUNNING'" @click="Stop(props.rowData.Serial, $event)">Stop</button>
            <button class="btn btn-xs btn-success" v-if="props.rowData.Status !== 'RUNNING'" @click="Start(props.rowData.Serial, $event)">Start</button>
            <button class="btn btn-xs btn-default" @click="Configure(props.rowData, $event)">Configure</button>
          </div>
        </template>
      </vuetable>
    </div>
    <devices-configure ref="Dialog"></devices-configure>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import Spinner from 'Components/Spinner.vue';
  import DevicesConfigure from './DevicesConfigure.vue';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data(){ 
      return { Devices: [], ShowSpinner: true,
        Columns: [
          {name: 'Driver', title: 'Driver'},
          {name: 'Name', title: 'Name'},
          {name: 'Serial', title: 'Serial'}, 
          {name: 'Analyzers', title: 'Analyzers'}, 
          {name: 'Status', title: 'Status'}, 
          {name: '__slot:actions', title: ''}
        ]
      }
    },
    methods: {
      LoadTable: function() {
        $.ajax({ url: "getDevices", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) { 
            var TableData = { data: result };
            this.$refs.table.setData(TableData); 
            this.ShowSpinner = false;
          }.bind(this)
        });
      },
      Stop: function(id, evt) {
        $(evt.target).blur();
        $.ajax({ url: "stopDevice", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Serial: id }), 
          success: function(result) { 
            this.LoadTable();
            this.$root.$children[0].$refs.sidebar.$refs.analyzer.CheckAnalyzer();
          }.bind(this)
        });
      },
      Start: function(id, evt) {
        $(evt.target).blur();
        $.ajax({ url: "startDevice", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Serial: id }), 
          success: function(result) { 
            this.LoadTable();
            this.$root.$children[0].$refs.sidebar.$refs.analyzer.CheckAnalyzer();
          }.bind(this)
        });
      },
      Configure: function(id, evt) {
        this.$refs.Dialog.Load(id);
      }
    },
    mounted: function() {
      this.ShowSpinner = true;
      this.LoadTable();
    },
    updated: function(){
        oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    components: { Vuetable, DevicesConfigure, Spinner}
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
  }
  .table >>> th:nth-child(3) {
    width: 250px;
    text-align: center;
  }
  .table >>> td:nth-child(3) {
    text-align: center;
  }
  .table >>> th:nth-child(4) {
    width: 20px;
    text-align: center;
  }
  .table >>> td:nth-child(4) {
    text-align: center;
  }
  .table >>> th:nth-child(5) {
    width: 90px;
    text-align: center;
  }
  .table >>> td:nth-child(5) {
    text-align: center;
  }
  .table >>> th:nth-child(6) {
    width: 120px;
  }
}
</style>