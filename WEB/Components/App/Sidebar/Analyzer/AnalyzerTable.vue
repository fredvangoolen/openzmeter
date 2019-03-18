<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers</h4></div>
      <div class="pull-right">
        <button type="button" class="btn btn-default btn-xs" @click="$parent.Close"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
    </div>
    <div class="modal-body">
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-success btn-xs" v-bind:class="{ disabled: CurrentlySelected(props.rowData) }" v-on:click="Select(props.rowData)">Select</button>
            <button type="button" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown"><span class="caret"></span></button>
            <ul class="dropdown-menu pull-right">
              <li><a v-on:click="Configure(props.rowData)" >Configure</a></li>
              <li><a v-on:click="ACL(props.rowData)">ACL</a></li>
              <li><a v-on:click="Tariffs(props.rowData)">Tariff</a></li>
              <li><a v-on:click="Alerts(props.rowData)">Alerts</a></li>
              <li role="separator" class="divider"></li>
              <li><a v-on:click="Maintenance(props.rowData)">Maintenance</a></li>
            </ul>
          </div>
        </template>
      </vuetable>
    </div>
  </div>
</template>

<script>
  import Vue from 'vue';
  import Vuetable from 'vuetable-2';
  export default {
    data() {
      return {
        Show: false,
        Columns: [
          {name: 'Online', title: 'Online', callback: 'OnlineStatus'},
          {name: 'Name', title: 'Description'}, 
          {name: 'ID', title: 'Serial'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    methods: {
      GetAnalyzers() {
        $.ajax({ url: "getAnalyzers", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) { 
            var TableData = { data: result };
            this.$refs.table.setData(TableData);        
          }.bind(this)
        });
      },
      OnlineStatus: function(value) {
        return (value === true) ? '&#9635;' : '&#9633;';
      },
      Load: function(analyzers) {
        this.Show = true;
        this.$nextTick(function() {
          this.GetAnalyzers();
        });
      },
      CurrentlySelected: function(id) {
        if(this.$root.SelectedAnalyzer === null) return false;
        if(id.ID === this.$root.SelectedAnalyzer.ID) return true;
        return false;
      },
      Select: function(id) {
        localStorage.setItem("SelAnalyzer", id.ID);
        this.$root.SelectedAnalyzer = id;
        this.$nextTick(function() {
          this.$root.$emit('AnalyzerChanged');
          this.$parent.Close();
        });
      },
      Configure: function(id) {
        this.Show = false;
        this.$parent.$refs.Configure.Load(id.ID); 
      },
      ACL: function(id) {
        alert("ACL not yet implemented"); 
      },
      Tariffs: function(id) { 
        this.Show = false;
        this.$parent.$refs.Tariffs.Load(id.ID); 
      },
      Alerts: function(id) {
        alert("Alerts not yet implemented");
      },
      Maintenance: function(id) {
        alert("Maintenance not yet implemented");
      }
    },
    components: { Vuetable }
  }
</script>

<style scoped>
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
</style>
