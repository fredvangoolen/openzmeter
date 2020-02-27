<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" @click="Previous"><img :src="require('Imgs/resultset_previous.png')"></button>
      </div>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ SelectedText }}<span class="caret"></span></button>
        <ul class="dropdown-menu pull-right" ref="periodTags">
          <li :class="{active: (value == 'LastHours')}"><a @click="$emit('input', 'LastHours')">Last 2 hour</a></li>
          <li :class="{active: (value == 'LastDay')}"><a @click="$emit('input', 'LastDay')">Last 24 hours</a></li>
          <li :class="{active: (value == 'LastWeek')}"><a @click="$emit('input', 'LastWeek')">Last 7 days</a></li>
          <li :class="{active: (value == 'LastMonth')}"><a @click="$emit('input', 'LastMonth')">Last 30 days</a></li>
          <li :class="{active: (value == 'LastYear')}"><a @click="$emit('input', 'LastYear')">Last 365 days</a></li>
          <li role="separator" class="divider"></li>
          <li :class="{active: (value == 'Day')}"><a @click="$emit('input', 'Day')">Today</a></li>
          <li :class="{active: (value == 'Week')}"><a @click="$emit('input', 'Week')">This week</a></li>
          <li :class="{active: (value == 'Month')}"><a @click="$emit('input', 'Month')">This month</a></li>
          <li :class="{active: (value == 'Year')}"><a @click="$emit('input', 'Year')">This year</a></li>
          <li role="separator" class="divider"></li>
          <li :class="{active: (Array.isArray(value))}"><a @click="EditPeriod()">Custom</a></li>
        </ul>
      </div>
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" @click="Next"><img :src="require('Imgs/resultset_next.png')"></button>
      </div>
    </div>
    <dlg ref="Dialog" v-if="ShowDialog">
      <template v-slot:header>
        <div class="pull-left"><h4 class="modal-title">Data range</h4></div>
        <div class="pull-right btn-group">
          <button type="button" class="btn btn-default btn-xs" @click="Apply"><img :src="require('Imgs/tick.png')" alt="Apply"></button>
          <button type="button" class="btn btn-default btn-xs" @click="ShowDialog = false"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
        </div>
      </template>
      <div class="row">
        <div class="col-xs-12 col-sm-6"><label class="control-label">From</label><date-picker ref="StartDate" :options="{ format: 'DD/MM/YYYY - HH:mm:ss', viewMode: 'days' }" v-on:change="SetStartDate"></date-picker></div>
        <div class="col-xs-12 col-sm-6"><label class="control-label">To</label><date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY - HH:mm:ss', viewMode: 'days' }" v-on:change="SetEndDate"></date-picker></div>
      </div>
    </Dlg>
  </div>
</template>

<script>
  import moment from 'moment';
