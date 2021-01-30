<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title"> {{ $t('Title') }}</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Main' }" @click="View='Main'">{{ $t('Main') }}</button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Expression' }" @click="View='Expression'">{{ $t('Expression') }}</button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('update', Alarm)" data-title="tooltip" title="Back"><img :src="require('Imgs/arrow_undo.png')"></button>
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
      <div class="col-xs-12">
        <label class="control-label">{{ $t('Name') }}</label>
        <input type="text" class="form-control form-control-sm" v-model="Alarm.Name"/>
      </div>
      <div class="col-xs-6">
        <label class="control-label">{{ $t('Priority') }}</label>
        <option-picker :options="[{name: 'LOW', value: 0}, {name: 'NORMAL', value: 1}, {name: 'HIGH', value:2}]" v-model="Alarm.Priority" :big="true"></option-picker>
      </div>
      <div class="col-xs-6">
        <label class="control-label">{{ $t('Aggregation') }}</label>
        <option-picker :options="[{name: $t('200 miliseconds'), value: '200MS'}, {name: '3 seconds', value: '3S'}, {name: '1 minute', value: '1M'}, {name: '10 minutes', value: '10M'}, {name: '15 minutes', value: '15M'}, {name: '1 hour', value: '1H'}]" v-model="Alarm.Aggreg" :big="true"></option-picker>
      </div>
      <div class="col-xs-6">
        <label class="control-label">{{ $t('Occurrences') }}</label>
        <input-spin :min="1" :max="1000" v-model="Alarm.Samples" :increment="1" :decimals="0" :big="true"></input-spin>
      </div>
      <div class="col-xs-6">
        <label class="control-label">{{ $t('Window') }}</label>
        <input-spin :min="1" :max="1000" v-model="Alarm.Window" :increment="1" :decimals="0" :big="true"></input-spin>
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
      this.Alarm = Object.assign({ }, this.analyzer.Alarms[this.alarm]);
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
        this.$http.post('testAlarm', { Analyzer: this.analyzer.Analyzer, Expression: this.Alarm.Expression }).then(
          (result) => {
            this.ShowSpinner = false;
            this.Data = this.FillValues(result.body);
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

<i18n>
  {
    "en": {
      "Title": "Analyzers » Alarms » Edit",
      "Main": "Main",
      "Expression": "Expression",
      "Name": "Name",
      "Priority": "Priority",
      "Aggregation": "Aggregation",
      "Occurrences": "Occurrences",
      "Window": "Window",
      "200 miliseconds": "200 miliseconds"
    },
    "es": {
      "Title": "Analyzador » Alarmas » Editar",
      "Main": "Principal",
      "Expression": "Expresión",
      "Name": "Nombre",
      "Priority": "Prioridad",
      "Aggregation": "Agregación",
      "Occurrences": "Ocurrencias",
      "Window": "Ventana",
      "200 miliseconds": "200 milisegundos"
    }
  }
</i18n>