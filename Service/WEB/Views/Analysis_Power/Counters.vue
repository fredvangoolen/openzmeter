<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg" v-if="data3S!=null">From 3 seconds aggregation<br>{{ analyzer.FormatTime(data3S.Sampletime) }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-html="(data3S!=null) ? FormatSum(data3S) : '-'"></div>
        <div class="counters-phases" v-if="(data3S!=null)&&(data3S.Active_Power.length>1)">
          <div v-for="(phase, idx) in FormatPhases(data3S)" :key="idx" v-html="phase"></div>
        </div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg" v-if="data1M!=null">From 1 minute aggregation<br>{{ analyzer.FormatTime(data1M.Sampletime) }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-html="(data1M!=null) ? FormatSum(data1M) : '-'"></div>
        <div class="counters-phases" v-if="(data1M!=null)&&(data1M.Active_Power.length>1)">
          <div v-for="(phase, idx) in FormatPhases(data1M)" :key="idx" v-html="phase"></div>
        </div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Period stats:</div>
      <div class="counters-aggreg" v-if="dataGraph!=null">From {{ analyzer.FormatAggregation(dataGraph.Aggreg) }} aggregation<br>From {{ analyzer.FormatDateTime(dataGraph.From) }} to {{ analyzer.FormatDateTime(dataGraph.To) }}</div>
      <div class="line">
        <div class="float-left">
          <div class="counters-stat-type">Min value:</div>
          <div class="counters-stat-time">{{ (Min.Time!=0) ? analyzer.FormatDateTime(Min.Time) : '-' }}</div>
        </div>
        <div class="counters-stat-val" v-html="(Min.Time!=0) ? FormatHTML(Min.Value) : '-'"></div>
      </div>
      <div class="line line-border">
        <div class="float-left">
          <div class="counters-stat-type">Max value:</div>
          <div class="counters-stat-time">{{ (Max.Time!=0) ? analyzer.FormatDateTime(Max.Time) : '-' }}</div>
        </div>
        <div class="counters-stat-val" v-html="(Max.Time!=0) ? FormatHTML(Max.Value) : '-'"></div>
      </div>
    </div>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: { type: Object }, serie: { type: String, required: true }, data3S: { type: Object }, data1M: { type: Object }, dataGraph: { type: Object } },
    data: function() {
      return { };
    },
    updated: function() {
      this.$emit('update:height', this.$el.clientHeight);
    },
    computed: {
      Max: function() {
        var Max = { Value: -Infinity, Time: 0, Count: 0 };
        var Value = 0;
        if(this.dataGraph == null) return Max;
        for(var x in this.dataGraph.Sampletime) {
          if((this.serie == 'Active_Power') && (this.dataGraph.Active_Power[x] == null)) continue;
          if((this.serie == 'Reactive_Power') && (this.dataGraph.Reactive_Power[x] == null)) continue;
          if((this.serie == 'Apparent_Power') && ((this.dataGraph.Active_Power[x] == null) || (this.dataGraph.Reactive_Power[x] == null))) continue;
          if(this.serie == 'Active_Power') Value = this.dataGraph.Active_Power[x];
          if(this.serie == 'Reactive_Power') Value = this.dataGraph.Reactive_Power[x];
          if(this.serie == 'Apparent_Power') Value = Math.sqrt(Math.pow(this.dataGraph.Active_Power[x], 2.0) + Math.pow(this.dataGraph.Reactive_Power[x], 2.0));
          if(Value > Max.Value) {
            Max = { Value: Value, Time: this.dataGraph.Sampletime[x], Count: 0 };
          } else if(Value == Max.Value) {
            Max.Count++;
          }
        }
        return Max;
      },
      Min: function() {
        var Min = { Value: +Infinity, Time: 0, Count: 0 };
        var Value = 0;
        if(this.dataGraph == null) return Min;
        for(var x in this.dataGraph.Sampletime) {
          if((this.serie == 'Active_Power') && (this.dataGraph.Active_Power[x] == null)) continue;
          if((this.serie == 'Reactive_Power') && (this.dataGraph.Reactive_Power[x] == null)) continue;
          if((this.serie == 'Apparent_Power') && ((this.dataGraph.Active_Power[x] == null) || (this.dataGraph.Reactive_Power[x] == null))) continue;
          if(this.serie == 'Active_Power') Value = this.dataGraph.Active_Power[x];
          if(this.serie == 'Reactive_Power') Value = this.dataGraph.Reactive_Power[x];
          if(this.serie == 'Apparent_Power') Value = Math.sqrt(Math.pow(this.dataGraph.Active_Power[x], 2.0) + Math.pow(this.dataGraph.Reactive_Power[x], 2.0));
          if(Value < Min.Value) {
            Min = { Value: Value, Time: this.dataGraph.Sampletime[x], Count: 0 };
          } else if(Value == Min.Value) {
            Min.Count++;
          }
        }
        return Min;
      }
    },
    methods: {
      FormatHTML: function(val) {
        var Units = this.BestUnits(val);
        return oZm.RoundHTML(oZm.ScaleToUnits(val, Units), 2) + ' ' + Units + ((this.serie == 'Active_Power') ? 'W' : (this.serie == 'Reactive_Power') ? 'VAr' : 'VA');
      },
      FormatSum: function(val) {
        var Sum = 0;
        for(var n in val['Active_Power']) Sum = Sum + (this.serie == 'Active_Power') ? val.Active_Power[n] : (this.serie == 'Reactive_Power') ? val.Reactive_Power[n] : Math.sqrt(Math.pow(val.Active_Power[n], 2.0) + Math.pow(val.Reactive_Power[n], 2.0));
        return this.FormatHTML(Sum);
      },
      FormatPhases: function(values) {
        var Return = '';
        if(values.length > 1) {
          for(var n = 0; n < values.length; n++) {
            var Format = oZm.Series[this.variable].FormatValue(values[n]);
            Return += Format.Value + Format.Units +'<br>';
          }
        }
        return Return;
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
    border-left: 1px solid #FAE7CC;
    margin-left: 5px;
  }
  .panel-reactive-power .counter-phases {
    border-left: 1px solid #F5DBC2;
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
  .panel-reactive-power .line-border {
    border-top: 1px solid #F5DBC2
  }
</style>