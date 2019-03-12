<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ Aggregation }}</div>
        <div class="header-buttons" ref="Toolbar">
          <toolbar-phases variable="DetailsPhaseSingle" v-on:PhasesChanged="SetPhases"></toolbar-phases>
          <toolbar-tools variable="To" ref="Tools" :options="['EXPORT']"></toolbar-tools>
        </div>
      </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <cubism v-for="(serie, index) in Series" :serie="'HARMONICS'" :key="serie" :index="index" ref="rows"></cubism>
    </div>
  </div>
</template>

<script>
  import Cubism from 'Components/Cubism.vue';
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'JS/moment.js';
  import Spinner from 'Components/Spinner.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Dygraph from 'JS/dygraph.js';
  export default {
    data() {
      return { Aggregation: 'Loading data...', DateWindow: [0, 0], ShowSpinner: true, Time: 'Loading data...', Series: oZm.Series['HARMONICS'].Names, Data: null, Phase: null };
    },
    methods: {
      Redraw() {
        this.$nextTick(function() {    
          for(var n = 0; n < this.Series.length; n++) $(this.$refs.rows[n].Graph.graphDiv).parent().removeAttr('style');
          window.dispatchEvent(new Event('resize'));
        });
      },
      SetData: function(result) {
        if(result['Sampletime'] === null) return;
        if(result['Sampletime'].length === 0) return;
        if(this.Data === null) this.Data = [];
        this.DateWindow = [result['From'], result['To']];
        this.Aggregation = 'From ' + oZm.GetAggregLongName(result['Aggregation_Source']) + ' aggregation';
        this.LastSample = oZm.LastSampletime(result, this.LastSample);  
        this.Data = oZm.GraphDataAppend(result, 'Voltage_Harmonics', this.Data);
        this.CalcOptions();
        this.ShowSpinner = false; 
      },
      CalcOptions: function() {
        if((this.$root.SelectedAnalyzer === null) || (this.Data === null) || (this.Phase === null)) return;
        for(var n = 0; n < this.Series.length; n++) {
          var Data = [];    
          for(var i = 0; i < this.Data.length; i++) Data.push([this.Data[i][0], 20 * Math.log10(Math.max(0.000000001, this.Data[i][this.Phase + 1][n]))]);
          this.$refs.rows[n].SetData({ dateWindow: this.DateWindow, file: Data});
        }
      },
      SetPhases: function(phase) {
        this.Phase = phase;
        this.CalcOptions();
      }
    },
    mounted: function() {
      var Graphs = [];
      for(var i = 0; i < this.Series.length; i++) Graphs.push(this.$refs.rows[i].Graph);
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
    },
    components: { Cubism, Spinner, ToolbarPhases, ToolbarTools }
  }
</script>