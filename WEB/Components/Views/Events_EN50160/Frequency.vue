<template>
  <div class="panel panel-frequency">
    <div class="panel-heading">
      <div class="float-left">Grid frequency</div>
      <div class="float-right" v-if="Warning"><img :src="require('Imgs/warning16.png')" data-toggle="tooltip" title="Limit exceded"></div>
      <div class="clearfix"></div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <graph :options="GraphOptions" ref="graph"></graph>
    </div>
    <div class="panel-footer">
      <div class="text-center">99.5% of the year between {{ Nominal }}±1%Hz with no events allowed outside {{ Nominal * 0.94 }} to {{ Nominal * 1.04 }}Hz</div>
        <progress-bar :value="Frequency" :text="FrequencyText"></progress-bar>
        <div style="float: left; font-size: 10px">90%</div>
        <div style="float:right; font-size: 10px">100%</div>
        <div class="clearfix"></div>
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
      return { Nominal: 50, Frequency: 0, FrequencyText: '', ShowSpinner: true, Warning: true };
    },
    created: function() {
      this.GraphOptions = { strokeWidth: 1, labels: ["Variation", "Time"], stepPlot: true, plotter: Graph.multiColumnBarPlotter, colors: [oZm.Series['FREQUENCY'].Color], interactionModel: {}, legend: 'follow' };
    },
    mounted: function() {
      this.SetAnalyzer();
    },
    methods: {
      SetData(data, frequencies) {
        var In  = (data.FrequencyIN / data.FrequencySamples) * 100.0;
        var ToShow = [];
        var Analyzer = this.$root.SelectedAnalyzer;  
        for(var i = 0; i < data.Frequency.length; i++) ToShow.push([i, data.Frequency[i]]);
        this.Warning = (In >= 99.5);
        this.Frequency = (In - 90) * 10;
        this.FrequencyText = oZm.Round2Plain(In) + '% of time in range';
        var Options = {
          dateWindow: [-0.5, data.Frequency.length - 1.5],
          axes: {
            y: { pixelsPerLabel: 24, axisLabelWidth: 24,
                 valueFormatter: function (x) { return oZm.Round2(x * 100.0 / data.FrequencySamples) + ' %'; },
                 axisLabelFormatter: function (x) { return Math.round(x * 100 / data.FrequencySamples, 0) + '%'; }
               },
            x: { pixelsPerLabel: 30, axisLabelWidth: 30,
                 valueFormatter: function (x) {
                   if((x < 0) || (x >= frequencies.length - 1)) return "";
                   if(x === 0) {
                     return "Less than " + oZm.Round2(Analyzer.NominalFreq * (1 + frequencies[x + 1] / 100.0)) + ' Hz';
                  } else if(x === (frequencies.length - 2)) {
                    return "More than " + oZm.Round2(Analyzer.NominalFreq * (1 + frequencies[x] / 100.0)) + ' Hz';
                  } else {
                    return "From " + oZm.Round2(Analyzer.NominalFreq * (1 + frequencies[x] / 100.0)) + " to " + oZm.Round2(Analyzer.NominalFreq * (1 + frequencies[x + 1] / 100.0)) + ' Hz';
                  }
                },
                axisLabelFormatter: function (x) {
                  if((x < 0) || (x >= frequencies.length - 1)) return "";
                  if(x === 0) {
                    return '≤' + oZm.Round2(Analyzer.NominalFreq * (1 + frequencies[x + 1] / 100.0));
                  } else if(x === (frequencies.length - 2)) {
                    return '≥' + oZm.Round2(Analyzer.NominalFreq * (1 + frequencies[x] / 100.0));
                  } else {
                    return oZm.Round2(Analyzer.NominalFreq * (1 + frequencies[x] / 100.0)) + "<br/>" + oZm.Round2(Analyzer.NominalFreq * (1 + frequencies[x + 1] / 100.0));
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
        this.Nominal = '';
        if(this.$root.SelectedAnalyzer === null) return;
        this.Nominal = this.$root.SelectedAnalyzer.NominalFreq;
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
    background-color: #616161;
  }
</style>