//  import DatePicker from 'Components/DatePicker.vue';
//  import Dlg from 'Components/Dialog.vue';
  export default {
    props: { storage: { type: String, default: null }, value: { type: [String, Array], default: null } },
    data: function () {
      return { ShowDialog: false, Value: null };
    },
    watch: {
      value: function(val) {
        if(this.storage != null) localStorage.setItem(this.storage, JSON.stringify(val));
      }
    },
//    beforeDestroy: function () {
//      clearInterval(this.Timer);
//    },
    created: function () {
      if(this.storage != null) {
        var Stored = JSON.parse(localStorage.getItem(this.storage));
        if(this.value != Stored) this.$emit('input', Stored);
      } else if(this.value == null) {
        this.$emit('input', 'Day');
      }
    },
    computed: {
      SelectedText: function() {
        if(this.value == 'LastHours') return 'Last 2 hours';
        if(this.value == 'LastDay') return 'Last 24 hours';
        if(this.value == 'LastDay') return 'Last 24 hours';
        if(this.value == 'LastWeek') return 'Last 7 days';
        if(this.value == 'LastMonth') return 'Last 30 days';
        if(this.value == 'LastYear') return 'Last 365 days';
        if(this.value == 'Day') return 'Today';
        if(this.value == 'Week') return 'This week';
        if(this.value == 'Month') return 'This month';
        if(this.value == 'Year') return 'This year';
        return 'Custom';
      }
//    },

//      this.UpdateEnabled = true;
//      this.Aggreg = null;
//      this.LastSample = 0;
//      this.Timer = null;
//      this.DateWindow = [0, 0];
//      this.TmpDateWindow = [0, 0];
//      var Conf = JSON.parse(localStorage.getItem(this.variable));
//      if(Conf !== null) {
//        if(Conf.To !== undefined) this.To = Conf.To;
//        if(Conf.Period !== undefined) this.Period = Conf.Period;
//      }
//      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
//    mounted: function() {
//      this.SelectedText = this.$refs.periodTags.querySelector('.active').innerText;
//    },
//    updated: function() {
//      this.SelectedText = this.$refs.periodTags.querySelector('.active').innerText;
//    },
    methods: {
//      StopUpdate: function() {
//        this.UpdateEnabled = false;
//        clearInterval(this.Timer);
//      },
//      UpdateCompleted: function(lastsampletime) {
//        this.LastSample = lastsampletime;
//        clearInterval(this.Timer);
//        if(this.UpdateEnabled) this.Timer = setTimeout(this.EmitUpdate.bind(this), this.AggregMS);
//      },
//      SetAnalyzer: function() {
//        this.LastSample = 0;
//        this.PeriodClick(this.Period, this.To);
//      },
//      SetAggregation: function (name) {
//        this.Aggreg = name;
//        if (name === '3S')
//          this.AggregMS = 3000;
//        if (name === '1M')
//          this.AggregMS = 60000;
//        if (name === '10M')
//          this.AggregMS = 600000;
//        if (name === '15M')
//          this.AggregMS = 900000;
//        if (name === '1H')
//          this.AggregMS = 3600000;
//        this.LastSample = 0;
//        this.EmitUpdate();
//      },
//      EmitUpdate: function () {
//        clearInterval(this.Timer);
//        if ((this.$root.SelectedAnalyzer === null) || (this.Aggreg === null))
//          return;
//        var To, Period;
//        if (this.To === 'natural') {
//          To = moment().endOf(this.Period).valueOf();
//          Period = To - moment().startOf(this.Period).valueOf();
//          this.DateWindow = [To - Period, To];
//        } else if (this.To === 'last') {
//          To = moment().valueOf();
//          Period = (this.Period === 'day') ? 86400000 : (this.Period === 'week') ? 604800000 : (this.Period === 'month') ? 2592000000 : 31536000000;
//          this.DateWindow = [To - Period, To];
//          To = 0;
//        } else {
//          To = this.To;
//          Period = this.Period;
//          this.DateWindow = [To - Period, To];
//        }
//        this.$emit('PeriodChanged', To, Period, this.LastSample, this.Aggreg);
//      },
//      PopState: function() {
//        if(this.Prev !== null) {
//          this.Period = this.Prev.Period;
//          this.To = this.Prev.To;
//          this.PeriodClick(this.Period, this.To);
//          this.Prev = null;
//          this.UpdateEnabled = true;
//        }
//      },
//      PeriodClick: function (period, to, event) {
//        this.LastSample = 0;
//        this.Period = period;
//        this.To = to;
//        localStorage.setItem(this.variable, JSON.stringify({To: this.To, Period: this.Period}));
//        this.EmitUpdate();
//        if(event !== undefined) this.SelectedText = event.target.firstChild.textContent;
//      },
      Previous: function () {
        var Period = this.Period;
        if(Number.isInteger(Period) === false) Period = (Period === 'day') ? 86400000 : (Period === 'week') ? 604800000 : (Period === 'month') ? 2592000000 : 31536000000;
        var To = this.DateWindow[1] - Period;
        this.SetCustom(To - Period, To);
      },
      Next: function () {
        var Period = this.Period;
        if(Number.isInteger(Period) === false) Period = (Period === 'day') ? 86400000 : (Period === 'week') ? 604800000 : (Period === 'month') ? 2592000000 : 31536000000;
        var To = this.DateWindow[1] + Period;
        this.SetCustom(To - Period, To);
      },
      SetCustom: function (from, to, saveState) {
        if(saveState === true) {
          if(this.Prev === null) this.Prev = { Period: this.Period, To: this.To};
        } else {
          this.Prev = null;
        }
        this.SelectedText = 'Custom';
        this.To = Math.round(to);
        this.Period = Math.round(to - from);
        this.LastSample = 0;
        if(this.Prev === null) localStorage.setItem(this.variable, JSON.stringify({To: this.To, Period: this.Period}));
        this.EmitUpdate();
      },
      EditPeriod: function () {
        this.TmpDateWindow = Array.from(this.DateWindow);
        this.$refs.Dialog.Show();
      },
      SetStartDate: function (date) {
        if(date.valueOf() < this.TmpDateWindow[1]) this.TmpDateWindow[0] = date.valueOf();
      },
      SetEndDate: function (date) {
        if(date.valueOf() > this.TmpDateWindow[0]) this.TmpDateWindow[1] = date.valueOf();
      },
//      Apply: function () {
//        if((this.DateWindow[0] !== this.TmpDateWindow[0]) || (this.DateWindow[1] !== this.TmpDateWindow[1])) this.SetCustom(this.TmpDateWindow[0], this.TmpDateWindow[1]);
//        this.Back();
//      }
    }
  }
</script>

<style scoped>
  .input-group-btn .narrow {
    padding: 1px 0px;
  }
  .dropdown-toggle {
    border-radius: 0px;
  }
</style>