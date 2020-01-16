<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg" v-if="data10M!=null">From 10 minutes aggregation<br>{{ analyzer.FormatTime(data10M.Sampletime) }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-html="(data10M!=null) ? FormatRMS(data10M) : '-'"></div>
        <div class="counters-phases" v-if="(data10M!=null)&&(data10M.Voltage.length>1)">
          <div v-for="(phase, idx) in FormatPhases(data10M)" :key="idx" v-html="phase"></div>
        </div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Period stats:</div>
      <div class="counters-aggreg" v-if="dataGraph!=null">From {{ analyzer.FormatAggregation(dataGraph.Aggregation_Source) }} aggregation<br>From {{ analyzer.FormatDateTime(dataGraph.From) }} to {{ analyzer.FormatDateTime(dataGraph.To) }}</div>
      <div class="line">
        <div class="float-left">
          <div class="counters-stat-type">Min value:</div>
          <div class="counters-stat-time">{{ (Min.Time!=0) ? analyzer.FormatDateTime(Min.Time) : '-' }}<span v-if="Min.Count > 1">(and {{ Min.Count }} times more)</span></div>
        </div>
        <div class="counters-stat-val" :class="{ 'counters-stat-val2': (Min.Value.length > 1) }">
          <div v-for="(phase, idx) in Min.Value" :key="idx" v-html="isFinite(phase) ? FormatHTML(phase) : '-'"></div>
        </div>
      </div>
      <div class="line line-border">
        <div class="float-left">
          <div class="counters-stat-type">Max value:</div>
          <div class="counters-stat-time">{{ (Max.Time!=0) ? analyzer.FormatDateTime(Max.Time) : '-' }}<span v-if="Max.Count > 1">(and {{ Max.Count }} times more)</span></div>
        </div>
        <div class="counters-stat-val" :class="{ 'counters-stat-val2': (Max.Value.length>1) }">
          <div v-for="(phase, idx) in Max.Value" :key="idx" v-html="isFinite(phase) ? FormatHTML(phase) : '-'"></div>
        </div>
      </div>
      <div class="line line-border">
        <div class="float-left">
          <div class="counters-stat-type">Average value:</div>
          <div class="counters-stat-time">From {{ Avg.Count }} samples</div>
        </div>
        <div class="counters-stat-val" :class="{ 'counters-stat-val2': (Avg.Value.length > 1) }">
          <div v-for="(phase, idx) in Avg.Value" :key="idx" v-html="isFinite(phase) ? FormatHTML(phase) : '-'"></div>
        </div>
      </div>
      <div class="line line-border">
        <div class="float-left">
          <div class="counters-stat-type">Std deviation:</div>
          <div class="counters-stat-time" v-html="isFinite(Std.Percent) ? FormatHTML(Std.Percent) : '-'"></div>
        </div>
        <div class="counters-stat-val" :class="{ 'counters-stat-val2': (Std.Value.length > 1) }">
          <div v-for="(phase, idx) in Std.Value" :key="idx" v-html="isFinite(phase) ? FormatHTML(phase) : '-'"></div></div>
      </div>
    </div>
  </div>
</template>

