<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text" style="font-size: 24px; line-height: 16px">Details</div>
      <div class="header-buttons">
        <span>{{ Aggregation }}</span>
        <toolbar-phases v-on:PhasesChanged="SetPhases" variable="DashboardDetails" mode="VOLTAGE" :multi="false"></toolbar-phases>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <cubism v-for="(serie, index) in VisibleSeries" :serie="serie" :key="serie" ref="rows" :index="index"></cubism>
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
        VisibleSeries: ['ACTIVE_POWER', 'REACTIVE_POWER', 'APPARENT_POWER', 'POWER_FACTOR', 'PHI'],
        Series: ['ACTIVE_POWER', 'REACTIVE_POWER', 'VOLTAGE', 'CURRENT', 'POWER_FACTOR', 'PHI'],
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
        for(var n = 0; n < this.VisibleSeries.length; n++) {
          if(this.Data[this.VisibleSeries[n]].length === 0) continue;
          var Data = [];
          for(var i = 0; i < this.Data[this.VisibleSeries[n]].length; i++) Data.push([this.Data[this.VisibleSeries[n]][i][0], this.Data[this.VisibleSeries[n]][i][this.Phase + 1]]);
          this.$refs.rows[n].SetData({ dateWindow: this.DateWindow, file: Data});
        }
      },
      UpdateBars() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '1M', Series: this.Series, From: 0, Period: 86400000, LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            if(result['Sampletime'] === null) return;
            if(result['Sampletime'].length === 0) return;
            this.DateWindow = [result['From'], result['To']];
            this.Aggregation = 'From ' + oZm.GetAggregLongName(result['Aggregation_Source']) + ' aggregation';
            this.LastSample = oZm.LastSampletime(result, this.LastSample);        
            result['Apparent_Power'] = [];        
            for(var n = 0; n < result['Sampletime'].length; n++) result['Apparent_Power'].push(result['Voltage'][n].map( function(val, index) { return val * result['Current'][n][index]; }));
            for(var i = 0; i < this.VisibleSeries.length; i++) this.Data[this.VisibleSeries[i]] = oZm.GraphDataAppend(result, oZm.Series[this.VisibleSeries[i]].Serie, this.Data[this.VisibleSeries[i]]);
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
      },
      UpdateNow() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: this.Series, Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            result['Apparent_Power'] = result['Voltage'].map( function(val, index) { return val * result['Current'][index]; });
            for(var n = 0; n < this.VisibleSeries.length; n++) this.$refs.rows[n].SetNowLabel(result[oZm.Series[this.VisibleSeries[n]].Serie][this.Phase]);
          }.bind(this),
          complete: function() {
            if(this.TimerNow !== null) clearTimeout(this.TimerNow);
            this.TimerNow = setTimeout(this.UpdateNow, 3000);
          }.bind(this)
        });
      },
      SetAnalyzer() {
        this.LastSample = 0;
        for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = [];
        this.UpdateBars();  
        this.UpdateNow();
      }
    },
    created: function() { 
      for(var i = 0; i < this.VisibleSeries.length; i++) this.Data[this.VisibleSeries[i]] = [];
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    beforeDestroy: function() {
      if(this.Timer !== null) clearTimeout(this.Timer);
      if(this.TimerNow !== null) clearTimeout(this.TimerNow);
    },
    mounted: function() {
      var Graphs = [];
      for(var i = 0; i < this.VisibleSeries.length; i++) Graphs.push(this.$refs.rows[i].Graph);
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
      this.UpdateBars();
      this.UpdateNow();
    },
    components: { Cubism, Spinner, ToolbarPhases }
  }   
</script>