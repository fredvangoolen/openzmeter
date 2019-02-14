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
  import ToolbarPhases from 'Components/ToolbarPhases.vue'
  export default {
    data() {
      return {
        ShowSpinner: true,
        Series: ['ACTIVE_POWER', 'REACTIVE_POWER', 'POWER_FACTOR', 'PHI', 'VOLTAGE_THD', 'CURRENT_THD'],
        Aggregation: 'Loading data...',
        LastSample: 0,
        Phase: 0,
        DateWindow: [0, 0],
        Data: {},
        UpdateGraphTimer: null
      };
    },
    methods: {
      SetPhases(phases) {
        this.Phase = 0;
        for(var n = 0; n < phases.length; n++) {
          if(phases[n] !== true) continue;
          this.Phase = n;
          break;
        }
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
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '1M', Series: this.Series, From: 0, Period: 86400000,LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            if(result['Sampletime'] === null) return;
            if(result['Sampletime'].length === 0) return;
            this.ShowSpinner = false;
            this.DateWindow = [result['From'], result['To']];
            this.Aggregation = 'From ' + oZm.GetAggregLongName(result['Aggregation_Source']) + ' aggregation';
            this.LastSample = oZm.LastSampletime(result, this.LastSample);        
            for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = oZm.GraphDataAppend(result, oZm.Series[this.Series[i]].Serie, this.Data[this.Series[i]]);
            this.CalcOptions();
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphTimer !== null) clearTimeout(this.UpdateGraphTimer);
            this.UpdateGraphTimer = setTimeout(this.GetSeries, 30000);
          }.bind(this), 
          error: function() {
            this.ShowSpinner = true;
          }.bind(this)
        });
      },
      SetAnalyzer() {
        this.LastSample = 0;
        for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = [];
        this.UpdateBars();  
      }
    },
    created: function() { 
      for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = [];
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    beforeDestroy: function() {
      if(this.UpdateGraphTimer !== null) clearTimeout(this.UpdateGraphTimer);
    },
    mounted: function() {
 
      
//      this.$refs.cubism.GetSeries();
//    ActiveChart      = AppendCubismGraph($('#EvolutionPanel'), ActiveData,      "Active Power",   [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return (x === null) ? '~' : (Round2(ScaleToUnits(x, Units)) + '<em>' + Units + 'W</em>'); });        
             /*   $('#labelParametersAggregation').text('From ' + GetAggregLongName(result['Aggregation_Source']) + ' aggregation');
  ActiveData = GraphDataAppend(result, 'Active_Power', ActiveData);
  ReactiveData = GraphDataAppend(result, 'Reactive_Power', ReactiveData);
  PowerFactorData = GraphDataAppend(result, 'Power_Factor', PowerFactorData);
  PhiData  = GraphDataAppend(result, 'Phi', PhiData);
  THDVData = GraphDataAppend(result, 'Voltage_THD', THDVData);
  THDIData = GraphDataAppend(result, 'Current_THD', THDIData);
  if(ReactiveChart === null) {
    
    ReactiveChart    = AppendCubismGraph($('#EvolutionPanel'), ReactiveData,    "Reactive Power", [result['From'], result['To']], function (x) { var Units = GetBestUnits(x); return (x === null) ? '~' : (Round2(ScaleToUnits(x, Units)) + '<em>' + Units + 'VAr</em>'); });
    PowerFactorChart = AppendCubismGraph($('#EvolutionPanel'), PowerFactorData, "Power Factor",   [result['From'], result['To']], function (x) { return (x === null) ? '~' : Round2(x); });
    PhiChart         = AppendCubismGraph($('#EvolutionPanel'), PhiData,         "Angle",          [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(Math.abs(x)) + '<em>' + ((x < 0) ? 'º CAP' : (x > 0) ? 'º IND' : 'º') + '</em>'); });
    THDVChart        = AppendCubismGraph($('#EvolutionPanel'), THDVData,        "Voltage THD",    [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(x * 100.0) + '<em>%</em>'); });
    THDIChart        = AppendCubismGraph($('#EvolutionPanel'), THDIData,        "Current THD",    [result['From'], result['To']], function (x) { return (x === null) ? '~' : (Round2(x * 100.0) + '<em>%</em>'); });
    Dygraph.synchronize([ActiveChart, ReactiveChart, PowerFactorChart, PhiChart, THDVChart, THDIChart], { range: false, zoom: false });
  } else {
    ActiveChart.updateOptions({ file: ActiveData, dateWindow: [result['From'], result['To']] });
    ReactiveChart.updateOptions({ file: ReactiveData, dateWindow: [result['From'], result['To']] });
    PowerFactorChart.updateOptions({ file: PowerFactorData, dateWindow: [result['From'], result['To']] });
    PhiChart.updateOptions({ file: PhiData, dateWindow: [result['From'], result['To']] });
    THDVChart.updateOptions({ file: THDVData, dateWindow: [result['From'], result['To']] });
    THDIChart.updateOptions({ file: THDIData, dateWindow: [result['From'], result['To']] });
  }
            
            

*/      
      if(this.$root.SelectedAnalyzer !== null) this.UpdateBars();
    },
    components: { Cubism, Spinner, ToolbarPhases }
  }   
</script>

<style>
    
</style>
