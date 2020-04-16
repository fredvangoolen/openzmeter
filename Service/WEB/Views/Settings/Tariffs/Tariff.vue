<template>
  <div>
    <dlg :loading="ShowSpinner">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Tariffs template</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Save" :class="{disabled: (Error!=null)}"><img :src="require('Imgs/diskette.png')"></button>
          <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Main' }" @click="View='Main'">Main</button>
          <button type="button" class="btn btn-default btn-xs" :class="{ active: View=='Periods' }" @click="View='Periods'">Periods</button>
          <button type="button" class="btn btn-default btn-xs" @click="View='Seasons'">Seasons</button>
          <button type="button" class="btn btn-default btn-xs" @click="View='Holidays'">Holidays</button>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
        </div>
      </template>
      <div class="alert alert-danger text-center" v-if="Error!=null">{{ Error }}</div>
      <div class="alert alert-success text-center" v-if="Error==null">Tariff contains {{ Tariff.Template.Seasons.length }}  seasons, a total of {{ Tariff.Template.Periods.length }}  day periods and {{ Tariff.Template.Holidays.length }} holidays</div>
      <div class="row" v-if="View=='Main'">
        <div class="col-xs-12">
          <label class="control-label">Name</label>
          <input type="text" class="form-control" v-model="Tariff.Name">
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Start date</label>
          <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: 'UTC' }" v-model="Tariff.StartDate" :big="true"></date-picker>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">End date</label>
          <date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: 'UTC' }" v-model="Tariff.EndDate" :big="true"></date-picker>
        </div>
        <div class="col-xs-12">
          <label class="control-label">Description</label>
          <textarea class="form-control" rows="5" v-model="Tariff.Template.Description" style="resize: none"></textarea>
        </div>
      </div>
      <div class="row" v-if="View=='Periods'">
        <div class="col-xs-12">
          <table class="period">
            <tr>
              <td class="first-row" colspan="2">
                <div class="btn-group">
                  <button class="btn btn-default btn-xs" type="button"><img :src="require('Imgs/cross.png')"></button>
                </div>
                <strong>Period 1</strong>
                <div class="pull-right">
                  <color-picker  :columns="2" :colors="['#fffff0', '#ffecd6', '#ffd8bc', '#ffc4a2', '#ffae88', '#ff986c', '#ff7f50', '#f0ffff', '#d4f2f1', '#b7e5e2', '#98d8d3', '#78ccc5', '#55bfb8', '#20b2aa']"></color-picker>
                  <div class="btn-group">
                    <button class="btn btn-default btn-xs" type="button"><img :src="require('Imgs/math_functions.png')"></button>
                    <button class="btn btn-default btn-xs" type="button"><img :src="require('Imgs/plus.png')"></button>
                  </div>
                </div>
              </td>
            </tr>
            <tr>
              <td>Default power cost</td>
              <td><input-spin :max="100000" :min="-100000" prepend="/kW/Day" :decimals="6" :increment="1" :value="0"></input-spin></td>
            </tr><tr>
              <td>Default energy cost</td>
              <td><input-spin :max="100000" :min="-100000" prepend="/kWh" :decimals="6" :increment="1" :value="1"></input-spin></td>
            </tr>
            <tr>
              <td colspan="2">
                <table class="period_vars">
                  <tr>
                    <td><strong>Concept</strong></td>
                    <td><strong>Variable</strong></td>
                    <td><strong>Type</strong></td>
                  </tr>
                  <tr>
                    <td>
                      <div class="input-group input-group-xs">
                        <span class="input-group-btn">
                          <button class="btn btn-default" type="button"><img :src="require('Imgs/cross.png')"></button>
                        </span>
                        <input type="text" class="form-control">
                      </div>
                    </td>
                    <td>
                      <div class="input-group input-group-xs">
                        <input type="text" class="form-control rounded">
                      </div>
                    </td>
                    <td>
                      <option-picker v-model="Type" :options="[  {name: 'Info', value: 'INFO'}, {name: 'General', value: 'GENERAL'}, {name: 'Energy', value: 'ENERGY'}, {name: 'Power', value: 'POWER'} ]" ></option-picker>
                    </td>
                  </tr>
                </table>
              </td>
            </tr>
          </table>
        </div>
      </div>


    </dlg>
    <!--edit-periods :tariff="Tariff" v-if="View=='Periods'"   @close="View=null; Tariff=$event;"></edit-periods-->
    <edit-seasons :tariff="Tariff" v-if="View=='Seasons'"   @close="View=null; Tariff=$event;"></edit-seasons>
    <edit-holidays :tariff="Tariff" v-if="View=='Holidays'" @close="View=null; Tariff=$event;"></edit-holidays>
  </div>
