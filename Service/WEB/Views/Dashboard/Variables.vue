<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Main variables</div>
      <div class="header-buttons">
        <span v-if="data!=null">From {{ analyzer.FormatAggregation(data.Aggregation_Source) }}</span>
        <toolbar-phases storage="DashboardDetails" :analyzer="analyzer" v-model="Phase"></toolbar-phases>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="data==null"></spinner>
      <cubism v-for="(serie, index) in Series" :serie="serie" :key="serie" ref="rows" :index="index" v-on:mouseOver="MouseOver"></cubism>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  import 'JS/dygraph-synchronizer.js';
  export default {
    props: { analyzer: { type: Object }, data: Object, dataNow: Object },
    data() {
      return {
        LastSample: 0, Phase: null, DateWindow: [0, 0], Data: {}, Timer: null, TimerNow: null, NowValues: [],
        Series: ['ACTIVE_POWER', 'REACTIVE_POWER', 'POWER_FACTOR', 'PHI', 'VOLTAGE_THD', 'CURRENT_THD'],
        Aggregation: 'Loading data...'
      };
    },
    methods: {
      MouseOver: function(val) {
        this.mouseOverCount = this.mouseOverCount + ((val === true) ? +1 : -1);
      },
//      SetPhases: function(phase) {
//        this.Phase = phase;
//        this.CalcOptions();
//      },
      CalcOptions: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        for(var n = 0; n < this.Series.length; n++) {
          if(this.Data[this.Series[n]].length === 0) continue;
          var Data = [];
          for(var i = 0; i < this.Data[this.Series[n]].length; i++) Data.push([this.Data[this.Series[n]][i][0], this.Data[this.Series[n]][i][this.Phase + 1]]);
          this.$refs.rows[n].SetData({ dateWindow: this.DateWindow, file: Data});
        }
      },
      UpdateBars() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '1M', Series: this.Series, To: 0, Period: 86400000, LastSample: this.LastSample, Analyzer: this.analyzer.Analyzer }),
          success: (result) => {
            if(result['Sampletime'] === null) return;
            if(result['Sampletime'].length === 0) return;
            this.DateWindow = [result['From'], result['To']];
            this.Aggregation = 'From ' + this.analyzer.FormatAggregation(result['Aggregation_Source']) + ' aggregation';
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
            for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = oZm.GraphDataAppend(result, oZm.Series[this.Series[i]].Serie, this.Data[this.Series[i]]);
            if(this.mouseOverCount === 0) this.CalcOptions();
            this.ShowSpinner = false;
          },
          complete: () => {
            clearTimeout(this.Timer);
            this.Timer = setTimeout(this.UpdateBars, 30000);
          },
          error: function() {
            this.ShowSpinner = true;
          }.bind(this)
        });
      },
      UpdateNow: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: this.Series, Analyzer: this.analyzer.Analyzer }),
          success: (result) => {
            if(this.mouseOverCount !== 0) return;
            for(var n = 0; n < this.Series.length; n++) this.$refs.rows[n].SetNowLabel(result[oZm.Series[this.Series[n]].Serie][this.Phase]);
          },
          complete: () => {
            clearTimeout(this.TimerNow);
            this.TimerNow = setTimeout(this.UpdateNow, 3000);
          }
        });
      },
      SetAnalyzer: function() {
        this.LastSample = 0;
        this.ShowSpinner = true;
        for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = [];
        this.UpdateBars();
        this.UpdateNow();
      }
    },
    created: function() {
      this.mouseOverCount = 0;
      for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = [];
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
//    beforeDestroy: function() {
//      if(this.Timer !== null) clearTimeout(this.Timer);
//      if(this.TimerNow !== null) clearTimeout(this.TimerNow);
//    },
    mounted: function() {
      var Graphs = [];
      for(var i = 0; i < this.Series.length; i++) Graphs.push(this.$refs.rows[i].Graph);
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
      if(this.$root.SelectedAnalyzer !== null) this.UpdateBars();
      if(this.$root.SelectedAnalyzer !== null) this.UpdateNow();
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 20px;
    line-height: 24px;
  }
</style>