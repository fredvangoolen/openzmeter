<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Tariffs &raquo; Edit &raquo; Billing</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="row">
        <div class="col-xs-12">
          <table>
            <tr><td><strong>Concept</strong></td><td><strong>Type</strong></td><td class="text-right"><strong>Value</strong></td></tr>
            <tr v-for="(tax, idx) in Tariff.Config.Taxs">
              <td><div class="input-group input-group-xs">
                <span class="input-group-btn"><button class="btn btn-default delete" type="button" @click="Delete(idx)"><img :src="require('Imgs/cross.png')"></button></span>
                <input type="text" class="form-control" @change="NameChanged($event, idx)" :value="tax.Name">
              </div></td>
              <td><option-picker :options="TypeNames" @OptionChanged="TypeChanged($event, idx)" :selected="TypeNames[TypesEnum.indexOf(tax.Type)]"></option-picker></td>
              <td><div class="input-group input-group-xs">
                <input type="text" class="form-control text-right" @change="ValueChanged($event, idx)" :value="Round6(tax.Value)"> 
                <span class="input-group-addon">{{ (tax.Type == 'FIXED_QUANTITY') ? '€/Bill cycle' : '%' }}</span>
              </div></td>
            </tr>
            <tr>
              <td colspan="3"><button type="button" class="btn btn-default btn-xs" style="width:100%" @click="Add"><img :src="require('Imgs/plus.png')"> New concept</button></td>
            </tr>
          </table>
        </div>
        <div class="col-xs-6">
          <label class="control-label">Bill day</label>
          <option-picker :options="BillDayNames" :selected="BillDayNames[Day]" v-on:OptionChanged="DayChanged"></option-picker>
        </div>
        <div class="col-xs-6">
          <label class="control-label">Bill period</label>
          <option-picker :options="PeriodNames" :selected="PeriodNames[Period]" v-on:OptionChanged="PeriodChanged"></option-picker>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import OptionPicker from 'Components/OptionPicker.vue';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data: function() {
      return { Show: false, Day: 0, Period: 0, TypeNames: ['Total', 'Energy', 'Power', 'Energy+Power', 'Fixed'],
                PeriodNames: ['Each month', 'Each two month', 'Each three month'],
                BillDayNames: ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th', '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th']
              };
    },
    methods: {
      Round6: oZm.Round6Plain,
      Load: function(tariff) {
        if(tariff.Config.Billing.length < 1) {
          this.Day = moment(tariff.StartDate).date() - 1;
        } else {
          this.Day = moment(tariff.Config.Billing[0]).date() - 1;
        }
        this.Day = Math.max(0, Math.min(this.Day, this.BillDayNames.length - 1));
        if(tariff.Config.Billing.length < 2) {
          this.Period = 0;
        } else {
          this.Period = moment(tariff.Config.Billing[1]).diff(moment(tariff.Config.Billing[0]), 'months') - 1;
        }
        this.Period = Math.max(0, Math.min(this.Period, this.PeriodNames.length - 1));
        this.Tariff = tariff;
        this.CreateBuildPeriods();
        this.Show = true;
      },
      PeriodChanged: function(value) {
        this.Period = this.PeriodNames.indexOf(value);
        this.CreateBuildPeriods();
      },
      DayChanged: function(value) {
        this.Day = this.BillDayNames.indexOf(value);
        this.CreateBuildPeriods();
      },
      CreateBuildPeriods: function() {
        this.Tariff.Config.Billing = [];
        var Date = moment(this.Tariff.StartDate);
        while(true) {
          Date = Date.add(this.Period + 1, 'M').date(this.Day + 1);
          if(Date.valueOf() >= this.Tariff.EndDate) break;
          this.Tariff.Config.Billing.push(Date.startOf('day').valueOf() - 1);
        }
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Tariff.Show = true;
      },
      Add: function() {
        this.Tariff.Config.Taxs.push({Name: 'Unnamed tax', Type: 'PERCENT_TOTAL', Value: 0.0});
      },
      Delete: function(idx) {
        if(confirm("Delete '"+this.Tariff.Config.Taxs[idx].Name+"' tax?")){
          this.Tariff.Config.Taxs.splice(idx, 1);
        }
      },
      TypeChanged: function(name, idx) {
        this.Tariff.Config.Taxs[idx].Type = this.TypesEnum[this.TypeNames.indexOf(name)];
        this.Tariff = Object.assign({}, this.Tariff);
      },
      ValueChanged: function(value, idx) {
        var Value = parseFloat(value.target.value);
        if(isNaN(Value) === true) Value = 0.0;
        this.Tariff.Config.Taxs[idx].Value = Value;
        this.Tariff = Object.assign({}, this.Tariff);
      },
      NameChanged: function(value, idx) {
        this.Tariff.Config.Taxs[idx].Name = value.target.value;
        this.Tariff = Object.assign({}, this.Tariff);
      }
    },
    created: function() {
      this.TypesEnum = ['PERCENT_TOTAL', 'PERCENT_ENERGY', 'PERCENT_POWER', 'PERCENT_ENERGY_POWER', 'FIXED_QUANTITY'];
    },
    components: { OptionPicker }
  }
</script>

<style scoped>
  table, .input-group {
    width: 100%;
  }
  table tr td {
    padding: 1px 3px;
  }
  .input-group-addon {
    border-left: none;
  }
  .input-group-addon:nth-child(2) {
    width: 70px;
  }
  .dropdown-toggle {
    width: 120px;
  }
  button.delete{
    width: 25px;
  }
</style>