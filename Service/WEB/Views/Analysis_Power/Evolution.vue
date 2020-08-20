<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">From {{ (data!=null) ? analyzer.FormatAggregation(data.Aggreg) : '-' }} aggregation {{ MouseOver ? '(Graph paused)' : '' }}</div>
      <div class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="false" :current="false" v-model="Phase"></toolbar-phases>
        <div class="btn-group">
          <button type="button" class="btn btn-default btn-xs" v-if="Names.length==20" data-toggle="tooltip" title="Show full 50 harmonics view" @click="Names=Names.concat(['21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th', '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th', '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th'])"><img :src="require('Imgs/table_rows_insert_below_word.png')"></button>
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
    props: { analyzer: Object, data: Object },
    data() {
      return { MouseOver: false, Phase: null, Names: ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th'] };
    },
    computed: {
      Data: function() {
        var Series = { TimeZone: (this.analyzer == null) ? null : this.analyzer.TimeZone };
        for(var n = 0; n < this.Names.length; n++) Series[this.Names[n]] = this.FillSerie(n);
        return Object.seal(Series);
      }
    },
    methods: {
      Formatter: function(x) {
        var Units = this.BestUnits(x);
        return this.Round(this.ScaleUnits(x, Units), 2, true) + ' ' + Units + 'W';
      },
      FillSerie: function(idx) {
        var Data = [];
        if((this.data == null) || (this.Phase == null)) return { From: null, To: null, TimeZone: null, Rows: Data };
        var TimeCol = this.data.Headers.indexOf('Sampletime');
        var DataCol = this.data.Headers.indexOf('Power_Harmonics');
        for(var i = 0; i < this.data.Rows.length; i++) Data.push([this.data.Rows[i][TimeCol], this.data.Rows[i][DataCol][this.Phase][idx]]);
        return { From: this.data.From, To: this.data.To, TimeZone: this.analyzer.TimeZone, Rows: Data };
      },
      Export: function() {
        if(this.data == null) return [];
        var Data = [];
        var Header = ['Time'];
        for(var n in this.analyzer.HarmonicsNames) Header = Header.concat(this.analyzer.GetSeriesNames(this.analyzer.HarmonicsNames[n], this.analyzer.VoltageChannels))
        Data.push(Header);
        for(var n in this.data.Sampletime) {
          var Row = [this.data.Sampletime[n]];
          for(var i in this.analyzer.HarmonicsNames) {
            for(var j in this.data.Power_Harmonics[n]) Row.push(this.Round(this.data.Power_Harmonics[n][j][i], 3));
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