<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Rates &raquo; Edit &raquo; Billing</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="row">
        <div class="col-xs-12">
          <table>
            <tr><td><strong>Concept</strong></td><td><strong>Type</strong></td><td class="text-right"><strong>Value</strong></td></tr>
            <tr v-for="(tax, idx) in Rate.Config.Taxs">
              <td>
                <div class="input-group input-group-xs">
                  <span class="input-group-btn"><button class="btn btn-default delete" type="button" @click="Delete(idx)"><img :src="require('Imgs/cross.png')"></button></span>
                  <input type="text" class="form-control" @change="NameChanged($event, idx)" :value="tax.Name">
                </div>
              </td>
              <td>
            <option-picker :options="TypeNames" v-on:change="TypeChanged($event, idx)" :selected="TypeNames[TypesEnum.indexOf(tax.Type)]"></option-picker>
            </td>
            <td>
              <div class="input-group input-group-xs">
                <input type="text" class="form-control text-right" @change="ValueChanged($event, idx)" :value="Round6(tax.Value)"> 
                <span class="input-group-addon" style="width: 70px">{{ (tax.Type == 'FIXED_QUANTITY') ? '€/Bill cycle' : '%' }}</span>
              </div>
            </td>
            </tr>
            <tr>
              <td colspan="3"><button type="button" class="btn btn-default btn-xs" style="width:100%" @click="Add"><img :src="require('Imgs/plus.png')"> New concept</button></td>
            </tr>
          </table>
        </div>
        <div class="col-xs-12" style="height: 5px;"></div>
        <div class="col-xs-12">
          <table>
            <tr><td style="min-width: 120px"><strong>From</strong></td><td><strong>To</strong></td><td class="text-right" style="min-width: 120px"><strong>Duration</strong></td></tr>
            <tr v-for="(date, idx) in Rate.Config.Billing">
              <td>
                <div class="input-group input-group-xs">
                  <span class="input-group-btn"><button class="btn btn-default delete" type="button" v-on:click="DeleteBill(idx)"><img :src="require('Imgs/cross.png')"></button></span>
                  <input type="text" class="form-control" disabled="" :value="FormatDate(FromDate(idx))">
                </div>
              </td>
              <td width="100%">
            <date-picker ref="StartDate" :big="false" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: Analyzer.TimeZone }" ></date-picker>
            </td>
            <td class="text-right">
              <div class="input-group input-group-xs">
                <input type="text" class="form-control text-right" :value="CalcDays(idx)"> 
                <span class="input-group-addon">Days</span>
              </div>
            </td>
            </tr>
            <tr>
              <td>
                <div class="input-group input-group-xs">
                  <input type="text" class="form-control" disabled="" :value="FormatDate(FromDate(Rate.Config.Billing.length))">
                </div>
              </td>
              <td width="100%">
                <div class="input-group input-group-xs">
                  <input type="text" class="form-control" disabled="" :value="FormatDate(EndDate(Rate.Config.Billing.length))">
                </div>
              </td>
              <td>
                <div class="input-group input-group-xs">
                  <input type="text" class="form-control text-right" disabled="" :value="CalcDays(Rate.Config.Billing.length)">
                  <span class="input-group-addon">Days</span>
                </div>
              </td>
            </tr>
            <tr>
              <td><button type="button" class="btn btn-default btn-xs" style="width:100%" @click="AddBillDay"><img :src="require('Imgs/cross.png')"> Remove last</button></td>
              <td style="display: flex;">
            <option-picker class="flex-item" :options="BillDayNames" :selected="BillDayNames[0]" v-on:change="Day=$event"></option-picker>
            <option-picker class="flex-item" :options="PeriodNames" :selected="PeriodNames[0]" v-on:change="Period=$event"></option-picker>
            <button type="button" class="btn btn-default btn-xs flex-item" v-on:click="BuildPeriods" data-toggle="tooltip" title="Create or replace bill days according to selections"><img :src="require('Imgs/wand.png')"></button>              
            </td>
            <td>
              <button type="button" class="btn btn-default btn-xs" style="width:100%" @click="AddBillDay"><img :src="require('Imgs/plus.png')"> Add new</button></td>
            </tr>
          </table>
        </div>

      </div>
    </div>
  </div>
</template>

<script>
  
  import DatePicker from 'Components/DatePicker.vue';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data: function() {
      return { Show: false, Day: 0, Period: 0, Analyzer: null, Rate: null,
              };
    },
    methods: {
      Round6: oZm.Round6Plain,


      CalcDays: function(idx) {
        console.log("from " + this.FromDate(idx));
        console.log("to " + this.EndDate(idx).add(1, 'ms'));
        return this.EndDate(idx).add(1, 'ms').diff(this.FromDate(idx), 'days'); 
      },
      Load: function(analyzer, rate) {
        this.Analyzer = analyzer;
        this.Rate = rate;
        this.Show = true;
      },

      Back: function() {
        this.Show = false;
        this.$parent.$refs.Rate.Show = true;
      },
      Add: function() {
        this.Rate.Config.Taxs.push({Name: 'Unnamed tax', Type: 'PERCENT_TOTAL', Value: 0.0});
      },
      Delete: function(idx) {
        if(confirm("Delete '"+this.Tariff.Config.Taxs[idx].Name+"' tax?")){
          this.Rate.Config.Taxs.splice(idx, 1);
        }
      },
      TypeChanged: function(name, idx) {
        this.Rate.Config.Taxs[idx].Type = this.TypesEnum[this.TypeNames.indexOf(name)];
        this.Rate = Object.assign({}, this.Rate);
      },

      NameChanged: function(value, idx) {
        this.Rate.Config.Taxs[idx].Name = value.target.value;
        this.Rate = Object.assign({}, this.Rate);
      },
      AddBillDay: function() {
        
      }
    },

    
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
  .dropdown-toggle {
    width: 120px;
  }
  button.delete{
    width: 25px;
  }
  .flex-item {
    margin-right: 2px;
  }
  table {
    background: #fafafa;
    padding: 6px;
    border-collapse: separate;
    border-radius: 6px;
    border: 1px solid #eee;    
  }
</style>