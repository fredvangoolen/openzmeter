<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg" v-if="data15M!=null">From 15 minutes aggregation<br>{{ FormatTime(data15M.Sampletime) }}</div>
      <div class="clearfix"></div>
      <div class="current-value" v-html="CurrentValue15M"></div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg" v-if="data1H!=null">From 1 hour aggregation<br>{{ FormatTime(data1H.Sampletime) }}</div>
      <div class="clearfix"></div>
      <div class="current-value" v-html="CurrentValue1H"></div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Period stats:</div>
      <div class="counters-aggreg" v-if="dataGraph!=null">{{ FormatAggregation(dataGraph.Aggregation_Source) }}<br>From {{ FormatDateTime(dataGraph.From) }} to {{ FormatDateTime(dataGraph.To) }}</div>
      <div class="line" v-bind:class="{'line-border-reactive': (serie == 'REACTIVE_ENERGY')}">
        <div class="float-left">
          <div class="counters-stat-type">Max value:</div>
          <div class="counters-stat-time">{{ isFinite(Max.Value) ? FormatDateTime(Max.Time) : '-' }}</div>
        </div>
        <div class="counters-stat-val" v-html="isFinite(Max.Value) ? FormatHTML(Max.Value) : '-'"></div>
      </div>
      <div class="line line-border" v-bind:class="{'line-border-reactive': (serie == 'REACTIVE_ENERGY')}">
        <div class="float-left">
          <div class="counters-stat-type">Total in period:</div>
          <div class="counters-stat-time">From {{ (Sum.Count > 0) ? Sum.Count : '-' }} samples</div>
        </div>
        <div class="counters-stat-val" v-html="isFinite(Sum.Value) ? FormatHTML(Sum.Value) : '-'"></div>
      </div>
    </div>
  </div>
</template>

