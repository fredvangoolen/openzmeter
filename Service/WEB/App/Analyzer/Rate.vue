<template>
  <div>
  <dlg :loading="ShowSpinner" v-if="(View!=='PowerCostExpression')&&(View!=='EnergyCostExpression')">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Rates &raquo; Edit</h4></div>
      <div class="pull-right btn-group" v-if="Rate.Periods==null">
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', null)"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
      </div>
      <toolbar-pager v-if="Rate.Periods==null" class="pull-right" :pageSize="10" :input="Tariffs" :output.sync="FilterTariffs" :filtercols="['Name', 'StartDate', 'EndDate']"></toolbar-pager>
      <div class="pull-right btn-group" v-if="Rate.Periods!=null">
        <button type="button" class="btn btn-default btn-xs" @click="Save"><img :src="require('Imgs/diskette.png')" alt="Save rates config"></button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Main' }" @click="View='Main'">Main</button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Periods' }" @click="View='Periods'">Periods</button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Concepts' }" @click="View='Concepts'">Concepts</button>
        <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Cycles' }" @click="View='Cycles'">Cycles</button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')" alt="Back"></button>
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
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error!=null" v-html="Error"></div>
      <div class="row" v-if="View=='Main'">
        <div class="col-xs-8">
          <label class="control-label">Name</label>
          <input type="text" class="form-control" v-model="Rate.Name">
        </div>
        <div class="col-xs-4">
          <label class="control-label">Group:</label>
          <option-picker :big="true" v-model="Rate.Group" :options="[{name: 'Contracted', value: 0}, {name: 'Compare group 1', value: 1}, {name: 'Compare group 2', value: 2}, {name: 'Compare group 3', value: 3}, {name: 'Compare group 4', value: 4}, {name: 'Compare group 5', value: 5}, {name: 'Compare group 6', value: 6}, {name: 'Compare group 7', value: 7}, {name: 'Compare group 8', value: 8}, {name: 'Compare group 9', value: 9}, {name: 'Compare group 10', value: 10}]"></option-picker>
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
                <button type="button" class="btn btn-default btn-xs" @click="Rate.Taxes.push({Description: 'Unnamed tax', Type: 'PERCENT_TOTAL', Value: 0.0})" data-toggle="tooltip" title="Add concept"><img :src="require('Imgs/plus.png')"></button>
              </div>
              <toolbar-pager class="pull-right" :pageSize="10" :input="Rate.Taxes" :output.sync="FilterTaxes" :filtercols="['Description']"></toolbar-pager>
              <div class="clearfix"></div>
            </div>
            <div class="panel-body">
              <vuetable :fields="ColumnsTaxes" :api-mode="false" :data="FilterTaxes" class="taxes table small table-striped">
                <template slot="tax" slot-scope="props">
                  <div class="input-group input-group-xs">
                    <span class="input-group-btn">
                      <button class="btn btn-default" type="button" @click="Rate.Taxes.splice(props.rowData.rowNum, 1)"><img :src="require('Imgs/cross.png')"></button>
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
              <div class="btn-group pull-right">
                <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Add end bill cycle day" :disabled="ValidBillDate" @click="AddBillDay"><img :src="require('Imgs/plus.png')"></button>
                <button type="button" class="btn btn-default btn-xs dropdown-toggle" data-toggle="dropdown" title="Create bills on selected day of month"><img :src="require('Imgs/wand.png')"><span class="caret"></span></button>
                <ul class="dropdown-menu" style="max-height: 142px; overflow-y: auto">
                  <li v-for="(name, idx) in BillDayNames" :key="idx"><a @click="BuildPeriods(idx)">{{ name }}</a></li>
                </ul>
              </div>
              <div class="pull-right btn-group" style="width: 110px">
                <date-picker :big="false" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', minDate: Rate.StartDate, maxDate: Rate.EndDate, disabledDates: Rate.Billing.concat([ Rate.StartDate, Rate.EndDate]), date: SelDate }" @input="SelDate=TimestampToDateString($event)"></date-picker>
              </div>
              <toolbar-pager class="pull-right" style="width: 120px" :pageSize="10" :input="Cycles" :output.sync="FilterCycles" :filtercols="['From', 'To', 'Days']"></toolbar-pager>
              <div class="clearfix"></div>
            </div>
            <div class="panel-body">
              <div class="row">
                <vuetable :fields="ColumnsCycles" :api-mode="false" :data="FilterCycles" class="bills table small table-striped">
                  <template slot="actions" slot-scope="props">
                    <div class="btn-group" v-if="!props.rowData.IsLast">
                      <button class="btn btn-xs btn-danger" @click="DelBill(props.rowData)">Delete</button>
                    </div>
                  </template>
                </vuetable>
              </div>
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
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  export default {
    props: { analyzer: Object, rate: Object },
    data() {
      return { View: 'Main', Rate: null, ShowSpinner: false, FilterTariffs: [], Tariffs: [], FilterTaxes: [], FilterCycles: [], FilterCycles: [], SelDate: null, Error: null,
        BillDayNames: ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th', '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th'],
        ColumnsTariffs: [
          { name: 'Name', title: 'Name' },
          { name: 'UserName', title: 'Owner' },
          { name: '__slot:actions', title: '' }
        ],
        ColumnsTaxes: [
          { name: '__slot:tax', title: 'Concept' },
          { name: '__slot:type', title: 'Type' },
          { name: '__slot:value', title: 'Value' },
        ],
        ColumnsCycles: [
          { name: 'From', title: 'From' },
          { name: 'To', title: 'To' },
          { name: 'Days', title: 'Days' },
          { name: '__slot:actions', title: '' },
        ]
      };
    },
    created: function() {
      if(this.rate == null) {
        this.Rate = { Analyzer: this.analyzer.Analyzer, Name: 'Unname rate', Description: '', Group: 0, StartDate: moment().format('DD/MM/YYYY'), EndDate: moment().format('DD/MM/YYYY'), Module: 'NONE', ContractedPower: 0, PowerCost: '0', EnergyCost: '0', Taxes: [], Billing: [] };
      } else {
        this.Rate = Object.assign({ Analyzer: this.analyzer.Analyzer}, this.rate);
      }
      this.SelDate = this.Rate.StartDate;
      if(this.Rate.Periods == null) this.LoadTariffs();
    },
    computed: {
      Vars: function() {
        var Vars = [{ Name: 'Period', Description: 'Index of evaluated period (From 1 to ' + this.Rate.Periods.length + ')', Value: null, Units: ''}];
        for(var V = 0; V < this.analyzer.PriceModules[this.Rate.Module].length; V++) Vars.push({ Name: this.analyzer.PriceModules[this.Rate.Module][V].Name, Description: this.analyzer.PriceModules[this.Rate.Module][V].Description, Value: this.Round(this.analyzer.PriceModules[this.Rate.Module][V].Value, 4), Units: ''});
        return Vars;
      },
      Cycles: function() {
        var Bills = [];
        var Bill = { From: this.Rate.StartDate };
        for(var B = 0; B < this.Rate.Billing.length; B++) {
          Bill.To = this.Rate.Billing[B];
          Bill.Days = moment(Bill.To, 'DD/MM/YYYY').diff(moment(Bill.From, 'DD/MM/YYYY'), 'days') + 1;
          Bill.ID = B;
          Bill.IsLast = false;
          Bills.push(Bill);
          Bill = { From: moment(this.Rate.Billing[B], 'DD/MM/YYYY').add(1, 'days').format('DD/MM/YYYY')  };
        }
        Bill.To = this.Rate.EndDate;
        Bill.Days = moment(Bill.To, 'DD/MM/YYYY').diff(moment(Bill.From, 'DD/MM/YYYY'), 'days') + 1;
        Bill.IsLast = true;
        Bills.push(Bill);
        return Bills;
      },
      ValidBillDate: function() {
        var Used = this.Rate.Billing.concat([ this.Rate.StartDate, this.Rate.EndDate]);
        return Used.includes(this.SelDate);
      }
    },
    methods: {
      Save: function() {
        this.ShowSpinner = true;
        this.$http.post((this.Rate.RateID == null) ? 'addRate' : 'setRate', this.Rate).then(
          (result) => { this.$emit('close'); },
          (fail) => {
            this.ShowSpinner = false;
            this.Error = 'Failed to save rate!<br/>' + fail.body;
            setTimeout(() => { this.Error = null; }, 5000);
          }
        );
      },
      TimestampToDateString: function(val) {
        return moment(val).format('DD/MM/YYYY');
      },
      DateStringToTimestamp: function(val) {
        return moment(val, 'DD/MM/YYYY').valueOf();
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
            if(Array.isArray(Rate.PowerCost) == true) Rate.PowerCost = Rate.Periods.map((val) => { return val.DefaultPowerCost; });
            if(Array.isArray(Rate.EnergyCost) == true) Rate.EnergyCost = Rate.Periods.map((val) => { return val.DefaultEnergyCost; });
            this.Rate = Rate;
            this.ShowSpinner = false;
          },
          (fail) => { }
        );
      },
      DelBill: function(day) {
        if(confirm("Delete day '" + day.To + "' ?")) this.Rate.Billing.splice(day.ID, 1);
      },
      AddBillDay: function() {
        this.Rate.Billing.push(this.SelDate);
        this.Rate.Billing.sort((a, b) => {
          var A = this.DateStringToTimestamp(a);
          var B = this.DateStringToTimestamp(b);
          if(A > B) return 1;
          if(A < B) return -1;
          return 0;
        });
      },
      BuildPeriods: function(day) {
        var Bills = [];
        var Date = moment(this.Rate.StartDate, 'DD/MM/YYYY');
        var From = moment(this.Rate.StartDate, 'DD/MM/YYYY');
        var To   = moment(this.Rate.EndDate, 'DD/MM/YYYY');
        for(var M = 0; ; M++) {
          Date = moment(From).add(M, 'M').date(day);
          if(Date.isSameOrAfter(To)) break;
          if(Date.isAfter(From)) Bills.push(Date.format('DD/MM/YYYY'));
        }
        this.$set(this.Rate, 'Billing', Bills);
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
  .taxes.table >>> th:nth-child(3) {
    width: 200px;
  }
  table tr td {
    padding: 2px 0px;
  }
  .bills.table >>> th:nth-child(4) {
    width: 62px;
  }
  .bills.table >>> td:nth-child(4) {
    text-align: right;
  }
</style>