<template>
  <div>
  <dlg :loading="ShowSpinner" v-if="(View!=='PowerCostExpression')&&(View!=='EnergyCostExpression')">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Rates &raquo; Edit</h4></div>
      <div class="pull-right btn-group" v-if="Rate.Periods==null">
        <button type="button" class="btn btn-default btn-xs" @click="$emit('endEdit', index, Rate)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
      <toolbar-pager v-if="Rate.Periods==null" class="pull-right" :pageSize="10" :input="Tariffs" :output.sync="FilterTariffs" :filtercols="['Name', 'StartDate', 'EndDate']"></toolbar-pager>
      <div class="pull-right btn-group" v-if="Rate.Periods!=null">
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Main' }" @click="View='Main'">Main</button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Periods' }" @click="View='Periods'">Periods</button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Concepts' }" @click="View='Concepts'">Concepts</button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Cycles' }" @click="View='Cycles'">Cycles</button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('endEdit', index, Rate)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
    </template>
    <div class="row" v-if="Rate.Periods==null">
      <spinner v-if="ShowSpinner"></spinner>
      <vuetable :fields="ColumnsTariffs" :api-mode="false" :data="FilterTariffs" class="tariffs table small table-striped">
        <template slot="actions" slot-scope="props">
          <div class="btn-group" >
            <button class="btn btn-xs btn-default" @click="LoadTariff(props.rowData)">Load</button>
          </div>
        </template>
      </vuetable>
    </div>
    <div v-if="Rate.Periods!=null">
      <div class="row" v-if="View=='Main'">
        <div class="col-xs-12">
          <label class="control-label">Name</label>
          <input type="text" class="form-control" v-model="Rate.Name">
        </div>
        <div class="col-xs-12">
          <label class="control-label">Description</label>
          <textarea class="form-control" rows="5" v-model="Rate.Description" style="resize: none"></textarea>
        </div>
        <div class="col-xs-4">
          <label class="control-label">From</label>
          <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" :big="true" :value="DateStringToTimestamp(Rate.StartDate)" @input="Rate.StartDate=TimestampToDateString($event)"></date-picker>
        </div>
        <div class="col-xs-4">
          <label class="control-label">To</label>
          <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" :big="true" :value="DateStringToTimestamp(Rate.EndDate)" @input="Rate.EndDate=TimestampToDateString($event)"></date-picker>
        </div>
        <div class="col-xs-4">
          <label class="control-label">Price source</label>
          <option-picker :options="[ {name: 'NONE', value: 'NONE'}, {name: 'ESP - ESIOS', value: 'ESP_ESIOS'} ]" v-model="Rate.Module" :big="true"></option-picker>
        </div>
        <div class="col-xs-12"><hr style="margin-top: 10px; margin-bottom: 5px;"/></div>
        <div class="col-xs-12" style="margin-top: 5px">
          <button type="button" class="btn btn-default btn-xs btn-danger" style="width:100%" @click="LoadTariffs()">Change tariff template</button>
        </div>
      </div>
      <div class="row" v-if="View=='Periods'">
        <div class="col-xs-12">
          <div class="panel panel-dialog" style="margin-bottom: 6px">
            <div class="panel-heading">
              Contracted Power
              <div class="pull-right btn-group">
                <button type="button" class="btn btn-default btn-xs" :class="{ active: !Array.isArray(Rate.ContractedPower) }" @click="Rate.ContractedPower=Array.isArray(Rate.ContractedPower) ? Rate.ContractedPower[0] : Array(Rate.Periods.length).fill(Rate.ContractedPower)" data-toggle="tooltip" title="Link all periods value"><img :src="require('Imgs/link.png')"></button>
              </div>
              <div class="clearfix"></div>
            </div>
            <div class="panel-body" v-if="!Array.isArray(Rate.ContractedPower)">
              <input-spin :max="100000" :min="0" :prepend="Rate.PowerUnits" :decimals="6" :increment="0.1" v-model="Rate.ContractedPower"></input-spin>
            </div>
            <div class="panel-body" v-if="Array.isArray(Rate.ContractedPower)">
              <table style="width:100%">
                <tr v-for="(period, idx) in Rate.ContractedPower" :key="idx">
                  <td class="text-center small" style="width:30px"><strong>P{{ idx + 1 }}</strong></td>
                  <td>
                    <input-spin :max="100000" :min="0" :prepend="Rate.PowerUnits" :decimals="6" :increment="0.1" v-model="Rate.ContractedPower[idx]"></input-spin>
                  </td>
                </tr>
              </table>
            </div>
          </div>
          <div class="panel panel-dialog" style="margin-bottom: 6px">
            <div class="panel-heading">
              Power Cost
              <div class="pull-right btn-group">
                <button type="button" class="btn btn-default btn-xs" :class="{ active: (typeof(Rate.PowerCost) != 'string') }" @click="Rate.PowerCost=Rate.Periods.map((val) => { return val.DefaultPowerCost;})" data-toggle="tooltip" title="Power cost is a fixed value">Fixed</button>
                <button type="button" class="btn btn-default btn-xs" :class="{ active: (typeof(Rate.PowerCost) == 'string') }" @click="Rate.PowerCost='0'" data-toggle="tooltip" title="Power cost computed from avaliable variables">Computed</button>
              </div>
              <div class="clearfix"></div>
            </div>
            <div class="panel-body" v-if="Array.isArray(Rate.PowerCost)">
              <table style="width:100%">
                <tr v-for="(period, idx) in Rate.PowerCost" :key="idx">
                  <td class="text-center small" style="width:30px"><strong>P{{ idx + 1 }}</strong></td>
                  <td>
                    <input-spin :max="100000" :min="0" :prepend="Rate.Currency + '/' + Rate.PowerUnits + '/day'" :decimals="6" :increment="0.1" v-model="Rate.PowerCost[idx]"></input-spin>
                  </td>
                  <td style="width:30px">
                    <div class="btn-group">
                      <button type="button" class="btn btn-default btn-xs" @click="$set(Rate.PowerCost, idx, Rate.Periods[idx].DefaultPowerCost)" data-toggle="tooltip" title="Set tariff default costs"><img :src="require('Imgs/wand.png')"></button>
                    </div>
                  </td>
                </tr>
              </table>
            </div>
            <div class="panel-body" v-if="!Array.isArray(Rate.PowerCost)">
              <div class="input-group input-group-xs">
                <input type="text" class="form-control" value="Configured from expression" :disabled="true">
                <span class="input-group-btn">
                  <button type="button" class="btn btn-default btn-xs" @click="View='PowerCostExpression'" data-toggle="tooltip" title="Set power costs expression"><img :src="require('Imgs/math_functions.png')"></button>
                </span>
              </div>
            </div>
          </div>
          <div class="panel panel-dialog" style="margin-bottom: 0px">
            <div class="panel-heading">
              Energy Cost
              <div class="pull-right btn-group">
                <button type="button" class="btn btn-default btn-xs" :class="{ active: (typeof(Rate.EnergyCost) != 'string') }" @click="Rate.EnergyCost=Rate.Periods.map((val) => { return val.DefaultEnergyCost;})" data-toggle="tooltip" title="Energy cost is a fixed value">Fixed</button>
                <button type="button" class="btn btn-default btn-xs" :class="{ active: (typeof(Rate.EnergyCost) == 'string') }" @click="Rate.EnergyCost='0'" data-toggle="tooltip" title="Energy cost computed from avaliable variables">Computed</button>
              </div>
              <div class="clearfix"></div>
            </div>
            <div class="panel-body" v-if="Array.isArray(Rate.EnergyCost)">
              <table style="width:100%">
                <tr v-for="(period, idx) in Rate.EnergyCost" :key="idx">
                  <td class="text-center small" style="width:30px"><strong>P{{ idx + 1 }}</strong></td>
                  <td>
                    <input-spin :max="100000" :min="0" :prepend="Rate.Currency + '/' + Rate.EnergyUnits" :decimals="6" :increment="0.1" v-model="Rate.EnergyCost[idx]"></input-spin>
                  </td>
                  <td style="width:30px">
                    <div class="btn-group">
                      <button type="button" class="btn btn-default btn-xs" @click="Rate.EnergyCost[idx]=Rate.Periods[idx].DefaultEnergyCost" data-toggle="tooltip" title="Set tariff default costs"><img :src="require('Imgs/wand.png')"></button>
                    </div>
                  </td>
                </tr>
              </table>
            </div>
            <div class="panel-body" v-if="!Array.isArray(Rate.EnergyCost)">
              <div class="input-group input-group-xs">
                <input type="text" class="form-control" value="Configured from expression" :disabled="true">
                <span class="input-group-btn">
                  <button type="button" class="btn btn-default btn-xs" @click="View='EnergyCostExpression'" data-toggle="tooltip" title="Set energy costs expression"><img :src="require('Imgs/math_functions.png')"></button>
                </span>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="row" v-if="View=='Concepts'">
        <div class="col-xs-12">
          <div class="panel panel-dialog" style="margin-bottom: 0px">
            <div class="panel-heading">
              <div class="pull-right btn-group">
                <button type="button" class="btn btn-default btn-xs" @click="Rate.Taxs.push({Description: 'Unnamed tax', Type: 'PERCENT_TOTAL', Value: 0.0})" data-toggle="tooltip" title="Add concept"><img :src="require('Imgs/plus.png')"></button>
              </div>
              <toolbar-pager class="pull-right" :pageSize="10" :input="Rate.Taxs" :output.sync="FilterTaxs" :filtercols="['Description']"></toolbar-pager>
              <div class="clearfix"></div>
            </div>
            <div class="panel-body">
              <vuetable :fields="ColumnsTaxs" :api-mode="false" :data="FilterTaxs" class="taxs table small table-striped">
                <template slot="tax" slot-scope="props">
                  <div class="input-group input-group-xs">
                    <span class="input-group-btn">
                      <button class="btn btn-default" type="button" @click="Rate.Taxs.splice(props.rowData.rowNum, 1)"><img :src="require('Imgs/cross.png')"></button>
                    </span>
                    <input type="text" class="form-control" v-model="props.rowData.Description">
                  </div>
                </template>
                <template slot="type" slot-scope="props">
                  <option-picker :options="[ {name: 'Total', value: 'PERCENT_TOTAL'}, {name: 'Energy', value: 'PERCENT_ENERGY'}, {name: 'Power', value: 'PERCENT_POWER'}, {name: 'Energy+Power', value: 'PERCENT_ENERGY_POWER'}, {name: 'Fixed', value: 'FIXED_QUANTITY'} ]" v-model="props.rowData.Type"></option-picker>
                </template>
                <template slot="value" slot-scope="props">
                  <input-spin :max="100000" :min="-100000" :prepend="(props.rowData.Type == 'FIXED_QUANTITY') ? '€/Bill cycle' : '%'" :decimals="6" :increment="1" v-model="props.rowData.Value"></input-spin>
                </template>
              </vuetable>
            </div>
          </div>
        </div>
      </div>
      <div class="row" v-if="View=='Cycles'">
        <div class="col-xs-12">
          <div class="panel panel-dialog" style="margin-bottom: 0px">
            <div class="panel-heading">
              <div class="pull-left">
                <option-picker style="width: 64px; display:inline-block;" :options="BillDayNames.map((val, idx) => { return { name: val, value: idx }; })" v-model="Day"></option-picker>
                <option-picker style="width: 140px; display:inline-block;" :options="PeriodNames.map((val, idx) => { return { name: val, value: idx }; })" v-model="Period"></option-picker>
                <div class="btn-group">
                  <button type="button" class="btn btn-default btn-xs flex-item" @click="BuildPeriods" data-toggle="tooltip" title="Create or replace bill days according to selections"><img :src="require('Imgs/wand.png')"></button>
                </div>
              </div>
              <div class="pull-right btn-group">
                <button type="button" class="btn btn-default btn-xs" @click="Rate.Config.Taxs.push({Description: 'Unnamed tax', Type: 'PERCENT_TOTAL', Value: 0.0})" data-toggle="tooltip" title="Add concept"><img :src="require('Imgs/plus.png')"></button>
              </div>
              <toolbar-pager class="pull-right" :pageSize="10" :input="Rate.Config.Taxs" :output.sync="FilterTaxs" :filtercols="['Description']"></toolbar-pager>
              <div class="clearfix"></div>
            </div>
            <div class="panel-body">
              <vuetable :fields="ColumnsCycles" :api-mode="false" :data="FilterCycles" class="taxs table small table-striped">
                <template slot="from" slot-scope="props">
                  <div class="input-group input-group-xs">
                    <span class="input-group-btn">
                      <button class="btn btn-default" type="button" @click="Rate.Config.Taxs.splice(props.rowData.rowNum, 1)"><img :src="require('Imgs/cross.png')"></button>
                    </span>
                    <input type="text" class="form-control" v-model="props.rowData.Description">
                  </div>
                </template>
                <template slot="to" slot-scope="props">
                  <option-picker :options="[ {name: 'Total', value: 'PERCENT_TOTAL'}, {name: 'Energy', value: 'PERCENT_ENERGY'}, {name: 'Power', value: 'PERCENT_POWER'}, {name: 'Energy+Power', value: 'PERCENT_ENERGY_POWER'}, {name: 'Fixed', value: 'FIXED_QUANTITY'} ]" v-model="props.rowData.Type"></option-picker>
                </template>
                <template slot="days" slot-scope="props">
                  <input-spin :max="100000" :min="-100000" :prepend="(props.rowData.Type == 'FIXED_QUANTITY') ? '€/Bill cycle' : '%'" :decimals="6" :increment="1" v-model="props.rowData.Value"></input-spin>
                </template>
              </vuetable>




          <table>
            <tr>
              <td style="min-width: 120px"><strong>From</strong></td>
              <td style="width: 100%"><strong>To</strong></td>
              <td class="text-right" style="min-width: 120px"><strong>Duration</strong></td>
            </tr>
            <tr v-for="(date, idx) in Rate.Config.Billing.concat([Rate.EndDate])" :key="idx">
              <td>
                <div class="input-group input-group-xs">
                  <span class="input-group-btn" v-if="idx!=Rate.Config.Billing.length">
                    <button class="btn btn-default" type="button" @click="Rate.Config.Billing.splice(idx, 1)"><img :src="require('Imgs/cross.png')"></button>
                  </span>
                  <input type="text" class="form-control" disabled="" :value="FormatDate(StartDate(idx))">
                </div>
              </td>
              <td>
                <date-picker :big="false" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" :value="EndDate(idx)" @input="SetBillPeriods(idx, $event)" :disabled="idx==Rate.Config.Billing.length"></date-picker>
              </td>
              <td class="text-right">
                <div class="input-group input-group-xs">
                  <input type="text" class="form-control text-right" disabled="" :value="CalcDays(idx)">
                  <span class="input-group-addon">Days</span>
                </div>
              </td>
            </tr>
          </table>





            </div>
          </div>
        </div>
      </div>
    </div>
  </dlg>
    <dlg v-if="View=='PowerCostExpression'">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Power cost expression</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="View='Periods'"><img :src="require('Imgs/arrow_undo.png')" alt="Go back"></button>
        </div>
      </template>
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label">Expresion</label>
          <input-math v-model="Rate.PowerCost" :vars="Vars"></input-math>
        </div>
      </div>
    </dlg>
    <dlg v-if="View=='EnergyCostExpression'">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Energy cost expression</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="View='Periods'"><img :src="require('Imgs/arrow_undo.png')" alt="Go back"></button>
        </div>
      </template>
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label">Expresion</label>
          <input-math v-model="Rate.EnergyCost" :vars="Vars"></input-math>
        </div>
      </div>
    </dlg>
  </div>
