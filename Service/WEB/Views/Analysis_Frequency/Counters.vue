<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Current value:</div>
<<<<<<< HEAD
      <div class="counters-aggreg" v-if="dataNow!=null">From 3 seconds aggregation<br>{{ FormatTime(dataNow.Sampletime) }}</div>
        <div class="clearfix"></div>
        <div class="current-value" v-html="dataNow!=null ? FormatHTML(dataNow.Frequency) + ' Hz' : 'No data'"></div>
      </div>
      <div class="subpanel clearfix">
        <div class="counters-title">Period stats:</div>
        <div class="counters-aggreg" v-if="dataGraph != null">{{ FormatAggregation(dataGraph.Aggregation_Source) }}<br>From {{ FormatDateTime(dataGraph.From) }} to {{ FormatDateTime(dataGraph.To) }}</div>
        <div class="line">
          <div class="float-left">
            <div class="counters-stat-type">Min value:</div>
            <div class="counters-stat-time">{{ isFinite(Min.Value) ? FormatDateTime(Min.Time) : '-' }}<span v-if="Min.Count > 1">(and {{ Min.Count }} times more)</span></div>
          </div>
          <div class="counters-stat-val" v-html="isFinite(Min.Value) ? FormatHTML(Min.Value) + ' Hz' : '-'"></div>
=======
      <div class="counters-aggreg" v-if="dataNow!=null">From 3 seconds aggregation<br>{{ analyzer.FormatTime(dataNow.Sampletime) }}</div>
        <div class="clearfix"></div>
        <div class="current-value" v-html="dataNow!=null ? Round(dataNow.Frequency, 3, true) + ' Hz' : 'No data'"></div>
      </div>
      <div class="subpanel clearfix">
        <div class="counters-title">Period stats:</div>
        <div class="counters-aggreg" v-if="dataGraph!=null">{{ analyzer.FormatAggregation(dataGraph.Aggregation_Source) }}<br>From {{ analyzer.FormatDateTime(dataGraph.From) }} to {{ analyzer.FormatDateTime(dataGraph.To) }}</div>
        <div class="line">
          <div class="float-left">
            <div class="counters-stat-type">Min value:</div>
            <div class="counters-stat-time">{{ (Min.Time!=0) ? analyzer.FormatDateTime(Min.Time) : '-' }}<span v-if="Min.Count > 1">(and {{ Min.Count }} times more)</span></div>
          </div>
          <div class="counters-stat-val" v-html="(Min.Time!=0) ? Round(Min.Value, 3, true) + ' Hz' : '-'"></div>
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
        </div>
        <div class="line line-border">
          <div class="float-left">
            <div class="counters-stat-type">Max value:</div>
<<<<<<< HEAD
            <div class="counters-stat-time">{{ isFinite(Max.Value) ? FormatDateTime(Max.Time) : '-'}}<span v-if="Max.Count > 1">(and {{ Max.Count }} times more)</span></div>
          </div>
          <div class="counters-stat-val" v-html="isFinite(Max.Value) ? FormatHTML(Max.Value) + ' Hz' : '-'"></div>
=======
            <div class="counters-stat-time">{{ (Max.Time!=0) ? analyzer.FormatDateTime(Max.Time) : '-'}}<span v-if="Max.Count > 1">(and {{ Max.Count }} times more)</span></div>
          </div>
          <div class="counters-stat-val" v-html="(Max.Time!=0) ? Round(Max.Value, 3, true) + ' Hz' : '-'"></div>
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
        </div>
        <div class="line line-border">
          <div class="float-left">
            <div class="counters-stat-type">Average value:</div>
<<<<<<< HEAD
            <div class="counters-stat-time">From {{ isFinite(Avg.Value) ? Avg.Count : '-' }} samples</div>
          </div>
          <div class="counters-stat-val" v-html="isFinite(Avg.Value) ? FormatHTML(Avg.Value) + ' Hz' : '-'"></div>
=======
            <div class="counters-stat-time">From {{ Avg.Count }} samples</div>
          </div>
          <div class="counters-stat-val" v-html="(Avg.Count>0) ? Round(Avg.Value, 3, true) + ' Hz' : '-'"></div>
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
        </div>
        <div class="line line-border">
          <div class="float-left">
            <div class="counters-stat-type">Std deviation:</div>
<<<<<<< HEAD
            <div class="counters-stat-time">{{ (isFinite(Std.Value) ? FormatPlain(Std.Percent) : '-') + '% within range' }}</div>
          </div>
          <div class="counters-stat-val" v-html="isFinite(Std.Value) ? FormatHTML(Std.Value) + ' Hz' : '-'"></div>
=======
            <div class="counters-stat-time">{{ isFinite(Std.Value) ? Round(Std.Percent, 2, false) : '-' }}% within range</div>
          </div>
          <div class="counters-stat-val" v-html="isFinite(Std.Value) ? Round(Std.Value, 3, true) + ' Hz' : '-'"></div>
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
        </div>
      </div>
  </div>
</template>

