<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">From {{ (data!=null) ? analyzer.FormatAggregation(data.Aggreg) : '-' }} aggregation {{ MouseOver ? '(Graph paused)' : '' }}</div>
      <div class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="false" :current="true" v-model="Phase"></toolbar-phases>
        <toolbar-options storage="DetailsOptions" :options="['Log']" v-model="Options"></toolbar-options>
        <div class="btn-group">
          <button type="button" class="btn btn-default btn-xs" v-if="Names.length==20" title="Show full 50 harmonics view" @click="ShowAll"><img :src="require('Imgs/table_rows_insert_below_word.png')"></button>
          <button-export file="Evolution" :rows="Export"></button-export>
        </div>
      </div>
    </div>
    <div class="panel-body" @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
      <spinner v-if="data==null"></spinner>
      <cubism ref="g" v-for="(serie, index) in Names" :key="index" :index="index" :data="Data" :title="serie" :format="Formatter"></cubism>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { data: Object },
    data: function() {
      return { MouseOver: false, Options: null, Phase: null, Names: [] };
    },
    computed: {
      Data: function() {
        var Series = { TimeZone: (this.analyzer == null) ? null : this.analyzer.TimeZone };
        for(var n = 0; n < this.Names.length; n++) Series[this.Names[n]] = this.FillSerie(n);
        return Object.seal(Series);
      }
    },
    created: function() {
      if(this.analyzer != null) this.Names = this.analyzer.HarmonicsNames.slice(0, 20);
    },
    watch: {
      analyzer: function() {
        if(this.analyzer != null) this.Names = this.analyzer.HarmonicsNames.slice(0, 20);
      }
    },
    methods: {
      ShowAll: function() {
        if(this.analyzer != null) this.Names = this.analyzer.HarmonicsNames;
      },
      Formatter: function(val) {
        return this.Round(val, 2, true) + (this.Options['Log'] ? 'dB' : 'A');
      },
      FillSerie: function(idx) {
        var Data = [];
        if((this.data == null) || (this.Options == null) || (this.Phase == null)) return { From: null, To: null, TimeZone: null, Rows: Data };
        var TimeCol = this.data.Headers.indexOf('Time');
        var DataCol = this.data.Headers.indexOf('Current_Harmonics');
        for(var i = 0; i < this.data.Rows.length; i++) Data.push([this.data.Rows[i][TimeCol], this.Options['Log'] ? 20 * Math.log10(Math.max(0.001, this.data.Rows[i][DataCol][this.Phase][idx])) : this.data.Rows[i][DataCol][this.Phase][idx]]);
        return { From: this.data.From, To: this.data.To, TimeZone: this.analyzer.TimeZone, Rows: Data };
      },
      Export: function() {
        if(this.data == null) return [];
        var Data = [];
        var Header = ['Time'];
        for(var n in this.analyzer.HarmonicsNames) Header = Header.concat(this.analyzer.SerieNames(this.analyzer.HarmonicsNames[n]));
        Data.push(Header);
        var TimeCol = this.data.Headers.indexOf('Time');
        var DataCol = this.data.Headers.indexOf('Current_Harmonics');
        for(var n = 0; n < this.data.Rows.length; n++) {
          var Row = [this.analyzer.FormatDateTime(this.data.Rows[n][TimeCol])];
          for(var i = 0; i < this.analyzer.HarmonicsNames.length; i++) {
            for(var j = 0; j < this.data.Rows[n][DataCol].length; j++) Row.push(this.Round(this.data.Rows[n][DataCol][j][i], 3));
          }
          Data.push(Row);
        }
        return Object.seal(Data);
      }
    },
    mounted: function() {
      var Graphs = [];
      for(var i in this.$refs['g']) Graphs.push(this.$refs['g'][i].Graph);
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
</style>