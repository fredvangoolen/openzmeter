<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">From {{ (data!=null) ? analyzer.FormatAggregation(data.Aggregation_Source) : '-' }} aggregation {{ Paused ? '(Paused)' : '' }}</div>
      <div class="header-buttons" ref="Toolbar">
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="false" :current="false" v-model="Phase"></toolbar-phases>
        <div class="btn-group">

        </div>
        <!--toolbar-tools variable="To" ref="tools" :options="['EXPORT']" v-on:ExportClick="ExportClick"></toolbar-tools-->
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="data==null"></spinner>
      <cubism :index="0" ref="g0" title="Power Factor" :data="Data" :now="DataNow" :format="Formatter_Power_Factor" :paused="Paused!=0" @mouseOver="(val) => { Paused+= val; }"></cubism>
      <cubism :index="1" ref="g1" title="Voltage THD" :data="Data" :now="DataNow" :format="Formatter_Voltage_THD" :paused="Paused!=0" @mouseOver="(val) => { Paused+= val; }"></cubism>
      <cubism :index="2" ref="g2" title="Phi" :data="Data" :now="DataNow" :format="Formatter_Phi" :paused="Paused!=0" @mouseOver="(val) => { Paused+= val; }"></cubism>
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
      return { Phase: null, Paused: 0 };
    },
    computed: {
      Data: function() {
        if((this.data == null) || (this.analyzer == null) || (this.Phase == null)) return null;
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
        return Series;
      },
      DataNow: function() {
        if((this.dataNow == null) || (this.analyzer == null) || (this.Phase == null)) return null;
        var Series = { 'Power Factor': null, 'Voltage THD': null, 'Phi': null };
        Series['Power Factor'] = this.dataNow.Power_Factor[this.Phase];
        Series['Voltage THD'] = this.dataNow.Voltage_THD[this.Phase];
        Series['Phi'] = this.dataNow.Phi[this.Phase];
        return Series;
      }
    },
    methods: {
      Formatter_Power_Factor: function(val) { return this.Round(val, 2, true); },
      Formatter_Voltage_THD: function(val) { return this.Round(val, 2, true) + ' %'; },
      Formatter_Phi: function(val) { return this.Round(val, 2, true); },
      MouseOver: function(val) {
        this.mouseOverCount = this.mouseOverCount + ((val === true) ? +1 : -1);
      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Header = ['Date/Time'];
        for(var i = 0; i < this.Series.length; i++) {
          for(var n = 0; n < oZm.Series[this.Series[i]].Names(Analyzer).length; n++) Header.push(oZm.Series[this.Series[i]].Names(Analyzer)[n]);
        }
        var Data = [];
        for(var j = 0; j < this.Data[this.Series[0]].length; j++) {
          var Row = [moment(this.Data[this.Series[0]][j][0]).format('HH:mm:ss')];
          for(var i = 0; i < this.Series.length; i++) {
            for(var n = 0; n < oZm.Series[this.Series[i]].Names(Analyzer).length; n++) Row.push(this.Data[this.Series[i]][j][n + 1]);
          }
          Data.push(Row);
        }
        this.$refs.tools.SaveCVS('VoltageParameters.csv', [Header].concat(Data));
      }
    },
    mounted: function() {
      var Graphs = [this.$refs.g0.Graph, this.$refs.g1.Graph, this.$refs.g2.Graph];
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
    }
  }
</script>