<script>
<<<<<<< HEAD
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
=======
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
  export default {
    props: { analyzer: Object, dataNow: Object, dataGraph: Object },
    data() {
      return { };
    },
    updated: function() {
      this.$emit('update:height', this.$el.clientHeight);
    },
    computed: {
      Min: function() {
        var Min = { Value: +Infinity, Time: 0, Count: 0 };
        if(this.dataGraph == null) return Min;
<<<<<<< HEAD
        for(var x in this.dataGraph.Frequency) {
          if(this.dataGraph.Frequency[x] === null) continue;
          if(this.dataGraph.Frequency[x] < Min.Value) {
            Min = { Value: this.dataGraph.Frequency[x], Time: this.dataGraph.Sampletime[x], Count: 0 };
          } else if(this.dataGraph.Frequency[x] === Min) {
=======
        for(var x in this.dataGraph.Sampletime) {
          if(this.dataGraph.Frequency[x] == null) continue;
          if(this.dataGraph.Frequency[x] < Min.Value) {
            Min = { Value: this.dataGraph.Frequency[x], Time: this.dataGraph.Sampletime[x], Count: 0 };
          } else if(this.dataGraph.Frequency[x] == Min) {
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
            Min.Count++;
          }
        }
        return Min;
      },
      Max: function() {
        var Max = { Value: -Infinity, Time: 0, Count: 0 };
        if(this.dataGraph == null) return Max;
<<<<<<< HEAD
        for(var x in this.dataGraph.Frequency) {
          if(this.dataGraph.Frequency[x] === null) continue;
          if(this.dataGraph.Frequency[x] > Max.Value) {
            Max = { Value: this.dataGraph.Frequency[x], Time: this.dataGraph.Sampletime[x], Count: 0 };
          } else if(this.dataGraph.Frequency[x] === Max) {
=======
        for(var x in this.dataGraph.Sampletime) {
          if(this.dataGraph.Frequency[x] == null) continue;
          if(this.dataGraph.Frequency[x] > Max.Value) {
            Max = { Value: this.dataGraph.Frequency[x], Time: this.dataGraph.Sampletime[x], Count: 0 };
          } else if(this.dataGraph.Frequency[x] == Max) {
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
            Max.Count++;
          }
        }
        return Max;
      },
      Std: function() {
<<<<<<< HEAD
        var Std = { Value: 0, Percent: 0 };
        var Avg = 0, Count = 0;
        if(this.dataGraph == null) return Std;
        for(var x in this.dataGraph.Frequency) {
          if(this.dataGraph.Frequency[x] === null) continue;
          if((this.dataGraph.Frequency[x] > this.analyzer.NominalFreq * 0.99) && (this.dataGraph.Frequency[x] < this.analyzer.NominalFreq * 1.01)) Std.Percent++;
          Std.Value += Math.pow(this.dataGraph.Frequency[x], 2.0);
          Avg += this.dataGraph.Frequency[x];
          Count++;
        }
        Std.Value = Math.sqrt((Std.Value / Count) - Math.pow(Avg / Count, 2))
        Std.Percent = Std.Percent / Count * 100;
=======
        var Std = { Value: 0, Percent: 0, Count: 0 };
        var Avg = 0;
        if(this.dataGraph == null) return Std;
        for(var x in this.dataGraph.Sampletime) {
          if(this.dataGraph.Frequency[x] == null) continue;
          if((this.dataGraph.Frequency[x] > this.analyzer.NominalFreq * 0.99) && (this.dataGraph.Frequency[x] < this.analyzer.NominalFreq * 1.01)) Std.Percent++;
          Std.Value += Math.pow(this.dataGraph.Frequency[x], 2.0);
          Avg += this.dataGraph.Frequency[x];
          Std.Count++;
        }
        Std.Value = Math.sqrt((Std.Value / Std.Count) - Math.pow(Avg / Std.Count, 2));
        Std.Percent = Std.Percent / Std.Count * 100;
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
        return Std;
      },
      Avg: function() {
        var Avg = { Value: 0, Count: 0 };
        if(this.dataGraph == null) return Avg;
<<<<<<< HEAD
        for(var x in this.dataGraph.Frequency) {
          if(this.dataGraph.Frequency[x] === null) continue;
=======
        for(var x in this.dataGraph.Sampletime) {
          if(this.dataGraph.Frequency[x] == null) continue;
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
          Avg.Count++;
          Avg.Value += this.dataGraph.Frequency[x];
        }
        Avg.Value = Avg.Value / Avg.Count;
        return Avg;
       }
<<<<<<< HEAD
    },
    methods: {
      FormatTime: function(val) { return moment(val).tz(this.analyzer.TimeZone).format('HH:mm:ss'); },
      FormatDateTime: function(val) { return moment(val).tz(this.analyzer.TimeZone).format('hh:mm:ss - DD/MM/YYYY'); },
      FormatAggregation: function(val) { return 'From ' + oZm.GetAggregLongName(val) + ' aggregation'; },
      FormatHTML: function(val) { return oZm.RoundHTML(val, 3); },
      FormatPlain: function(val) { return oZm.RoundPlain(val, 2); },
=======
>>>>>>> eda5f001a831b33a1f525854c29e15e34550c1e4
    }
  }
</script>

<style scoped>
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
  }
  .line {
    clear: both;
  }
  .line-border {
    border-top: 1px solid #b3e0b3;
  }
</style>