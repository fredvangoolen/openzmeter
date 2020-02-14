<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Alarms &raquo; Edit</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Main' }" @click="View='Main'">Main</button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Expression' }" @click="Evaluate">Expression</button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('endEdit', index, Alarm)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </template>
    <div class="alert alert-success text-center" style="margin-bottom: 5px; padding: 5px" v-if="(EvalResult!=null)&&(View=='Expression')&&(Error==null)">Expresion evaluated to: <span v-html="Round(EvalResult, 2, true)"></span><br/>Alarm is {{ (EvalResult==0.0) ? 'NOT' : '' }} triggered</div>
    <div class="row" v-if="View=='Main'">
      <div class="col-xs-12">
        <label class="control-label">Name</label>
        <input type="text" class="form-control form-control-sm" v-model="Alarm.Name"/>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Priority</label>
        <option-picker :options="[{name: 'LOW', value: 0}, {name: 'NORMAL', value: 1}, {name: 'HIGH', value:2}]" v-model="Alarm.Priority" :big="true"></option-picker>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Aggregation</label>
        <option-picker :options="[{name: '3 seconds', value: '3S'}, {name: '1 minute', value: '1M'}, {name: '10 minutes', value: '10M'}, {name: '15 minutes', value: '15M'}, {name: '1 hour', value: '1H'}]" v-model="Alarm.Aggreg" :big="true"></option-picker>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Occurrences</label>
        <input-spin :min="1" :max="1000" v-model="Alarm.Samples" :increment="1" :decimals="0"></input-spin>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Window</label>
        <input-spin :min="1" :max="1000" v-model="Alarm.Window" :increment="1" :decimals="0"></input-spin>
      </div>
    </div>
    <div class="row" v-if="View=='Expression'">
      <div class="col-xs-12">
        <label class="control-label">Expression</label>
        <div class="input-group">
          <input type="text" class="form-control" ref="expression" v-model="Alarm.Expression">
          <span class="input-group-btn">
            <button class="btn btn-default" data-toggle="tooltip" title="Evaluate" type="button" v-on:click="Evaluate"><img :src="require('Imgs/math_functions.png')"></button>
          </span>
        </div>
      </div>
      <div class="col-xs-12">
        <div style="width:100%; height: 200px; overflow-y: auto; margin-top: 5px">
          <vuetable ref="table" :fields="columns" :api-mode="false" :data="Values" class="table small table-striped">
            <template slot="actions" slot-scope="props">
              <div class="btn-group">
                <button type="button" class="btn btn-primary btn-xs" v-on:click="Insert(props.rowData)">Insert</button>
              </div>
            </template>
          </vuetable>
        </div>
      </div>
    </div>
  </dlg>
</template>

<script>
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  import Dlg from 'Components/Dialog.vue';
  import InputSpin from 'Components/InputSpin.vue';
  import OptionPicker from 'Components/OptionPicker.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object, index: Number },
    data: function() {
      return { ShowSpinner: false, Alarm: null, Values: [], EvalResult: null, View: 'Main', Priorities: ['LOW', 'NORMAL', 'HIGH'],
        columns: [
          { name: 'Name', title: 'Variable' },
          { name: 'Value', title: 'Value', callback: this.ValueFormat },
          { name: 'Units', title: 'Units' },
          { name: '__slot:actions', title: '' }
        ]
      };
    },
    created: function() {
      this.Alarm = Object.assign({}, this.analyzer.Alarms[this.index]);
    },
    methods: {
      ValueFormat: function(val) {
        return this.Round(val, 2, false);
      },
      Evaluate: function() {
        this.View = 'Expression';
        this.$http.post('testAlarm', { Analyzer: this.analyzer.Analyzer, Aggreg: this.Alarm.Aggreg, Expression: this.Alarm.Expression }).then(
          (result) => {
            this.EvalResult = result.data.Result;
            this.Error = null;
            this.Values = result.data.Variables;
          },
          (fail) => {
            this.Error = fail.data.Error;
            this.Values = fail.data.Variables;
            this.EvalResult = null;
          }
        );
      },
      Insert: function(row) {
        var txt = jQuery(this.$refs.expression);
        var caretPos = txt[0].selectionStart;
        var textAreaTxt = txt.val();
        $(txt).focus();
        this.$set(this.Alarm, 'Expression', textAreaTxt.substring(0, caretPos) + row.Name + textAreaTxt.substring(caretPos));
      }
    },
    components: { vuetable }
  }
</script>

<style scoped>
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(2) {
      width: 100px;
      text-align: right;
    }
    .table >>> td:nth-child(2) {
      text-align: right;
    }
    .table >>> th:nth-child(3) {
      width: 20px;
    }
    .table >>> th:nth-child(4) {
      width: 30px;
    }
  }
</style>