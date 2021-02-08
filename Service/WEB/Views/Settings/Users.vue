<template>
  <div class="panel panel-other" v-show="Enabled">
    <div class="panel-heading">
      <div class="header">Users</div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" title="Add user" @click="Add"><img :src="require('Imgs/plus.png')"> Add</button>
      </div>
      <toolbar-pager class="pull-right" :pageSize="10" :input="Users" :output.sync="FilterUsers" :filtercols="['User', 'Name', 'TimeZone', 'Email']"></toolbar-pager>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :data="FilterUsers" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" >
            <button class="btn btn-xs btn-default" @click="Edit(props.rowData)">Edit</button>
            <button class="btn btn-xs btn-danger" @click="Delete(props.rowData)">Delete</button>
          </div>
        </template>
      </vuetable>
    </div>
    <user-edit v-if="ShowDialog" :user="User" @close="LoadUsers"></user-edit>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import UserEdit from 'App/User/Dialog.vue';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data() {
      return { Users: [], FilterUsers: [], ShowSpinner: true, ShowDialog: false, Enabled: false, User: null,
        Columns: [
          {name: 'User', title: 'User name' },
          {name: 'Name', title: 'Name' },
          {name: 'TimeZone', title: 'Time zone' },
          {name: 'Email', title: 'E-Mail' },
          {name: '__slot:actions', title: '' }
        ]
      };
    },
    created: function () {
      this.LoadUsers();
    },
    watch: {
      Filter: function() {
        this.LoadUsers();
      }
    },
    methods: {
      LoadUsers: function () {
        this.ShowDialog = false;
        this.ShowSpinner = true;
        this.$http.post('getUsers', { }).then(
          (result) => {
            this.Enabled = true;
            this.Users = result.data;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      Add: function() {
        this.User = null;
        this.ShowDialog = true;
      },
      Delete: function(user) {
        this.ShowSpinner = true;
        this.$http.post('delUser', { User: user.User }).then(
          (result) => {
            this.LoadUsers();
          },
          (fail) => {
            this.ShowSpinner = false;
          }
        );
      },
      Edit: function(user) {
        this.ShowSpinner = true;
        this.$http.post('getUser', { User: user.User }).then(
          (result) => {
            this.User = result.data;
            this.ShowDialog = true;
            this.ShowSpinner = false;
          },
          (fail) => {
            this.ShowSpinner = false;
          }
        );
      }
    },
    updated: function () {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    components: { Vuetable, UserEdit }
  }
</script>

<style scoped>
  .header {
    display: inline-block;
    font-size: 20px;
    line-height: 22px;
  }
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(3) {
      text-align: center;
    }
    .table >>> th:nth-child(4) {
      text-align: center;
    }
    .table >>> td:nth-child(3) {
      text-align: center;
    }
    .table >>> td:nth-child(4) {
      text-align: center;
    }
    .table >>> th:nth-child(5) {
      width: 120px;
      text-align: center;
    }
    .table >>> td:nth-child(5) {
      text-align: center;
    }
  }
</style>