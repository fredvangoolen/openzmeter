<template>
  <div class="panel panel-frequency">
    <div class="panel-heading">
      <div class="float-left">Grid frequency</div>
      <div class="float-right" v-if="(((data.FrequencyIN / data.FrequencySamples) * 100.0) < 99.5)">
        <img :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Limit exceded">
      </div>
      <div class="clearfix"></div>
    </div>
    <div class="panel-body">
      <graph :options="GraphOptions" :height="200"></graph>
    </div>
    <div class="panel-footer">
      <div class="text-center">99.5% of the year between {{ Round(analyzer.NominalFrequency, 2) }}Hz±1% with no events allowed outside {{ Round(analyzer.NominalFrequency * 0.94, 2) }} to {{ Round(analyzer.NominalFrequency * 1.04, 2) }}Hz</div>
        <progress-bar :value="(((data.FrequencyIN / data.FrequencySamples) * 100.0) - 95) * 20" :text="Round(((data.FrequencyIN / data.FrequencySamples) * 100.0), 2) + '% of time in range'"></progress-bar>
        <div style="float: left; font-size: 10px">95%</div>
        <div style="float:right; font-size: 10px">100%</div>
        <div class="clearfix text-center">{{ Round(((data.FrequencyIN / data.FrequencySamples) * 100.0), 2) + '% of time in range' }}</div>
    </div>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  export default {
    props: { data: Object, analyzer: Object, ranges: Array },
    data: function() {
      return { };
    },
    computed: {
      GraphOptions: function() {
        var Opt = { file: [], axes: { x: {}, y: {} } };
        Opt.strokeWidth = 1;
        Opt.labels = ["Variation", "Time"];
        Opt.stepPlot = true;
        Opt.plotter = Graph.Plotters.MultiBar;
        Opt.colors = [this.analyzer.Color.Frequency];
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
        Opt.axes.y.axisLabelWidth = 26;
        Opt.axes.x.pixelsPerLabel = 30;
        Opt.axes.x.axisLabelWidth = 30;
        Opt.axes.x.valueFormatter = (x) => {
          if((x < 0) || (x >= this.ranges.length - 1)) return "";
          if(x == 0) {
            return "Less than " + this.Round(this.analyzer.NominalFrequency * (1 + this.ranges[x + 1] / 100.0)) + ' Hz';
          } else if(x == (this.ranges.length - 2)) {
            return "More than " + this.Round(this.analyzer.NominalFrequency * (1 + this.ranges[x] / 100.0)) + ' Hz';
          } else {
            return "From " + this.Round(this.analyzer.NominalFrequency * (1 + this.ranges[x] / 100.0), 2) + " to " + this.Round(this.analyzer.NominalFrequency * (1 + this.ranges[x + 1] / 100.0), 2) + ' Hz';
          }
        };
        Opt.axes.x.axisLabelFormatter = (x) => {
          if((x < 0) || (x >= this.ranges.length - 1)) return "";
          if(x == 0) {
            return '≤' + this.Round(this.analyzer.NominalFrequency * (1 + this.ranges[x + 1] / 100.0), 2);
          } else if(x == (this.ranges.length - 2)) {
            return '≥' + this.Round(this.analyzer.NominalFrequency * (1 + this.ranges[x] / 100.0));
          } else {
            return this.Round(this.analyzer.NominalFrequency * (1 + this.ranges[x] / 100.0), 2) + "<br/>" + this.Round(this.analyzer.NominalFrequency * (1 + this.ranges[x + 1] / 100.0), 2);
          }
        };
        if(this.data != null) {
          for(var i = 0; i < this.data.Frequency.length; i++) Opt.file.push([i, this.data.Frequency[i]]);
          Opt.axes.y.valueFormatter = (x) => { return this.Round(x * 100.0 / this.data.FrequencySamples, 2) + ' %'; };
          Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(x * 100 / this.data.FrequencySamples, 0) + '%'; };
        }
        return Object.seal(Opt);
      }
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