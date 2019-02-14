<template>
  <div class="modal-content">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers</h4></div>
      <div class="pull-right">
        <button type="button" class="btn btn-default btn-xs" @click="$parent.$emit('close')"><img src="Imgs/arrow_undo.png"></button>
      </div>
    </div>
    <div class="modal-body">
      <vuetable id="AnalyzerTable" ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive"></vuetable>
    </div>
  </div>
</template>

<script>
  import Vue from 'vue';
  import Vuetable from 'vuetable-2';
  import AnalyzerTableActions from './AnalyzerTableActions.vue';
  Vue.component('AnalyzerTableActions', AnalyzerTableActions);
  export default {
    props: { analyzers: [] },
    data() {
      return { 
        Columns: [
          {name: 'Online', title: 'Online'},
          {name: 'Name', title: 'Description'}, 
          {name: '__component:AnalyzerTableActions', title: ''}
        ]
      };
    },
    mounted: function() {
      var TableData = {data: this.analyzers};
      this.$refs.table.setData(TableData);
    },
    components: { Vuetable }
  }
</script>

<style>
  #AnalyzerTable .vuetable-th-Online {
    width: 60px;
    text-align: center;
  }
  #AnalyzerTable tr td:first-child {
    text-align: center;
  }
  #AnalyzerTable .vuetable-th-component-id {
    width: 90px;
  }
  #AnalyzerTable tr td:nth-child(3) {
    text-align: center;
  }
</style>
