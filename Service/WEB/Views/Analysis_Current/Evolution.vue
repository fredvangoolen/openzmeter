<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">From {{ (data!=null) ? analyzer.FormatAggregation(data.Aggreg) : '-' }} aggregation {{ MouseOver ? '(Graph paused)' : '' }}</div>
      <div class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="false" :current="true" v-model="Phase"></toolbar-phases>
        <div class="btn-group">
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
      return { MouseOver: false, Phase: null, Names: ['1st', '2nd', '3rd', '4th', '5th', '6th', '7th', '8th', '9th', '10th', '11th', '12th', '13th', '14th', '15th', '16th', '17th', '18th', '19th', '20th', '21th', '22th', '23th', '24th', '25th', '26th', '27th', '28th', '29th', '30th', '31th', '32th', '33th', '34th', '35th', '36th', '37th', '38th', '39th', '40th', '41th', '42th', '43th', '44th', '45th', '46th', '47th', '48th', '49th', '50th'] };
    },
    computed: {
      Data: function() {
        if((this.data == null) || (this.analyzer == null) || (this.Phase == null)) return null;
        var Series = {};
        Series.Sampletime = this.data.Sampletime;
        Series.From = this.data.From;
        Series.To = this.data.To;
        Series.TimeZone = this.analyzer.TimeZone;
        for(var n in this.Names) {
          var Serie = [];
          for(var i in this.data.Current_Harmonics) Serie.push(20 * Math.log10(Math.max(0.001, this.data.Current_Harmonics[i][this.Phase][n])));
          Series[this.Names[n]] = Serie;
        }
        return Series;
      },
      Export: function() {
        if(this.data == null) return [];
        var Data = [];
        var Header = ['Time'];
        for(var n in this.analyzer.HarmonicsNames) Header.concat(this.analyzer.GetSeriesNames(this.analyzer.HarmonicsNames[n], this.analyzer.CurrentChannels))
        Data.push(Header);
        for(var n in this.data.Sampletime) {
          var Row = [this.data.Sampletime[n]];
          for(var i in this.analyzer.HarmonicsNames) {
            for(var j in this.data.Current_Harmonics[n]) Row.push(this.data.Current_Harmonics[n][j][i]);
          }
          Data.push(Row);
        }
        return Data;
      }
    },
    methods: {
      Formatter: function(val) {
        return this.Round(val, 2, true) + ' dB';
      }
    },
    mounted: function() {
      var Graphs = [];
      for(var i in this.$refs['g']) Graphs.push(this.$refs['g'][i].Graph);
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
    }
  }
</script>