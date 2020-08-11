<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <div class="input-group-btn">
        <button class="btn btn-default narrow" type="button" @click="Previous"><img :src="require('Imgs/resultset_previous.png')"></button>
      </div>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ SelectedText }}<span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
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
          <li :class="{active: (Array.isArray(value))}"><a @click="ShowDialog=true">Custom</a></li>
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
          <button type="button" class="btn btn-default btn-xs" @click="$emit('input', Array.from(Range))"><img :src="require('Imgs/tick.png')" alt="Apply"></button>
          <button type="button" class="btn btn-default btn-xs" @click="ShowDialog=false"><img :src="require('Imgs/arrow_undo.png')" alt="Discard"></button>
        </div>
      </template>
      <div class="row">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">From</label>
          <date-picker :options="{ format: 'DD/MM/YYYY - HH:mm:ss', viewMode: 'days', timeZone: analyzer.TimeZone }" v-model="Range[0]"></date-picker>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">To</label>
          <date-picker :options="{ format: 'DD/MM/YYYY - HH:mm:ss', viewMode: 'days', timeZone: analyzer.TimeZone }" v-model="Range[1]"></date-picker>
        </div>
      </div>
    </dlg>
  </div>
</template>

<script>
  import moment from 'moment';
  export default {
    props: { storage: String, value: [String, Array], analyzer: Object },
    data: function () {
      return { ShowDialog: false, Value: null, Range: null };
    },
    watch: {
      value: function(val) {
        this.ShowDialog = false;
        if(this.storage != null) localStorage.setItem(this.storage, JSON.stringify(val));
        this.Range = this.analyzer.CalcRange(val);
      }
    },
    created: function () {
      if(this.storage != null) {
        var Stored;
        try {
          Stored = JSON.parse(localStorage.getItem(this.storage));
        } catch(e) {
          Stored = 'Day';
        }
        if(Array.isArray(Stored) == true) {
          if(Stored.length != 2) Stored = 'Day';
        } else if((Stored != 'LastHours') && (Stored != 'LastDay') && (Stored != 'LastDay') && (Stored != 'LastWeek') && (Stored != 'LastMonth') && (Stored != 'LastYear') && (Stored != 'Day') && (Stored != 'Week') && (Stored != 'Month') && (Stored != 'Year')) {
          Stored = 'Day';
        }
        if(JSON.stringify(this.value) != JSON.stringify(Stored)) this.$emit('input', Stored);
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
    },
    methods: {
      Previous: function () {
        var Period = this.Range[1] - this.Range[0];
        this.$emit('input', [this.Range[0] - Period, this.Range[1] - Period]);
      },
      Next: function () {
        var Period = this.Range[1] - this.Range[0];
        this.$emit('input', [this.Range[0] + Period, this.Range[1] + Period]);
      }
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