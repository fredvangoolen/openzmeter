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
      <cubism ref="cubism" v-on:AggregationSet="SetAggregation" :Series="Series"></cubism>
    </div>
  </div>
</template>

<script>
  import Cubism from './Cubism.vue';
  import * as oZm from '../JS/openZmeter.js';
  import ToolbarPhases from './ToolbarPhases.vue'
  export default {
    data() {
      return {
        Series: ['ACTIVE_POWER', 'REACTIVE_POWER', 'POWER_FACTOR', 'PHI', 'VOLTAGE_THD', 'CURRENT_THD'],
        Aggregation: 'Loading data...'
      };
    },
    methods: {
      SetAggregation(aggreg) {
        this.Aggregation = 'From ' + oZm.GetAggregLongName(aggreg) + ' aggregation';
      }
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
      
    },
    components: { Cubism, ToolbarPhases }
  }   
</script>

<style>
    
</style>
