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
      <cubism v-for="(serie, index) in Series" :serie="serie" :key="serie" ref="rows" :index="index"></cubism>
    </div>
  </div>
</template>

<script>
  import Cubism from 'Components/Cubism.vue';
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import Spinner from 'Components/Spinner.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Dygraph from 'JS/dygraph.js';
  export default {
    data() {
      return { ShowSpinner: true, Aggregation: 'Loading data...', Series: ['POWER_FACTOR', 'VOLTAGE_THD', 'PHI'], Phase: null, Data: {} };
    },
    methods: {
      Redraw() {
        this.$nextTick(function() {    
          for(var n = 0; n < this.Series.length; n++) $(this.$refs.rows[n].Graph.graphDiv).parent().removeAttr('style');
          window.dispatchEvent(new Event('resize'));
        });
      },
      SetPhases: function(phase) {
        this.Phase = phase;
        this.CalcOptions();
      },
      SetData: function(result) {
        if(result['Sampletime'] === null) return;
        if(result['Sampletime'].length === 0) return;
        if(this.Data === null) this.Data = [];
        this.DateWindow = [result['From'], result['To']];
        this.Aggregation = 'From ' + oZm.GetAggregLongName(result['Aggregation_Source']) + ' aggregation';
        this.LastSample = oZm.LastSampletime(result, this.LastSample);
        for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = oZm.GraphDataAppend(result, oZm.Series[this.Series[i]].Serie, this.Data[this.Series[i]]);
        this.CalcOptions();
        this.ShowSpinner = false; 
      },
      SetDataNow(result) {
        for(var n = 0; n < this.Series.length; n++) this.$refs.rows[n].SetNowLabel(result[oZm.Series[this.Series[n]].Serie][this.Phase]);
      },
      CalcOptions() {
        for(var n = 0; n < this.Series.length; n++) {
          if(this.Data[this.Series[n]].length === 0) continue;
          var Data = [];
          for(var i = 0; i < this.Data[this.Series[n]].length; i++) Data.push([this.Data[this.Series[n]][i][0], this.Data[this.Series[n]][i][this.Phase + 1]]);
          this.$refs.rows[n].SetData({ dateWindow: this.DateWindow, file: Data});
        }
      }
    },
    created: function() {
      for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = [];
    },
    mounted: function() {
      var Graphs = [];
      for(var i = 0; i < this.Series.length; i++) Graphs.push(this.$refs.rows[i].Graph);
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
    },
    components: { Cubism, Spinner, ToolbarPhases, ToolbarTools }
  }
  
</script>