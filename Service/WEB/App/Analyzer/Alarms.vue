<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="View==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Alarms and events</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save alarm and events config"></button>
          <button type="button" class="btn btn-default btn-xs" @click="Add"><img :src="require('Imgs/plus.png')" alt="Add alarm"></button>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
        </div>
      </template>
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null">{{ Error }}</div>
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label" style="line-height: 22px">Events detection</label>
          <toggle class="float-right" onText="Yes" offText="No" v-model="analyzer.EventsEnabled"></toggle>
          <div class="help">Enable standar event detection according to ?</div>
        </div>
        <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
        <div class="col-xs-12">
          <label class="control-label" style="line-height: 22px">Alarms detection</label>
          <toggle class="float-right" onText="Yes" offText="No" v-model="analyzer.AlarmsEnabled"></toggle>
          <div class="help">Evaluate math expressions with analysis params and generate alarms</div>
        </div>
      </div>
      <vuetable ref="table" :fields="columns" :api-mode="false" :data="analyzer.Alarms" class="table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-default btn-xs" @click="Edit(props.rowIndex)">Edit</button>
            <button type="button" class="btn btn-danger btn-xs" @click="Delete(props.rowIndex)">Delete</button>
          </div>
        </template>
      </vuetable>
    </dlg>
    <analyzer-alarm v-if="View=='Alarm'" @endEdit="EndEdit" :analyzer="analyzer" :index="Index"></analyzer-alarm>
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
      return { ShowSpinner: false, Index: null, View: null, Error: null,
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
    methods: {
      EndEdit: function(index, value) {
        this.analyzer.Alarms[index] = value;
        this.View = null;
      },
      Add: function() {
        this.analyzer.Alarms.push({ Name: 'Unnamed alarm', Priority: 1, Aggreg: '1M', Samples: 10, Window: 20 });
        this.Index = this.analyzer.Alarms.length - 1;
        this.View = 'Alarm';
      },
      Edit: function(row) {
        this.Index = row;
        this.View = 'Alarm';
      },
      Delete: function(row) {
        if(confirm("Delete '" + this.analyzer.Alarms[row].Name + "' alarm?")) this.analyzer.Alarms.splice(row, 1);
      },
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post('setAnalyzer', this.analyzer).then(
          (result) => {
            if(result.status == 200) {
              this.$http.post('getAnalyzer', { Analyzer: this.analyzer.Analyzer }).then(
                (result) => {
                  this.ShowSpinner = false;
                  if(result.status == 200) this.$emit('close', result.data);
                }
              );
            } else {
              this.ShowSpinner = false;
              this.Error = 'Failed to save analyzer!';
              setTimeout(() => { this.Error = ''; }, 5000);
            }
          },
        );
      },
      PriorityFormat: function(val) {
        if(val == 0) return 'Low';
        if(val == 1) return 'Normal';
        if(val == 2) return 'High';
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
  .help {
    font-size: 80%;
  }
</style>
