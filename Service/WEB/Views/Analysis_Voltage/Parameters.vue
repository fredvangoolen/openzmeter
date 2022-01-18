<template>
  <q-tab-panel class="q-pa-none">
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">From {{ (data!=null) ? analyzer.FormatAggregation(data.Aggreg) : '-' }} aggregation {{ MouseOver ? '(Graph paused)' : '' }}</div>
      <div class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :multi="false" :current="false" v-model="Phase"></toolbar-phases>
        <div class="btn-group">
          <button-export file="Variables" :rows="Export"></button-export>
        </div>
      </div>
    </div>
    <div class="panel-body" @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
      <spinner v-if="data==null"></spinner>
      <cubism :index="0" ref="g0" title="Power Factor" :data="Data" :now="DataNow" :format="Formatter_NoUnits" :paused="MouseOver"></cubism>
      <cubism :index="1" ref="g1" title="Voltage THD" :data="Data" :now="DataNow" :format="Formatter_Percent" :paused="MouseOver"></cubism>
      <cubism :index="2" ref="g2" title="Phi" :data="Data" :now="DataNow" :format="Formatter_NoUnits" :paused="MouseOver"></cubism>
      <cubism v-show="analyzer.VoltageChannels>1" :index="3" ref="g3" title="Unbalance" :data="Data" :now="DataNow" :format="Formatter_Percent" :paused="MouseOver"></cubism>
    </div>
  </div>
  </q-tab-panel>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
//  import moment from 'moment';
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { data: Object, dataNow: Object },
    data: function() {
      return { Phase: null, MouseOver: false };
    },
    computed: {
      Data: function() {
        return Object.seal({ TimeZone: (this.analyzer == null) ? null : this.analyzer.TimeZone, 'Power Factor': this.FillSerie('Power_Factor'), 'Voltage THD': this.FillSerie('Voltage_THD'), 'Phi': this.FillSerie('Phi'), 'Unbalance': this.FillSerie('Unbalance') });
      },
      DataNow: function() {
        if((this.dataNow == null) || (this.Phase == null)) return null;
        var Series = { 'Power Factor': null, 'Voltage THD': null, 'Phi': null, 'Unbalance': null };
        Series['Power Factor'] = this.dataNow.Power_Factor[this.Phase];
        Series['Voltage THD'] = this.dataNow.Voltage_THD[this.Phase];
        Series['Phi'] = this.dataNow.Phi[this.Phase];
        Series['Unbalance'] = this.dataNow.Unbalance;
        return Object.seal(Series);
      }
    },
    methods: {
      Formatter_Percent: function(val) { return this.Round(val, 2, true) + ' %'; },
      Formatter_NoUnits: function(val) { return this.Round(val, 2, true); },
      FillSerie: function(name) {
        var Data = [];
        if(this.data == null) return { From: null, To: null, TimeZone: null, Rows: Data };
        var TimeCol = this.data.Headers.indexOf('Time');
        var DataCol = this.data.Headers.indexOf(name);
        for(var i = 0; i < this.data.Rows.length; i++) Data.push([this.data.Rows[i][TimeCol], this.data.Rows[i][DataCol][this.Phase]]);
        return { From: this.data.From, To: this.data.To, TimeZone: this.analyzer.TimeZone, Rows: Data };
      },
      Export: function() {
        if(this.data == null) return [];
        var Header = [['Time']
          .concat(this.analyzer.SerieNames('Power Factor'))
          .concat(this.analyzer.SerieNames('Voltage THD'))
          .concat(this.analyzer.SerieNames('Phi'))
        ];
        if(this.analyzer.VoltageChannels > 1) Header[0].concat('Unbalance');
        var TimeCol = this.data.Headers.indexOf('Time');
        var PowerFactorCol = this.data.Headers.indexOf('Power_Factor');
        var VoltageTHDCol = this.data.Headers.indexOf('Voltage_THD');
        var PhiCol = this.data.Headers.indexOf('Phi');
        var UnbalanceCol = this.data.Headers.indexOf('Unbalance');
        for(var n = 0; n < this.data.Rows.length; n++) {
          var Row = [this.analyzer.FormatDateTime(this.data.Rows[n][TimeCol])];
          for(var i in this.data.Rows[n][PowerFactorCol]) Row.push(this.Round(this.data.Rows[n][PowerFactorCol][i], 3));
          for(var i in this.data.Rows[n][VoltageTHDCol]) Row.push(this.Round(this.data.Rows[n][VoltageTHDCol][i], 3));
          for(var i in this.data.Rows[n][PhiCol]) Row.push(this.Round(this.data.Rows[n][PhiCol][i], 3));
          if(this.analyzer.VoltageChannels > 1) Row.push(this.Round(this.data.Rows[n][UnbalanceCol][i], 3));
          Header.push(Row);
        }
        return Object.seal(Header);
      }
    },
    mounted: function() {
      var Graphs = [this.$refs.g0.Graph, this.$refs.g1.Graph, this.$refs.g2.Graph];
      if(this.analyzer.VoltageChannels > 1) Graphs.push(this.$refs.g3.Graph);
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
</style>