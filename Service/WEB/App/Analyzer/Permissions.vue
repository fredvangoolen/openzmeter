<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="View==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Access</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="Add tariff"></button>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close', null)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
        </div>
      </template>
      <vuetable :fields="Columns" :api-mode="false" :data="Permissions" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" @click="Edit(props.rowData)">Edit</button>
            <button type="button" class="btn btn-danger btn-xs" @click="Delete(props.rowData)">Delete</button>
          </div>
        </template>
      </vuetable>
    </dlg>
    <analyzer-permission :analyzer="analyzer" :permission="Permission" v-if="View=='Permission'" @close="GetPermissions"></analyzer-permission>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  import AnalyzerPermission from './Permission.vue';
  export default {
    props: { analyzer: Object },
    data() {
      return { View: null, Permissions: [], ShowSpinner: true,
        Columns: [
          {name: 'User', title: 'User Name'},
          {name: 'Read', title: 'Read', callback: this.BooleanFormat },
          {name: 'Write', title: 'Write', callback: this.BooleanFormat },
          {name: 'Configure', title: 'Configure', callback: this.BooleanFormat },
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    updated: function() {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    created: function() {
      this.GetPermissions();
    },
    watch: {
      analyzer: function() {
        this.GetPermissions();
      }
    },
    methods: {
      BooleanFormat: function(value) {
        return (value === true) ? '&#9635;' : '&#9633;';
      },
      GetPermissions: function() {
        this.View = null;
        this.ShowSpinner = true;
        this.ShowSpinner = true;
        if(this.analyzer == null) return;
        this.$http.post('getPermissions', { Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.Permissions = result.data;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      Add: function() {
        this.Permission = null;
        this.View = 'Permission';
      },
      Edit: function(row) {
        this.Permission = row;
        this.View = 'Permission';
      },
      Delete: function(row) {
        //Elimino
        if(confirm("Delete '"+this.serial+"' device?")){
          this.Load(this.Serial);
        }
      }
    },
    components: { vuetable, AnalyzerPermission }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(2), .table >>> th:nth-child(3), .table >>> th:nth-child(4) {
      width: 70px;
      text-align: center;
    }
    .table >>> td:nth-child(2), .table >>> td:nth-child(3), .table >>> td:nth-child(4) {
      text-align: center;
    }
    .table >>> th:nth-child(5) {
      width: 94px;
    }
    .table >>> td:nth-child(5) {
      text-align: right;
    }
  }
</style>