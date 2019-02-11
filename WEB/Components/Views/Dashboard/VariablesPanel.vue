<template>
  <div class="panel panel-other">
    <!-- div class="spinnerdiv"></div-->
    <div class="panel-heading clearfix">
      <div class="huge header-text">Main variables</div>
      <div class="header-buttons">
        <toolbar-phases></toolbar-phases>
        <div>{{ Aggregation }}</div>
      </div>
    </div>
    <div class="panel-body">
      <div ref="Spinner" class="spinnerdiv" v-bind:class="{ 'hide': !ShowSpinner }"></div>
      <cubism v-for="(serie, index) in Series" :serie="serie" :key="serie" :first="index == 0"></cubism>
    </div>
  </div>
</template>

<script>
  import Cubism from 'Components/Cubism.vue';
  import * as oZm from 'JS/openZmeter.js';
  import ToolbarPhases from 'Components/ToolbarPhases.vue'
  export default {
    data() {
      return {
        ShowSpinner: true,
        Series: ['ACTIVE_POWER', 'REACTIVE_POWER', 'POWER_FACTOR', 'PHI', 'VOLTAGE_THD', 'CURRENT_THD'],
        Aggregation: 'Loading data...',
        LastSample: 0,
        Data: {}
      };
    },
    methods: {
      SetAggregation(aggreg) {
        this.Aggregation = 'From ' + oZm.GetAggregLongName(aggreg) + ' aggregation';
      },
      UpdateBars() {
        if(this.Analyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '1M', Series: this.Series, From: 0, Period: 86400000,LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.ID }), 
          success: function(result) {
            this.ShowSpinner = false;
            this.$emit('AggregationSet', result['Aggregation_Source']);
            if(result['Sampletime'] === null) return;
            if(result['Sampletime'].length === 0) return;
            for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = oZm.GraphDataAppend(result, oZm.Series[this.Series[i]].Serie, this.Data[this.Series[i]]);
            this.$emit('SeriesUpdate', this.Data, result['From'], result['To'], 0);
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
          }.bind(this),
          complete: function() {
            if(this.UpdateIndicatorsTimer !== null) clearTimeout(this.UpdateIndicatorsTimer);
            this.UpdateIndicatorsTimer = setTimeout(this.GetSeries, 60000);
          }.bind(this), 
          error: function() {
            this.ShowSpinner = true;
          }.bind(this)
        });
      },
      SetAnalyzer() {
        this.LastSample = 0;
        this.UpdateBars();  
      }
    },
    created: function() { 
      for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = [];
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    mounted: function() {
//      this.$refs.cubism.AppendRow("ACTIVE_POWER");
//      this.$refs.cubism.AppendRow("REACTIVE_POWER");
//      this.$refs.cubism.AppendRow("REACTIVE_POWER");
      
      
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
    components: { Cubism, ToolbarPhases }
  }   
</script>

<style>
    
</style>
