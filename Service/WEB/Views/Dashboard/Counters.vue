<template>
  <div class="row q-col-gutter-sm q-pa-xs">
    <div class="col-12" >
      <q-card class="full-height q-pa-xs" flat bordered>
        <q-item dense class="q-pa-none">
          <q-item-section avatar>
            <q-item-title class="text-weight-bold text-subtitle2">Period stats:</q-item-title>
            <q-item-title class="text-caption text-right">From {{ FormatAggregation(data.Aggreg) }} aggregation</q-item-title>
          </q-item-section>
          <q-item-section class="text-caption text-right">From {{ Analyzer.FormatDateTime(data.From) }}<br/>To {{ Analyzer.FormatDateTime(data.To) }}</q-item-section>
        </q-item>
        <q-separator />
        <q-item dense class="q-pa-none">
          <q-item-section avatar>
            <q-item-title class="text-weight-bold text-subtitle2">Max value:</q-item-title>
            <q-item-title class="text-caption">{{ (Max.Time!=0) ? Analyzer.FormatDateTime(Max.Time) : '-' }} <span v-if="Max.Count > 1">(and {{ Max.Count }} times more)</span></q-item-title>
          </q-item-section>
          <q-item-section class="text-h4 text-right"><div v-html="(Max.Time!=0) ? FormatHTML(Max.Value) : '-'" /></q-item-section>
        </q-item>
        <q-separator />
        <q-item dense class="q-pa-none">
          <q-item-section avatar>
            <q-item-title class="text-weight-bold text-subtitle2">Total in period:</q-item-title>
            <q-item-title class="text-caption">From {{ Sum.Count }} samples</q-item-title>
          </q-item-section>
          <q-item-section class="text-h4 text-right"><div v-html="(Sum.Count>0) ? FormatHTML(Sum.Value) : '-'" /></q-item-section>
        </q-item>
      </q-card>
    </div>
    <div class="col-12">
      <q-card class="full-height q-pa-xs" flat bordered>
        <q-item dense class="q-pa-none">
          <q-item-section avatar class="text-weight-bold text-subtitle2">Period costs:</q-item-section>
          <q-item-section class="text-h4 text-right"><div v-html="(Sum.Count > 0) ? Round(Sum.Budget, 2, true) + ' €' : '-'" /></q-item-section>
        </q-item>
      </q-card>
    </div>
  </div>
</template>

<script>
  export default {
    props: { data: Object },
    data: function() {
      return { };
    },
    updated: function() {
      this.$emit('update:height', this.$el.clientHeight);
    },
    computed: {
      Sum: function() {
        var Sum = { Value: 0, Count: 0, Budget: 0 };
        if(this.data.Headers == null) return Sum;
        var EnergyCol = this.data.Headers.indexOf('Active_Energy');
        var CostCol = this.data.Headers.indexOf('Cost');
        for(var x in this.data.Rows) {
          Sum.Value += this.data.Rows[x][EnergyCol];
          if(this.data.Rows[x][CostCol] != null) Sum.Budget += this.data.Rows[x][CostCol];
          Sum.Count++;
        }
        return Sum;
      },
      Max: function() {
        var Max = { Value: -Infinity, Time: 0, Count: 0 };
        if(this.data.Headers == null) return Max;
        var TimeCol = this.data.Headers.indexOf('Time');
        var EnergyCol = this.data.Headers.indexOf('Active_Energy');
        for(var x in this.data.Rows) {
          var Value = this.data.Rows[x][EnergyCol];
          if(Value > Max.Value) {
            Max = { Value: Value, Time: this.data.Rows[x][TimeCol], Count: 0 };
          } else if(Value == Max.Value) {
            Max.Count++;
          }
        }
        console.log(Max);
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