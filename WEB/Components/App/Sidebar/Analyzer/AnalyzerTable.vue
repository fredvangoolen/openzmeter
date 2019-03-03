<template>
  <div class="modal-content">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers</h4></div>
      <div class="pull-right">
        <button type="button" class="btn btn-default btn-xs" @click="$parent.$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
    </div>
    <div class="modal-body">
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-success btn-xs" v-bind:class="{ disabled: CurrentlySelected(props.rowData) }" v-on:click="Select(props.rowData)">Select</button>
            <button type="button" class="btn btn-default dropdown-toggle btn-xs" data-toggle="dropdown"><span class="caret"></span></button>
            <ul class="dropdown-menu pull-right">
              <li><a href="#" v-on:click="Configure(props.rowData)" >Configure</a></li>
              <li><a href="#" v-on:click="ACL(props.rowData)">ACL</a></li>
              <li><a href="#" v-on:click="Tariffs(props.rowData)">Tariff</a></li>
              <li><a href="#" v-on:click="Alerts(props.rowData)">Alerts</a></li>
              <li role="separator" class="divider"></li>
              <li><a href="#" v-on:click="Maintenance(props.rowData)">Maintenance</a></li>
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
    props: { analyzers: Array },
    data() {
      return { 
        Columns: [
          {name: 'Online', title: 'Online'},
          {name: 'Name', title: 'Description'}, 
          {name: 'ID', title: 'Serial'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    methods: {
      CurrentlySelected: function(id) {
        if(this.$root.SelectedAnalyzer == null) return false;
        if(id.ID == this.$root.SelectedAnalyzer.ID) return true;
        return false;
      },
      Select: function(id) {
        localStorage.setItem("SelAnalyzer", id.ID);
        this.$root.SelectedAnalyzer = id;
        this.$nextTick(function() {
          this.$root.$emit('AnalyzerChanged');
          this.$parent.$emit('close');
        });
      },
      Configure: function(id) { alert("Configure not yet implemented"); },
      ACL(id) {alert("ACL not yet implemented");},
      Tariffs(id) { this.$parent.$parent.$parent.showTariffs(this.rowData.ID); },
      Alerts(id) {alert("Alerts not yet implemented");},
      Maintenance(id) {alert("Maintenance not yet implemented");}
    },
    mounted: function() {
      var TableData = {data: this.analyzers};
      this.$refs.table.setData(TableData);
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
