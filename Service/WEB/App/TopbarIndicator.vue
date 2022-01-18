<template>
  <q-card class="indicator" :style="{ 'background-color': SerieViewColor(serie) }">
    <div>
      <div v-for="(val, idx) in Value.Value" v-html="val" :key="idx"></div>
    </div>
    <div>{{ Value.Units }}{{ SerieUnits(serie) }}<br><span style="font-size: 10px; font-weight: normal; font-stretch: condensed">{{ Value.Aggreg }}</span></div>
  </q-card>

  <!--div  :style="{ 'border-color': BackColor }">
    <div>
      <div>
        <div class="indicator" :style="{ 'background-color': Analyzer.SerieColor(serie), 'border-color': Analyzer.SerieColor(serie) }">
        <span v-for="(val, idx) in Value.Value" v-html="val" :key="idx"></span>
      </div>
      <div>{{ Value.Units }}{{ analyzer.SerieUnits(serie) }}<br><span style="font-size: 10px; font-weight: normal; font-stretch: condensed">{{ Value.Aggreg }}</span></div>
    </div>
  </div-->
</template>

<script>
  export default {
    props: { serie: String, grouped: Boolean },
    data: function() {
      return { };
    },
    created: function() {
      if(['Voltage', 'Current', 'Frequency', 'Active_Power'].includes(this.serie)) this.Analyzer.NowSeries.Suscribe([this.serie]);
    },
    unmounted: function() {
      if(['Voltage', 'Current', 'Frequency', 'Active_Power'].includes(this.serie)) this.Analyzer.NowSeries.Unsuscribe([this.serie]);
    },
    computed: {
      Value: function() {
        var Val = { Value: ['Offline'], Aggreg: '', Units: '' };
        if(this.serie == 'Active_Energy') return Val;
        if(this.Analyzer.NowSeries.Loading == true) Val.Value = ['Wait'];
        if(this.Analyzer.NowSeries.Loading == false) {
          Val.Value = ['Offline'];
          if(this.Analyzer.NowSeries[this.serie].Value != null) {
            if(Array.isArray(this.Analyzer.NowSeries[this.serie].Value) == false) {
              Val.Units = this.BestUnits(this.Analyzer.NowSeries[this.serie].Value);
              Val.Value = [this.Round(this.ScaleUnits(this.Analyzer.NowSeries[this.serie].Value, Val.Units), 3, true)];
            } else if(this.Analyzer.NowSeries[this.serie].Value.length == 1) {
              Val.Units = this.BestUnits(this.Analyzer.NowSeries[this.serie].Value[0]);
              Val.Value = [this.Round(this.ScaleUnits(this.Analyzer.NowSeries[this.serie].Value[0], Val.Units), 2, true)];
            } else if(this.grouped == true) {
              var Aggreg = 0.0;
              if((this.serie == 'Voltage') || (this.serie == 'Current')) {
                Val.Aggreg = "RMS";
                Aggreg = this.ArrayRMS(this.Analyzer.NowSeries[this.serie].Value);
              } else if((this.serie == 'Active_Power') || (this.serie == 'Active_Energy')) {
                Val.Aggreg = "SUM";
                Aggreg = this.ArraySUM(this.Analyzer.NowSeries[this.serie].Value);
              }
              Val.Units = this.BestUnits(Aggreg);
              Val.Value = ['<strong>' + this.Round(this.ScaleUnits(Aggreg, Val.Units), 2, true) + '</strong>'];
            } else {
              Val.Units = this.BestUnits(this.ArrayAbsMax(this.Analyzer.NowSeries[this.serie].Value));
              Val.Value = this.Analyzer.NowSeries[this.serie].Value.map(val => this.Round(this.ScaleUnits(val, Val.Units), 2, true));
            }
          }
        }
        return Val;
      }
    }
  }
</script>

<style scoped>
  .indicator {
    display: inline-block;
    height: 46px;
    margin: 2px 1px;
    padding: 1px;
    display: table;
  }
  .indicator > div:nth-child(1) {
    display: table-cell;
    text-align: right;
    height: 44px;
    color: #000;
    padding: 1px;
    background-color: #FFF;
    vertical-align: middle;
    min-width: 44px;
    font-size: 11px;
    line-height: 10px;
    border-radius: 3px;
  }
  .indicator > div:nth-child(2) {
    display: table-cell;
    text-align: center;
    height: 44px;
    color: #FFF;
    padding: 4px;
    vertical-align: middle;
    font-weight: bold;
  }
</style>