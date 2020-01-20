<template>
  <div class="panel panel-other" v-show="Enabled">
    <div class="panel-heading">
      <div class="huge-small header-text">Users</div>
      <div class="pull-right input-group input-group-xs" style="width:200px">
        <input type="text" class="form-control" placeholder="Filter..." disabled>
        <span class="input-group-btn">
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Add user" @click="Add"><img :src="require('Imgs/plus.png')"> Add</button>
        </span>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <vuetable ref="table" :fields="Columns" :data="Users" :api-mode="false" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" >
            <button class="btn btn-xs btn-default" v-on:click="Edit(props.rowData, $event)">Edit</button>
            <button class="btn btn-xs btn-danger" v-on:click="Delete(props.rowData, $event)">Delete</button>
          </div>
        </template>
      </vuetable>
    </div>
    <user-edit v-if="ShowDialog" :user="User"></user-edit>
  </div>
</template>

<script>
  import Vuetable from 'vuetable-2';
  import UserEdit from 'App/User/Dialog.vue';
  import 'CSS/table-responsive.css';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data() {
      return { Users: [], ShowSpinner: true, ShowDialog: false, Enabled: false, User: null,
        Columns: [
          {name: 'User', title: 'User name' },
          {name: 'Name', title: 'Name' },
          {name: 'TimeZone', title: 'Time zone' },
          {name: 'E-Mail', title: 'EMail' },
          {name: '__slot:actions', title: '' }
        ]
      };
    },
    created: function () {
      this.LoadUsers();
    },
    methods: {
      LoadUsers: function () {
        this.ShowSpinner = true;
        this.$http.post('getUsers', {}).then(
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
      Edit: function(user, evt) {
        $(evt.target).blur();
        this.ShowSpinner = true;
        this.$http.post('getUser', { User: user.User }).then(
          (result) => {
            this.User = result.data;
            this.ShowDialog = true;
          },
          (fail) => {
            this.ShowSpinner = false;
          }
        );
      },
      Save: function () {
        this.$refs.settings.ShowSpinner = true;
        $.ajax({url: "setDevice", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify(this.$refs.settings.Settings),
          success: (result) => {
            this.$refs.settings.ShowSpinner = false;
            this.Close();
            this.LoadTable();
          },
          error: () => {
            this.$refs.settings.Error = 'Failed to save device settings!';
            setTimeout(() => {
              this.$refs.settings.Error = '';
            }, 1000);
          }
        });
      },
      Close: function () {
        $(this.$refs.dialog).modal('hide');
        this.$root.$el.removeChild(this.$refs.dialog);
        this.ShowDialog = false;
      }
    },
    updated: function () {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    components: { Vuetable, UserEdit }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1) {
      text-align: center;
    }
    .table >>> td:nth-child(1) {
      text-align: center;
    }
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
  .modal-dialog {
    z-index: 1100;
    color: #333;
    font-size: 14px;
  }
  .checkbox {
    margin-top: 1px;
    margin-bottom: 0px;
    margin-right: 4px;
  }
  .modal-body {
    min-height: 100px;
  }
</style>