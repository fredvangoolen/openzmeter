<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTime(data.Sampletime) }}</div>
        <div class="header-buttons" ref="Toolbar">
          <toolbar-phases storage="DetailsPhases" :analyzer="analyzer" :multi="false" :current="false" v-model="Phase"></toolbar-phases>
          <div class="btn-group">

          </div>
          <!--toolbar-tools variable="To" ref="tools" :options="['EXPORT']" v-on:ExportClick="ExportClick"></toolbar-tools-->
        </div>
      </div>
    <div class="panel-body">
      <spinner v-if="data==null"></spinner>
      <cubism :index="0" ref="g0" title="Power Factor" :data="Data" :format="Formatter_Power_Factor"></cubism>
      <cubism :index="1" ref="g1" title="Current THD" :data="Data" :format="Formatter_Current_THD"></cubism>
      <cubism :index="2" ref="g2" title="Phi" :data="Data" :format="Formatter_Phi"></cubism>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { analyzer: Object, data: Object },
    data() {
      return { Phase: null };
    },
    computed: {
      Data: function() {
        if((this.data == null) || (this.analyzer == null) || (this.Phase == null)) return null;
        var Series = { 'Power Factor': [], 'Current THD': [], 'Phi': [] };
        Series.Sampletime = this.data.Sampletime;
        Series.From = this.data.From;
        Series.To = this.data.To;
        Series.TimeZone = this.analyzer.TimeZone;
        for(var i in this.data.Sampletime) {
          Series['Power Factor'].push(this.data.Power_Factor[i][this.Phase]);
          Series['Current THD'].push(this.data.Current_THD[i][this.Phase]);
          Series['Phi'].push(this.data.Phi[i][this.Phase]);
        }
        return Series;
      }
    },
    methods: {
      Formatter_Power_Factor: function(val) { return this.Round(val, 2, true); },
      Formatter_Current_THD: function(val) { return this.Round(val, 2, true) + ' %'; },
      Formatter_Phi: function(val) { return this.Round(val, 2, true); },

      MouseOver: function(val) {
        this.mouseOverCount = this.mouseOverCount + ((val === true) ? +1 : -1);
      },
      Redraw() {
        this.$nextTick(function() {
          for(var n = 0; n < this.Series.length; n++) $(this.$refs.rows[n].Graph.graphDiv).parent().removeAttr('style');
          window.dispatchEvent(new Event('resize'));
        });
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
        this.$refs.tools.SaveCVS('CurrentParameters.csv', [Header].concat(Data));
      },
      SetPhases: function(phase) {
        this.$refs.phases.SetPhases(phase);
        this.Phase = phase;
        this.CalcOptions();
      },
      SetData: function(result) {
        if(result['Sampletime'] === null) return;
        if(result['Sampletime'].length === 0) return;
        if(this.Data === null) this.Data = [];
        this.DateWindow = [result['From'], result['To']];
        this.Aggregation = 'From ' + this.analyzer.GetAggregLongName(result['Aggregation_Source']) + ' aggregation';
        for(var i = 0; i < this.Series.length; i++) this.Data[this.Series[i]] = oZm.GraphDataAppend(result, oZm.Series[this.Series[i]].Serie, this.Data[this.Series[i]]);
        if(this.mouseOverCount === 0) this.CalcOptions();
        this.ShowSpinner = false;
      },
      SetDataNow(result) {
        if(this.mouseOverCount !== 0) return;
        for(var n = 0; n < this.Series.length; n++) this.$refs.rows[n].SetNowLabel(result[oZm.Series[this.Series[n]].Serie][this.Phase]);
      }
    },
    mounted: function() {
      var Graphs = [this.$refs['g0'].Graph, this.$refs['g1'].Graph, this.$refs['g2'].Graph];
      Dygraph.synchronize(Graphs, { range: false, zoom: false });
    }
  }
</script>