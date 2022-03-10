<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers » Alarms » Edit</h4></div>
      <div class="pull-right btn-group">
        <z-button :active="View=='Main'" @click="View='Main'" tooltip="Main alarm properties" caption="Main" />
        <z-button :active="View=='Expression'" @click="View='Expression'" tooltip="Alarm expression configuration" caption="Expression" />
        <z-button @click="$emit('update', Alarm)" tooltip="Go back" :image="require('Imgs/arrow_undo.png')" />
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null">{{ Error }}</div>
    <div class="alert alert-success text-center" style="margin-bottom: 5px; padding: 5px" v-if="(Data!=null)&&(Data.Result!=null)&&(View=='Expression')">Expresion evaluated to: <span v-html="Round(Data.Result, 3, true)"></span><br/>Alarm is {{ (Data.Result==0.0) ? 'NOT' : '' }} triggered</div>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="(Data!=null)&&(Data.Result==null)&&(View=='Expression')">Expresion evaluation fail:<br/>{{ Data.Error }}</div>
    <div class="row" v-if="View=='Main'">
      <div class="col-xs-12">
        <label class="control-label" style="line-height: 22px">Enabled</label>
        <toggle class="float-right" onText="Yes" offText="No" v-model="Alarm.Enabled"></toggle>
      </div>
    </div>
    <div class="row" v-if="View=='Main'">
      <div class="col-xs-12">
        <label class="control-label">Name</label>
        <div class="input-group input-group-xs">
          <input type="text" class="form-control rounded" v-model="Alarm.Name"/>
        </div>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Priority</label>
        <option-picker :options="[{name: 'LOW', value: 0}, {name: 'NORMAL', value: 1}, {name: 'HIGH', value:2}]" v-model="Alarm.Priority"></option-picker>
      </div>
      <div class="col-xs-6">
        <label class="control-label">Aggregation</label>
        <option-picker :options="[{name: '200 miliseconds', value: '200MS'}, {name: '3 seconds', value: '3S'}, {name: '1 minute', value: '1M'}, {name: '10 minutes', value: '10M'}, {name: '15 minutes', value: '15M'}, {name: '1 hour', value: '1H'}]" v-model="Alarm.Aggreg"></option-picker>
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
        <label class="control-label">Expresion</label>
        <input-math v-model="Alarm.Expression" :vars="Vars" @evaluate="Evaluate"></input-math>
      </div>
    </div>
  </dlg>
</template>

<script>
  export default {
    props: { analyzer: Object, alarm: Number },
    data: function() {
      return { ShowSpinner: false, Alarm: null, Data: null, Vars: [], View: 'Main', Error: null };
    },
    created: function() {
      this.Alarm = Object.assign({}, this.analyzer.Alarms[this.alarm]);
    },
    methods: {
      FillValues: function(data) {
        var Result = [];
        if(data == null) return Result;
        for(var n in data.Variables) {
          var Var = { Name: n, Value: this.Round(data.Variables[n], 3) };
          Var.Description = '';
          if(Var.Name == 'ActivePower') Var.Units = 'W';
          if(Var.Name == 'ApparentPower') Var.Units = 'W';
          if(Var.Name == 'Current') Var.Units = 'A';
          if(Var.Name == 'CurrentTHD') Var.Units = '%';
          if(Var.Name == 'Frequency') Var.Units = 'Hz';
          if(Var.Name == 'NominalFrequency') Var.Units = 'Hz';
          if(Var.Name == 'NominalVoltage') Var.Units = 'V';
          if(Var.Name == 'Phi') Var.Units = '°';
          if(Var.Name == 'ReactivePower') Var.Units = 'W';
          if(Var.Name == 'Voltage') Var.Units = 'V';
          if(Var.Name == 'VoltageTHD') Var.Units = '%';
          Result.push(Var);
        }
        this.Vars = Result;
        return data;
      },
      Evaluate: function() {
        this.ShowSpinner = true;
        this.View = 'Expression';
        this.Data = [];
        this.Post('testAlarm', { Analyzer: this.analyzer.Analyzer, Expression: this.Alarm.Expression }).then(
          (result) => {
            this.ShowSpinner = false;
            this.Data = this.FillValues(result);
          },
          (fail) => {
            this.ShowSpinner = false;
            if(fail.status == 400) this.Data = this.FillValues(fail.body);
          }
        );
      }
    }
  }
</script>

<style scoped>
  .input-group {
    width: 100%;
  }
  .rounded {
    border-radius: 4px !important;
  }
</style>