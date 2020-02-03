<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="View==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Alarms</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="Add alarm"></button>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
        </div>
      </template>
      <vuetable ref="table" :fields="columns" :api-mode="false" :data="Alarms" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" @click="Edit(props.rowData)">Edit</button>
            <button type="button" class="btn btn-danger btn-xs" @click="Delete(props.rowData)">Delete</button>
          </div>
        </template>
      </vuetable>
    </dlg>
    <analyzer-alarm v-if="View=='Alarm'" @close="View=null" :analyzer="analyzer" :alarm="Alarm"></analyzer-alarm>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import AnalyzerAlarm from './Alarm.vue';
  export default {
    props: { analyzer: Object },
    data() {
      return { Alarms: [], ShowSpinner: true, Alarm: null, View: null,
        columns: [
          { name: 'Name', title: 'Name' },
          { name: 'UserName', title: 'User' },
          { name: 'Priority', title: 'Priority', callback: this.PriorityFormat },
          { name: '__slot:actions', title: '' }
        ]
      };
    },
    updated: function() {
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    created: function() {
      this.GetAlarms();
    },
    methods: {
      GetAlarms: function() {
        this.ShowSpinner = true;
        if(this.analyzer == null) return;
        this.$http.post('getAlarms', { Analyzer: this.analyzer.Analyzer }).then(
          (result) => {
            this.Alarms = result.data;
            this.ShowSpinner = false;
          },
          (fail) => {}
        );
      },
      Edit: function(row) {
        $.ajax({ url: "getAlarm", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Analyzer: this.Analyzer.Analyzer, AlarmID: row.AlarmID }),
          success: (result) => {
            this.$refs.Dialog.Close();
            this.$parent.$refs.Alarm.Show(this.Analyzer, result);
          },
          complete: () => {
            this.$refs.Dialog.ShowSpinner = false;
          }
        });
      },
      Delete: function(row) {
        if(confirm("Delete '" + row.Name + "' alarm?")) {
          this.$refs.Dialog.ShowSpinner = true;
          $.ajax({ url: "delAlarm", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ AlarmID: row.AlarmID, Analyzer: this.Analyzer.Analyzer }),
            success: () => {
              this.GetAlarms();
            },
            Error: () => {
              this.$refs.Dialog.ShowSpinner = false;
            }
          });
        }
      },
      Add: function() {
        this.Alarm = null;
        this.View = 'Alarm';
      },
      PriorityFormat: function(val) {
        if(val === 0) return 'Low';
        if(val === 1) return 'Normal';
        if(val === 2) return 'High';
      }
    },
    components: { vuetable, AnalyzerAlarm }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(3) {
      width: 94px;
      text-align: center;
    }
    .table >>> td:nth-child(3) {
      text-align: center;
    }
    .table >>> th:nth-child(4) {
      width: 94px;
    }
    .table >>> td:nth-child(4) {
      text-align: right;
    }
  }
</style>