</template>

<script>
  import EditPeriods from './Periods.vue';
  import EditSeasons from './Seasons.vue';
  import EditHolidays from './Holidays.vue';
  import moment from 'moment';
  export default {
    props: { tariff: Object },
    data: function() {
      return {
        Type: 'GENERAL',



        Tariff: null, View: 'Main', TimeZones: moment.tz.names().map( (name) => { return { name: name, value: name }; } ), ShowSpinner: false };
    },
    computed: {
      Error: function() {
        this.Tariff.StartDate = moment(this.Tariff.StartDate).tz(this.Tariff.TimeZone).startOf('day').valueOf();
        this.Tariff.EndDate = moment(this.Tariff.EndDate).tz(this.Tariff.TimeZone).endOf('day').valueOf();
        var FindSeason = (date) => {
          for(var i = 0; i < this.Tariff.Template.Seasons.length; i++) {
            if(this.Tariff.Template.Seasons[i].StartDate === date) return i;
          }
          return null;
        };
        for(var i = 0; i < this.Tariff.Template.Seasons.length; i++) {
          this.Tariff.Template.Seasons[i].StartDate = moment(this.Tariff.Template.Seasons[i].StartDate).tz(this.Tariff.TimeZone).startOf('day').valueOf();
          this.Tariff.Template.Seasons[i].EndDate = moment(this.Tariff.Template.Seasons[i].EndDate).tz(this.Tariff.TimeZone).endOf('day').valueOf();
          for(var h = 0; h < 24; h++) {
            if((this.Tariff.Template.Seasons[i].DayPeriods[h] < 0) || (this.Tariff.Template.Seasons[i].DayPeriods[h] >= this.Tariff.Template.Periods.length)) return 'Periods in season "' + tariff.Template.Seasons[i].Name + '" out of range!';
          }
        }
        var Used = this.Tariff.Template.Seasons.length;
        var Date = this.Tariff.StartDate;
        while(Used > 0) {
          var Find = FindSeason(Date);
          if(Find == null) break;
          Date = this.Tariff.Template.Seasons[Find].EndDate;
          Used = Used - 1;
          if(Used > 0) Date = Date + 1;
        }
        if((Used > 0) || (Date != this.Tariff.EndDate)) return 'Defined tariff period is not correctly covered by defined seasons!';
        return null;
      }
    },
    created: function() {
      if(this.tariff == null) {
        this.Tariff = {
          TimeZone: moment.tz.guess(),
          StartDate: moment().startOf('year').valueOf(),
          EndDate: moment().endOf('year').valueOf(),
          Name: 'No name',
          Template: {
            Description: 'Add tariff description here',
            Seasons: [],
            Periods: [ { ReactiveMode: 'NONE', PowerMode: 'NONE', Color: '#fffff0' } ],
            Holidays: [],
            WeekendPeriod: null,
            HolidayPeriod: null
          }
        };
      } else {
        this.Tariff = Object.assign({}, this.tariff);
      }
    },
    methods: {
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
  table.period {
    width: 100%;
    border: 1px solid #ccc;
  }
  table.period tr td {
    padding: 2px;
  }
  table.period .first-row {
    background: #eee;
    border: 1px solid #ccc;
  }
  table.period_vars {
    width: 100%;
    border-top: 1px solid #ccc;
  }
  table.period_vars tr:first-child {
    font-size: 85%;
  }
  table.period_vars .input-group {
    width: 100%;
  }
  table.period_vars td:nth-child(2), table.period_vars td:nth-child(3) {
    width: 100px;
  }
  .rounded {
    border-radius: 4px!important;
  }
  div .alert {
    margin-bottom: 5px;
    padding: 5px
  }
</style>