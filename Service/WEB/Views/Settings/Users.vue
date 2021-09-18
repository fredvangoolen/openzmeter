<template>
  <q-card v-show="Enabled">
    <q-toolbar-title class="bg-grey-9 text-white">
      <div class="q-ml-sm">Users</div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" title="Add user" @click="Add"><img :src="require('Imgs/plus.png')"> Add</button>
      </div>
      <toolbar-pager class="pull-right" :pageSize="10" :input="Users" :output.sync="FilterUsers" :filtercols="['User', 'Name', 'TimeZone', 'Email']"></toolbar-pager>
    </q-toolbar-title>
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
  </q-card>
</template>

<script>
  import UserEdit from 'App/User/Dialog.vue';
  export default {
    data() {
      return { Users: [], FilterUsers: [], ShowSpinner: true, ShowDialog: false, Enabled: false, User: null,
        Columns: [
          {name: 'User', title: 'User name' },
          {name: 'Name', title: 'Name' },
          {name: 'TimeZone', title: 'Time zone', titleClass: 'text-center', dataClass: 'text-center' },
          {name: 'Email', title: 'E-Mail', titleClass: 'text-center', dataClass: 'text-center' },
          {name: '__slot:actions', title: '', width: '96px', dataClass: 'text-right' }
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
        this.Post('getUsers', { }).then(
          (result) => {
            this.Enabled = true;
            this.Users = result;
            this.ShowSpinner = false;
          },
          (fail) => { }
        );
      },
      Add: function() {
        this.User = null;
        this.ShowDialog = true;
      },
      Delete: function(user) {
        this.ShowSpinner = true;
        this.Post('delUser', { User: user.User }).then(
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
        this.Post('getUser', { User: user.User }).then(
          (result) => {
            this.User = result;
            this.ShowDialog = true;
            this.ShowSpinner = false;
          },
          (fail) => {
            this.ShowSpinner = false;
          }
        );
      }
    },
    components: { UserEdit }
  }
</script>

<style scoped>
  .header {
    display: inline-block;
    font-size: 20px;
    line-height: 22px;
  }
</style>