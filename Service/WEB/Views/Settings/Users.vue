<template>
  <q-card v-show="Users.Loading == false">
    <q-toolbar class="bg-grey-9 text-white">
      <q-toolbar-title>Users</q-toolbar-title>
    </q-toolbar>
    <q-toolbar class="bg-grey-2 q-pa-none">
      <q-space />
      <q-input dense debounce="300" label="Search" v-model="Filter" class="q-mr-sm">
        <template v-slot:prepend>
          <q-icon name="search" />
        </template>
      </q-input>
      <q-btn stretch unelevated color="primary" @click="Add">Add</q-btn>
    </q-toolbar>
    <q-separator />
    <q-table dense :columns="Columns" :rows="Users.Rows" :filter="Filter" row-key="user" class="q-ma-xs">
      <template v-slot:no-data="data">
        <div class="full-width row flex-center q-gutter-sm">{{ data.message }}</div>
      </template>
      <template v-slot:body-cell-actions="props">
        <q-td :props="props">
          <q-btn-dropdown size="sm" split unelevated color="primary" @click="Edit(props.row)" label="Edit">
            <q-list dense>
              <q-item clickable @click="Delete(props.row)">Delete</q-item>
            </q-list>
          </q-btn-dropdown>
        </q-td>
      </template>
    </q-table>
    <user-edit v-if="ShowDialog" :user="User" @close="LoadUsers" />
    <q-inner-loading :showing="ShowSpinner"><q-spinner-dots size="50px" color="orange-10" /></q-inner-loading>
  </q-card>
</template>

<script>
  let UserEdit = () => import('App/User/Dialog.vue');
  export default {
    data: function() {
      return { ShowSpinner: false, Filter: '', ShowDialog: false, Enabled: false, User: null, Users: this.APIList(this.API.getUsers),
        Columns: [
          { name: 'user', label: 'User name', field: row => row.User, align: 'left' },
          { name: 'uame', label: 'Name', field: row => row.Name, align: 'left' },
          { name: 'timezone', label: 'Time zone', field: row => row.TimeZone, align: 'center' },
          { name: 'email', label: 'E-Mail', field: row => row.Email, align: 'center' },
          { name: 'actions', label: '' }
        ]
      };
    },
    watch: {
      'Users.Loading': function(val) {
        this.$emit('done');
      }
    },
    mounted: function () {
      this.$emit('queries', 1);
    },
    unmounted: function() {
      this.Users.Stop();
    },
    methods: {
      Add: function() {
        this.User = null;
        this.ShowDialog = true;
      },
      Delete: function(user) {
        this.ShowSpinner = true;
        this.Post('delUser', { User: user.User }).then(
          () => {
            this.LoadUsers();
          },
          () => {
            this.ShowSpinner = false;
          }
        );
      },
      Edit: function(user) {
        this.ShowSpinner = true;
        this.API.getUser(user.User).then(
          (result) => {
            this.User = result;
            this.ShowDialog = true;
            this.ShowSpinner = false;
          },
          () => {
            this.ShowSpinner = false;
          }
        );
      }
    },
    components: { UserEdit }
  }
</script>