<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" v-on:click="Previous"><img :src="require('Imgs/resultset_previous.png')"></button>
      </div>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ SelectedText }}<span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li v-bind:class="{active: ((Period == 'day') && (To == 'last'))}"><a @click="PeriodClick('day', 'last')">Last 24 hours</a></li>
          <li v-bind:class="{active: ((Period == 'week') && (To == 'last'))}"><a @click="PeriodClick('week', 'last')">Last 7 days</a></li>
          <li v-bind:class="{active: ((Period == 'month') && (To == 'last'))}"><a @click="PeriodClick('month', 'last')">Last 30 days</a></li>
          <li v-bind:class="{active: ((Period == 'year') && (To == 'last'))}"><a @click="PeriodClick('year', 'last')">Last 365 days</a></li>
          <li role="separator" class="divider"></li>
          <li v-bind:class="{active: ((Period == 'day') && (To == 'natural'))}"><a @click="PeriodClick('day', 'natural')">Today</a></li>
          <li v-bind:class="{active: ((Period == 'week') && (To == 'natural'))}"><a @click="PeriodClick('week', 'natural')">This week</a></li>
          <li v-bind:class="{active: ((Period == 'month') && (To == 'natural'))}"><a @click="PeriodClick('month', 'natural')">This month</a></li>
          <li v-bind:class="{active: ((Period == 'year') && (To == 'natural'))}"><a @click="PeriodClick('year', 'natural')">This year</a></li>
          <li role="separator" class="divider"></li>
          <li v-bind:class="{active: (Number.isInteger(To) == true)}"><a @click="EditPeriod()">Custom</a></li>
        </ul>
      </div>
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" v-on:click="Next"><img :src="require('Imgs/resultset_next.png')"></button>
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
      return { Period: 'day', To: 'last', SelectedText: 'Last 24 hours', ShowDialog: false };
    },
    beforeDestroy: function() {
      clearInterval(this.Timer);
    },
    created: function() {
      this.Aggreg = null;
      this.LastSample = 0;
      this.Timer = null;
      this.DateWindow = [0, 0];
      this.Update = true;
      this.TmpDateWindow;
      var Conf = JSON.parse(localStorage.getItem(this.variable));
      if(Conf.To !== undefined) this.To = Conf.To;
      if(Conf.Period !== undefined) this.Period = Conf.Period;
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
      this.PeriodClick(this.Period, this.To);
    },
    methods: {
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
        if((this.DateWindow[0] != this.TmpDateWindow[0]) || (this.DateWindow[1] != this.TmpDateWindow[1])) {
          this.SetCustom(this.TmpDateWindow[0], this.TmpDateWindow[1]);
          this.EmitUpdate();
        }
        this.Back();
      },
      Back: function() {
        $(this.$refs.dialog).modal('hide');
        this.ShowDialog = false;
      },
      PeriodClick: function(period, to) {
        this.LastSample = 0;
        this.Period = period;
        this.To = to;
        if((this.To !== 'last') && (this.To !== 'natural')) {
          this.SelectedText = 'Custom';
          if(this.Period === 'day') this.SelectedText = 'Day';
          if(this.Period === 'week') this.SelectedText = 'Week';
          if(this.Period === 'month') this.SelectedText = 'Month';
          if(this.Period === 'year') this.SelectedText = 'Year';
        } else {
          if((this.Period !== 'day') && (this.Period !== 'week') && (this.Period !== 'month') && (this.Period !== 'year')) this.Period = 'day';
          if((this.Period === 'day') && (this.To === 'last')) this.SelectedText = 'Last 24 hours';
          if((this.Period === 'week') && (this.To === 'last')) this.SelectedText = 'Last 7 days';
          if((this.Period === 'month') && (this.To === 'last')) this.SelectedText = 'Last 30 days';
          if((this.Period === 'year') && (this.To === 'last')) this.SelectedText = 'Last 365 days';
          if((this.Period === 'day') && (this.To === 'natural')) this.SelectedText = 'Today';
          if((this.Period === 'week') && (this.To === 'natural')) this.SelectedText = 'This week';
          if((this.Period === 'month') && (this.To === 'natural')) this.SelectedText = 'This month';
          if((this.Period === 'year') && (this.To === 'natural')) this.SelectedText = 'This year';
        }
        localStorage.setItem(this.variable, JSON.stringify({ To: this.To, Period: this.Period }));
        this.EmitUpdate();
      },
      SetLastSample: function(lastsample) {
        this.LastSample = lastsample;
      },
      Previous: function() {},
      Next: function() {},
      SetAnalyzer() {
        this.LastSample = 0;
        this.EmitUpdate();
      },
      SetAggregation: function(name, force) {
        this.Aggreg = name;
        if(name === '3S')  this.AggregMS = 3000;
        if(name === '1M')  this.AggregMS = 60000;
        if(name === '10M') this.AggregMS = 600000;
        if(name === '15M') this.AggregMS = 900000;
        if(name === '1H')  this.AggregMS = 3600000;
        this.LastSample = 0;
        if(force !== false) this.EmitUpdate();
      },
      SetCustom: function(from, to) {
        from = Math.round(from);
        to = Math.round(to);
        this.PeriodClick(to - from, to);
      },
      //      SetAutoUpdate(autoUpdate) {
//        this.AutoUpdate = autoUpdate;
//        this.LastSample = 0;
//        if(autoUpdate === false) this.$refs.tools.WriteTo(this.DateWindow[1]);
//        if((autoUpdate === true) || (this.Data === [])) this.UpdateGraph();
//      },
//      PeriodChange(to, period, lastsample) {
//        this.LastSample = 0;
//        this.UpdateGraph();
//      },

      
//      Updated: function(from, to, last, auto, aggreg) {
//        clearInterval(this.Timer);
//        if(auto === true) this.Timer = setTimeout(this.UpdateGraph.bind(this), this.Aggreg / 2 );
//    
//         this.Enabled = true;
//      },
      EmitUpdate: function() {
        var To, Period;
        if(this.To === 'natural') {
          if(this.Period === 'day') {
            Period = 86400000;
            To = moment().endOf('day').valueOf();
          } else if(this.Period === 'week') {
            Period = 604800000;
            To = moment().endOf('week').valueOf();
          } else if(this.Period === 'month') {
            To = moment().endOf('month').valueOf();
            Period = To - moment().startOf('month').valueOf();
          } else if(this.Period === 'year') {
            To = moment().endOf('year').valueOf();
            Period = To - moment().startOf('year').valueOf();
          }
          this.DateWindow = [To - Period, To];
        } else if(this.To === 'last') {
          To = moment().valueOf();
          if(this.Period === 'day') Period = 86400000;
          else if(this.Period === 'week') Period = 604800000;
          else if(this.Period === 'month') Period = 2592000000;
          else if(this.Period === 'year') Period = 31536000000;
          this.DateWindow = [To - Period, To];
          To = 0;
        } else {
          To = this.To;
          if(this.Period === 'day') Period = 86400000;
          else if(this.Period === 'week') Period = 604800000;
          else if(this.Period === 'month') Period = 2592000000;
          else if(this.Period === 'year') Period = 31536000000;
          else Period = this.Period;
          this.DateWindow = [To - Period, To];
        }
        if((this.$root.SelectedAnalyzer === null) || (this.Aggreg === null)) return;
        this.$emit('PeriodChanged', this.DateWindow, To, Period, this.LastSample, this.Aggreg);
        console.log("Load " + this.Period + " up to " + this.To);
      }
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