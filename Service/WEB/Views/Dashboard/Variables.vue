<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Main variables</div>
      <div class="header-buttons">
        <span v-if="data!=null">From {{ analyzer.FormatAggregation(data.Aggregation_Source) }} aggregation {{ Paused ? '(Paused)' : '' }}</span>
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" v-model="Phase"></toolbar-phases>
        <div class="btn-group">
          <button-export file="Main_Variables" :rows="Export"></button-export>
        </div>
      </div>
    </div>
    <div class="panel-body" @mouseenter="Paused=true" @mouseleave="Paused=false">
      <spinner v-if="data==null"></spinner>
      <cubism :index="0" ref="g0" title="Active Power" :data="Data" :now="DataNow" :format="Formatter_Active_Power" :paused="Paused"></cubism>
      <cubism :index="1" ref="g1" title="Reactive Power" :data="Data" :now="DataNow" :format="Formatter_Reactive_Power" :paused="Paused"></cubism>
      <cubism :index="2" ref="g2" title="Power Factor" :data="Data" :now="DataNow" :format="Formatter_Power_Factor" :paused="Paused"></cubism>
      <cubism :index="3" ref="g3" title="Phi" :data="Data" :now="DataNow" :format="Formatter_Phi" :paused="Paused"></cubism>
      <cubism :index="4" ref="g4" title="Voltage THD" :data="Data" :now="DataNow" :format="Formatter_Percent" :paused="Paused"></cubism>
      <cubism :index="5" ref="g5" title="Current THD" :data="Data" :now="DataNow" :format="Formatter_Percent" :paused="Paused"></cubism>
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
      return { Phase: null, Paused: false };
    },
    computed: {
      Data: function() {
        var Series = { 'Active Power': [], 'Reactive Power': [], 'Power Factor': [], 'Phi': [], 'Voltage THD': [], 'Current THD': [] };
        if((this.data == null) || (this.analyzer == null) || (this.Phase == null)) return Series;
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
      DataNow: function() {
        var Series = { 'Active Power': null, 'Reactive Power': null, 'Power Factor': null, 'Phi': null, 'Voltage THD': null, 'Current THD': null };
        if((this.dataNow == null) || (this.analyzer == null) || (this.Phase == null)) return Series;
        Series['Active Power'] = this.dataNow.Active_Power[this.Phase];
        Series['Reactive Power'] = this.dataNow.Reactive_Power[this.Phase];
        Series['Power Factor'] = this.dataNow.Power_Factor[this.Phase];
        Series['Phi'] = this.dataNow.Phi[this.Phase];
        Series['Voltage THD'] = this.dataNow.Voltage_THD[this.Phase];
        Series['Current THD'] = this.dataNow.Current_THD[this.Phase];
        return Series;
      },
      Export: function() {
        var Header = ['Sample'];

        return [['Sample']];
      }
    },
    methods: {
      Formatter_Active_Power: function(val) { var Units = this.BestUnits(val); return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'W'; },
      Formatter_Reactive_Power: function(val) { var Units = this.BestUnits(val); return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'var'; },
      Formatter_Power_Factor: function(val) { return this.Round(val, 2, true); },
      Formatter_Phi: function(val) { return this.Round(val, 2, true) + ((val < 0) ? '° IND' : (val > 0) ? '° CAP' : '°'); },
      Formatter_Percent: function(val) { return this.Round(val, 2, true) + ' %'; },
    },
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