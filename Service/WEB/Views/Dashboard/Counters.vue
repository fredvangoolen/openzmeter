<template>
  <div>
    <div class="subpanel clearfix">
      <div class="counters-title">Period stats:</div>
      <div class="counters-aggreg" v-if="data!=null">From {{ analyzer.FormatAggregation(data.Aggregation_Source) }} aggregation<br>From {{ analyzer.FormatDateTime(data.From) }} to {{ analyzer.FormatDateTime(data.To) }}</div>
      <div class="line line-border">
        <div class="float-left">
          <div class="counters-stat-type">Max value:</div>
            <div class="counters-stat-time">{{ Max.Time }}</div>
          </div>
        <div class="counters-stat-val" v-html="Max.Value"></div>
      </div>
      <div class="line line-border">
        <div class="float-left">
          <div class="counters-stat-type">Total in period:</div>
          <div class="counters-stat-time">From {{ Sum.Samples }} samples</div>
        </div>
        <div class="counters-stat-val" v-html="Sum.Value"></div>
      </div>
    </div>
    <div class="subpanel clearfix">
      <div class="counters-title">Period costs:</div>
        <div class="clearfix"></div>
      <div class="current-value" v-html="Sum.Budget"></div>
    </div>
  </div>
</template>

<script>
  export default {
    props: { analyzer: Object, data: Object,  },
    data: function() {
      return {};
    },
    updated: function() {
      this.$emit('update:height', this.$el.clientHeight);
    },
    computed: {
      Max: function() {
        return { Value: 0, Time:0};
      },
      Sum: function() {
        return { Value: 0, Time:0, Budget: 0 };
      }
    },

    UpdateCounters: function(aggreg) {
        this.TimeStatsFrom = moment(this.$refs.period.DateWindow[0]).format('DD/MM/YYYY HH:mm:ss');
        this.TimeStatsTo = moment(this.$refs.period.DateWindow[1]).format('DD/MM/YYYY HH:mm:ss');
        var AccumSamples = 0, AccumValue = 0.0, MaxValue = -Infinity, MaxCount = 0, MaxTime = null, Cost = 0.0;
        for(var x = 0; x < this.Data.length; x++) {
          if(this.Data[x][1] === null) continue;
          if(this.Data[x][1] > MaxValue) {
            MaxValue = this.Data[x][1];
            MaxCount = 0;
            MaxTime = this.Data[x][0];
          } else if(this.Data[x][1] === MaxValue) {
            MaxCount++;
          }
          if(this.Costs[x][1] !== null) Cost += this.Costs[x][1];
          AccumValue += this.Data[x][1];
          AccumSamples++;
        }
        this.AccumSamples = AccumSamples;
        this.MaxTime = moment(MaxTime).format("hh:mm:ss - DD/MM/YYYY") + ((MaxCount > 1) ? ' (and ' + MaxCount + ' times more)' : '');
        var UnitsMax = oZm.Series['ACTIVE_ENERGY'].FormatValue(Number.isFinite(MaxValue) ? MaxValue : 0.0);
        this.MaxValue = UnitsMax.Value + ' ' + UnitsMax.Units;
        var UnitsAccum = oZm.Series['ACTIVE_ENERGY'].FormatValue(AccumValue);
        this.AccumValue = UnitsAccum.Value + ' ' + UnitsAccum.Units;
        this.BudgetValue = oZm.Round2(Cost) + ' <em>€</em>';
        this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
      }
  }
</script>

<style scoped>
  .dashboard-energy .header-text {
    font-size: 24px;
    line-height: 16px;
  }
  .header-text {
    font-size: 24px;
    line-height: 16px;
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
  }
  .line {
    clear: both;
  }
  .line-border {
    border-top: 1px solid #c7c7e6;
  }
</style>