<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Main variables</div>
      <div class="header-buttons">
        <span v-if="data!=null">From {{ analyzer.FormatAggregation(data.Aggregation_Source) }} aggregation</span>
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" v-model="Phase"></toolbar-phases>
        <div class="btn-group">
          <button-export file="Main_Variables" :rows="Export"></button-export>
        </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="data==null"></spinner>
      <cubism :index="0" ref="g0" title="Active Power" :data="Data" :format="Formatter_Active_Power"></cubism>
      <cubism :index="1" ref="g1" title="Reactive Power" :data="Data" :format="Formatter_Reactive_Power"></cubism>
      <cubism :index="2" ref="g2" title="Power Factor" :data="Data" :format="Formatter_Power_Factor"></cubism>
      <cubism :index="3" ref="g3" title="Phi" :data="Data" :format="Formatter_Phi"></cubism>
      <cubism :index="4" ref="g4" title="Voltage THD" :data="Data" :format="Formatter_Percent"></cubism>
      <cubism :index="5" ref="g5" title="Current THD" :data="Data" :format="Formatter_Percent"></cubism>
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
      return { Phase: null };
    },
    computed: {
      Data: function() {
        if((this.data == null) || (this.analyzer == null) || (this.Phase == null)) return null;
        var Series = { 'Active Power': [], 'Reactive Power': [], 'Power Factor': [], 'Phi': [], 'Voltage THD': [], 'Current THD': [] };
        Series.Sampletime = this.data.Sampletime;
        Series.From = this.data.From;
        Series.To = this.data.To;
        Series.TimeZone = this.analyzer.TimeZone;
        for(var i in this.data.Sampletime) {
          Series['Active Power'].push(this.data.Active_Power[i][this.Phase]);
          Series['Reactive Power'].push(this.data.Reactive_Power[i][this.Phase]);
          Series['Power Factor'].push(this.data.Power_Factor[i][this.Phase]);
          Series['Phi'].push(this.data.Phi[i][this.Phase]);
          Series['Voltage THD'].push(this.data.Voltage_THD[i][this.Phase]);
          Series['Current THD'].push(this.data.Current_THD[i][this.Phase]);
        }
        return Series;
      },
      Export: function() {
        return [['Sample']];
      }
    },
    methods: {
      Formatter_Active_Power: function(val) { var Units = this.BestUnits(val); return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'W'; },
      Formatter_Reactive_Power: function(val) { var Units = this.BestUnits(val); return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'var'; },
      Formatter_Power_Factor: function(val) { return this.Round(val, 2, true); },
      Formatter_Phi: function(val) { return this.Round(val, 2, true) + ((val < 0) ? '° IND' : (val > 0) ? '° CAP' : '°'); },
      Formatter_Percent: function(val) { return this.Round(val, 2, true) + ' %'; },
//      MouseOver: function(val) {
//        this.mouseOverCount = this.mouseOverCount + ((val === true) ? +1 : -1);
//      },
//      SetPhases: function(phase) {
//        this.Phase = phase;
//        this.CalcOptions();
//      },
//      CalcOptions: function() {
//        if(this.$root.SelectedAnalyzer === null) return;
//        for(var n = 0; n < this.Series.length; n++) {
//          if(this.Data[this.Series[n]].length === 0) continue;
//          var Data = [];
//          for(var i = 0; i < this.Data[this.Series[n]].length; i++) Data.push([this.Data[this.Series[n]][i][0], this.Data[this.Series[n]][i][this.Phase + 1]]);
//          this.$refs.rows[n].SetData({ dateWindow: this.DateWindow, file: Data});
//        }
//      },
//      UpdateBars() {
//        if(this.$root.SelectedAnalyzer === null) return;
//        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '1M', Series: this.Series, To: 0, Period: 86400000, LastSample: this.LastSample, Analyzer: this.analyzer.Analyzer }),
//          success: (result) => {
//            if(result['Sampletime'] === null) return;
//            if(result['Sampletime'].length === 0) return;
//            this.DateWindow = [result['From'], result['To']];
//            this.Aggregation = 'From ' + this.analyzer.FormatAggregation(result['Aggregation_Source']) + ' aggregation';
//            this.LastSample = oZm.LastSampletime(result, this.LastSample);
//            for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = oZm.GraphDataAppend(result, oZm.Series[this.Series[i]].Serie, this.Data[this.Series[i]]);
//            if(this.mouseOverCount === 0) this.CalcOptions();
//            this.ShowSpinner = false;
//          },
//          complete: () => {
//            clearTimeout(this.Timer);
//            this.Timer = setTimeout(this.UpdateBars, 30000);
//          },
//          error: function() {
//            this.ShowSpinner = true;
//          }.bind(this)
//        });
//      },
//      UpdateNow: function() {
//        if(this.$root.SelectedAnalyzer === null) return;
//        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: this.Series, Analyzer: this.analyzer.Analyzer }),
//          success: (result) => {
//            if(this.mouseOverCount !== 0) return;
//            for(var n = 0; n < this.Series.length; n++) this.$refs.rows[n].SetNowLabel(result[oZm.Series[this.Series[n]].Serie][this.Phase]);
//          },
//          complete: () => {
//            clearTimeout(this.TimerNow);
//            this.TimerNow = setTimeout(this.UpdateNow, 3000);
//          }
//        });
//      },
//      SetAnalyzer: function() {
//        this.LastSample = 0;
//        this.ShowSpinner = true;
//        for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = [];
//        this.UpdateBars();
//        this.UpdateNow();
//      }
    },
//    created: function() {
//      this.mouseOverCount = 0;
//      for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = [];
//      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
//    },
//    beforeDestroy: function() {
//      if(this.Timer !== null) clearTimeout(this.Timer);
//      if(this.TimerNow !== null) clearTimeout(this.TimerNow);
//    },
    mounted: function() {
      var Graphs = [ this.$refs.g0.Graph, this.$refs.g1.Graph, this.$refs.g2.Graph, this.$refs.g3.Graph, this.$refs.g4.Graph, this.$refs.g5.Graph ];
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 20px;
    line-height: 24px;
  }
</style>