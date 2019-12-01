<template>
  <dlg ref="Dialog">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Alarms &raquo; Edit</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Error != '') }" v-on:click="Save"><img :src="require('Imgs/diskette.png')" alt="Save alarm"></button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: View=='Main' }" v-on:click="View='Main'">Main</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: View=='Expression' }" v-on:click="View='Expression'; Evaluate()">Expression</button>
        <button type="button" class="btn btn-default btn-xs" v-on:click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != ''">{{ Error }}</div>
    <div class="row" v-bind:style="{ display: (View=='Main') ? 'block' : 'none' }">
      <div class="col-xs-12">
        <label class="control-label">Name</label>
        <input type="text" class="form-control form-control-sm" v-model="Alarm.Name"/>
      </div>
      <div class="col-xs-12">
        <label class="control-label">Alarm message</label>
        <textarea class="form-control" rows="2" v-model="Alarm.Message" style="resize: none"></textarea>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Priority</label>
        <option-picker :options="Priorities" :selected="Priorities[Alarm.Priority]" v-on:change="Alarm.Priority=Priorities.indexOf($event)" :big="true"></option-picker>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Aggregation</label>
        <option-picker ref="aggreg" :options="['3 seconds', '1 minute', '10 minutes', '15 minutes', '1 hour']" v-on:change="AggregationChanged" :big="true"></option-picker>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Occurrences</label>
        <input-spin :min="1" :max="1000" :value="Alarm.Samples" :increment="1" :decimals="0" v-on:change="Alarm.Samples=$event"></input-spin>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Window</label>
        <input-spin :min="1" :max="1000" :value="Alarm.Window" :increment="1" :decimals="0" v-on:change="Alarm.Window=$event"></input-spin>
      </div>
    </div>
    <div class="row" v-bind:style="{ display: (View=='Expression') ? 'block' : 'none' }">
      <div class="col-xs-12">
        <label class="control-label">Expression</label>
        <div class="input-group">
          <span class="input-group-addon">
            <img v-if="EvalResult===true" :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Alarm expression is valid and evaluate as triggered"/>
            <img v-if="EvalResult===false" :src="require('Imgs/tick.png')" data-toggle="tooltip" title="Alarm expression is valid and evaluate as not triggered!"/>
            <img v-if="EvalResult===null" :src="require('Imgs/cross.png')" data-toggle="tooltip" title="Alarm expression is invalid"/>
          </span>
          <input type="text" class="form-control" ref="expression" v-model="Alarm.Expression">
          <span class="input-group-btn">
            <button class="btn btn-default" data-toggle="tooltip" title="Evaluate" type="button" v-on:click="Evaluate"><img :src="require('Imgs/math_functions.png')"></button>
          </span>
        </div>
      </div>
      <div class="col-xs-12">
        <div style="width:100%; height: 200px; overflow-y: auto">
          <vuetable ref="table" :fields="columns" :api-mode="false" class="table small table-striped">
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
    data: function() {
      return { Analyzer: null, Alarm: {}, Error: '', EvalResult: null, View: 'Main', Priorities: ['LOW', 'NORMAL', 'HIGH'],
        columns: [
          { name: 'Name', title: 'Variable' },
          { name: 'Value', title: 'Value', callback: this.ValueFormat },
          { name: 'Units', title: 'Units' },
          { name: '__slot:actions', title: '' }
        ]
      };
    },
    methods: {
      AggregationChanged: function(val) {
        if(val === '3 seconds') {
          this.Alarm.Aggreg = '3S';
        } else if(val === '1 minute') {
          this.Alarm.Aggreg = '1M';
        } else if(val === '10 minutes') {
          this.Alarm.Aggreg = '10M';
        } else if(val === '15 minutes') {
          this.Alarm.Aggreg = '15M';
        } else if(val === '1 hour') {
          this.Alarm.Aggreg = '1H';
        }
      },
      ValueFormat: function(val) {
        return oZm.Round2Plain(val, 2);
      },
      Evaluate: function() {
        if(this.Alarm.Expression === undefined) this.Alarm.Expression = "";
        $.ajax({url: 'testAlarm', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ Analyzer: this.Analyzer.Analyzer, Aggreg: this.Alarm.Aggreg, Expression: this.Alarm.Expression }),
          success: (result) => {
            if(result.Error != undefined) {
              this.EvalResult = null;
              this.Error = result.Error;
            } else if(result.Result === 0.0) {
              this.EvalResult = false;
              this.Error = "";
            } else {
              this.EvalResult = true;
              this.Error = "";
            }
            this.$refs.table.setData({ data: result.Variables });
          }
        });
      },
      Insert: function(row) {
        var txt = jQuery(this.$refs.expression);
        var caretPos = txt[0].selectionStart;
        var textAreaTxt = txt.val();
        $(txt).focus();
        this.Alarm.Expression = textAreaTxt.substring(0, caretPos) + row.Name + textAreaTxt.substring(caretPos);
      },
      Show: function(analyzer, alarm) {
        this.Analyzer = analyzer;
        this.View = 'Main';
        this.Alarm = (alarm === null) ? {Aggreg: '3S', Expression: '', Message:'', Priority: 1, Samples: 10, Window: 50, Name: 'Unnamed alarm', } : alarm;
        this.$refs.Dialog.Show();
        this.$nextTick(() => {
          if(this.Alarm.Aggreg === '3S') {
            this.$refs.aggreg.SetOption('3 seconds');
          } else if(this.Alarm.Aggreg === '1M') {
            this.$refs.aggreg.SetOption('1 minute');
          } else if(this.Alarm.Aggreg === '10M') {
            this.$refs.aggreg.SetOption('10 minutes');
          } else if(this.Alarm.Aggreg === '15M') {
            this.$refs.aggreg.SetOption('15 minutes');
          } else if(this.Alarm.Aggreg === '1H') {
            this.$refs.aggreg.SetOption('1 hour');
          }
        });
      },
      Back() {
        this.$refs.Dialog.Close();
        this.$parent.$refs.Alarms.Show(this.Analyzer);
      },
      Save: function() {
        this.$refs.Dialog.ShowSpinner = true;
        $.ajax({ url: (this.Alarm.AlarmID === undefined) ? "addAlarm" : "setAlarm", timeout: 20000, type: "POST", dataType: "text", contentType: 'application/json', data: JSON.stringify( Object.assign({ Analyzer: this.Analyzer.Analyzer }, this.Alarm) ), 
          success: () => {
            this.$refs.Dialog.ShowSpinner = false;
            this.Back();
          },
          error: () => {
            this.$refs.Dialog.ShowSpinner = false;
            this.Error = 'Failed to save alarm to analyzer!';
            setTimeout(() => { this.Error = ''; }, 1000); 
          }
        });
      }
    },
    components: { Dlg, InputSpin, OptionPicker, vuetable }
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