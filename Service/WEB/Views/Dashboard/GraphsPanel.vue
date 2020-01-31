<template>
  <div ref="panel" class="panel" :class="(serie=='VOLTAGE') ? 'panel-voltage' : (serie=='CURRENT') ? 'panel-current' : (serie=='ACTIVE_POWER') ? 'panel-active-power' : 'panel-frequency'">
    <div class="panel-heading">
      <div class="huge header-text">{{ (serie=='VOLTAGE') ? 'Voltage' : (serie=='CURRENT') ? 'Current' : (serie=='ACTIVE_POWER') ? 'Active Power' : 'Frequency' }}</div>
      <div class="header-buttons">
        <div style="margin-top: -3px">
          <div style="display: inline-block; font-size: 24px; padding-right: 4px; vertical-align: top"><span v-html="LastValue"></span> <em class="huge-small-unit">{{ UnitsScale }}{{ SerieProps.Units }}</em>
            <div style="font-size: 14px; line-height: 14px" v-if="data!=null">From {{ analyzer.FormatAggregation(data.Aggregation_Source) }}</div>
          </div>
          <div v-if="PhasesValue.length > 1" style="display: inline-block; border-left: 1px solid #FFF; padding-left: 2px; line-height: 12px; font-size: 12px; vertical-align: top"><div v-for="(val, idx) in PhasesValue" :key="idx">{{ val }}</div></div>
       </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="data==null"></spinner>
      <graph ref="Chart" :options="GraphOptions" :height="150"></graph>
    </div>
    <div class="panel-footer clearfix">
      <router-link :to="SerieProps.Link" style="line-height: 22px"><span class="pull-left">View Details</span></router-link>
      <div class="pull-right" ref="Toolbar">
        <toolbar-phases :analyzer="analyzer" v-model="Phases" v-if="serie != 'FREQUENCY'" :variable="'Dashboard_' + serie" :current="serie=='CURRENT'" :multi="true"></toolbar-phases>
        <div class="btn-group">
          <button-export :rows="[]" ></button-export>
          <button-expand></button-expand>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';
  import 'JS/dygraph-crosshair.js';
  export default {
    props: { serie: String, analyzer: Object, data: Object },
    data() {
      return { ShowSpinner: true, PhasesValue: [], LastValue:'', Phases: null, UnitsScale: '', SerieProps: {} };
    },
    computed: {
      GraphOptions: function() {
        var Opt = { file: [], axes: { x: {}, y: {} }};
        Opt.strokeWidth = 0.5;
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.legend = 'follow';
        Opt.labels = ['Time'].concat(this.SerieProps.Names(this.analyzer));
        Opt.colors = oZm.SerieStyle(this.SerieProps, this.analyzer, 0).Colors;
        Opt.selectMode = 'euclidian';
        Opt.plugins = [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ];
        Opt.xAxisHeight = 12;
        Opt.legendFormatter = Graph.LegendFormatter_Follow;
        if(this.Phases != null) Opt.visibility =  this.Phases;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 30;
        Opt.axes.y.valueFormatter = (x) => { var Result = oZm.FormatSerieValue(x, this.SerieProps); return Result.ValuePlain + ' <em class="x-small">' + Result.Units + '</em>'; };
        Opt.axes.y.axisLabelFormatter = (x) => { return oZm.FormatSerieValue(x, this.SerieProps, oZm.GetBestUnits(Math.max(Math.abs(Range[0]), Math.abs(Range[1])))).ValuePlain; };
        Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatTime(x); };
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatTime(x); };
        if(this.data != null) {
          Opt.dateWindow = [this.data.From, this.data.To];
          for(var n in this.data.Sampletime) {
            var Row = [this.data.Sampletime[n]];
            if(this.serie == 'VOLTAGE') {
              for(var i in this.data.Voltage[n]) Row.push(this.data.Voltage[n][i]);
            } else if(this.serie == 'CURRENT') {
              for(var i in this.data.Current[n]) Row.push(this.data.Current[n][i]);
            } else if(this.serie == 'ACTIVE_POWER') {
              for(var i in this.data.Active_Power[n]) Row.push(this.data.Active_Power[n][i]);
            } else if(this.serie == 'FREQUENCY') {
              Row.push(this.data.Frequency[n]);
            }
            Opt.file.push(Row);
          }
        }
        return Opt;
      }
    },
    methods: {
      UpdateGraph() {
//        if(this.$parent.Sync == false) {
//          var Graphs = [];
//          if(this.$parent.$refs.voltage.$refs.Chart.Graph) Graphs.push(this.$parent.$refs.voltage.$refs.Chart.Graph);
//          if(this.$parent.$refs.current.$refs.Chart.Graph) Graphs.push(this.$parent.$refs.current.$refs.Chart.Graph);
//          if(this.$parent.$refs.active_power.$refs.Chart.Graph) Graphs.push(this.$parent.$refs.active_power.$refs.Chart.Graph);
//          if(this.$parent.$refs.frequency.$refs.Chart.Graph) Graphs.push(this.$parent.$refs.frequency.$refs.Chart.Graph);
//          //, this.$parent.$refs.current.$refs.Chart.Graph, this.$parent.$refs.active_power.$refs.Chart.Graph, this.$parent.$refs.frequency.$refs.Chart.Graph];
//          if(Graphs.length == 4) {
//            Dygraph.synchronize(Graphs, { range: false, zoom: false });
//            this.$parent.Sync = true;
//          }
//        }
//        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '3S', Series: [this.serie], To: 0, LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.Analyzer, Period: 7200000 }),
          success: (result) => {
//            if((result['Sampletime'] === null) || (result['Sampletime'].length === 0)) return;
//            this.Data = oZm.GraphDataAppend(result, this.SerieProps.Serie, this.Data);
//            var LastSample = result[this.SerieProps.Serie].slice(-1)[0];
            if(Array.isArray(LastSample) === false) {
              this.LastValue = oZm.FormatSerieValue(LastSample, this.SerieProps).ValueHTML;
              this.PhasesValue = [];
            } else {
              this.LastValue = 0;
              this.PhasesValue = [];
              this.UnitsScale = oZm.GetBestUnits(oZm.ArrayAbsMax(LastSample));
              if(this.type === 'ACTIVE_POWER') {
                for(var n = 0; n < LastSample.length; n++) this.LastValue = this.LastValue + LastSample[n];
              } else {
                var PhasesCount = oZm.Series['VOLTAGE'].Names(this.analyzer).length;
                for(var n = 0; n < PhasesCount; n++) this.LastValue = this.LastValue + Math.pow(LastSample[n], 2.0);
                this.LastValue = Math.sqrt(this.LastValue / PhasesCount);
              }
              this.LastValue = oZm.FormatSerieValue(this.LastValue, this.SerieProps, this.UnitsScale).ValueHTML;
              this.PhasesValue = LastSample.map((n) => {
                return oZm.FormatSerieValue(n, this.SerieProps, this.UnitsScale).ValuePlain;
              });
            }
//            this.DateWindow = [result['From'], result['To']];
//            this.LastSample = oZm.LastSampletime(result, this.LastSample);
//            if(this.Data.length > 0) this.CalcOptions();
//            this.ShowSpinner = false;
          },
//          complete: () => {
//            this.UpdateGraphTimer = setTimeout(this.UpdateGraph, 5000);
//          },
//          error: () => {
//            this.ShowSpinner = true;
//          }
        });
      },
//      SetPhases(phases) {
//        this.Phases = phases;
//        if(this.Data.length > 0) this.CalcOptions();
//      },
      MaximizeClick() {
        this.$refs.tools.Maximize(this.SerieProps.Title, this.$refs.Chart.Graph, this.$refs.Toolbar);
      },
      MinimizeClick() {
        this.$refs.Chart.Redraw();
      },
      ExportClick() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS(this.SerieProps.Title + '.csv', [['Time'].concat(this.SerieProps.Names(Analyzer))].concat(this.Data));
      },
//      SetAnalyzer() {
//        this.LastSample = 0;
//        this.Data = [];
//        this.ShowSpinner = true;
//        this.UpdateGraph();
//      }
    },
    created: function() {
//      this.Data = [];
//      this.DateWindow = [0, 0];
//      this.LastSample = 0;
//      this.UpdateGraphTimer = null;
//      this.Phases = null;
      this.SerieProps = oZm.Series[this.serie];
    },
//    beforeDestroy: function() {
//      if(this.UpdateGraphTimer !== null) clearTimeout(this.UpdateGraphTimer);
//    },
//    mounted: function() {
//      this.SetAnalyzer();
//    }
  }
</script>