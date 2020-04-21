<template>
  <div>
    <dlg :loading="ShowSpinner">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Tariffs template</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Save" :class="{disabled: (Error!=null)}"><img :src="require('Imgs/diskette.png')"></button>
          <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Main' }" @click="View='Main'">Main</button>
          <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Periods' }" @click="View='Periods'">Periods</button>
          <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Days' }" @click="View='Days'">Days</button>
          <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Calendar' }" @click="View='Calendar'">Calendar</button>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
        </div>
      </template>
      <div class="alert alert-danger text-center" v-if="Error!=null">{{ Error }}</div>
      <div class="row" v-if="View=='Main'">
        <div class="col-xs-12">
          <label class="control-label">Name</label>
          <input type="text" class="form-control" v-model="Tariff.Name">
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Start year</label>
          <input-spin :min="1990" :max="Tariff.EndYear" v-model="Tariff.StartYear" :increment="1" :decimals="0" :big="true"></input-spin>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">End year</label>
          <input-spin :min="Tariff.StartYear" :max="2100" v-model="Tariff.EndYear" :increment="1" :decimals="0" :big="true"></input-spin>
        </div>
        <div class="col-xs-12">
          <label class="control-label">Description</label>
          <textarea class="form-control" rows="5" v-model="Tariff.Template.Description" style="resize: none"></textarea>
        </div>
      </div>
      <div class="row" v-if="View=='Periods'">
        <div class="col-xs-12">
          <div style="display:flex">
            <detail-tabs style="flex-grow:2" key="Period_Tabs" :tabs="new Array(Tariff.Template.Periods.length).fill(0).map((n, idx) => { return 'P' + (idx + 1)})" v-model="ActivePeriod"></detail-tabs>
            <div>
              <div class="btn-group">
                <button class="btn btn-default btn-xs" type="button" v-if="Tariff.Template.Periods.length>1" @click="DeletePeriod()" data-toggle="tooltip" title="Delete last period"><img :src="require('Imgs/cross.png')"></button>
                <button class="btn btn-default btn-xs" type="button"><img :src="require('Imgs/plus.png')" @click="AddPeriod()" data-toggle="tooltip" title="Add new period"></button>
              </div>
            </div>
          </div>
          <div v-for="(period, idx) in Tariff.Template.Periods" :key="idx">
            <div class="panel-other" v-if="ActivePeriod=='P'+(idx+1)">
              <div class="panel-heading">
                <div class="pull-right btn-group">
                  <button class="btn btn-default btn-xs" type="button"><img :src="require('Imgs/math_functions.png')"></button>
                  <button class="btn btn-default btn-xs" type="button"><img :src="require('Imgs/plus.png')"></button>
                </div>
                <div class="clearfix"></div>
              </div>
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
                  <input-spin :max="100000" :min="0" prepend="/kW/Day" :decimals="6" :increment="0.01" v-model="period.DefaultPowerCost" style="padding-right:4px"></input-spin>
                </div>
                <div style="flex-grow:2">
                  <label class="small">Default energy cost:</label>
                  <input-spin :max="100000" :min="0" prepend="/kWh" :decimals="6" :increment="0.01" v-model="period.DefaultEnergyCost"></input-spin>
                </div>
              </div>
              <div class="clearfix"></div>
              <table class="period_vars">
                <tr>
                  <td><strong>Concept</strong></td>
                  <td><strong>Variable</strong></td>
                  <td><strong>Type</strong></td>
                </tr>
                <tr><td colspan="3" v-if="period.Concepts.length==0" class="text-center small" style="border-top: 1px solid #ccc">No concepts for this period</td></tr>
                <tr v-for="(concept, idx) in period.Concepts" :key="idx">
                  <td>
                    <div class="input-group input-group-xs">
                      <span class="input-group-btn">
                        <button class="btn btn-default" type="button"><img :src="require('Imgs/cross.png')"></button>
                      </span>
                      <input type="text" class="form-control" v-model="concept.Description">
                    </div>
                  </td>
                  <td>
                    <div class="input-group input-group-xs">
                      <input type="text" class="form-control rounded" v-model="concept.Variable">
                    </div>
                  </td>
                  <td>
                    <option-picker v-model="concept.Type" :options="[ {name: 'Info', value: 'INFO'}, {name: 'General', value: 'GENERAL'}, {name: 'Energy', value: 'ENERGY'}, {name: 'Power', value: 'POWER'} ]" ></option-picker>
                  </td>
                </tr>
              </table>
            </div>
          </div>
        </div>
      </div>
      <div class="row" v-if="View=='Days'">
        <div class="col-xs-12">
          <div style="display: flex">
            <detail-tabs style="flex-grow:2" key="Day_Tabs" :tabs="new Array(Tariff.Template.Days.length).fill(0).map((n, idx) => { return 'D' + (idx + 1)})" v-model="ActiveDay"></detail-tabs>
            <div>
              <div class="btn-group">
                <button class="btn btn-default btn-xs" type="button"><img :src="require('Imgs/cross.png')"></button>
                <button class="btn btn-default btn-xs" type="button"><img :src="require('Imgs/plus.png')"></button>
              </div>
            </div>
          </div>
          <div v-for="(day, idx) in Tariff.Template.Days" :key="idx">
            <div class="panel-other" v-if="ActiveDay=='D'+(idx+1)">
              <div class="panel-heading">
                <div class="pull-right">
                  <option-picker :options="[{name: 'P1', value: 0}, {name: 'P2', value: 1}]" v-model="ActivePeriodIdx" prefix="Fill with period"></option-picker>
                </div>
                <div class="clearfix"></div>
              </div>
              <div class=" col-xs-12" style="display:flex">
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
                <day-config v-model="day.Quarters" :colors="['#fffff0', '#ffc4a2']" :names="['P1', 'P2']" :selected="ActivePeriodIdx"></day-config>
              </div>
            </div>
          </div>
        </div>
      </div>
      <div class="row" v-if="View=='Calendar'">
        <div class="col-xs-12">
          <detail-tabs key="Year_Tabs" :tabs="['2019', '2020']" v-model="ActiveYear"></detail-tabs>
          <div v-for="(year, idx) in Tariff.Template.Years" :key="idx">
            <div class="panel-other" v-if="ActiveYear==idx">
              <div class="panel-heading">
                <div class="pull-right">
                  <option-picker :options="[{name: 'D1', value: 0}, {name: 'D2', value: 1}]" v-model="ActiveDayIdx" prefix="Fill with day"></option-picker>
                </div>
                <div class="clearfix"></div>
              </div>
              <div class="day-stripes-table">
                <year-config :year="parseInt(idx)" v-model="Tariff.Template.Years[idx]" :colors="['#fffff0', '#ffc4a2']" :names="['D1', 'D2']" :selected="ActiveDayIdx"></year-config>
              </div>
            </div>
          </div>
        </div>
      </div>
    </dlg>
  </div>
