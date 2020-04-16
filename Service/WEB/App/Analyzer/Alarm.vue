<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Alarms &raquo; Edit</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Reload variables and evaluate" v-if="View=='Expression'" @click="Evaluate"><img :src="require('Imgs/math_functions.png')"></button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('endEdit', index, Alarm)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Main' }" @click="View='Main'">Main</button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Expression' }" @click="View='Expression'">Expression</button>
      </div>
    </template>
    <div class="alert alert-success text-center" style="margin-bottom: 5px; padding: 5px" v-if="(Data!=null)&&(Data.Result!=null)&&(View=='Expression')">Expresion evaluated to: <span v-html="Round(Data.Result, 3, true)"></span><br/>Alarm is {{ (Data.Result==0.0) ? 'NOT' : '' }} triggered</div>
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
        <option-picker :options="[{name: '200 miliseconds', value: '200MS'}, {name: '3 seconds', value: '3S'}, {name: '1 minute', value: '1M'}, {name: '10 minutes', value: '10M'}, {name: '15 minutes', value: '15M'}, {name: '1 hour', value: '1H'}]" v-model="Alarm.Aggreg" :big="true"></option-picker>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Occurrences</label>
        <input-spin :min="1" :max="1000" v-model="Alarm.Samples" :increment="1" :decimals="0" :big="true"></input-spin>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Window</label>
        <input-spin :min="1" :max="1000" v-model="Alarm.Window" :increment="1" :decimals="0" :big="true"></input-spin>
      </div>
    </div>
    <div class="row" v-if="View=='Expression'">
      <div class="col-xs-12">
        <label class="control-label">Expression</label>
        <input-math ref="expression" v-model="Alarm.Expression" :vars="[]"></input-math>
      </div>
      <div class="col-xs-12">
        <div style="width:100%; height: 200px; overflow-y: auto; margin-top: 5px">
          <vuetable ref="table" :fields="Columns" :api-mode="false" :data="Values" class="table small table-striped">
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
  export default {
    props: { analyzer: Object, index: Number },
    data: function() {
      return { ShowSpinner: false, Alarm: null, Data: null, View: 'Main',
        Columns: [
          { name: 'Name', title: 'Variable' },
          { name: 'Value', title: 'Value' },
          { name: 'Units', title: 'Units' },
          { name: '__slot:actions', title: '' }
        ]
      };
    },
    created: function() {
      this.Alarm = Object.assign({}, this.analyzer.Alarms[this.index]);
    },
    watch: {
      View: function() {
        if(this.View =='Expression') this.Evaluate();
      }
    },
    computed: {
      Values: function() {
        var Result = [];
        if(this.Data == null) return Result;
        for(var n in this.Data.Variables) {
          var Var = { Name: n, Value: this.Round(this.Data.Variables[n], 3) };
          if(Var.Name == 'Voltage') Var.Units = 'V';
          if(Var.Name == 'NominalVoltage') Var.Units = 'V';
          if(Var.Name == 'Frequency') Var.Units = 'Hz';
          if(Var.Name == 'NominalFrequency') Var.Units = 'Hz';
          Result.push(Var);
        }
        return Result;
      }
    },
    methods: {
      ValueFormat: function(val) {
        return this.Round(val, 2, false);
      },
      Evaluate: function() {
        this.ShowSpinner = true;
        this.View = 'Expression';
        this.$http.post('testAlarm', { Analyzer: this.analyzer.Analyzer, Expression: this.Alarm.Expression }).then(
          (result) => {
            this.ShowSpinner = false;
            this.Data = result.body;
          },
          (fail) => {
            this.ShowSpinner = false;
            if(fail.status == 400) this.Data = fail.body;
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