<template>
  <dlg :loading="ShowSpinner">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Analyzers &raquo; Rates &raquo; Edit</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ disabled: (Error != '') }" v-on:click="Save"><img :src="require('Imgs/diskette.png')" alt="Save rate"></button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: View=='Main' }" v-on:click="View='Main'">Main</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: View=='Periods' }" v-on:click="View='Periods'">Periods</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: View=='Concepts' }" v-on:click="View='Concepts'">Concepts</button>
        <button type="button" class="btn btn-default btn-xs" v-bind:class="{ active: View=='Cycles' }" v-on:click="View='Cycles'">Cycles</button>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', null)"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </template>
    <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != ''">{{ Error }}</div>
    <div class="row" v-if="View=='Main'">
      <div class="alert alert-success text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error == ''">{{ Rate.Template.Description }}<br><strong>Tariff contains {{ Rate.Template.Seasons.length }}  seasons, a total of {{ Rate.Template.Periods.length }}  day periods and {{ Rate.Template.Holidays.length }} holidays<br>Rate contains {{ Rate.Config.Taxs.length }}  concepts and {{ Rate.Config.Billing.length + 1 }} bills</strong></div>
      <div class="col-xs-12">
        <label class="control-label">Base Tariff</label>
        <div class="input-group">
          <input type="text" class="form-control" ref="Base">
          <div class="input-group-btn">
            <button type="button" class="btn btn-default dropdown-toggle" data-toggle=""><span class="caret"></span></button>
            <ul class="dropdown-menu dropdown-menu-right" role="menu"></ul>
          </div>
        </div>
      </div>
      <div class="col-xs-12 col-sm-6"><label class="control-label">From</label><date-picker ref="StartDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-on:change="SetStartDate"></date-picker></div>
      <div class="col-xs-12 col-sm-6"><label class="control-label">To</label><date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" v-on:change="SetEndDate"></date-picker></div>
    </div>
    <div class="row periods" v-if="View=='Periods'">
      <div class="col-xs-12">
        <table>
          <tr><td></td><td><strong>Contracted power <em>(kW)</em></strong></td><td><strong>Power costs <em>(€/kW/day)</em></strong></td><td><strong>Energy costs <em>(€/kWh)</em></strong></td></tr>
          <tr v-for="(period, idx) in rate.Config.Periods">
            <td><strong>P{{idx + 1}}</strong></td>
            <td><div class="input-group input-group-xs"><input type="text" class="form-control" v-on:change="PeriodItemChanged($event, 'Contracted_Power', idx)" :value="Round6(period.Contracted_Power / 1000)"></div></td>
            <td><div class="input-group input-group-xs"><input type="text" class="form-control" v-on:change="PeriodItemChanged($event, 'Power_Cost', idx)" :value="Round6(period.Power_Cost * 1000)"></div></td>
            <td><div class="input-group input-group-xs"><input type="text" class="form-control" v-on:change="PeriodItemChanged($event, 'Energy_Cost', idx)" :value="Round6(period.Energy_Cost * 1000)"></div></td>
          </tr>
        </table>
      </div>
    </div>
    <div class="row" v-if="View=='Concepts'">
      <div class="col-xs-12">
        <table>
          <tr><td><strong>Concept</strong></td><td><strong>Type</strong></td><td class="text-right"><strong>Value</strong></td></tr>
          <tr v-for="(tax, idx) in rate.Config.Taxs">
            <td>
              <div class="input-group input-group-xs">
                <span class="input-group-btn"><button class="btn btn-default" type="button" v-on:click="Rate.Config.Taxs.splice(idx, 1)"><img :src="require('Imgs/cross.png')"></button></span>
                <input type="text" class="form-control" v-on:change="tax.Name=$event.target.value" :value="tax.Name">
              </div>
            </td>
            <td>
              <option-picker :options="[ {name: 'Total', value: 'PERCENT_TOTAL'}, {name: 'Energy', value: 'PERCENT_ENERGY'}, {name: 'Power', value: 'PERCENT_POWER'}, {name: 'Energy+Power', value: 'PERCENT_ENERGY_POWER'}, {name: 'Fixed', value: 'FIXED_QUANTITY'} ]" v-model="tax.Type"></option-picker>
            </td>
            <td>
              <div class="input-group input-group-xs">
                <input type="text" class="form-control text-right" v-on:change="ValueConceptChanged($event, idx)" :value="Round6(tax.Value)">
                <span class="input-group-addon" style="width: 70px">{{ (tax.Type == 'FIXED_QUANTITY') ? '€/Bill cycle' : '%' }}</span>
              </div>
            </td>
          </tr>
          <tr>
            <td colspan="3"><button type="button" class="btn btn-default btn-xs" style="width:100%" v-on:click="Rate.Config.Taxs.push({Name: 'Unnamed tax', Type: 'PERCENT_TOTAL', Value: 0.0})"><img :src="require('Imgs/plus.png')"> New concept</button></td>
          </tr>
        </table>
      </div>
    </div>
    <div class="row" v-if="View=='Cycles'">
      <div class="col-xs-12">
        <table>
          <tr><td style="min-width: 120px"><strong>From</strong></td><td style="width: 100%"><strong>To</strong></td><td class="text-right" style="min-width: 120px"><strong>Duration</strong></td></tr>
          <tr v-for="(date, idx) in Rate.Config.Billing.concat([Rate.EndDate])">
            <td>
              <div class="input-group input-group-xs">
                <span class="input-group-btn" v-if="idx!=Rate.Config.Billing.length"><button class="btn btn-default" type="button" v-on:click="Rate.Config.Billing.splice(idx, 1)"><img :src="require('Imgs/cross.png')"></button></span>
                <input type="text" class="form-control" disabled="" :value="FormatDate(StartDate(idx))">
              </div>
            </td>
            <td>
              <date-picker :big="false" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: analyzer.TimeZone }" :date="EndDate(idx)" v-on:change="SetBillPeriods(idx, $event)" :disabled="idx==Rate.Config.Billing.length"></date-picker>
            </td>
            <td class="text-right">
              <div class="input-group input-group-xs">
                <input type="text" class="form-control text-right" disabled="" :value="CalcDays(idx)">
                <span class="input-group-addon">Days</span>
              </div>
            </td>
          </tr>
          <tr>
            <td><button type="button" class="btn btn-default btn-xs" style="width:100%" v-on:click="AddBillDay"><img :src="require('Imgs/plus.png')"> New bill</button></td>
            <td style="display: flex">
              <option-picker class="flex-item" :options="BillDayNames" :selected="BillDayNames[0]" v-on:change="Day=$event"></option-picker>&nbsp;
              <option-picker class="flex-item" :options="PeriodNames" :selected="PeriodNames[0]" v-on:change="Period=PeriodNames.indexOf($event)"></option-picker>&nbsp;
              <button type="button" class="btn btn-default btn-xs flex-item" v-on:click="BuildPeriods" data-toggle="tooltip" title="Create or replace bill days according to selections"><img :src="require('Imgs/wand.png')"></button>
            </td>
            <td></td>
          </tr>
        </table>
      </div>
    </div>
  </dlg>