</template>

<script>
  import EditPeriods from './Periods.vue';
  import EditSeasons from './Seasons.vue';
  import EditHolidays from './Holidays.vue';
  import moment from 'moment';
  import calendar from 'JS/bootstrap-year-calendar.js';
  export default {
    props: { tariff: Object },
    data: function() {
      return {
        ActivePeriod: 'P1', ActiveDay: 'D1', ActivePeriodIdx: 0, ActiveDayIdx: 0, ActiveYear: '2020',



        Tariff: null, View: 'Main', TimeZones: moment.tz.names().map( (name) => { return { name: name, value: name }; } ), ShowSpinner: false };
    },
    computed: {

      Error: function() {
//        this.Tariff.StartDate = moment(this.Tariff.StartDate).tz(this.Tariff.TimeZone).startOf('day').valueOf();
//        this.Tariff.EndDate = moment(this.Tariff.EndDate).tz(this.Tariff.TimeZone).endOf('day').valueOf();
//        var FindSeason = (date) => {
//          for(var i = 0; i < this.Tariff.Template.Seasons.length; i++) {
//            if(this.Tariff.Template.Seasons[i].StartDate === date) return i;
//          }
//          return null;
//        };
//        for(var i = 0; i < this.Tariff.Template.Seasons.length; i++) {
//          this.Tariff.Template.Seasons[i].StartDate = moment(this.Tariff.Template.Seasons[i].StartDate).tz(this.Tariff.TimeZone).startOf('day').valueOf();
//          this.Tariff.Template.Seasons[i].EndDate = moment(this.Tariff.Template.Seasons[i].EndDate).tz(this.Tariff.TimeZone).endOf('day').valueOf();
//          for(var h = 0; h < 24; h++) {
//            if((this.Tariff.Template.Seasons[i].DayPeriods[h] < 0) || (this.Tariff.Template.Seasons[i].DayPeriods[h] >= this.Tariff.Template.Periods.length)) return 'Periods in season "' + tariff.Template.Seasons[i].Name + '" out of range!';
//          }
//        }
//        var Used = this.Tariff.Template.Seasons.length;
//        var Date = this.Tariff.StartDate;
//        while(Used > 0) {
//          var Find = FindSeason(Date);
//          if(Find == null) break;
//          Date = this.Tariff.Template.Seasons[Find].EndDate;
//          Used = Used - 1;
//          if(Used > 0) Date = Date + 1;
//        }
//        if((Used > 0) || (Date != this.Tariff.EndDate)) return 'Defined tariff period is not correctly covered by defined seasons!';
//        return null;
      }
    },
    created: function() {
//      this.Calendar = $(this.$refs.calendar).calendar({ language: "custom" });
      if(this.tariff == null) {
//        this.Tariff = {
//          TimeZone: moment.tz.guess(),
//          StartDate: moment().startOf('year').valueOf(),
//          EndDate: moment().endOf('year').valueOf(),
//          Name: 'No name',
//          Template: {
//            Description: 'Add tariff description here',
//            Seasons: [],
//            Periods: [ { ReactiveMode: 'NONE', PowerMode: 'NONE', Color: '#fffff0' } ],
//            Holidays: [],
//            WeekendPeriod: null,
//            HolidayPeriod: null
//          }
//        };
      } else {
        this.Tariff = Object.assign({}, this.tariff);
      }
    },
    methods: {
      DeletePeriod: function() {
        if(confirm("Delete period 'P" + this.Tariff.Template.Periods.length + "'?")) this.Tariff.Template.Periods.splice(-1);
      },
      AddPeriod: function() {
        this.Tariff.Template.Periods.push({ "Color":"#fffff0", DefaultPowerCost: 0.0, DefaultEnergyCost: 0.0, Script: "", Name: "Period " + (this.Tariff.Template.Periods.length + 1), Concepts: [ ]} );
        this.ActivePeriod = 'P' + this.Tariff.Template.Periods.length;
      },
      Save: function() {
        this.$http.post((this.Tariff.TariffID == null) ? 'addTariff' : 'setTariff', this.Tariff).then(
          (result) => {
            this.$emit('close');
          },
          (fail) => {
            this.Error = 'Failed to save tariff!';
            setTimeout(() => { this.Error = null; }, 1000);
          }
        );
      }
    },
    components: { EditPeriods, EditSeasons, EditHolidays }
  }
</script>

<style scoped>
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
    width: 80px;
  }
  .rounded {
    border-radius: 4px!important;
  }
  div .alert {
    margin-bottom: 5px;
    padding: 5px
  }
</style>