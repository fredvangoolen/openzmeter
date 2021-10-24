<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Current value:</div>
      <div class="counters-aggreg" v-if="dataNow!=null">From 3 seconds aggregation<br>{{ analyzer.FormatTime(dataNow.Sampletime) }}</div>
        <div class="clearfix"></div>
        <div class="current-value" v-html="dataNow!=null ? Round(dataNow.Frequency, 3, true) + ' Hz' : 'No data'"></div>
      </div>
      <div class="subpanel clearfix">
        <div class="counters-title">Period stats:</div>
        <div class="counters-aggreg" v-if="dataGraph!=null">From {{ analyzer.FormatAggregation(dataGraph.Aggreg) }} aggregation<br>From {{ analyzer.FormatDateTime(dataGraph.From) }} to {{ analyzer.FormatDateTime(dataGraph.To) }}</div>
        <div class="line">
          <div class="float-left">
            <div class="counters-stat-type">Min value:</div>
            <div class="counters-stat-time">{{ (Min.Time!=0) ? analyzer.FormatDateTime(Min.Time) : '-' }}<span v-if="Min.Count > 1">(and {{ Min.Count }} times more)</span></div>
          </div>
          <div class="counters-stat-val" v-html="(Min.Time!=0) ? Round(Min.Value, 3, true) + ' Hz' : '-'"></div>
        </div>
        <div class="line line-border">
          <div class="float-left">
            <div class="counters-stat-type">Max value:</div>
            <div class="counters-stat-time">{{ (Max.Time!=0) ? analyzer.FormatDateTime(Max.Time) : '-'}}<span v-if="Max.Count > 1">(and {{ Max.Count }} times more)</span></div>
          </div>
          <div class="counters-stat-val" v-html="(Max.Time!=0) ? Round(Max.Value, 3, true) + ' Hz' : '-'"></div>
        </div>
        <div class="line line-border">
          <div class="float-left">
            <div class="counters-stat-type">Average value:</div>
            <div class="counters-stat-time">From {{ Avg.Count }} samples</div>
          </div>
          <div class="counters-stat-val" v-html="(Avg.Count>0) ? Round(Avg.Value, 3, true) + ' Hz' : '-'"></div>
        </div>
        <div class="line line-border">
          <div class="float-left">
            <div class="counters-stat-type">Std deviation:</div>
            <div class="counters-stat-time">{{ isFinite(Std.Value) ? Round(Std.Percent, 2, false) : '-' }}% within range</div>
          </div>
          <div class="counters-stat-val" v-html="isFinite(Std.Value) ? Round(Std.Value, 3, true) + ' Hz' : '-'"></div>
        </div>
      </div>
  </div>
</template>

<script>
  export default {
    props: { dataNow: Object, dataGraph: Object },
    data: function() {
      return { };
    },
    updated: function() {
      this.$emit('update:height', this.$el.clientHeight - 10);
    },
    computed: {
      Min: function() {
        var Min = { Value: +Infinity, Time: 0, Count: 0 };
        if((this.dataGraph == null) || (this.dataGraph.Rows.length == 0)) return Min;
        var TimeCol = this.dataGraph.Headers.indexOf('Time');
        var ValueCol = this.dataGraph.Headers.indexOf('Frequency');
        for(var x = 0; x < this.dataGraph.Rows.length; x++) {
          if(this.dataGraph.Rows[x][ValueCol] == null) continue;
          if(this.dataGraph.Rows[x][ValueCol] < Min.Value) {
            Min = { Value: this.dataGraph.Rows[x][ValueCol], Time: this.dataGraph.Rows[x][TimeCol], Count: 0 };
          } else if(this.dataGraph.Rows[x][ValueCol] == Min) {
            Min.Count++;
          }
        }
        return Min;
      },
      Max: function() {
        var Max = { Value: -Infinity, Time: 0, Count: 0 };
        if((this.dataGraph == null) || (this.dataGraph.Rows.length == 0)) return Max;
        var TimeCol = this.dataGraph.Headers.indexOf('Time');
        var ValueCol = this.dataGraph.Headers.indexOf('Frequency');
        for(var x = 0; x < this.dataGraph.Rows.length; x++) {
          if(this.dataGraph.Rows[x][ValueCol] == null) continue;
          if(this.dataGraph.Rows[x][ValueCol] > Max.Value) {
            Max = { Value: this.dataGraph.Rows[x][ValueCol], Time: this.dataGraph.Rows[x][TimeCol], Count: 0 };
          } else if(this.dataGraph.Rows[x][ValueCol] == Max) {
            Max.Count++;
          }
        }
        return Max;
      },
      Std: function() {
        var Std = { Value: 0, Percent: 0, Count: 0 };
        var Avg = 0;
        if((this.dataGraph == null) || (this.dataGraph.Rows.length == 0)) return Std;
        var TimeCol = this.dataGraph.Headers.indexOf('Time');
        var ValueCol = this.dataGraph.Headers.indexOf('Frequency');
        for(var x = 0; x < this.dataGraph.Rows.length; x++) {
          if(this.dataGraph.Rows[x][ValueCol] == null) continue;
          if((this.dataGraph.Rows[x][ValueCol] > this.analyzer.NominalFreq * 0.99) && (this.dataGraph.Rows[x][ValueCol] < this.analyzer.NominalFreq * 1.01)) Std.Percent++;
          Std.Value += Math.pow(this.dataGraph.Rows[x][ValueCol], 2.0);
          Avg += this.dataGraph.Rows[x][ValueCol];
          Std.Count++;
        }
        Std.Value = Math.sqrt((Std.Value / Std.Count) - Math.pow(Avg / Std.Count, 2));
        Std.Percent = Std.Percent / Std.Count * 100;
        return Std;
      },
      Avg: function() {
        var Avg = { Value: 0, Count: 0 };
        if((this.dataGraph == null) || (this.dataGraph.Rows.length == 0)) return Avg;
        var TimeCol = this.dataGraph.Headers.indexOf('Time');
        var ValueCol = this.dataGraph.Headers.indexOf('Frequency');
        for(var x = 0; x < this.dataGraph.Rows.length; x++) {
          if(this.dataGraph.Rows[x][ValueCol] == null) continue;
          Avg.Count++;
          Avg.Value += this.dataGraph.Rows[x][ValueCol];
        }
        Avg.Value = Avg.Value / Avg.Count;
        return Avg;
       }
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