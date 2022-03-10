<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">From {{ (data!=null) ? analyzer.FormatAggregation(data.Aggreg) : '-' }} aggregation {{ MouseOver ? '(Graph paused)' : '' }}</div>
      <div class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="false" :current="false" v-model="Phase"></toolbar-phases>
        <z-button-group>
          <button-export file="Variables" :rows="Export"></button-export>
        </z-button-group>
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
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { analyzer: Object, data: Object, dataNow: Object },
    data: function() {
      return { Phase: null, MouseOver: false };
    },
    computed: {
      Data: function() {
        return Object.seal({ TimeZone: (this.analyzer == null) ? null : this.analyzer.TimeZone, 'Active Power': this.FillSerie('Active_Power'), 'Reactive Power': this.FillSerie('Reactive_Power'), 'Apparent Power': this.FillSerie('Apparent_Power'), 'Power Factor': this.FillSerie('Power_Factor'), 'Phi': this.FillSerie('Phi') });
      },
      DataNow: function() {
        if((this.dataNow == null) || (this.analyzer == null) || (this.Phase == null)) return null;
        var Series = { 'Active Power': null, 'Reactive Power': null, 'Apparent Power': null, 'Power Factor': null, 'Phi': null };
        Series['Active Power'] = this.dataNow.Active_Power[this.Phase];
        Series['Reactive Power'] = this.dataNow.Reactive_Power[this.Phase];
        Series['Apparent Power'] = this.dataNow.Apparent_Power[this.Phase];
        Series['Power Factor'] = this.dataNow.Power_Factor[this.Phase];
        Series['Phi'] = this.dataNow.Phi[this.Phase];
        return Object.seal(Series);
      }
    },
    methods: {
      Formatter_Active_Power: function(val) { var Units = this.BestUnits(val); return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'W'; },
      Formatter_Reactive_Power: function(val) { var Units = this.BestUnits(val); return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'var'; },
      Formatter_Apparent_Power: function(val) { var Units = this.BestUnits(val); return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'va'; },
      Formatter_Power_Factor: function(val) { return this.Round(val, 2, true); },
      Formatter_Phi: function(val) { return this.Round(val, 2, true) + ((val < 0) ? '° IND' : (val > 0) ? '° CAP' : '°'); },
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
        var Header = [['Sample']
          .concat(this.analyzer.SerieNames('Active Power'))
          .concat(this.analyzer.SerieNames('Reactive Power'))
          .concat(this.analyzer.SerieNames('Apparent Power'))
          .concat(this.analyzer.SerieNames('Power Factor'))
          .concat(this.analyzer.SerieNames('Phi'))
        ];
        var TimeCol = this.data.Headers.indexOf('Time');
        var ActiveCol = this.data.Headers.indexOf('Active_Power');
        var ReactiveCol = this.data.Headers.indexOf('Reactive_Power');
        var PowerFactorCol = this.data.Headers.indexOf('Power_Factor');
        var VoltageCol = this.data.Headers.indexOf('Voltage');
        var CurrentCol = this.data.Headers.indexOf('Current');
        var PhiCol = this.data.Headers.indexOf('Phi');
        for(var n = 0; n < this.data.Rows.length; n++) {
          var Row = [this.analyzer.FormatDateTime(this.data.Rows[n][TimeCol])];
          for(var i in this.data.Rows[n][ActiveCol]) Row.push(this.Round(this.data.Rows[n][ActiveCol][i], 3));
          for(var i in this.data.Rows[n][ReactiveCol]) Row.push(this.Round(this.data.Rows[n][ReactiveCol][i], 3));
          for(var i in this.data.Rows[n][VoltageCol]) Row.push(this.Round(this.data.Rows[n][VoltageCol][i] * this.data.Rows[n][CurrentCol][i], 3));
          for(var i in this.data.Rows[n][PowerFactorCol]) Row.push(this.Round(this.data.Rows[n][PowerFactorCol][i], 3));
          for(var i in this.data.Rows[n][PhiCol]) Row.push(this.Round(this.data.Rows[n][PhiCol][i], 3));
          Header.push(Row);
        }
        return Object.seal(Header);
      }
    },
    mounted: function() {
      var Graphs = [this.$refs.g0.Graph, this.$refs.g1.Graph, this.$refs.g2.Graph, this.$refs.g3.Graph, this.$refs.g4.Graph];
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
</style>