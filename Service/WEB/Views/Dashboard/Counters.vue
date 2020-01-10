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
    props: { analyzer: Object, data: Object },
    data: function() {
      return {};
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

</style>