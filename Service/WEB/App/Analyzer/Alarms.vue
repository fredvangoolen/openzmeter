<template>
  <dlg ref="Dialog">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Alarms</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-on:click="Add"><img :src="require('Imgs/plus.png')" alt="Add alarm"></button>
        <button type="button" class="btn btn-default btn-xs" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </template>
    <vuetable ref="table" :fields="columns" :api-mode="false" class="table small table-striped">
      <template slot="actions" slot-scope="props">
        <div class="btn-group">
          <button type="button" class="btn btn-default btn-xs" v-on:click="Edit(props.rowData)">Edit
          </button>
          <button type="button" class="btn btn-danger btn-xs" v-on:click="Delete(props.rowData)">Delete
          </button>
        </div>
      </template>
    </vuetable>
  </dlg>
</template>

<script>
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import Dlg from 'Components/Dialog.vue';
  export default {
    data() {
      return {
        Analyzer: null,

        alarms: [
          {name: "Alarm 1", id: 1, window: 10, time: 2, formula: "Active > 10", aggregation: '3S'},
          {name: "Alarm 2", id: 1, window: 10, time: 2, formula: "Active > 10", aggregation: '3S'},
          {name: "Alarm 3", id: 1, window: 10, time: 2, formula: "Active > 10", aggregation: '3S'},
          {name: "Alarm 4", id: 1, window: 10, time: 2, formula: "Active > 10", aggregation: '3S'}
        ],

        columns: [
          {name: 'id', title: 'ID'},
          {name: 'name', title: 'Name'},
          {name: 'aggregation', title: 'Aggregation'},
          {name: 'window', title: 'Window'},
          {name: 'time', title: 'Time'},
          {name: 'formula', title: 'Formula'}
        ],
      }
    },
    created() {

    },

    updated() {

    },

    methods: {
      Edit: function(row) {


      },
      Delete: function(row) {

      },
      Show: function(analyzer) {
        this.Analyzer = analyzer;
        this.$refs.Dialog.Show();
        this.$nextTick(() => {
          this.$refs.table.setData({data: this.alarms});
        });
      },
      Add: function() {
        this.$refs.Dialog.Close();
        this.$parent.$refs.Alarm.Show(this.Analyzer);
      },
      Back: function() {
        this.$parent.$refs.Table.Load();
      }
    },
    components: { vuetable, Dlg }
  }
</script>

<style scoped>

</style>
