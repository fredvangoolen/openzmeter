<template>
  <q-card v-show="Enabled">
    <q-toolbar class="bg-grey-9 text-white q-py-none">
      <q-toolbar-title>Users</q-toolbar-title>
      <q-input dense debounce="300" label="Search" color="grey-3" label-color="white" v-model="Filter" input-class="text-white" class="q-mr-sm">
         <template v-slot:prepend>
            <q-icon name="search" color="white"></q-icon>
          </template>
      </q-input>
      <q-btn color="positive" @click="Add">Add</q-btn>
    </q-toolbar>
    <q-table :loading="ShowSpinner" dense :columns="Columns" :data="Users" :filter="Filter" row-key="user" class="q-ma-xs">
      <template v-slot:body-cell-actions="props">
        <q-td :props="props">
          <q-btn-group>
          <q-btn size="sm" @click="Edit(props.row)">Edit</q-btn>
          <q-separator vertical></q-separator>
          <q-btn size="sm" @click="Delete(props.row)">Delete</q-btn>
          </q-btn-group>
        </q-td>
      </template>
    </q-table>
    <user-edit v-if="ShowDialog" :user="User" @close="LoadUsers"></user-edit>
  </q-card>
</template>

<script>
  import UserEdit from 'App/User/Dialog.vue';
  export default {
    data() {
      return { Users: [], Filter: '', ShowSpinner: true, ShowDialog: false, Enabled: false, User: null,
        Columns: [
          { name: 'user', label: 'User name', field: row => row.User, align: 'left' },
          { name: 'uame', label: 'Name', field: row => row.Name, align: 'left' },
          { name: 'timezone', label: 'Time zone', field: row => row.TimeZone, align: 'center' },
          { name: 'email', label: 'E-Mail', field: row => row.Email, align: 'center' },
          { name: 'actions', label: '' }
        ]
      };
    },
    created: function () {
      this.LoadUsers();
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