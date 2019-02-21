<template>
  <div class="modal-content">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers</h4></div>
      <div class="pull-right">
        <button type="button" class="btn btn-default btn-xs" @click="$parent.$emit('close')"><img src="Imgs/arrow_undo.png"></button>
      </div>
    </div>
    <div class="modal-body">
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
                    <template slot="actions" slot-scope="props">
                        <div class="table-button-container">
                            <button class="ui button" @click="editRow(props.rowData)">
                                <i class="fa fa-edit"></i> 
                                View
                            </button>
                        </div>
                    </template>
      </vuetable>
    </div>
  </div>
</template>

<script>
  import Vue from 'vue';
  import Vuetable from 'vuetable-2';
  import AnalyzerTableActions from './AnalyzerTableActions.vue';
  Vue.component('AnalyzerTableActions', AnalyzerTableActions);
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
    mounted: function() {
      var TableData = {data: this.analyzers};
      this.$refs.table.setData(TableData);
    },
    components: { Vuetable }
  }
</script>

<style scoped>
  .vuetable-th-Online {
    width: 60px;
    text-align: center;
  }
  tr td:first-child {
    text-align: center;
  }
  .vuetable-th-component-id {
    width: 90px;
  }
  tr td:nth-child(3) {
    text-align: center;
  }
</style>
