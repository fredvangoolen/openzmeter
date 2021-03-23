<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Permissions</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save users permissions"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', null)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </template>
    <div class="col-xs-12">
      <div class="input-group input-group-xs">
        <span class="input-group-addon">User</span>
        <input type="text" class="form-control" v-model="NewUser">
        <span class="input-group-btn"><button type="button" class="btn btn-default" @click="Add" :disabled="NewUser.length==0"><img :src="require('Imgs/plus.png')"></button></span>
      </div>
    </div>
    <vuetable :fields="Columns" :api-mode="false" :data="Analyzer.Users" class="table small table-striped">
      <template slot="actions" slot-scope="props">
        <div class="btn-group" v-if="(props.rowData.User!='admin')&&(props.rowData.User!=Analyzer.User)">
          <button type="button" class="btn btn-danger btn-xs" @click="Delete(props.rowData.User)">Delete</button>
        </div>
      </template>
      <template slot="read" slot-scope="props">
        <input type="checkbox" v-model="props.rowData.Read" :disabled="(props.rowData.User=='admin')||(props.rowData.User==Analyzer.User)">
      </template>
      <template slot="write" slot-scope="props">
        <input type="checkbox" v-model="props.rowData.Write" :disabled="(props.rowData.User=='admin')||(props.rowData.User==Analyzer.User)">
      </template>
      <template slot="config" slot-scope="props">
        <input type="checkbox" v-model="props.rowData.Config" :disabled="(props.rowData.User=='admin')||(props.rowData.User==Analyzer.User)">
      </template>
    </vuetable>
  </dlg>
</template>

<script>
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object },
    data() {
      return { Analyzer: [], ShowSpinner: false, NewUser: '',
        Columns: [
          {name: 'User', title: 'User Name'},
          {name: '__slot:read', title: 'Read' },
          {name: '__slot:write', title: 'Write' },
          {name: '__slot:config', title: 'Configure' },
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    updated: function() {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    created: function() {
      this.Analyzer = Object.assign({}, this.analyzer);
    },
    methods: {
      Add: function() {
        for(var n = 0; n < this.Analyzer.Users.length; n++) {
          if(this.Analyzer.Users[n].User == this.NewUser) {
            this.NewUser = '';
            return;
          }
        }
        this.Analyzer.Users.push({ User: this.NewUser, Read: true, Write: true, Config: true });
        this.NewUser = '';
      },
      Delete: function(row) {
        if(confirm("Delete '" + row + "' user?") == false) return;
        for(var n = 0; n < this.Analyzer.Users.length; n++) {
          if(this.Analyzer.Users[n].User != row) continue;
          this.Analyzer.Users.splice(n, 1);
          return;
        }
      },
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post('setAnalyzer', this.Analyzer, { headers: { 'Accept': 'application/json', 'Content-Type': 'application/json' } }).then(
          (result) => { this.$emit('close', this.Analyzer); },
          (fail) => {
            this.ShowSpinner = false;
            this.Error = 'Failed to save analyzer!';
            setTimeout(() => { this.Error = null; }, 5000);
          }
        );
      }
    },
    components: { vuetable }
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
      width: 62px;
    }
    .table >>> td:nth-child(5) {
      text-align: right;
    }
  }
</style>