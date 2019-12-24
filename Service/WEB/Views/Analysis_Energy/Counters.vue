<template>
  <div class="col-md-6 col-lg-4 col-xl-3" ref="counters">
    <div class="subpanel">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg">From 15 minutes aggregation<br>{{ Time15M }}</div>
      <div class="clearfix"></div>
      <div class="current-value" v-html="CurrentValue15M"></div>
    </div>
    <div class="subpanel">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg">From 1 hour aggregation<br>{{ Time1H }}</div>
      <div class="clearfix"></div>
      <div class="current-value" v-html="CurrentValue1H"></div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Period stats:</div>
      <div class="counters-aggreg">{{ CountersAggregation }}</div>
      <div class="line" v-bind:class="{'line-border-reactive': (variable == 'REACTIVE_ENERGY')}">
        <div class="float-left">
          <div class="counters-stat-type">Max value:</div>
          <div class="counters-stat-time">{{ MaxTime }}</div>
        </div>
        <div class="counters-stat-val" v-html="MaxValue"></div>
      </div>
      <div class="line line-border" v-bind:class="{'line-border-reactive': (variable == 'REACTIVE_ENERGY')}">
        <div class="float-left">
          <div class="counters-stat-type">Total in period:</div>
          <div class="counters-stat-time">From {{ AccumSamples }} samples</div>
        </div>
        <div class="counters-stat-val" v-html="AccumValue"></div>
      </div>
    </div>
  </div>
</template>

<script>
  import ToolbarAggregation from 'Components/ToolbarAggregation.vue';
  import ToolbarPeriod from 'Components/ToolbarPeriod.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: ['variable'],
    data () {
      return { CountersAggregation: 'Loading data...', MaxTime: '', MaxValue: '',
                Time15M: '', CurrentValue15M: '', CurrentValue15MPhases: '', 
                Time1H: '', CurrentValue1H: '', CurrentValue1HPhases: '', 
                TimeStatsFrom: '...', TimeStatsTo: '...', AccumSamples: '', AccumValue: '?'
              };
    },
    methods: {
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
      SetStats: function(window, data, aggreg) {
        this.CountersAggregation = 'From ' + oZm.GetAggregLongName(aggreg) + ' aggregation';
        this.TimeStatsFrom = moment(window[0]).format('DD/MM/YYYY HH:mm:ss');
        this.TimeStatsTo = moment(window[1]).format('DD/MM/YYYY HH:mm:ss');
        var AccumValue = 0.0, MaxValue = -Infinity, AccumSamples = 0, MaxCount = 0, MaxTime = null;
        for(var x = 0; x < data.length; x++) {
          if(data[x][1] === null) continue;
          AccumSamples++;
          AccumValue += data[x][1];
          if(data[x][1] > MaxValue) {
            MaxValue = data[x][1];
            MaxCount = 0;
            MaxTime = data[x][0];
          } else if(data[x][1] === MaxValue) {
            MaxCount++;
          }
        }
        this.AccumSamples = AccumSamples;
        this.MaxTime = moment(MaxTime).format("hh:mm:ss - DD/MM/YYYY") + ((MaxCount > 1) ? ' (and ' + MaxCount + ' times more)' : '');
        var Format = oZm.Series[this.variable].FormatValue(AccumValue);
        this.AccumValue = Format.Value + ' <em>' + Format.Units +'</em>';
        var Format = oZm.Series[this.variable].FormatValue(MaxValue);
        this.MaxValue = Format.Value + ' <em>' + Format.Units +'</em>';
      }
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
