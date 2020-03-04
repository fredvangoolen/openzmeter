<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">From {{ (data!=null) ? analyzer.FormatAggregation(data.Aggreg) : '-' }} aggregation {{ MouseOver ? '(Graph paused)' : '' }}</div>
      <div class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="false" :current="false" v-model="Phase"></toolbar-phases>
        <div class="btn-group">
          <button-export file="Variables" :rows="Export"></button-export>
        </div>
      </div>
    </div>
    <div class="panel-body" @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
      <spinner v-if="data==null"></spinner>
      <cubism :index="0" ref="g0" title="Power Factor" :data="Data" :now="DataNow" :format="Formatter_Power_Factor" :paused="MouseOver"></cubism>
      <cubism :index="1" ref="g1" title="Voltage THD" :data="Data" :now="DataNow" :format="Formatter_Voltage_THD" :paused="MouseOver"></cubism>
      <cubism :index="2" ref="g2" title="Phi" :data="Data" :now="DataNow" :format="Formatter_Phi" :paused="MouseOver"></cubism>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { analyzer: Object, data: Object, dataNow: Object },
    data() {
      return { Phase: null, MouseOver: false };
    },
    computed: {
      Data: function() {
        if((this.data == null) || (this.Phase == null)) return null;
        var Series = { 'Power Factor': [], 'Voltage THD': [], 'Phi': [] };
        Series.Sampletime = this.data.Sampletime;
        Series.From = this.data.From;
        Series.To = this.data.To;
        Series.TimeZone = this.analyzer.TimeZone;
        for(var i in this.data.Sampletime) {
          Series['Power Factor'].push(this.data.Power_Factor[i][this.Phase]);
          Series['Voltage THD'].push(this.data.Voltage_THD[i][this.Phase]);
          Series['Phi'].push(this.data.Phi[i][this.Phase]);
        }
        return Object.seal(Series);
      },
      DataNow: function() {
        if((this.dataNow == null) || (this.Phase == null)) return null;
        var Series = { 'Power Factor': null, 'Voltage THD': null, 'Phi': null };
        Series['Power Factor'] = this.dataNow.Power_Factor[this.Phase];
        Series['Voltage THD'] = this.dataNow.Voltage_THD[this.Phase];
        Series['Phi'] = this.dataNow.Phi[this.Phase];
        return Object.seal(Series);
      }
    },
    methods: {
      Formatter_Power_Factor: function(val) { return this.Round(val, 2, true); },
      Formatter_Voltage_THD: function(val) { return this.Round(val, 2, true) + ' %'; },
      Formatter_Phi: function(val) { return this.Round(val, 2, true); },
      Export: function() {
        if(this.data == null) return [];
        var Header = ['Sample']
          .concat(this.analyzer.GetSeriesNames('Power Factor', this.analyzer.VoltageChannels))
          .concat(this.analyzer.GetSeriesNames('Voltage THD', this.analyzer.VoltageChannels))
          .concat(this.analyzer.GetSeriesNames('Phi', this.analyzer.VoltageChannels));
        var Data = [Header];
        for(var n in this.data.Sampletime) {
          var Row = [this.data.Sampletime[n]];
          for(var i in this.data.Power_Factor[n]) Row.push(this.Round(this.data.Power_Factor[n][i], 3));
          for(var i in this.data.Voltage_THD[n]) Row.push(this.Round(this.data.Voltage_THD[n][i], 3));
          for(var i in this.data.Phi[n]) Row.push(this.Round(this.data.Phi[n][i], 3));
          Data.push(Row);
        }
        return Object.seal(Data);
      }
    },
    mounted: function() {
      var Graphs = [this.$refs.g0.Graph, this.$refs.g1.Graph, this.$refs.g2.Graph];
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
    }
  }
</script>