<script>
  import moment from 'moment';
  import 'moment-timezone';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: { type: Object}, serie: { type: String, required: true }, data15M: { type: Object }, data1H: { type: Object }, dataGraph: { type: Object } },
    data () {
      return { CountersAggregation: 'Loading data...', MaxTime: '', MaxValue: '',
                CurrentValue15M: '', CurrentValue15MPhases: '',
                CurrentValue1H: '', CurrentValue1HPhases: '',
                TimeStatsFrom: '...', TimeStatsTo: '...'
              };
    },
    updated: function() {
      this.$emit('update:height', this.$el.clientHeight);
    },
    computed: {
      Sum: function() {
        var Sum = { Value: 0, Count: 0 };
        if(this.dataGraph == null) return Sum;
        var Divisor = (this.dataGraph.Aggregation_Source == '15M') ? 4 : 1;
        for(var x in this.dataGraph.Sampletime) {
          if((this.serie == 'ACTIVE_ENERGY') && (this.dataGraph.Active_Power[x] == null)) continue;
          if((this.serie == 'REACTIVE_ENERGY') && (this.dataGraph.Reactive_Power[x] == null)) continue;
          Sum.Count++;
          Sum.Value += this.dataGraph[(this.serie == 'ACTIVE_ENERGY') ? 'Active_Power' : 'Reactive_Power'][x] / Divisor;
        }
        return Sum;
      },
      Max: function() {
        var Max = { Value: -Infinity, Time: 0, Count: 0 };
        if(this.dataGraph == null) return Max;
        var Divisor = (this.dataGraph.Aggregation_Source == '15M') ? 4 : 1;
        for(var x in this.dataGraph.Sampletime) {
          if((this.serie == 'ACTIVE_ENERGY') && (this.dataGraph.Active_Power[x] == null)) continue;
          if((this.serie == 'REACTIVE_ENERGY') && (this.dataGraph.Reactive_Power[x] == null)) continue;
          var Value = this.dataGraph[(this.serie == 'ACTIVE_ENERGY') ? 'Active_Power' : 'Reactive_Power'][x] / Divisor;
          if(Value > Max.Value) {
            Max = { Value: Value, Time: this.dataGraph.Sampletime[x], Count: 0 };
          } else if(Value == Max) {
            Max.Count++;
          }
        }
        return Max;
      }
    },
    methods: {
      FormatTime: function(val) { return moment(val).tz(this.analyzer.TimeZone).format('HH:mm:ss'); },
      FormatDateTime: function(val) { return moment(val).tz(this.analyzer.TimeZone).format('hh:mm:ss - DD/MM/YYYY'); },
      FormatAggregation: function(val) { return 'From ' + oZm.GetAggregLongName(val) + ' aggregation'; },
      FormatHTML: function(val) {
        var Units = oZm.GetBestUnits(val);
        return oZm.RoundHTML(oZm.ScaleToUnits(val, Units), 2) + ' ' + Units + ((this.serie == 'ACTIVE_ENERGY') ? 'Wh' : 'VArh');
      },

      FormatPhases: function(values) {
        var Return = '';
        if(values.length > 1) {
          for(var n = 0; n < values.length; n++) {
            var Format = oZm.Series[this.variable].FormatValue(values[n]);
            Return += Format.Value + '<em>' + Format.Units +'</em><br>';
          }
        }
        return Return;
      },
      SumPhases: function(values) {
        var Sum = 0;
        for(var n = 0; n < values.length; n++) Sum += values[n];
        return Sum;
      },
      FormatAggregBlock(result, quarter) {
        var Variable = (this.variable == 'ACTIVE_ENERGY') ? 'ACTIVE_POWER' : 'REACTIVE_POWER';
        if(result === null) return ['', 'No data', ''];
        if(result['Sampletime'].length === 0) return ['', 'No data', ''];
        var Pos = result['Sampletime'].length - 1;
        var Time = 'At ' + moment(result.Sampletime[Pos]).format('HH:mm:ss');
        var Value = 'No data';
        var Phases = '';
        if(result[oZm.Series[Variable].Serie][Pos] !== null) {
          if(quarter) {
            for(var n = 0; n < result[oZm.Series[Variable].Serie][Pos].length; n++) result[oZm.Series[Variable].Serie][Pos][n] = result[oZm.Series[Variable].Serie][Pos][n] / 4.0;
          }
          Phases = this.FormatPhases(result[oZm.Series[Variable].Serie][Pos]);
          var Format = oZm.Series[this.variable].FormatValue(this.SumPhases(result[oZm.Series[Variable].Serie][Pos]));
          Value = Format.Value + '<em>' + Format.Units +'</em>';
        }
        return [Time, Value, Phases];
      },
      Set15MValues: function(result) {
        [this.Time15M, this.CurrentValue15M, this.CurrentValue15MPhases] = this.FormatAggregBlock(result, true);
      },
      Set1HValues: function(result) {
        [this.Time1H, this.CurrentValue1H, this.CurrentValue1HPhases] = this.FormatAggregBlock(result);
      },
    }
  }
</script>

<style scoped>
  .counters-phases {
    text-align: left;
    display: inline-block;
    font-size: 12px;
    line-height: 12px;
    font-weight: bold;
    padding-left: 2px;
    border-left: 1px solid #f0ad4e;
    margin-left: 5px;
  }
  .counters-title {
    float: left;
    text-align: left;
    font-size: 16px;
  }
  .counters-aggreg {
    float: right;
    text-align: right;
    font-size: 12px;
  }
  .counters-main-val {
    font-size: 36px
  }
  .counters-stat-type {
    text-align: left;
    font-weight: bold;
  }
  .counters-stat-time {
    font-size: 12px;
    text-align: left;
  }
  .counters-stat-val {
    font-size: 28px;
    line-height: 37px;
    float: right;
  }
  .current-value {
    font-size: 36px;
    display: inline-block;
  }
  .line {
    clear: both;
  }
  .line-border {
    border-top: 1px solid #c7c7e6;
  }
  .line-border-reactive {
    border-top: 1px solid #acadd6;
  }
</style>
