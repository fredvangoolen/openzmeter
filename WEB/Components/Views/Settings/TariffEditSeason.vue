<template>
  <div class="modal-content" v-if="Show">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Seasons &raquo; Season</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Go back"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="row">
        <div class="col-xs-12">
          <label class="control-label">Name</label><input type="text" class="form-control" ref="Name" @change="SetName">
        </div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">From</label><date-picker ref="StartDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" v-on:DateChanged="SetStartDate"></date-picker></div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">To</label><date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY', viewMode: 'months' }" v-on:DateChanged="SetEndDate"></date-picker></div>
        <div class="col-xs-12">
          <label class="control-label">Hourly rates</label>
          <div class="row">
            <div class="col-lg-3" v-for="(period, hour) in Tariff.Details.Seasons[Index].DayPeriods">
              <div class="btn-group" style="width: 100%; margin-bottom: 2px; box-sizing: content-box">
                <div class="input-group input-group-xs">    
                  <span class="input-group-addon">{{ ("0" + hour).slice(-2) }}</span>
                  <div class="input-group-btn">
                    <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" style="width: 100%"><span style="display: inline-block; width: 30px;">P{{ period + 1 }}</span><span class="caret"></span></button>
                    <ul class="dropdown-menu pull-right">
                      <li v-for="(p, index) in Tariff.Details.Periods" v-bind:class="{ active: Tariff.Details.Seasons[Index].DayPeriods[hour] == index }"><a @click="SetHourPeriod(hour, index)">P{{ index + 1 }}</a></li>
                    </ul>    
                  </div>
                </div>
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
      return { Show: false, Tariff: {}, Index: 0 };
    },
    methods: {
      Load: function(tariff, index) {
        this.Index = index;
        for(var i = 0; i < tariff.Details.Seasons[index].DayPeriods.length; i++) {
          if(tariff.Details.Seasons[index].DayPeriods[i] >= tariff.Details.Periods.length) tariff.Details.Seasons[index].DayPeriods[i] = 0;
        }    
        this.Tariff = tariff;
        this.Show = true;
        this.$nextTick(function() {
          $(this.$refs.Name).val(tariff.Details.Seasons[index].Name);
          this.$refs.StartDate.SetDate(moment(tariff.Details.Seasons[index].StartDate));
          this.$refs.EndDate.SetDate(moment(tariff.Details.Seasons[index].EndDate));
        });
      },
      SetName: function(evt) {
        this.Tariff.Details.Seasons[this.Index].Name = evt.target.value;
      },
      SetStartDate: function(date) {
        this.Tariff.Details.Seasons[this.Index].StartDate = date.startOf('day').valueOf();
      },
      SetEndDate: function(date) {
        this.Tariff.Details.Seasons[this.Index].EndDate = date.endOf('day').valueOf();
      },
      Back: function() {
        this.Show = false;
        this.$parent.$refs.Seasons.Load(this.Tariff);
      },
      SetHourPeriod: function(hour, period) {
        this.$set(this.Tariff.Details.Seasons[this.Index].DayPeriods, hour, period);
      }
    },
    components: { DatePicker }
  }
</script>