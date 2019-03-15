<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg">From 3s seconds aggregation<br>{{ Time3S }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-html="CurrentValue3S"></div>
        <div class="counters-phases" v-html="CurrentValue3SPhases" v-if="CurrentValue3SPhases != ''"></div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg">From 1 minute aggregation<br>{{ Time1M }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-html="CurrentValue1M"></div>
        <div class="counters-phases" v-html="CurrentValue1MPhases" v-if="CurrentValue1MPhases != ''"></div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Period stats:</div>
      <div class="counters-aggreg">{{ CountersAggregation }}<br>From {{ TimeStatsFrom }} to {{ TimeStatsTo }}</div>
      <div class="line">
        <div class="float-left">
          <div class="counters-stat-type">Min value:</div>
          <div class="counters-stat-time">{{ MinTime }}</div>
        </div>
        <div class="counters-stat-val" v-html="MinValue"></div>
      </div>
      <div class="line line-border" v-bind:class="{'line-border-reactive': (variable == 'REACTIVE_POWER')}">
        <div class="float-left">
          <div class="counters-stat-type">Max value:</div>
          <div class="counters-stat-time">{{ MaxTime }}</div>
        </div>
        <div class="counters-stat-val" v-html="MaxValue"></div>
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
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: ['variable'],
    data () {
      return { CountersAggregation: 'Loading data...', MinTime: '', MinValue: '', MaxTime: '', MaxValue: '',
               Time3S: '', CurrentValue3S: '', CurrentValue3SPhases: '', 
               Time1M: '', CurrentValue1M: '', CurrentValue1MPhases: '', 
               TimeStatsFrom: '...', TimeStatsTo: '...'
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
      FormatAggregBlock(result) {
        if(result === null) return ['', 'No data', ''];
        
        if(result['Sampletime'].length === 0) return ['', 'No data', ''];
        var Pos = result['Sampletime'].length - 1;
        var Time = 'At ' + moment(result.Sampletime[Pos]).format('HH:mm:ss');
        var Value = 'No data';
        var Phases = '';
        if(result[oZm.Series[this.variable].Serie][Pos] !== null) {
          Phases = this.FormatPhases(result[oZm.Series[this.variable].Serie][Pos]);
          var Format = oZm.Series[this.variable].FormatValue(this.SumPhases(result[oZm.Series[this.variable].Serie][Pos]));
          Value = Format.Value + '<em>' + Format.Units +'</em>';
        }
        return [Time, Value, Phases];
      },
      Set3SValues: function(result) {
        [this.Time3S, this.CurrentValue3S, this.CurrentValue3SPhases] = this.FormatAggregBlock(result);
      },
      Set1MValues: function(result) {
          [this.Time1M, this.CurrentValue1M, this.CurrentValue1MPhases] = this.FormatAggregBlock(result);
      },
      SetStats: function(window, data, aggreg) {
        this.CountersAggregation = 'From ' + oZm.GetAggregLongName(aggreg) + ' aggregation';
        this.TimeStatsFrom = moment(window[0]).format('DD/MM/YYYY HH:mm:ss');
        this.TimeStatsTo = moment(window[1]).format('DD/MM/YYYY HH:mm:ss');
        var MinValue = +Infinity, MaxValue = -Infinity, MinCount = 0, MaxCount = 0, MinTime = null, MaxTime = null;
        for(var x = 0; x < data.length; x++) {
          if(data[x][1] === null) continue;
          if(data[x][1] < MinValue) {
            MinValue = data[x][1];
            MinCount = 0;
            MinTime = data[x][0];
          } else if(data[x][1] === MinValue) {
            MinCount++;
          }
          if(data[x][1] > MaxValue) {
            MaxValue = data[x][1];
            MaxCount = 0;
            MaxTime = data[x][0];
          } else if(data[x][1] === MaxValue) {
            MaxCount++;
          }
        }
        this.MinTime = moment(MinTime).format("hh:mm:ss - DD/MM/YYYY") + ((MinCount > 1) ? ' (and ' + MinCount + ' times more)' : '');
        this.MaxTime = moment(MaxTime).format("hh:mm:ss - DD/MM/YYYY") + ((MaxCount > 1) ? ' (and ' + MaxCount + ' times more)' : '');
        var Format = oZm.Series[this.variable].FormatValue(MinValue);
        this.MinValue = Format.Value + ' <em>' + Format.Units +'</em>';
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
    border-top: 1px solid #FAE7CC;
  }
  .line-border-reactive {
    border-top: 1px solid #F5DBC2
  }
</style>