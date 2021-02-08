<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="(View!='Expresion') && View!='ExpresionPeriods'">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Tariffs template</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Save" :class="{disabled: (Error!=null)}"><img :src="require('Imgs/diskette.png')"></button>
          <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Main' }" @click="View='Main'">Main</button>
          <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Periods' }" @click="View='Periods'">Billing terms</button>
          <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Calendar' }" @click="View='Calendar'">Calendar</button>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
        </div>
      </template>
      <div class="alert alert-danger text-center" v-if="Error!=null">{{ Error }}</div>
      <div class="row" v-if="View=='Main'">
        <div class="col-xs-12">
          <label class="control-label">Name</label>
          <input type="text" class="form-control" v-model="Tariff.Name" :disabled="tariff!=null">
        </div>
        <div class="col-xs-12">
          <label class="control-label">Description</label>
          <textarea class="form-control" rows="5" v-model="Tariff.Template.Description" style="resize: none"></textarea>
        </div>
      </div>
      <div class="row" v-if="View=='Periods'">
        <div class="col-xs-12">
          <table style="width: 100%"><tr><td>
            <detail-tabs key="Period_Tabs" :tabs="['General'].concat(Array(Tariff.Template.Periods.length).fill(0).map((n, idx) => { return 'P' + (idx + 1)}))" v-model="ActivePeriod" :light="true"></detail-tabs>
          </td><td style="width: 64px; text-align: right;">
            <div class="btn-group">
              <button class="btn btn-default btn-xs" type="button" :disabled="Tariff.Template.Periods.length==1" @click="DeletePeriod" title="Delete last period"><img :src="require('Imgs/cross.png')"></button>
              <button class="btn btn-default btn-xs" type="button"><img :src="require('Imgs/plus.png')" @click="AddPeriod" title="Add new period"></button>
            </div>
          </td></tr></table>
          <div class="panel-tab panel panel-dialog" style="margin-bottom: 0px" v-if="ActivePeriod=='General'">
            <div class="panel-heading">
              <div class="pull-right btn-group">
                <button class="btn btn-default btn-xs" type="button" title="Edit tariff script" @click="View='Expresion'"><img :src="require('Imgs/math_functions.png')"></button>
                <button class="btn btn-default btn-xs" type="button" title="Add new tariff concept" @click="Tariff.Template.Concepts.push({Description: 'Unnamed concept', Variable: '', Type: 'INFO'})"><img :src="require('Imgs/plus.png')"></button>
              </div>
              <toolbar-pager class="pull-right" :pageSize="10" :input="Tariff.Template.Concepts" :output.sync="FilterConcepts" :filtercols="['Description', 'Variable']"></toolbar-pager>
              <div class="clearfix"></div>
            </div>
            <div class="panel-body">
              <div class="row">
                <div class="col-xs-4">
                  <label class="small">Currency</label>
                  <div class="input-group input-group-xs" style="width: 100%">
                    <input type="text" class="form-control rounded" v-model="Tariff.Template.Currency">
                  </div>
                </div>
                <div class="col-xs-4">
                  <label class="small">Energy units</label>
                  <div class="input-group input-group-xs" style="width: 100%">
                    <option-picker v-model="Tariff.Template.EnergyUnits" :options="[ {name: 'kWh', value: 'kWh'} ]" ></option-picker>
                  </div>
                </div>
                <div class="col-xs-4">
                  <label class="small">Power units</label>
                  <div class="input-group input-group-xs" style="width: 100%">
                    <option-picker v-model="Tariff.Template.PowerUnits" :options="[ {name: 'kW', value: 'kW'}, {name: 'kVA', value: 'kVA'} ]" ></option-picker>
                  </div>
                </div>
                <vuetable :fields="Columns" :api-mode="false" :data="FilterConcepts" class="table small table-striped">
                  <template slot="concept" slot-scope="props">
                    <div class="input-group input-group-xs">
                      <span class="input-group-btn">
                        <button class="btn btn-default" type="button" @click="Tariff.Template.Concepts.splice(props.rowData.rowNum, 1)"><img :src="require('Imgs/cross.png')"></button>
                      </span>
                      <input type="text" class="form-control" v-model="props.rowData.Description">
                    </div>
                  </template>
                  <template slot="variable" slot-scope="props">
                    <div class="input-group input-group-xs" style="width:100%">
                      <input type="text" class="form-control rounded" v-model="props.rowData.Variable">
                    </div>
                  </template>
                  <template slot="type" slot-scope="props">
                    <option-picker v-model="props.rowData.Type" :options="[ {name: 'Info', value: 'INFO'}, {name: 'General cost', value: 'COST'}, {name: 'Energy cost', value: 'COST_ENERGY'}, {name: 'Power cost', value: 'COST_POWER'} ]" ></option-picker>
                  </template>
                </vuetable>
              </div>
            </div>
          </div>
          <div v-for="(period, idx) in Tariff.Template.Periods" :key="idx">
            <div class="panel panel-dialog panel-tab" v-if="ActivePeriod=='P'+(idx+1)" style="margin-bottom: 0px">
              <div class="panel-heading">
                <div class="pull-right btn-group">
                  <button class="btn btn-default btn-xs" type="button" title="Edit period script" @click="View='ExpresionPeriods'"><img :src="require('Imgs/math_functions.png')"></button>
                  <button class="btn btn-default btn-xs" type="button" title="Add new period concept" @click="period.Concepts.push({Description: 'Unnamed concept', Variable: '', Type: 'INFO'})"><img :src="require('Imgs/plus.png')"></button>
                </div>
                <toolbar-pager class="pull-right" :pageSize="10" :input="period.Concepts" :output.sync="FilterConcepts" :filtercols="['Description', 'Variable']"></toolbar-pager>
                <div class="clearfix"></div>
              </div>
              <div class="panel-body">
                <div class="row">
                  <div class=" col-xs-12" style="display:flex">
                    <div style="flex-grow:2">
                      <label class="small">Period name</label>
                      <div class="input-group input-group-xs" style="width: 100%; padding-right: 4px">
                        <input type="text" class="form-control rounded" v-model="period.Name">
                      </div>
                    </div>
                    <div>
                      <label class="small">Period color</label><br/>
                      <color-picker v-model="period.Color" :columns="2" :colors="['#fffff0', '#ffecd6', '#ffd8bc', '#ffc4a2', '#ffae88', '#ff986c', '#ff7f50', '#f0ffff', '#d4f2f1', '#b7e5e2', '#98d8d3', '#78ccc5', '#55bfb8', '#20b2aa']"></color-picker>
                    </div>
                  </div>
                  <div class="col-xs-12" style="display:flex">
                    <div style="flex-grow:2">
                      <label class="small">Default power cost:</label>
                      <input-spin :max="100000" :min="0" :prepend="Tariff.Template.Currency+'/'+Tariff.Template.PowerUnits+'/Day'" :decimals="6" :increment="0.01" v-model="period.DefaultPowerCost" style="padding-right:4px"></input-spin>
                    </div>
                    <div style="flex-grow:2">
                      <label class="small">Default energy cost:</label>
                      <input-spin :max="100000" :min="0" :prepend="Tariff.Template.Currency+'/'+Tariff.Template.EnergyUnits" :decimals="6" :increment="0.01" v-model="period.DefaultEnergyCost"></input-spin>
                    </div>
                  </div>
                  <vuetable :fields="Columns" :api-mode="false" :data="FilterConcepts" class="table small table-striped">
                    <template slot="concept" slot-scope="props">
                      <div class="input-group input-group-xs">
                        <span class="input-group-btn">
                          <button class="btn btn-default" type="button" @click="period.Concepts.splice(props.rowData.rowNum, 1)"><img :src="require('Imgs/cross.png')"></button>
                        </span>
                        <input type="text" class="form-control" v-model="props.rowData.Description">
                      </div>
                    </template>
                    <template slot="variable" slot-scope="props">
                      <div class="input-group input-group-xs" style="width:100%">
                        <input type="text" class="form-control rounded" v-model="props.rowData.Variable">
                      </div>
                    </template>
                    <template slot="type" slot-scope="props">
                      <option-picker v-model="props.rowData.Type" :options="[ {name: 'Info', value: 'INFO'}, {name: 'General cost', value: 'COST'}, {name: 'Energy cost', value: 'COST_ENERGY'}, {name: 'Power cost', value: 'COST_POWER'} ]" ></option-picker>
                    </template>
                  </vuetable>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="row" v-if="View=='Calendar'">
        <div class="col-xs-12" style="margin-bottom: 6px">
          <table style="width: 100%;"><tr><td>
            <detail-tabs key="Day_Tabs" :tabs="new Array(Tariff.Template.Days.length).fill(0).map((n, idx) => { return 'D' + (idx + 1)})" v-model="ActiveDay" :light="true"></detail-tabs>
          </td><td style="width: 64px; text-align: right">
            <div class="btn-group">
              <button class="btn btn-default btn-xs" type="button" :disabled="Tariff.Template.Days.length==1" title="Delete last day" @click="DeleteDay"><img :src="require('Imgs/cross.png')"></button>
              <button class="btn btn-default btn-xs" type="button" @click="AddDay"><img :src="require('Imgs/plus.png')" title="Add new day"></button>
            </div>
          </td></tr></table>
          <div v-for="(day, idx) in Tariff.Template.Days" :key="idx">
            <div class="panel panel-tab panel-dialog" v-if="ActiveDay=='D'+(idx+1)">
              <div class="panel-heading">
                <toolbar-color-items v-model="ActivePeriodIdx" prefix="P" :colors="Tariff.Template.Periods.map((val) => { return val.Color; })" :names="Tariff.Template.Periods.map((val, i) => { return 'P' + (i + 1) + ' - ' + val.Name; })"></toolbar-color-items>
              </div>
              <div class="col-xs-12" style="display:flex">
                <div style="flex-grow:2">
                  <label class="small">Day name</label>
                  <div class="input-group input-group-xs" style="width: 100%; padding-right: 4px">
                    <input type="text" class="form-control rounded" v-model="day.Name">
                  </div>
                </div>
                <div>
                  <label class="small">Day color</label><br/>
                  <color-picker v-model="day.Color" :columns="2" :colors="['#fffff0', '#ffecd6', '#ffd8bc', '#ffc4a2', '#ffae88', '#ff986c', '#ff7f50', '#f0ffff', '#d4f2f1', '#b7e5e2', '#98d8d3', '#78ccc5', '#55bfb8', '#20b2aa']"></color-picker>
                </div>
              </div>
              <div class="clearfix"></div>
              <div class="day-stripes-table">
                <day-config v-model="day.Quarters" :selected="ActivePeriodIdx" :colors="Tariff.Template.Periods.map((val) => { return val.Color; })" :names="Tariff.Template.Periods.map((val, i) => { return 'P' + (i + 1) + ' - ' + val.Name; })"></day-config>
              </div>
            </div>
          </div>
        </div>
        <div class="col-xs-12">
          <table style="width: 100%">
            <tr><td>
              <detail-tabs key="Year_Tabs" :tabs="Tariff.Template.Years.map((year) => { return String(year.Year); }).sort()" v-model="ActiveYear" :light="true"></detail-tabs>
            </td>
            <td style="width:130px; text-align: right;">
              <div class="btn-group">
                <button class="btn btn-default btn-xs" type="button" :disabled="Tariff.Template.Years.length==1" @click="DeleteYear" title="Delete this year"><img :src="require('Imgs/cross.png')"></button>
              </div>
              <div class="btn-group">
                <input-spin :max="2100" :min="2000" :decimals="0" :increment="1" v-model="NewYearSelect" style="width:90px">
                  <template v-slot:buttons>
                    <button class="btn btn-default btn-xs" type="button" :disabled="Tariff.Template.Years.map((year) => { return year.Year; }).includes(NewYearSelect)" @click="AddYear" title="Add new year"><img :src="require('Imgs/plus.png')"></button>
                  </template>
                </input-spin>
              </div>
            </td>
            </tr>
          </table>
          <div v-for="(year, idx) in Tariff.Template.Years" :key="idx">
            <div class="panel panel-tab panel-dialog" v-if="parseInt(ActiveYear)==year.Year">
              <div class="panel-heading">
                <toolbar-color-items v-model="ActiveDayIdx" prefix="D" :colors="Tariff.Template.Days.map((val) => { return val.Color; })" :names="Tariff.Template.Days.map((val, i) => { return 'D' + (i + 1) + ' - ' + val.Name; })"></toolbar-color-items>
              </div>
              <div class="day-stripes-table">
                <year-config v-model="Tariff.Template.Years[idx].Days" :selected="ActiveDayIdx" :year="year.Year" :colors="Tariff.Template.Days.map((val) => { return val.Color; })" :names="Tariff.Template.Days.map((val, i) => { return 'D' + (i + 1) + ' - ' + val.Name; })"></year-config>
              </div>
            </div>
          </div>
        </div>
      </div>
    </dlg>
    <dlg v-if="View=='Expresion'">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Tariff script</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="View='Periods'"><img :src="require('Imgs/arrow_undo.png')" alt="Go back"></button>
        </div>
      </template>
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label">Expresion</label>
          <input-math v-model="Tariff.Template.Script" :vars="VarsMain"></input-math>
        </div>
      </div>
    </dlg>
    <dlg v-if="View=='ExpresionPeriods'">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Period {{ ActivePeriod }} script</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="View='Periods'"><img :src="require('Imgs/arrow_undo.png')" alt="Go back"></button>
        </div>
      </template>
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label">Expresion</label>
          <input-math v-model="Tariff.Template.Periods[parseInt(ActivePeriod.substr(1)) - 1].Script" :vars="VarsPeriod"></input-math>
        </div>
      </div>
    </dlg>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  import moment from 'moment';
  export default {
    props: { tariff: Object, source: Object },
    data: function() {
      return { Error: null, View: 'Main', ActivePeriod: 'P1', ActivePeriodIdx: 0, ActiveDay: 'D1', ActiveDayIdx: 0, ActiveYear: '2020', NewYearSelect: moment().year(), Tariff: null, ShowSpinner: false, FilterConcepts: [],
               Columns: [
                 { name: '__slot:concept', title: 'Concept' },
                 { name: '__slot:variable', title: 'Variable' },
                 { name: '__slot:type', title: 'Type' }
               ]
      };
    },
    computed: {
      VarsPeriod: function() {
        var Vars = [];
        var Concepts = this.Tariff.Template.Periods[parseInt(this.ActivePeriod.substr(1)) - 1].Concepts;
        Vars.push({ Name: 'Cost', Description: '<strong>[OUT]</strong> Estimated contribution to period cost', Value: null, Units: this.Tariff.Template.Currency});
        for(var C = 0; C < Concepts.length; C++) {
          if(Concepts[C].Variable.length > 0) Vars.push({ Name: Concepts[C].Variable, Description: '<strong>[OUT]</strong> Defined in concepts', Value: null, Units: ''});
        }
        Vars.push({ Name: 'ActivePower', Description: 'Active power on evaluated period', Value: null, Units: 'W'});
        Vars.push({ Name: 'ActiveEnergy', Description: 'Active energy of evaluated period', Value: null, Units: 'Wh'});
        Vars.push({ Name: 'ReactivePower', Description: 'Reactive power on evaluated period', Value: null, Units: 'var'});
        Vars.push({ Name: 'ReactiveEnergy', Description: 'Reactive energy of evaluated period', Value: null, Units: 'varh'});
        Vars.push({ Name: 'ContractedPower', Description: 'Contracted power on evaluated period', Value: null, Units: this.Tariff.Template.PowerUnits});
        Vars.push({ Name: 'PowerCost', Description: 'Power cost on evaluated period', Value: null, Units: this.Tariff.Template.Currency + '/' + this.Tariff.Template.PowerUnits + '/day'});
        Vars.push({ Name: 'EnergyCost', Description: 'Energy cost on evaluated period', Value: null, Units: this.Tariff.Template.Currency + '/' + this.Tariff.Template.EnergyUnits});
        return Vars;
      },
      VarsMain: function() {
        var Vars = [];
        Vars.push({ Name: 'Cost', Description: '<strong>[OUT]</strong> Estimated contribution to period cost', Value: null, Units: this.Tariff.Template.Currency});
        for(var C = 0; C < this.Tariff.Template.Concepts.length; C++) {
          if(this.Tariff.Template.Concepts[C].Variable.length > 0) Vars.push({ Name: this.Tariff.Template.Concepts[C].Variable, Description: '<strong>[OUT]</strong> Defined in concepts', Value: null, Units: ''});
        }
        Vars.push({ Name: 'Period', Description: 'Index of evaluated period (From 1 to ' + this.Tariff.Template.Periods.length + ')', Value: null, Units: ''});
        Vars.push({ Name: 'ActivePower', Description: 'Active power on evaluated period', Value: null, Units: 'W'});
        Vars.push({ Name: 'ActiveEnergy', Description: 'Active energy of evaluated period', Value: null, Units: 'Wh'});
        Vars.push({ Name: 'ReactivePower', Description: 'Reactive power on evaluated period', Value: null, Units: 'var'});
        Vars.push({ Name: 'ReactiveEnergy', Description: 'Reactive energy of evaluated period', Value: null, Units: 'varh'});
        for(var P = 0; P < this.Tariff.Template.Periods.length; P++) {
          Vars.push({ Name: 'ContractedPower_P' + (P + 1), Description: 'Contracted power on period P' + (P + 1), Value: null, Units: this.Tariff.Template.PowerUnits});
          Vars.push({ Name: 'PowerCost_P' + (P + 1), Description: 'Power cost on period P' + (P + 1), Value: null, Units: this.Tariff.Template.Currency + '/' + this.Tariff.Template.PowerUnits + '/day'});
          Vars.push({ Name: 'EnergyCost_P' + (P + 1), Description: 'Energy cost on period P' + (P + 1), Value: null, Units: this.Tariff.Template.Currency + '/' + this.Tariff.Template.EnergyUnits});
        }
        return Vars;
      }
    },
    created: function() {
      if(this.source != null) {
        this.Tariff = Object.assign({}, this.source);
        this.Tariff.Name += '_Copy';
      } else if(this.tariff == null) {
        this.Tariff = { Name: 'No name', Template: { Description: 'Write a tariff description here', Currency: '€', EnergyUnits: 'kWh', PowerUnits: 'kW', Script: '', Concepts: [], Periods: [], Days: [], Years: [] } };
        this.AddPeriod();
        this.AddDay();
        this.AddYear();
      } else {
        this.Tariff = Object.assign({}, this.tariff);
      }
    },
    methods: {
      DeletePeriod: function() {
        if(this.Tariff.Template.Periods.length == 1) return;
        if(confirm("Delete period 'P" + this.Tariff.Template.Periods.length + "'?")) this.Tariff.Template.Periods.splice(-1);
        for(var D in this.Tariff.Template.Days) {
          for(var Q in this.Tariff.Template.Days[D].Quarters) this.Tariff.Template.Days[D].Quarters[Q] = Math.min(this.Tariff.Template.Days[D].Quarters[Q], this.Tariff.Template.Periods.length - 1);
        }
      },
      DeleteDay: function() {
        if(this.Tariff.Template.Days.length == 1) return;
        if(confirm("Delete day 'D" + this.Tariff.Template.Days.length + "'?")) this.Tariff.Template.Days.splice(-1);
        for(var Y in this.Tariff.Template.Years) {
          for(var D in this.Tariff.Template.Years[Y].Days) this.Tariff.Template.Years[Y].Days[D] = Math.min(this.Tariff.Template.Years[Y].Days[D], this.Tariff.Template.Days.length - 1);
        }
      },
      DeleteYear: function() {
        if(Object.keys(this.Tariff.Template.Years).length == 1) return;
        if(confirm("Delete year " + this.ActiveYear + "?")) this.Tariff.Template.Years = this.Tariff.Template.Years.filter(year => year.Year != this.ActiveYear);
        this.ActiveYear = String(Object.keys(this.Tariff.Template.Years)[0]);
      },
      AddPeriod: function() {
        this.Tariff.Template.Periods.push({ Color: '#fffff0', DefaultPowerCost: 0.0, DefaultEnergyCost: 0.0, Name: 'Period ' + (this.Tariff.Template.Periods.length + 1), Script: '', Concepts: [] });
        this.ActivePeriod = 'P' + this.Tariff.Template.Periods.length;
      },
      AddDay: function() {
        this.Tariff.Template.Days.push({ Color: '#fffff0', Name: 'Day ' + (this.Tariff.Template.Days.length + 1), Quarters: new Array(96).fill(0) });
        this.ActiveDay = 'D' + this.Tariff.Template.Days.length;
      },
      AddYear: function() {
        var NewYear = {Year: this.NewYearSelect, Days: new Array(moment([this.NewYearSelect, 11, 31]).dayOfYear()).fill(0) };
        if(this.Tariff.Template.Years.length > 0) {
          var PrevYear = this.Tariff.Template.Years.filter(year => year.Year == this.ActiveYear)[0];
          for(var d = 0; d < NewYear.Days.length; d++) NewYear.Days[d] = (PrevYear.Days[d] == null) ? 0 : PrevYear.Days[d];
        }
        this.Tariff.Template.Years.push(NewYear);
        this.ActiveYear = String(this.NewYearSelect);
      },
      Save: function() {
        this.$http.post((this.tariff == null) ? 'addTariff' : 'setTariff', this.Tariff).then(
          (result) => {
            this.$emit('close');
          },
          (fail) => {
            this.Error = fail.body;
            setTimeout(() => { this.Error = null; }, 5000);
          }
        );
      }
    },
    components: { vuetable }
  }
</script>

<style scoped>
  .panel-tab {
    margin-bottom: 0px;
    border-top-left-radius: 0px;
  }
  label.small {
    font-size: 85%;
    margin-bottom: 0px;
  }
  .panel-heading {
    margin-bottom: 4px;
  }
  table.period_vars tr td {
    padding: 2px 4px;
  }
  table.period_vars {
    margin-top: 6px;
    margin-bottom: 2px;
    width: 100%;
  }
  table.period_vars tr:first-child {
    font-size: 85%;
  }
  table.period_vars .input-group {
    width: 100%;
  }
  table.period_vars td:nth-child(3) {
    width: 110px;
  }
  .rounded {
    border-radius: 4px!important;
  }
  div .alert {
    margin-bottom: 5px;
    padding: 5px
  }
</style>