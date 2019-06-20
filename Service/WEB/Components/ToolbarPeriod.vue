<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" v-on:click="Previous" :disabled="Updating"><img :src="require('Imgs/resultset_previous.png')"></button>
      </div>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" :disabled="Updating">{{ SelectedText }}<span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li v-bind:class="{active: ((Period == 'day') && (To == 'last'))}"><a @click="PeriodClick('day', 'last', $event)">Last 24 hours</a></li>
          <li v-bind:class="{active: ((Period == 'week') && (To == 'last'))}"><a @click="PeriodClick('week', 'last', $event)">Last 7 days</a></li>
          <li v-bind:class="{active: ((Period == 'month') && (To == 'last'))}"><a @click="PeriodClick('month', 'last', $event)">Last 30 days</a></li>
          <li v-bind:class="{active: ((Period == 'year') && (To == 'last'))}"><a @click="PeriodClick('year', 'last', $event)">Last 365 days</a></li>
          <li role="separator" class="divider"></li>
          <li v-bind:class="{active: ((Period == 'day') && (To == 'natural'))}"><a @click="PeriodClick('day', 'natural', $event)">Today</a></li>
          <li v-bind:class="{active: ((Period == 'week') && (To == 'natural'))}"><a @click="PeriodClick('week', 'natural', $event)">This week</a></li>
          <li v-bind:class="{active: ((Period == 'month') && (To == 'natural'))}"><a @click="PeriodClick('month', 'natural', $event)">This month</a></li>
          <li v-bind:class="{active: ((Period == 'year') && (To == 'natural'))}"><a @click="PeriodClick('year', 'natural', $event)">This year</a></li>
          <li role="separator" class="divider"></li>
          <li v-bind:class="{active: (Number.isInteger(To) == true)}"><a @click="EditPeriod()">Custom</a></li>
        </ul>
      </div>
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" v-on:click="Next" :disabled="Updating"><img :src="require('Imgs/resultset_next.png')"></button>
      </div>
    </div>
    <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" ref="dialog" data-keyboard="false">
      <div class="modal-backdrop in"></div>
      <div class="modal-dialog">
        <div class="modal-content">
        <div class="modal-header">
          <div class="pull-left"><h4 class="modal-title">Data range</h4></div>
          <div class="pull-right btn-group">
            <button type="button" class="btn btn-default btn-xs" @click="Apply"><img :src="require('Imgs/tick.png')" alt="Apply"></button>
            <button type="button" class="btn btn-default btn-xs" @click="Back"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
          </div>
        </div>
        <div class="modal-body">
          <div class="row">
            <div class="col-xs-12 col-sm-6"><label class="control-label">From</label><date-picker ref="StartDate" :options="{ format: 'DD/MM/YYYY - HH:mm:ss', viewMode: 'days' }" v-on:DateChanged="SetStartDate"></date-picker></div>
            <div class="col-xs-12 col-sm-6"><label class="control-label">To</label><date-picker ref="EndDate" :options="{ format: 'DD/MM/YYYY - HH:mm:ss', viewMode: 'days' }" v-on:DateChanged="SetEndDate"></date-picker></div>
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
    props: { variable: String },
    data: function() {
      return { Period: 'day', To: 'last', SelectedText: 'Last 24 hours', ShowDialog: false, Updating: true };
    },
    beforeDestroy: function() {
      clearInterval(this.Timer);
    },
    created: function() {
      this.Aggreg = null;
      this.LastSample = 0;
      this.Timer = null;
      this.DateWindow = [0, 0];
      this.TmpDateWindow = [0, 0];
      var Conf = JSON.parse(localStorage.getItem(this.variable));
      if(Conf !== null) {
        if(Conf.To !== undefined) this.To = Conf.To;
        if(Conf.Period !== undefined) this.Period = Conf.Period;
      }
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    methods: {
      SetAnalyzer() {
        this.LastSample = 0;
        this.EmitUpdate();
      },
      SetAggregation: function(name) {
        this.Aggreg = name;
        if(name === '3S')  this.AggregMS = 3000;
        if(name === '1M')  this.AggregMS = 60000;
        if(name === '10M') this.AggregMS = 600000;
        if(name === '15M') this.AggregMS = 900000;
        if(name === '1H')  this.AggregMS = 3600000;
        this.LastSample = 0;
        this.EmitUpdate();
      },
      EmitUpdate: function() {
        clearInterval(this.Timer);
        if((this.$root.SelectedAnalyzer === null) || (this.Aggreg === null)) return;
        var To, Period;
        if(this.To === 'natural') {
          To = moment().endOf(this.Period).valueOf();
          Period = To - moment().startOf(this.Period).valueOf();
          this.DateWindow = [To - Period, To];
        } else if(this.To === 'last') {
          To = moment().valueOf();
          Period = (this.Period === 'day') ? 86400000 : (this.Period === 'week') ? 604800000 : (this.Period === 'month') ? 2592000000 : 31536000000;
          this.DateWindow = [To - Period, To];
          To = 0;
        } else {
          To = this.To;
          Period = this.Period;
          this.DateWindow = [To - Period, To];
        }
        this.Updating = true;
        this.$emit('PeriodChanged', To, Period, this.LastSample, this.Aggreg);
      },
      UpdateCompleted: function(lastsampletime) {
        this.LastSample = lastsampletime;
        this.Updating = false;
        clearInterval(this.Timer);
        this.Timer = setTimeout(this.EmitUpdate.bind(this), this.AggregMS / 2);
      },
      PeriodClick: function(period, to, event) {
        this.LastSample = 0;
        this.Period = period;
        this.To = to;
        this.SelectedText = event.target.firstChild.textContent;
        localStorage.setItem(this.variable, JSON.stringify({ To: this.To, Period: this.Period }));
        this.EmitUpdate();
      },
      Previous: function() {
        var Period = this.Period;
        if(Number.isInteger(Period) == false) Period = (Period === 'day') ? 86400000 : (Period === 'week') ? 604800000 : (Period === 'month') ? 2592000000 : 31536000000;
        var To = this.DateWindow[1] - Period;
        this.SetCustom(To - Period, To);
      },
      Next: function() {
        var Period = this.Period;
        if(Number.isInteger(Period) == false) Period = (Period === 'day') ? 86400000 : (Period === 'week') ? 604800000 : (Period === 'month') ? 2592000000 : 31536000000;
        var To = this.DateWindow[1] + Period;
        this.SetCustom(To - Period, To);
      },
      SetCustom: function(from, to) {
        this.SelectedText = 'Custom';
        this.To = Math.round(to);
        this.Period = Math.round(to - from);
        this.LastSample = 0;
        localStorage.setItem(this.variable, JSON.stringify({ To: this.To, Period: this.Period }));
        this.EmitUpdate();
      },
      EditPeriod: function() {
        this.ShowDialog = true;
        this.TmpDateWindow = Array.from(this.DateWindow);
        this.$nextTick(function() { 
          this.$refs.StartDate.SetDate(moment(this.TmpDateWindow[0]));
          this.$refs.EndDate.SetDate(moment(this.TmpDateWindow[1])); 
          $(this.$refs.dialog).modal('show');
        });
      },
      SetStartDate: function(date) {
        if(date.valueOf() < this.TmpDateWindow[1]) this.TmpDateWindow[0] = date.valueOf();
      },
      SetEndDate: function(date) {
        if(date.valueOf() > this.TmpDateWindow[0]) this.TmpDateWindow[1] = date.valueOf();
      },
      Apply: function() {
        if((this.DateWindow[0] != this.TmpDateWindow[0]) || (this.DateWindow[1] != this.TmpDateWindow[1])) this.SetCustom(this.TmpDateWindow[0], this.TmpDateWindow[1]);
        this.Back();
      },
      Back: function() {
        $(this.$refs.dialog).modal('hide');
        this.ShowDialog = false;
      },
    },
    components: {DatePicker}
  }
</script>

<style scoped>
  .dropdown-menu {
    z-index: 10000;
  }
  .input-group-btn .narrow {
    padding: 1px 0px;
  }
</style>