<script>
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object, data10M: Object, dataGraph: null },
    data () {
      return { };
    },
    updated: function() {
      this.$emit('update:height', this.$el.clientHeight);
    },
    computed: {
      Min: function() {
        var Val = +Infinity;
        var Min = { Value: [], Time: 0, Count: 0 };
        if(this.dataGraph == null) return Min;
        for(var x in this.dataGraph.Sampletime) {
          for(var p in this.dataGraph.Voltage[x]) {
            if(this.dataGraph.Voltage[x][p] == null) continue;
            if(this.dataGraph.Voltage[x][p] < Val) {
              Val = this.dataGraph.Voltage[x][p];
              Min = { Value: this.dataGraph.Voltage[x], Time: this.dataGraph.Sampletime[x], Count: 0 };
            } else if(this.dataGraph.Voltage[x][p] == Val) {
              Min.Count++;
            }
          }
        }
        return Min;
      },
      Max: function() {
        var Val = -Infinity;
        var Max = { Value: [], Time: 0, Count: 0 };
        if(this.dataGraph == null) return Max;
        for(var x in this.dataGraph.Sampletime) {
          for(var p in this.dataGraph.Voltage[x]) {
            if(this.dataGraph.Voltage[x][p] == null) continue;
            if(this.dataGraph.Voltage[x][p] > Val) {
              Val = this.dataGraph.Voltage[x][p];
              Max = { Value: this.dataGraph.Voltage[x], Time: this.dataGraph.Sampletime[x], Count: 0 };
            } else if(this.dataGraph.Voltage[x][p] == Val) {
              Max.Count++;
            }
          }
        }
        return Max;
      },
      Avg: function() {
        var Avg = { Value: [], Count: 0 };
        if(this.dataGraph == null) return Avg;
        for(var x in this.dataGraph.Sampletime) {
          if(Avg.Value.length == 0) Avg.Value = Array(this.dataGraph.Voltage[x].length).fill(0);
          for(var p in this.dataGraph.Voltage[x]) Avg.Value[p] = Avg.Value[p] + this.dataGraph.Voltage[x][p];
          Avg.Count++;
        }
        for(var p in this.dataGraph.Voltage[x]) Avg.Value[p] = Avg.Value[p] / Avg.Count;
        return Avg;
      },
      Std: function() {
        var Std = { Value: [], Percent: 0 };
        var Avg = [], Count = 0;
        if(this.dataGraph == null) return Std;
        for(var x in this.dataGraph.Sampletime) {
          if(Std.Value.length == 0) {
            Avg = Array(this.dataGraph.Voltage[x].length).fill(0);
            Std.Value = Array(this.dataGraph.Voltage[x].length).fill(0);
          }
          for(var p in this.dataGraph.Voltage[x]) {
            if((this.dataGraph.Voltage[x][p] > this.analyzer.NominalVoltage * 0.9) && (this.dataGraph.Voltage[x][p] < this.analyzer.NominalVoltage * 1.1)) Std.Percent++;
            Std.Value[p] += Math.pow(this.dataGraph.Voltage[x][p], 2.0);
            Avg[p] += this.dataGraph.Voltage[x][p];
          }
          Count++;
        }
        for(var p in this.dataGraph.Voltage[x]) {
          Std.Value[p] = Math.sqrt((Std.Value[p] / Count) - Math.pow(Avg[p] / Count, 2));
          Std.Percent = Std.Percent / (Count * 100 * Std.Value.length);
        }
        return Std;
      }
    },
    methods: {
      FormatHTML: function(val) {
        var Units = oZm.GetBestUnits(val);
        return this.Round(oZm.ScaleToUnits(val, Units), 2, true) + ' ' + Units + 'V';
      },
      FormatRMS: function(val) {
        var Sum = 0;
        for(var n in val.Voltage) Sum = Sum + Math.pow(val.Voltage[n], 2.0);
        return this.FormatHTML(Math.sqrt(Sum / val.Voltage.length));
      },
      FormatPhases: function(val) {
        var Return = [];
        for(var n in val.Voltage.length) Return.push(this.FormatHTML(val.Voltage[n]));
        return Return;
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
  .counters-stat-val2 {
    font-size: 12px !important;
    font-weight: bold;
    line-height: 12px;
  }
  .current-value {
    font-size: 30px;
    display: inline-block;
    margin: 0px 10px
  }
  .line {
    clear: both;
  }
  .line-border {
    border-top: 1px solid #efbab8;
  }
  .counters-phases {
    text-align: left;
    display: inline-block;
    font-size: 12px;
    line-height: 12px;
    font-weight: bold;
    padding-left: 2px;
    border-left: 1px solid #efbab8;
    margin-left: 5px;
  }
</style>
