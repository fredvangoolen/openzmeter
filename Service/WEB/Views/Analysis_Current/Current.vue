<template>
  <div>
    <graph :options="GraphOptions" :height="height"></graph>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  export default {
    props: { analyzer: Object, data: Object, phases: Array, height: Number },
    data() {
      return { };
    },
    computed: {
      GraphOptions: function(val) {
        var CurrentStyle = oZm.SerieStyle(oZm.Series['CURRENT'], this.analyzer, 0);
        var CurrentNames = oZm.Series['CURRENT'].Names(this.analyzer);
        var Opt = { file: [], axes: { x: {}, y: {} } };
        Opt.strokeWidth = 0.5;
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.LegendFormatter_Follow;
        Opt.interactionModel = Graph.ScrollableInteractionModel(this);
        Opt.xAxisHeight = 24;
        Opt.selectMode = 'euclidian';
        Opt.visibility = this.phases;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { var Result = oZm.FormatSerieValue(x, oZm.Series['CURRENT']); return Result.ValueHTML + ' <em class="x-small">' + Result.Units + '</em>'; };
        Opt.axes.y.axisLabelFormatter = (x) => { return oZm.FormatSerieValue(x, oZm.Series['CURRENT']).ValueHTML; };
        Opt.axes.x.axisLabelFormatter = (x) => { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); };
        Opt.axes.x.valueFormatter = (x) => { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); };
        Opt.labels = ['Time'].concat(CurrentNames);
        Opt.colors = CurrentStyle.Colors;
        if(this.data != null) {
          for(var n in this.data.Sampletime) Opt.file.push([this.data.Sampletime[n]].concat(this.data.Current[n]));
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.phases), { defMin: 0, defMax: 1, percent: 0.01, minWidth: 10 });
        }
        return Opt;
      }
    },
    methods: {
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS('Current.csv', [['Time'].concat(oZm.Series['CURRENT'].Names(Analyzer))].concat(this.Data));
      },
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>