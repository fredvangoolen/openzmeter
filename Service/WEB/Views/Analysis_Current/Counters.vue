<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg" v-if="data3S!=null">From 3 seconds aggregation<br>{{ analyzer.FormatTime(data3S.Sampletime) }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-html="(data3S!=null) ? FormatRMS(data3S) : '-'"></div>
        <div class="counters-phases" v-if="(data3S!=null)&&(data3S.Current.length>1)">
          <div v-for="(phase, idx) in FormatPhases(data3S)" :key="idx" v-html="phase"></div>
        </div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg" v-if="data1M!=null">From 1 minute aggregation<br>{{ analyzer.FormatTime(data1M.Sampletime) }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-html="(data1M!=null) ? FormatRMS(data1M) : '-'"></div>
        <div class="counters-phases" v-if="(data1M!=null)&&(data1M.Current.length>1)">
          <div v-for="(phase, idx) in FormatPhases(data1M)" :key="idx" v-html="phase"></div>
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
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object, data1M: Object, data3S: Object, dataGraph: Object },
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
          for(var p in this.dataGraph.Current[x]) {
            if(this.dataGraph.Current[x][p] == null) continue;
            if(this.dataGraph.Current[x][p] < Val) {
              Val = this.dataGraph.Current[x][p];
              Min = { Value: this.dataGraph.Current[x], Time: this.dataGraph.Sampletime[x], Count: 0 };
            } else if(this.dataGraph.Current[x][p] == Val) {
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
          for(var p in this.dataGraph.Current[x]) {
            if(this.dataGraph.Current[x][p] == null) continue;
            if(this.dataGraph.Current[x][p] > Val) {
              Val = this.dataGraph.Current[x][p];
              Max = { Value: this.dataGraph.Current[x], Time: this.dataGraph.Sampletime[x], Count: 0 };
            } else if(this.dataGraph.Current[x][p] == Val) {
              Max.Count++;
            }
          }
        }
        return Max;
      },
    },
    methods: {
      FormatHTML: function(val) {
        var Units = oZm.GetBestUnits(val);
        return this.Round(oZm.ScaleToUnits(val, Units), 2, true) + ' ' + Units + 'A';
      },
      FormatRMS: function(val) {
        var Sum = 0;
        for(var n in val.Current) Sum = Sum + Math.pow(val.Current[n], 2.0);
        return this.FormatHTML(Math.sqrt(Sum / val.Current.length));
      },
      FormatPhases: function(val) {
        var Return = [];
        for(var n in val.Current.length) Return.push(this.FormatHTML(val.Current[n]));
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
    border-top: 1px solid #cce0f1;
  }
  .counters-phases {
    text-align: left;
    display: inline-block;
    font-size: 12px;
    line-height: 12px;
    font-weight: bold;
    padding-left: 2px;
    border-left: 1px solid #cce0f1;
    margin-left: 5px;
  }
</style>
