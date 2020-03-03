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
      <cubism :index="0" ref="g0" title="Active Power" :data="Data" :now="DataNow" :format="Formatter_Active_Power" :paused="MouseOver"></cubism>
      <cubism :index="1" ref="g1" title="Reactive Power" :data="Data" :now="DataNow" :format="Formatter_Reactive_Power" :paused="MouseOver"></cubism>
      <cubism :index="2" ref="g2" title="Apparent Power" :data="Data" :now="DataNow" :format="Formatter_Apparent_Power" :paused="MouseOver"></cubism>
      <cubism :index="3" ref="g3" title="Power Factor" :data="Data" :now="DataNow" :format="Formatter_Power_Factor" :paused="MouseOver"></cubism>
      <cubism :index="4" ref="g4" title="Phi" :data="Data" :now="DataNow" :format="Formatter_Phi" :paused="MouseOver"></cubism>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import Dygraph from 'JS/dygraph.js';
  import 'JS/dygraph-synchronizer.js';
  export default {
    props: { analyzer: Object, data: Object, dataNow: Object },
    data: function() {
      return { Phase: null, MouseOver: false };
    },
    computed: {
      Data: function() {
        if((this.data == null) || (this.analyzer == null) || (this.Phase == null)) return null;
        var Series = { 'Active Power': [], 'Reactive Power': [], 'Apparent Power': [], 'Power Factor': [], 'Phi': [] };
        Series.Sampletime = this.data.Sampletime;
        Series.From = this.data.From;
        Series.To = this.data.To;
        Series.TimeZone = this.analyzer.TimeZone;
        for(var i in this.data.Sampletime) {
          Series['Active Power'].push(this.data.Active_Power[i][this.Phase]);
          Series['Reactive Power'].push(this.data.Reactive_Power[i][this.Phase]);
          Series['Apparent Power'].push(this.data.Voltage[i][this.Phase] * this.data.Current[i][this.Phase]);
          Series['Power Factor'].push(this.data.Power_Factor[i][this.Phase]);
          Series['Phi'].push(this.data.Phi[i][this.Phase]);
        }
        return Series;
      },
      DataNow: function() {
        if((this.dataNow == null) || (this.analyzer == null) || (this.Phase == null)) return null;
        var Series = { 'Active Power': null, 'Reactive Power': null, 'Apparent Power': null, 'Power Factor': null, 'Phi': null };
        Series['Active Power'] = this.dataNow.Active_Power[this.Phase];
        Series['Reactive Power'] = this.dataNow.Reactive_Power[this.Phase];
        Series['Apparent Power'] = this.dataNow.Apparent_Power[this.Phase];
        Series['Power Factor'] = this.dataNow.Power_Factor[this.Phase];
        Series['Phi'] = this.dataNow.Phi[this.Phase];
        return Series;
      },
      Export: function() {
        if((this.data == null) || (this.analyzer == null)) return [];
        var Header = ['Sample']
          .concat(this.analyzer.GetSeriesNames('Active Power', this.analyzer.VoltageChannels))
          .concat(this.analyzer.GetSeriesNames('Reactive Power', this.analyzer.VoltageChannels))
          .concat(this.analyzer.GetSeriesNames('Apparent Power', this.analyzer.VoltageChannels))
          .concat(this.analyzer.GetSeriesNames('Power Factor', this.analyzer.VoltageChannels))
          .concat(this.analyzer.GetSeriesNames('Phi', this.analyzer.VoltageChannels));
        var Data = [Header];
        for(var n in this.data.Sampletime) {
          var Row = [this.data.Sampletime[n]];
          for(var i in this.data.Active_Power[n]) Row.push(this.data.Active_Power[n][i]);
          for(var i in this.data.Reactive_Power[n]) Row.push(this.data.Reactive_Power[n][i]);
          for(var i in this.data.Voltage[n]) Row.push(this.data.Voltage[n][i] * this.data.Current[n][i]);
          for(var i in this.data.Power_Factor[n]) Row.push(this.data.Power_Factor[n][i]);
          for(var i in this.data.Phi[n]) Row.push(this.data.Phi[n][i]);
          Data.push(Row);
        }
        return Data;
      }
    },
    methods: {
      Formatter_Active_Power: function(val) { var Units = this.BestUnits(val); return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'W'; },
      Formatter_Reactive_Power: function(val) { var Units = this.BestUnits(val); return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'var'; },
      Formatter_Apparent_Power: function(val) { var Units = this.BestUnits(val); return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'va'; },
      Formatter_Power_Factor: function(val) { return this.Round(val, 2, true); },
      Formatter_Phi: function(val) { return this.Round(val, 2, true) + ((val < 0) ? '° IND' : (val > 0) ? '° CAP' : '°'); }
    },
    mounted: function() {
      var Graphs = [this.$refs.g0.Graph, this.$refs.g1.Graph, this.$refs.g2.Graph, this.$refs.g3.Graph, this.$refs.g4.Graph];
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
    }
  }
</script>