</template>

<script>
  import vuetable from 'vuetable-2';
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object, index: Number },
    data() {
      return { View: 'Main', Rate: null, Day: 0, Period: 0, ShowSpinner: false, FilterTariffs: [], Tariffs: [], FilterTaxs: [], FilterCycles: [],
        PeriodNames: ['Each month', 'Each two month', 'Each three month'],
        BillDayNames: ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th', '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th'],
        ColumnsTariffs: [
          { name: 'Name', title: 'Name' },
          { name: 'UserName', title: 'Owner' },
          { name: '__slot:actions', title: '' }
        ],
        ColumnsTaxs: [
          { name: '__slot:tax', title: 'Concept' },
          { name: '__slot:type', title: 'Type' },
          { name: '__slot:value', title: 'Value' },
        ],
        ColumnsCycles: [
          { name: '__slot:from', title: 'From' },
          { name: '__slot:to', title: 'To' },
          { name: '__slot:days', title: 'Days' },
        ]
      };
    },
    created: function() {
      this.Rate = this.analyzer.Rates[this.index];
      if(this.Rate.Periods == null) this.LoadTariffs();
    },
    computed: {
      Vars: function() {
        var Vars = [];
        Vars.push({ Name: 'Period', Description: 'Index of evaluated period (From 1 to ' + this.Rate.Periods.length + ')', Value: null, Units: ''});
        for(var V = 0; V < this.analyzer.RateModules[this.Rate.Module].length; V++) Vars.push({ Name: this.analyzer.RateModules[this.Rate.Module][V].Name, Description: this.analyzer.RateModules[this.Rate.Module][V].Description, Value: null, Units: ''});
        return Vars;
      },
    },
    methods: {
      TimestampToDateString: function(val) {
        return moment(val).format('DD/MM/YYYY');
      },
      DateStringToTimestamp: function(val) {
        return moment(val, 'DD/MM/YYYY').valueOf();
      },
      FormatDate: function(val) {
        return moment(val).tz(this.analyzer.TimeZone).format('DD/MM/YYYY');
      },
      LoadTariffs: function() {
        this.ShowSpinner = true;
        this.Rate.Periods = null;
        this.$http.post('getTariffs', {}).then(
          (result) => {
            this.Tariffs = result.body;
            this.ShowSpinner = false;
          },
          (fail) => { }
        );
      },
      LoadTariff: function(tariff) {
        this.ShowSpinner = true;
        this.$http.post('getTariff', { Name: tariff.Name }).then(
          (result) => {
            var Rate = Object.assign(this.Rate, result.body.Template);
            Rate.Name = result.body.Name;
            if(typeof(Rate.PowerCost) == 'string') Rate.PowerCost = Rate.Periods.map((val) => { return val.DefaultPowerCost; });
            if(typeof(Rate.EnergyCost) == 'string') Rate.EnergyCost = Rate.Periods.map((val) => { return val.DefaultEnergyCost; });
            this.Rate = Rate;
            this.ShowSpinner = false;
          },
          (fail) => { }
        );
      },






//      Show: function(analyzer, tariffs, rate) {
//        this.Analyzer = analyzer;
//        this.Tariffs = tariffs;
//        for(var n = 0; n < this.Tariffs.length; n++) {
//          this.Tariffs[n].id = this.Tariffs[n].TariffID;
//          this.Tariffs[n].name = this.Tariffs[n].Name;
//          this.Tariffs[n].description = 'From ' + moment.tz(this.Tariffs[n].StartDate, this.analyzer.TimeZone).format('DD/MM/YYYY') + ' to ' + moment.tz(this.Tariffs[n].EndDate, this.analyzer.TimeZone).format('DD/MM/YYYY');
//        }
//        this.Initial = rate;
//        this.View = 'Main';
//        if(rate === null) {
//          this.Rate = this.EmptyRate;
//        } else {
//          this.Rate = Object.assign({}, rate);
//        }
//        this.$refs.Dialog.Show();
//        this.$nextTick(() => {
//          if(this.Rate.RateID !== undefined) {
//            this.Tariffs.unshift({ id: 'CURRENT', name: '(*) ' + this.Rate.Name, description: 'From ' + moment.tz(this.Rate.StartDate, this.analyzer.TimeZone).format('DD/MM/YYYY') + ' to ' + moment.tz(this.Rate.EndDate, this.analyzer.TimeZone).format('DD/MM/YYYY')});
//          } else {
//            $(this.$refs.Base).attr('data-id', '');
//            $(this.$refs.Base).val('');
//            this.$refs.StartDate.Disable();
//            this.$refs.EndDate.Disable();
//          }
//          this.SetTariffs();
//        });
//      },
//      Round6: function(val) {
//        return oZm.RoundPlain(val, 6);
//      },

      SetBillPeriods: function(idx, val) {
        var Date = moment(val).tz(this.analyzer.TimeZone).endOf('day').valueOf();
        if(this.Rate.Config.Billing[idx] == Date) return;
        this.Rate.Config.Billing[idx] = Date;
        this.Rate.Config.Billing = this.Rate.Config.Billing.sort();
      },
      StartDate: function(idx) {
        if(idx == 0)
          return moment.tz(this.Rate.StartDate, this.analyzer.TimeZone).valueOf();
        else if((idx > 0) && (idx <= this.Rate.Config.Billing.length))
          return moment.tz(this.Rate.Config.Billing[idx - 1] + 1, this.analyzer.TimeZone).valueOf();
      },
      EndDate: function(idx) {
        if((idx >= 0) && (idx < this.Rate.Config.Billing.length))
          return moment.tz(this.Rate.Config.Billing[idx], this.analyzer.TimeZone).valueOf();
        else
          return moment.tz(this.Rate.EndDate, this.analyzer.TimeZone).valueOf();
      },
      CalcDays: function(idx) {
        var To = moment(this.EndDate(idx)).add(1, 'ms');
        var From = moment(this.StartDate(idx));
        return To.diff(From, 'days');
      },
//      SetStartDate: function(date) {
//        this.Rate.StartDate = date.startOf('day').valueOf();
//        this.Validate();
//      },
//      SetEndDate: function(date) {
//        this.Rate.EndDate = date.endOf('day').valueOf();
//        this.Validate();
//      },
//      Back: function() {
//        this.$refs.Dialog.Close();
//        this.$parent.$refs.Rates.Show(this.analyzer);
//      },
//      PeriodItemChanged: function(value, type, idx) {
//        var Value = parseFloat(value.target.value);
//        if(isNaN(Value) === true) Value = 0.0;
//        if(type === 'Contracted_Power')
//          Value = Value * 1000;
//        else
//          Value = Value / 1000;
//        this.Rate.Config.Periods[idx][type] = Value;
//        this.Rate = Object.assign({}, this.Rate);
//      },
//      ValueConceptChanged: function(value, idx) {
//        var Value = parseFloat(value.target.value);
//        if(isNaN(Value) === true) Value = 0.0;
//        this.Rate.Config.Taxs[idx].Value = Value;
        //this.Rate = Object.assign({}, this.Tariff);
//      },
      AddBillDay: function() {
        this.Rate.Config.Billing.push(this.Rate.EndDate);
      },
      BuildPeriods: function() {
        this.$set(this.Rate.Config, 'Billing', []);
        var Date = moment.tz(this.Rate.StartDate, this.analyzer.TimeZone);
        while(true) {
          Date = Date.add(this.Period + 1, 'M').date(this.Day + 1);
          if(Date.valueOf() >= this.Rate.EndDate) break;
          this.Rate.Config.Billing.push(Date.startOf('day').valueOf() - 1);
        }
      }
    },
    components: { vuetable }
  }
</script>

<style scoped>
  .tariffs.table >>> th:nth-child(2) {
    width: 120px;
    text-align: center;
  }
  .tariffs.table >>> td:nth-child(2) {
    text-align: center;
  }
  .tariffs.table >>> th:nth-child(3) {
    width: 54px;
  }
  .tariffs.table >>> td:nth-child(3) {
    text-align: right;
  }
  .taxs.table >>> th:nth-child(3) {
    width: 200px;
  }
  table tr td {
    padding: 2px 0px;
  }

</style>