</template>

<script>
  import bsSuggest from 'JS/bootstrap-suggest.js';
  import OptionPicker from 'Components/OptionPicker.vue';
  import moment from 'moment';
  import DatePicker from 'Components/DatePicker.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: ['analyzer', 'tariffs', 'value'],
    data() {
      return { Error: '', View: 'Main', Day: 0, Period: 0, ShowSpinner: false,
        PeriodNames: ['Each month', 'Each two month', 'Each three month'],
        BillDayNames: ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th', '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th']
      };
    },
    created: function() {
      this.EmptyRate = { StartDate: 0, EndDate: 0, Template: {Description: '', Seasons: [], Periods: [], Holidays: []}, Config: {Periods: [], Taxs: [], Billing: []} };
      this.Rate = this.EmptyRate;
    },
    methods: {
      Show: function(analyzer, tariffs, rate) {
        this.Analyzer = analyzer;
        this.Tariffs = tariffs;
        for(var n = 0; n < this.Tariffs.length; n++) {
          this.Tariffs[n].id = this.Tariffs[n].TariffID;
          this.Tariffs[n].name = this.Tariffs[n].Name;
          this.Tariffs[n].description = 'From ' + moment.tz(this.Tariffs[n].StartDate, this.analyzer.TimeZone).format('DD/MM/YYYY') + ' to ' + moment.tz(this.Tariffs[n].EndDate, this.analyzer.TimeZone).format('DD/MM/YYYY');
        }
        this.Initial = rate;
        this.View = 'Main';
        if(rate === null) {
          this.Rate = this.EmptyRate;
        } else {
          this.Rate = Object.assign({}, rate);
        }
        this.$refs.Dialog.Show();
        this.$nextTick(() => {
          if(this.Rate.RateID !== undefined) {
            this.Tariffs.unshift({ id: 'CURRENT', name: '(*) ' + this.Rate.Name, description: 'From ' + moment.tz(this.Rate.StartDate, this.analyzer.TimeZone).format('DD/MM/YYYY') + ' to ' + moment.tz(this.Rate.EndDate, this.analyzer.TimeZone).format('DD/MM/YYYY')});
          } else {
            $(this.$refs.Base).attr('data-id', '');
            $(this.$refs.Base).val('');
            this.$refs.StartDate.Disable();
            this.$refs.EndDate.Disable();
          }
          this.SetTariffs();
        });
      },
      Round6: function(val) {
        return oZm.RoundPlain(val, 6);
      },
      FormatDate: function(val) {
        return val.format('DD/MM/YYYY');
      },
      SetBillPeriods: function(idx, val) {
        if(this.Rate.Config.Billing[idx] === val.endOf('day').valueOf()) return;
        this.Rate.Config.Billing[idx] = val.endOf('day').valueOf();
        this.$nextTick(() => {
          this.Rate.Config.Billing.sort();
        });
      },
      StartDate: function(idx) {
        var Date;
        if(idx <= 0)
          Date = moment.tz(this.Rate.StartDate, this.analyzer.TimeZone);
        else if((idx > 0) && (idx <= this.Rate.Config.Billing.length))
          Date = moment.tz(this.Rate.Config.Billing[idx - 1] + 1, this.analyzer.TimeZone);
        else
          Date = moment.tz(this.Rate.EndDate + 1, this.analyzer.TimeZone);
        console.log("Start" + idx, Date);
        return Date;
      },
      EndDate: function(idx) {
        var Date;
        if(idx < 0)
          Date = moment.tz(this.Rate.StartDate - 1, this.analyzer.TimeZone);
        else if((idx >= 0) && (idx < this.Rate.Config.Billing.length))
          Date = moment.tz(this.Rate.Config.Billing[idx], this.analyzer.TimeZone);
        else
          Date = moment.tz(this.Rate.EndDate, this.analyzer.TimeZone);
        console.log("end"+idx,Date);
        return Date;
      },
      CalcDays: function(idx) {
        return this.EndDate(idx).add(1, 'ms').diff(this.StartDate(idx), 'days');
      },
      LoadCurrent: function() {
        $(this.$refs.Base).attr('data-id', 'CURRENT');
        $(this.$refs.Base).val(this.Tariffs[0].name);
        this.Rate.Template = Object.assign({}, this.Initial.Template);
        this.FillPeriods();
        this.Rate.Name = this.Initial.Name;
      },
      SetTariffs: function() {
        $(this.$refs.Base).bsSuggest("destroy");
        $(this.$refs.Base).bsSuggest( { hideOnSelect: true, effectiveFields: ['name', 'description'], showHeader: false, ignorecase: true, indexId: 0, indexKey: 1, data: { value: this.Tariffs } } );
        $(this.$refs.Base).on('onSetSelectValue', this.SelectBase.bind(this));
        $(this.$refs.Base).on('onUnsetSelectValue', this.DeselectBase.bind(this));
        if(this.Initial !== null) this.LoadCurrent();
        this.$refs.StartDate.SetDate(moment(this.Rate.StartDate));
        this.$refs.EndDate.SetDate(moment(this.Rate.EndDate));
        this.Validate();
        this.ShowSpinner = false;
      },
      Save: function() {
        this.$refs.Dialog.ShowSpinner = true;
        $.ajax({ url: (this.Rate.RateID === undefined) ? "addRate" : "setRate", timeout: 20000, type: "POST", dataType: "text", contentType: 'application/json', data: JSON.stringify( Object.assign({ Analyzer: this.analyzer.Analyzer }, this.Rate) ),
          success: () => {
            this.$refs.Dialog.ShowSpinner = false;
            this.Back();
          },
          error: () => {
            this.$refs.Dialog.ShowSpinner = false;
            this.Error = 'Failed to save rate to analyzer!';
            setTimeout(() => { this.Error = ''; }, 1000);
          }
        });
      },
      Validate: function() {
        this.Error = '';
        if(this.Rate.Template.Seasons.length === 0) {
          this.Error = 'Select a base tariff first';
          return;
        }
        if(this.Rate.StartDate < this.Rate.Template.Seasons[0].StartDate) {
          this.Error = 'Date ranges is outside selected tariff ranges';
          return;
        }
        if(this.Rate.EndDate > this.Rate.Template.Seasons.slice(-1)[0].EndDate) {
          this.Error = 'Date ranges is outside selected tariff ranges';
          return;
        }
      },
      SetStartDate: function(date) {
        this.Rate.StartDate = date.startOf('day').valueOf();
        this.Validate();
      },
      SetEndDate: function(date) {
        this.Rate.EndDate = date.endOf('day').valueOf();
        this.Validate();
      },
      Back: function() {
        this.$refs.Dialog.Close();
        this.$parent.$refs.Rates.Show(this.analyzer);
      },
      PeriodItemChanged: function(value, type, idx) {
        var Value = parseFloat(value.target.value);
        if(isNaN(Value) === true) Value = 0.0;
        if(type === 'Contracted_Power')
          Value = Value * 1000;
        else
          Value = Value / 1000;
        this.Rate.Config.Periods[idx][type] = Value;
        this.Rate = Object.assign({}, this.Rate);
      },
      ValueConceptChanged: function(value, idx) {
        var Value = parseFloat(value.target.value);
        if(isNaN(Value) === true) Value = 0.0;
        this.Rate.Config.Taxs[idx].Value = Value;
        //this.Rate = Object.assign({}, this.Tariff);
      },
      AddBillDay: function() {
        this.Rate.Config.Billing.push(this.Rate.EndDate);
      },
      BuildPeriods: function() {
        this.$set(this.Rate.Config, 'Billing', []);
        var Date = moment.tz(this.Rate.StartDate, this.Analyzer.TimeZone);
        while(true) {
          Date = Date.add(this.Period + 1, 'M').date(this.Day + 1);
          if(Date.valueOf() >= this.Rate.EndDate) break;
          this.Rate.Config.Billing.push(Date.startOf('day').valueOf() - 1);
        }
      },
      FillPeriods: function() {
        this.Rate.Config.Periods.length = this.Rate.Template.Periods.length;
        for(var n = 0; n < this.Rate.Template.Periods.length; n++) this.Rate.Config.Periods[n] = Object.assign({ Contracted_Power: 0.0, Energy_Cost: 0.0, Power_Cost: 0.0 }, this.Rate.Config.Periods[n]);
      },
      SelectBase: function(e, keywork) {
        this.$refs.StartDate.Enable();
        this.$refs.EndDate.Enable();
        if(keywork.id === 'CURRENT') {
          this.LoadCurrent();
          this.Validate();
        } else {
          this.ShowSpinner = true;
          $.ajax({ url: "getTariff", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ TariffID: keywork.id }),
            success: function(result) {
              this.Rate.Template = result.Template;
              this.Rate.Name = result.Name;
              this.FillPeriods();
              this.Validate();
              this.ShowSpinner = false;
            }.bind(this)
          });
        }
      },
      DeselectBase: function(e) {
        this.$refs.StartDate.Disable();
        this.$refs.EndDate.Disable();
        this.Error = 'Select a base tariff from list';
      }
    },
    beforeDestroy: function() {
      $(this.$refs.Base).off('onSetSelectValue', this.SelectBase.bind(this));
      $(this.$refs.Base).off('onUnsetSelectValue', this.DeselectBase.bind(this));
      $(this.$refs.Base).bsSuggest("destroy");
    },
    components: { OptionPicker, DatePicker }
  }
</script>

<style scoped>
  .dropdown-menu >>> .jhover {
    background: #337ab7;
  }
  .dropdown-menu >>> .table {
    margin: 4px 0px 0px 0px !important;
  }
  .dropdown-menu >>> td {
    border: none;
  }
  table, .input-group {
    width: 100%;
  }
  table tr td {
    padding: 1px 3px;
  }
  .periods input, .periods td {
    text-align: right;
  }
</style>