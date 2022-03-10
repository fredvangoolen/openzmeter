<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">Main variables</div>
      <div class="float-right">
        <span v-if="data!=null">From {{ analyzer.FormatAggregation(data.Aggreg) }} aggregation {{ Paused ? '(Paused)' : '' }}</span>
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" v-model="Phase"></toolbar-phases>
        <z-button-group>
          <button-export file="Main_Variables" :rows="Export"></button-export>
        </z-button-group>
      </div>
    </div>
    <div class="panel-body" @mouseenter="Paused=true" @mouseleave="Paused=false">
      <spinner v-if="data==null"></spinner>
      <cubism :index="0" ref="g0" title="Active Power" :data="Data" :now="DataNow" :format="Formatter_Active_Power" :paused="Paused" @init="Init"></cubism>
      <cubism :index="1" ref="g1" title="Reactive Power" :data="Data" :now="DataNow" :format="Formatter_Reactive_Power" :paused="Paused" @init="Init"></cubism>
      <cubism :index="2" ref="g2" title="Power Factor" :data="Data" :now="DataNow" :format="Formatter_Power_Factor" :paused="Paused" @init="Init"></cubism>
      <cubism :index="3" ref="g3" title="Phi" :data="Data" :now="DataNow" :format="Formatter_Phi" :paused="Paused" @init="Init"></cubism>
      <cubism :index="4" ref="g4" title="Voltage THD" :data="Data" :now="DataNow" :format="Formatter_Percent" :paused="Paused" @init="Init"></cubism>
      <cubism :index="5" ref="g5" title="Current THD" :data="Data" :now="DataNow" :format="Formatter_Percent" :paused="Paused" @init="Init"></cubism>
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
      return { Linked: false, Phase: null, Paused: false };
    },
    computed: {
      Data: function() {
        return Object.seal({ TimeZone: (this.analyzer == null) ? null : this.analyzer.TimeZone, 'Active Power': this.FillSerie('Active_Power'), 'Reactive Power': this.FillSerie('Reactive_Power'), 'Power Factor': this.FillSerie('Power_Factor'), 'Phi': this.FillSerie('Phi'), 'Voltage THD': this.FillSerie('Voltage_THD'), 'Current THD': this.FillSerie('Current_THD') });
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
        return Object.seal(Series);
      }
    },
    methods: {
      Formatter_Active_Power: function(val) { var Units = this.BestUnits(val); return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'W'; },
      Formatter_Reactive_Power: function(val) { var Units = this.BestUnits(val); return this.Round(this.ScaleUnits(val, Units), 2, true) + ' ' + Units + 'var'; },
      Formatter_Power_Factor: function(val) { return this.Round(val, 2, true); },
      Formatter_Phi: function(val) { return this.Round(val, 2, true) + ((val < 0) ? '° IND' : (val > 0) ? '° CAP' : '°'); },
      Formatter_Percent: function(val) { return this.Round(val, 2, true) + ' %'; },
      FillSerie: function(name) {
        var Data = [];
        if(this.data == null) return { From: null, To: null, TimeZone: null, Rows: Data };
        var TimeCol = this.data.Headers.indexOf('Time');
        var DataCol = this.data.Headers.indexOf(name);
        for(var i = 0; i < this.data.Rows.length; i++) Data.push([this.data.Rows[i][TimeCol], this.data.Rows[i][DataCol][this.Phase]]);
        return { From: this.data.From, To: this.data.To, TimeZone: this.analyzer.TimeZone, Rows: Data };
      },
      Export: function() {
        if((this.data == null) || (this.analyzer == null)) return [];
        var Header = [['Time']
          .concat(this.analyzer.SerieNames('Active Power'))
          .concat(this.analyzer.SerieNames('Reactive Power'))
          .concat(this.analyzer.SerieNames('Power Factor'))
          .concat(this.analyzer.SerieNames('Phi'))
          .concat(this.analyzer.SerieNames('Voltage THD'))
          .concat(this.analyzer.SerieNames('Current THD'))
        ];
        var TimeCol = this.data.Headers.indexOf('Time');
        var ActiveCol = this.data.Headers.indexOf('Active_Power');
        var ReactiveCol = this.data.Headers.indexOf('Reactive_Power');
        var PowerFactorCol = this.data.Headers.indexOf('Power_Factor');
        var PhiCol = this.data.Headers.indexOf('Phi');
        var VoltageTHDCol = this.data.Headers.indexOf('Voltage_THD');
        var CurrentTHDCol = this.data.Headers.indexOf('Current_THD');
        for(var n = 0; n < this.data.Rows.length; n++) {
          var Row = [this.analyzer.FormatDateTime(this.data.Rows[n][TimeCol])];
          for(var i in this.data.Rows[n][ActiveCol]) Row.push(this.Round(this.data.Rows[n][ActiveCol][i], 3));
          for(var i in this.data.Rows[n][ReactiveCol]) Row.push(this.Round(this.data.Rows[n][ReactiveCol][i], 3));
          for(var i in this.data.Rows[n][PowerFactorCol]) Row.push(this.Round(this.data.Rows[n][PowerFactorCol][i], 3));
          for(var i in this.data.Rows[n][PhiCol]) Row.push(this.Round(this.data.Rows[n][PhiCol][i], 3));
          for(var i in this.data.Rows[n][VoltageTHDCol]) Row.push(this.Round(this.data.Rows[n][VoltageTHDCol][i], 3));
          for(var i in this.data.Rows[n][CurrentTHDCol]) Row.push(this.Round(this.data.Rows[n][CurrentTHDCol][i], 3));
          Header.push(Row);
        }
        return Object.seal(Header);
      },
      Init: function() {
        if((this.Linked == true) || (this.$refs.g0 == null) || (this.$refs.g1 == null) || (this.$refs.g2 == null) || (this.$refs.g3.Graph == null) || (this.$refs.g4.Graph == null) || (this.$refs.g5.Graph == null)) return;
        var Graphs = [ this.$refs.g0.Graph, this.$refs.g1.Graph, this.$refs.g2.Graph, this.$refs.g3.Graph, this.$refs.g4.Graph, this.$refs.g5.Graph ];
        Dygraph.synchronize(Graphs, { range: false, zoom: false });
        this.Linked = true;
      }
    }
  }
</script>