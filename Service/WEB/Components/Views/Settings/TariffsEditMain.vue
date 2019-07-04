<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Tariffs template</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="$parent.Save" v-bind:class="{disabled: (Error != '')}"><img :src="require('Imgs/diskette.png')"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Periods">Periods</button>
        <button type="button" class="btn btn-default btn-xs" @click="Seasons">Seasons</button>
        <button type="button" class="btn btn-default btn-xs" @click="Holidays">Holidays</button>
        <button type="button" class="btn btn-default btn-xs" @click="$parent.Close"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="alert alert-danger text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error != ''">{{ Error }}</div>
      <div class="alert alert-success text-center" style="margin-bottom: 5px; padding: 5px" v-if="Error == ''">Tariff contains {{ Tariff.Template.Seasons.length }}  seasons, a total of {{ Tariff.Template.Periods.length }}  day periods and {{ Tariff.Template.Holidays.length }} holidays</div>
      <div class="row">
        <div class="col-xs-12 col-sm-6"><label class="control-label">Name</label><input type="text" class="form-control" ref="Name" @change="SetName"></div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Country/Zone</label>
          <div class="input-group">
            <input type="text" class="form-control" value="Spain" style="pointer-events: none" id="inputTariffCountry">
            <div class="input-group-btn">
              <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false"><span class="caret"></span></button>
              <ul class="dropdown-menu dropdown-menu-right">
                <li><a href="#">Action</a></li>
                <li><a href="#">Another action</a></li>
                <li><a href="#">Something else here</a></li>
                <li role="separator" class="divider"></li>
                <li><a href="#">Separated link</a></li>
              </ul>
            </div>
          </div>
        </div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">Start date</label><date-picker ref="StartDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" v-on:DateChanged="SetStartDate"></date-picker></div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">End date</label><date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" v-on:DateChanged="SetEndDate"></date-picker></div>
        <div class="col-xs-12"><label class="control-label">Description</label><textarea class="form-control" rows="5" ref="Description" @change="DescriptionChanged($event)" style="resize: none"></textarea></div>
        <div class="col-xs-12 col-sm-6">
          <div class="btn-group period-dropdown-group">
            <div class="input-group">
              <span class="input-group-addon" style="width: 100%"><strong>Weekends as</strong></span>
              <div class="input-group-btn">
                <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" aria-expanded="false"><span style="display: inline-block;width: 80px;">{{ (Tariff.Template.WeekendPeriod == null) ? 'Don\'t apply' : 'P' + (Tariff.Template.WeekendPeriod+1) }}</span><span class="caret"></span></button>
                <ul class="dropdown-menu pull-right">
                  <li v-for="(period, index) in Tariff.Template.Periods" v-bind:class="{active: Tariff.Template.WeekendPeriod == index}"><a @click="SetWeekEndPeriod(index)">P{{ index + 1 }}</a></li>
                  <li role="separator" class="divider"></li>
                  <li v-bind:class="{active: Tariff.Template.WeekendPeriod == null}"><a @click="SetWeekEndPeriod(null)">Don't apply</a></li>
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
                    <li v-for="(period, index) in Tariff.Template.Periods" v-bind:class="{active: Tariff.Template.HolidayPeriod == index}"><a @click="SetHolidayPeriod(index)">P{{ index + 1 }}</a></li>
                    <li role="separator" class="divider"></li>
                    <li v-bind:class="{active: Tariff.Template.HolidayPeriod == null}"><a @click="SetHolidayPeriod(null)">Don't apply</a></li>
                  </ul>
                </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import moment from 'JS/moment.js';
  import DatePicker from 'Components/DatePicker.vue';
  export default {
    data() {
      return { Error: '', Show: false, Tariff: {} };
    },
    methods: {
      SetName: function(evt) {
        this.Tariff.Name = evt.target.value;
      },
      SetWeekEndPeriod: function(period) {
        this.Tariff.Template.WeekendPeriod = period;
      },
      SetStartDate: function(date) {
        this.Tariff.StartDate = date.startOf('day').valueOf();
        this.Error = this.$parent.Check(this.Tariff);
      },
      SetEndDate: function(date) {
        this.Tariff.EndDate = date.endOf('day').valueOf();
        this.Error = this.$parent.Check(this.Tariff);
      },
      SetHolidayPeriod: function(period) {
        this.Tariff.Template.HolidayPeriod = period;
      },
      DescriptionChanged: function(value) {
        this.Tariff.Template.Description = value.target.value;      
      },
      Load: function(tariff) {
        $.ajax({ url: "getCountries", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) {
            this.Show = true;
            this.Tariff = tariff;
            if(this.Tariff.Template.WeekendPeriod >= this.Tariff.Template.Periods.length) this.Tariff.Template.WeekendPeriod = null;
            if(this.Tariff.Template.HolidayPeriod >= this.Tariff.Template.Periods.length) this.Tariff.Template.HolidayPeriod = null;
            this.$nextTick(function() {
              $(this.$refs.Name).val(tariff.Name);
              $(this.$refs.Description).val(tariff.Template.Description);      
              this.$refs.StartDate.SetDate(moment(tariff.StartDate));
              this.$refs.EndDate.SetDate(moment(tariff.EndDate));
              this.Error = this.$parent.Check(tariff);
            });

          }.bind(this)
        });
      
      },
      Periods() {
        this.Show = false;
        this.$parent.$refs.Periods.Load(this.Tariff);
      },
      Seasons() {
        this.Show = false;
        this.$parent.$refs.Seasons.Load(this.Tariff);
      },
      Holidays() {
        this.Show = false;
        this.$parent.$refs.Holidays.Load(this.Tariff);
      }
    },
    components: { DatePicker }
  }
</script>
<style scoped>
  .period-dropdown-group {
    margin-top: 8px;width: 100%
  }
</style>