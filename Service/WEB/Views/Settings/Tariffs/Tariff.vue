<template>
  <div>
    <dlg :loading="ShowSpinner" v-if="View==null">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Tariffs template</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Save" :class="{disabled: (Error!=null)}"><img :src="require('Imgs/diskette.png')"></button>
          <button type="button" class="btn btn-default btn-xs" @click="View='Periods'">Periods</button>
          <button type="button" class="btn btn-default btn-xs" @click="View='Seasons'">Seasons</button>
          <button type="button" class="btn btn-default btn-xs" @click="View='Holidays'">Holidays</button>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
        </div>
      </template>
      <div class="alert alert-danger text-center" v-if="Error!=null">{{ Error }}</div>
      <div class="alert alert-success text-center" v-if="Error==null">Tariff contains {{ Tariff.Template.Seasons.length }}  seasons, a total of {{ Tariff.Template.Periods.length }}  day periods and {{ Tariff.Template.Holidays.length }} holidays</div>
      <div class="row">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Name</label>
          <input type="text" class="form-control" v-model="Tariff.Name"></div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Time zone</label>
          <option-picker :options="TimeZones" v-model="Tariff.TimeZone" :big="true"></option-picker>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Start date</label>
          <date-picker :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: Tariff.TimeZone }" v-model="Tariff.StartDate" :big="true"></date-picker>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">End date</label>
          <date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: Tariff.TimeZone }" v-model="Tariff.EndDate" :big="true"></date-picker>
        </div>
        <div class="col-xs-12"><label class="control-label">Description</label><textarea class="form-control" rows="5" v-model="Tariff.Template.Description" style="resize: none"></textarea></div>
        <div class="col-xs-12 col-sm-6">
          <div class="btn-group period-dropdown-group">
            <div class="input-group">
              <span class="input-group-addon" style="width: 100%"><strong>Weekends as</strong></span>
              <div class="input-group-btn">
                <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" aria-expanded="false"><span style="display: inline-block;width: 80px;">{{ (Tariff.Template.WeekendPeriod == null) ? 'Don\'t apply' : 'P' + (Tariff.Template.WeekendPeriod+1) }}</span><span class="caret"></span></button>
                <ul class="dropdown-menu pull-right">
                  <li v-for="(period, index) in Tariff.Template.Periods" :key="index" :class="{active: Tariff.Template.WeekendPeriod == index}"><a @click="SetWeekEndPeriod(index)">P{{ index + 1 }}</a></li>
                  <li role="separator" class="divider"></li>
                  <li :class="{active: Tariff.Template.WeekendPeriod == null}"><a @click="SetWeekEndPeriod(null)">Don't apply</a></li>
                </ul>
              </div>
            </div>
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <div class="btn-group period-dropdown-group">
            <div class="input-group">
              <span class="input-group-addon" style="width: 100%"><strong>Holidays as</strong></span>
                <div class="input-group-btn">
                  <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" aria-expanded="false"><span style="display: inline-block;width: 80px;">{{ (Tariff.Template.HolidayPeriod == null) ? 'Don\'t apply' : 'P' + (Tariff.Template.HolidayPeriod+1) }}</span><span class="caret"></span></button>
                  <ul class="dropdown-menu pull-right">
                    <li v-for="(period, index) in Tariff.Template.Periods" :key="index" :class="{active: Tariff.Template.HolidayPeriod == index}"><a @click="SetHolidayPeriod(index)">P{{ index + 1 }}</a></li>
                    <li role="separator" class="divider"></li>
                    <li :class="{active: Tariff.Template.HolidayPeriod == null}"><a @click="SetHolidayPeriod(null)">Don't apply</a></li>
                  </ul>
                </div>
            </div>
          </div>
        </div>
      </div>
    </dlg>
    <edit-periods :tariff="Tariff" v-if="View=='Periods'"   @close="View=null; Tariff=$event;"></edit-periods>
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
      return { Tariff: null, View: null, TimeZones: moment.tz.names().map( (name) => { return { name: name, value: name }; } ), ShowSpinner: false };
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
  .period-dropdown-group {
    margin-top: 8px;width: 100%
  }
  div .alert {
    margin-bottom: 5px;
    padding: 5px
  }
</style>