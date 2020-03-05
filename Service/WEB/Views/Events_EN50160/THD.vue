<template>
  <div class="panel panel-reactive-power">
    <div class="panel-heading">
      <div class="float-left">Harmonic distorsion</div>
      <div class="float-right" v-if="Warning"><img :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Limit exceded"></div>
      <div class="clearfix"></div>
    </div>
    <div class="panel-body">
      <graph :options="GraphOptions" :height="200"></graph>
    </div>
    <div class="panel-footer">
      <div class="text-center">100% of the year under 8%</div>
        <progress-bar :value="THD" :text="THDText"></progress-bar>
        <div style="float: left; font-size: 10px">90%</div>
        <div style="float:right; font-size: 10px">100%</div>
        <div class="clearfix text-center">{{ THDText }}</div>
    </div>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    props: { data: Array, index: Number, analyzer: Object, ranges: Array },
    data: function() {
      return {THD: 0, THDText: '', ShowSpinner: true, Warning: true };
    },
    computed: {
      GraphOptions: function() {
        var Opt = { file: [], axes: { x: {}, y: {} } };
        Opt.strokeWidth = 1;
        Opt.labels = ["Distorsion", "Time"];
        Opt.stepPlot = true;
        Opt.plotter = Graph.Plotters.MultiBar;
        Opt.colors = [this.analyzer.Color.Reactive_Power];
        Opt.interactionModel = { };
        Opt.showRangeSelector = false;
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.drawHighlightPointCallback = Graph.Callbacks.DrawHighlightPoint_HighlightBar;
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.underlayCallback = Graph.Callbacks.Underlay_DrawBorder;
        Opt.legend = 'follow';
        Opt.dateWindow = [-0.5, this.ranges.length - 1.5];
        Opt.axes.y.pixelsPerLabel = 24;
        Opt.axes.y.axisLabelWidth = 24;
        Opt.axes.x.pixelsPerLabel = 30;
        Opt.axes.x.axisLabelWidth = 30;
        Opt.axes.x.valueFormatter = (x) => {
          if((x < 0) || (x >= this.ranges.length - 1)) return "";
          if(x == (this.ranges.length - 2)) {
            return "More than " + this.Round(this.ranges[x], 2) + ' %';
          } else {
            return "From " + this.Round(this.ranges[x], 2) + " to " + this.Round(this.ranges[x + 1], 2) + ' %';
          }
        };
        Opt.axes.x.axisLabelFormatter = (x) => {
          if((x < 0) || (x >= this.ranges.length - 1)) return "";
          if(x == (this.ranges.length - 2)) {
            return '≥' + this.Round(this.ranges[x], 2);
          } else {
            return this.Round(this.ranges[x], 2) + "<br/>" + this.Round(this.ranges[x + 1], 2);
          }
        };
        if(this.data != null) {
          for(var i = 0; i < this.data[this.index].THD.length; i++) Opt.file.push([i, this.data[this.index].THD[i]]);
          Opt.axes.y.valueFormatter = (x) => { return this.Round(x * 100.0 / this.data[this.index].THDSamples, 2) + ' %'; };
          Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(x * 100 / this.data[this.index].THDSamples, 0) + '%'; };
        }
        return Object.seal(Opt);

        var In  = (data.THDIN / data.THDSamples) * 100.0;
        var ToShow = [];
        var Analyzer = this.$root.SelectedAnalyzer;

        this.Warning = (In < 100);
        this.THD = (In - 90) * 10;
        this.THDText = oZm.Round2Plain(In) + '% of time in range';


      },
    }
  }
</script>

<style scoped>
  .text-center {
    font-size: 12px;
  }
  .panel-footer .progress {
    height: 12px;
    background-color: #FFFFFF;
    border:1px solid #BBB;
    margin-bottom: 0px;
  }
  .panel-footer .progress >>> div {
    background-color: #337ab7;
  }
</style>