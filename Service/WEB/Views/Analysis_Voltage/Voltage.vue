<template>
  <div @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
    <graph :options="GraphOptions" :height="height" :paused="MouseOver"></graph>
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
      return { MouseOver: false };
    },
    computed: {
      GraphOptions: function(val) {
        var VoltageStyle = oZm.SerieStyle(oZm.Series['VOLTAGE'], this.analyzer, 0);
        var VoltageNames = oZm.Series['VOLTAGE'].Names(this.analyzer);
        var Opt = { file: [], axes: { x: {}, y: {} } };
        Opt.panEdgeFraction = 1;
        Opt.interactionModel = Graph.InteractionModels.Scrollable(this);
        Opt.strokeWidth = 0.5;
        Opt.stepPlot = true;
        Opt.showRangeSelector = false;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.gridLineColor = "#BBB";
        Opt.gridLinePattern = [4, 4];
        Opt.legend = 'follow';
        Opt.xAxisHeight = 12;
        Opt.selectMode = 'euclidian';
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.visibility = this.phases;
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { return Graph.Formatters.Y_UnitScale(x, 'V', 2, true); };
        Opt.axes.y.axisLabelFormatter = (x, gran, opts, g) => { return Graph.Formatters.Y_UnitScaleFull(x, gran, opts, g, '', 2, true); };
        Opt.axes.x.ticker = (min, max, pixels) => { return Graph.Formatters.X_HourlyFull(min, max, pixels, (this.analyzer != null) ? this.analyzer.TimeZone : null); };
        Opt.axes.x.valueFormatter = (x) => { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); };
        Opt.labels = ['Time'].concat(VoltageNames);
        Opt.colors = VoltageStyle.Colors;
        if(this.data != null) {
          var TimeCol = this.data.Headers.indexOf('Sampletime');
          var ValueCol = this.data.Headers.indexOf('Voltage');
          for(var n = 0; n < this.data.Rows.length; n++) Opt.file.push([this.data.Rows[n][TimeCol]].concat(this.data.Rows[n][ValueCol]));
          Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.phases), { defMin: 0, defMax: 1, percent: 0.01, minWidth: 10 });
          Opt.dateWindow = [this.data.From, this.data.To];
        }
        return Object.seal(Opt);
      }
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 24px;
    line-height: 16px;
  }
</style>