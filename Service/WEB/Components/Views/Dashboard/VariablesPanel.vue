<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text" style="font-size: 24px; line-height: 16px">Main variables</div>
      <div class="header-buttons">
        <span>{{ Aggregation }}</span>
        <toolbar-phases v-on:PhasesChanged="SetPhases" variable="DashboardDetails" mode="VOLTAGE" :multi="false"></toolbar-phases>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <cubism v-for="(serie, index) in Series" :serie="serie" :key="serie" ref="rows" :index="index"></cubism>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Spinner from 'Components/Spinner.vue';
  import Cubism from 'Components/Cubism.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import Dygraph from 'JS/dygraph.js';
  import 'JS/dygraph-synchronizer.js';
  export default {
    data() {
      return {
        ShowSpinner: true,
        StopUpdating: false,
        Series: ['ACTIVE_POWER', 'REACTIVE_POWER', 'POWER_FACTOR', 'PHI', 'VOLTAGE_THD', 'CURRENT_THD'],
        Aggregation: 'Loading data...',
        LastSample: 0,
        Phase: 0,
        DateWindow: [0, 0],
        Data: {},
        Timer: null,
        TimerNow: null
      };
    },
    methods: {
      SetPhases(phase) {
        this.Phase = phase;
        this.CalcOptions();
      },
      CalcOptions() {
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
        if(this.StopUpdating==false){
          $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '1M', Series: this.Series, From: 0, Period: 86400000, LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.ID }), 
            success: function(result) {
              if(result['Sampletime'] === null) return;
              if(result['Sampletime'].length === 0) return;
              this.DateWindow = [result['From'], result['To']];
              this.Aggregation = 'From ' + oZm.GetAggregLongName(result['Aggregation_Source']) + ' aggregation';
              this.LastSample = oZm.LastSampletime(result, this.LastSample);        
              for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = oZm.GraphDataAppend(result, oZm.Series[this.Series[i]].Serie, this.Data[this.Series[i]]);
              this.CalcOptions();
              this.ShowSpinner = false;
            }.bind(this),
            complete: function() {
              if(this.Timer !== null) clearTimeout(this.Timer);
              this.Timer = setTimeout(this.UpdateBars, 30000);
            }.bind(this), 
            error: function() {
              this.ShowSpinner = true;
            }.bind(this)
          });
        }else{
          if(this.Timer !== null) clearTimeout(this.Timer);
          this.Timer = setTimeout(this.UpdateBars, 30000);
        }
      },
      UpdateNow() {
        if(this.$root.SelectedAnalyzer === null) return;
        if(this.StopUpdating==false){
          $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: this.Series, Analyzer: this.$root.SelectedAnalyzer.ID }), 
            success: function(result) {
              for(var n = 0; n < this.Series.length; n++) this.$refs.rows[n].SetNowLabel(result[oZm.Series[this.Series[n]].Serie][this.Phase]);
            }.bind(this),
            complete: function() {
              if(this.TimerNow !== null) clearTimeout(this.TimerNow);
              this.TimerNow = setTimeout(this.UpdateNow, 3000);
            }.bind(this)
          });
        }else{
          if(this.TimerNow !== null) clearTimeout(this.TimerNow);
          this.TimerNow = setTimeout(this.UpdateNow, 3000);
        }
      },
      SetAnalyzer() {
        this.LastSample = 0;
        this.ShowSpinner = true;
        for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = [];
        this.UpdateBars();  
        this.UpdateNow();
      }
    },
    created: function() { 
      for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = [];
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    beforeDestroy: function() {
      if(this.Timer !== null) clearTimeout(this.Timer);
      if(this.TimerNow !== null) clearTimeout(this.TimerNow);
    },
    mounted: function() {
      var Graphs = [];
      for(var i = 0; i < this.Series.length; i++) Graphs.push(this.$refs.rows[i].Graph);
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
      if(this.$root.SelectedAnalyzer !== null) this.UpdateBars();
      if(this.$root.SelectedAnalyzer !== null) this.UpdateNow();
    },
    components: { Cubism, Spinner, ToolbarPhases }
  }   
</script>