<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Period stats:</div>
      <div class="counters-aggreg" v-if="data!=null">From {{ analyzer.FormatAggregation(data.Aggreg) }} aggregation<br>From {{ analyzer.FormatDateTime(data.From) }} to {{ analyzer.FormatDateTime(data.To) }}</div>
      <div class="line">
        <div class="float-left">
          <div class="counters-stat-type">Max value:</div>
            <div class="counters-stat-time">{{ (Max.Time!=0) ? analyzer.FormatDateTime(Max.Time) : '-' }} <span v-if="Max.Count > 1">(and {{ Max.Count }} times more)</span></div>
          </div>
        <div class="counters-stat-val" v-html="(Max.Time!=0) ? FormatHTML(Max.Value) : '-'"></div>
      </div>
      <div class="line line-border">
        <div class="float-left">
          <div class="counters-stat-type">Total in period:</div>
          <div class="counters-stat-time">From {{ Sum.Count }} samples</div>
        </div>
        <div class="counters-stat-val" v-html="(Sum.Count>0) ? FormatHTML(Sum.Value) : '-'"></div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Period costs:</div>
        <div class="clearfix"></div>
      <div class="current-value" v-html="(Sum.Count > 0) ? Round(Sum.Budget, 2, true) + ' €' : '-'"></div>
    </div>
  </div>
</template>

<script>
  export default {
    props: { analyzer: Object, data: Object },
    data: function() {
      return { };
    },
    updated: function() {
      this.$emit('update:height', this.$el.clientHeight);
    },
    computed: {
      Sum: function() {
        var Sum = { Value: 0, Count: 0, Budget: 0 };
        if(this.data == null) return Sum;
        var Divisor = (this.data.Aggreg == '15M') ? 4 : 1;
        for(var x in this.data.Sampletime) {
          if(this.data.Active_Power[x] == null) continue;
          Sum.Count++;
          Sum.Value += this.data.Active_Power[x] / Divisor;
          if(this.data.Expense[x] != null) Sum.Budget += this.data.Expense[x];
        }
        return Sum;
      },
      Max: function() {
        var Max = { Value: -Infinity, Time: 0, Count: 0 };
        if(this.data == null) return Max;
        var Divisor = (this.data.Aggreg == '15M') ? 4 : 1;
        for(var x in this.data.Sampletime) {
          if(this.data.Active_Power[x] == null) continue;
          var Value = this.data.Active_Power[x] / Divisor;
          if(Value > Max.Value) {
            Max = { Value: Value, Time: this.data.Sampletime[x], Count: 0 };
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
        return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'Wh';
      },
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
    border-top: 1px solid #c7c7e6;
  }
</style>