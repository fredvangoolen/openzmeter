<template>
  <div class="panel panel-reactive-power">
    <div class="panel-heading">
      <div class="float-left">Harmonic distorsion</div>
      <div class="float-right" v-if="Warning"><img :src="require('Imgs/warning16.png')" data-toggle="tooltip" title="Limit exceded"></div>
      <div class="clearfix"></div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <graph :options="GraphOptions" ref="graph"></graph>
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
  import * as oZm from 'JS/openZmeter.js';
  import Graph from 'Components/Graph.vue';
  import Spinner from 'Components/Spinner.vue';
  import ProgressBar from 'Components/ProgressBar.vue';
  export default {
    data: function() {
      return {THD: 0, THDText: '', ShowSpinner: true, Warning: true };
    },
    created: function() {
      this.GraphOptions = { strokeWidth: 1, labels: ["Distorsion", "Time"], stepPlot: true, plotter: Graph.multiColumnBarPlotter, colors: [oZm.Series['REACTIVE_POWER'].Color], interactionModel: {}, legend: 'follow' };
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);  
    },
    mounted: function() {
      this.SetAnalyzer();
    },
    methods: {
      SetData(data, thds) {
        var In  = (data.THDIN / data.THDSamples) * 100.0;
        var ToShow = [];
        var Analyzer = this.$root.SelectedAnalyzer;  
        for(var i = 0; i < data.THD.length; i++) ToShow.push([i, data.THD[i]]);
        this.Warning = (In < 100);
        this.THD = (In - 90) * 10;
        this.THDText = oZm.Round2Plain(In) + '% of time in range';
        var Options = {
          dateWindow: [-0.5, thds.length - 1.5],
          axes: {
            y: { pixelsPerLabel: 24, axisLabelWidth: 24,
                 valueFormatter: function (x) { return oZm.Round2(x * 100.0 / data.THDSamples) + ' %'; },
                 axisLabelFormatter: function (x) { return Math.round(x * 100 / data.THDSamples, 0) + '%'; }
               },
            x: { pixelsPerLabel: 30, axisLabelWidth: 30,
                 valueFormatter: function (x) {
                   if((x < 0) || (x >= thds.length - 1)) return "";
                   if(x === (thds.length - 2)) {
                    return "More than " + oZm.Round2(thds[x]) + ' %';
                  } else {
                    return "From " + oZm.Round2(thds[x]) + " to " + oZm.Round2(thds[x + 1]) + ' %';
                  }
                },
                axisLabelFormatter: function (x) {
                  if((x < 0) || (x >= thds.length - 1)) return "";
                  if(x === (thds.length - 2)) {
                    return '≥' + oZm.Round2(thds[x]);
                  } else {
                    return oZm.Round2(thds[x]) + "<br/>" + oZm.Round2(thds[x + 1]);
                  }
                }
            }
          }, 
          file: ToShow
        };      
        this.$refs.graph.SetOptions(Options);
        this.ShowSpinner = false;
      },
      SetAnalyzer() {
        this.ShowSpinner = true;
      }
    },
    components: { Graph, Spinner, ProgressBar }
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