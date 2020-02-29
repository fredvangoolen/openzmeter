<template>
  <dlg>
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Seasons &raquo; Season</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close', Tariff)"><img :src="require('Imgs/arrow_undo.png')" alt="Go back"></button>
      </div>
    </template>
    <div class="row">
      <div class="col-xs-12">
        <label class="control-label">Name</label>
        <input type="text" class="form-control" v-model="Tariff.Template.Seasons[index].Name">
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">From</label>
        <date-picker :big="true" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: Tariff.TimeZone }" v-model="Tariff.Template.Seasons[index].StartDate"></date-picker>
      </div>
      <div class="col-xs-12 col-sm-6">
        <label class="control-label">To</label>
        <date-picker :big="true" :options="{ format: 'DD/MM/YYYY', viewMode: 'months', timeZone: Tariff.TimeZone }" v-model="Tariff.Template.Seasons[index].EndDate"></date-picker>
      </div>
      <div class="col-xs-12">
        <label class="control-label">Hourly rates</label>
        <div class="row">
          <div class="col-xs-3" v-for="(period, hour) in Tariff.Template.Seasons[index].DayPeriods" :key="hour">
            <div class="btn-group" style="width: 100%; margin-bottom: 2px; box-sizing: content-box">
              <div class="input-group input-group-xs">
                <span class="input-group-addon">{{ ("0" + hour).slice(-2) }}</span>
                <div class="input-group-btn">
                  <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" style="width: 100%"><span style="display: inline-block; width: 30px;">P{{ Tariff.Template.Seasons[index].DayPeriods[hour] + 1 }}</span><span class="caret"></span></button>
                  <ul class="dropdown-menu pull-right">
                    <li v-for="(p, key) in Tariff.Template.Periods" :key="key" :class="{ active: Tariff.Template.Seasons[index].DayPeriods[hour] == key }"><a @click="$set(Tariff.Template.Seasons[index].DayPeriods, hour, key)">P{{ key + 1 }}</a></li>
                  </ul>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </dlg>
</template>

<script>
  import moment from 'moment';
  import DatePicker from 'Components/DatePicker.vue';
  export default {
    props: { tariff: Object, index: Number },
    data() {
      return { Tariff: null };
    },
    created: function() {
      this.Tariff = Object.assign({}, this.tariff);
    },
    components: { DatePicker }
  }
</script>