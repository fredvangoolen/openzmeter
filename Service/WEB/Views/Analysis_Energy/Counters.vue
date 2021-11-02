<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg" v-if="data15M!=null">From 15 minutes aggregation<br>{{ analyzer.FormatTime(data15M.Sampletime) }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-html="(data15M!=null) ? FormatSum(data15M) : '-'"></div>
        <div class="counters-phases" v-if="(data15M!=null)&&(data15M.Active_Energy.length>1)">
          <div v-for="(phase, idx) in FormatPhases(data15M)" :key="idx" v-html="phase"></div>
        </div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Last value:</div>
      <div class="counters-aggreg" v-if="data1H!=null">From 1 hour aggregation<br>{{ analyzer.FormatTime(data1H.Sampletime) }}</div>
      <div class="clearfix"></div>
      <div>
        <div class="current-value" v-html="(data1H!=null) ? FormatSum(data1H) : '-'"></div>
        <div class="counters-phases" v-if="(data1H!=null)&&(data1H.Active_Energy.length>1)">
          <div v-for="(phase, idx) in FormatPhases(data1H)" :key="idx" v-html="phase"></div>
        </div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Period stats:</div>
      <div class="counters-aggreg" v-if="dataGraph!=null">From {{ analyzer.FormatAggregation(dataGraph.Aggreg) }} aggregation<br>From {{ analyzer.FormatDateTime(dataGraph.From) }} to {{ analyzer.FormatDateTime(dataGraph.To) }}</div>
      <div class="line">
        <div class="float-left">
          <div class="counters-stat-type">Max value:</div>
          <div class="counters-stat-time">{{ isFinite(Max.Value) ? analyzer.FormatDateTime(Max.Time) : '-' }}</div>
        </div>
        <div class="counters-stat-val" v-html="isFinite(Max.Value) ? FormatHTML(Max.Value) : '-'"></div>
      </div>
      <div class="line line-border">
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
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { serie: String, data15M: Object, data1H: Object, dataGraph: Object },
    data: function() {
      return { };
    },
    updated: function() {
      this.$emit('update:height', this.$el.clientHeight - 14);
    },
    computed: {
      Sum: function() {
        var Sum = { Value: 0, Count: 0 };
        if((this.dataGraph == null) || (this.dataGraph.Rows.length == 0)) return Sum;
        var TimeCol = this.dataGraph.Headers.indexOf('Time');
        var ValueCol = this.dataGraph.Headers.indexOf(this.serie);
        for(var x = 0; x < this.dataGraph.Rows.length; x++) {
          if((this.dataGraph.Rows[x][ValueCol] == null)) continue;
          Sum.Count++;
          Sum.Value += this.dataGraph.Rows[x][ValueCol];
        }
        return Sum;
      },
      Max: function() {
        var Max = { Value: -Infinity, Time: 0, Count: 0 };
        if((this.dataGraph == null) || (this.dataGraph.Rows.length == 0)) return Max;
        var TimeCol = this.dataGraph.Headers.indexOf('Time');
        var ValueCol = this.dataGraph.Headers.indexOf(this.serie);
        for(var x = 0; x < this.dataGraph.Rows.length; x++) {
          if((this.dataGraph.Rows[x][ValueCol] == null)) continue;
          var Value = this.dataGraph.Rows[x][ValueCol];
          if(Value > Max.Value) {
            Max = { Value: Value, Time: this.dataGraph.Rows[x][TimeCol], Count: 0 };
          } else if(Value == Max.Value) {
            Max.Count++;
          }
        }
        return Max;
      }
    },
    methods: {
      FormatHTML: function(val) {
        var Units = this.BestUnits(val);
        return oZm.RoundHTML(this.ScaleToUnits(val, Units), 2) + ' ' + Units + ((this.serie == 'Active_Energy') ? 'Wh' : 'VArh');
      },
      FormatSum: function(val) {
        var Sum = 0;
        for(var n = 0; n < this.analyzer.VoltageChannels; n++) Sum = Sum + val[this.serie][n];
        return this.FormatHTML(Sum);
      },
      FormatPhases: function(val, div) {
        var Units = this.BestUnits(this.ArrayAbsMax(val[this.serie]));
        var Result = [];
        for(var n = 0; n < this.analyzer.VoltageChannels; n++) Result.push(this.FormatHTML(val[this.serie][n]));
        return Result;
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
    border-left: 1px solid #c7c7e6;
    margin-left: 5px;
  }
  .panel-reactive-energy .counters-phases {
    border-left: 1px solid #acadd6;
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
  .panel-reactive-energy .line-border {
    border-top: 1px solid #acadd6;
  }
</style>
