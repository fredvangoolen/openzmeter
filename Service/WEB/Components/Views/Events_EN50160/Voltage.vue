<template>
  <div class="panel panel-voltage">
    <div class="panel-heading">
      <div class="float-left">Grid Voltage</div>
      <div class="float-right" v-if="Warning"><img :src="require('Imgs/exclamation.png')" data-toggle="tooltip" title="Limit exceded"></div>
      <div class="clearfix"></div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <graph :options="GraphOptions" ref="graph"></graph>
    </div>
    <div class="panel-footer">
      <div class="text-center">95% of the year between {{ Nominal }}V±10%</div>
        <progress-bar :value="Voltage" :text="VoltageText"></progress-bar>
        <div style="float: left; font-size: 10px">90%</div>
        <div style="float:right; font-size: 10px">100%</div>
        <div class="clearfix text-center">{{ VoltageText }}</div>
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
      return { Nominal: 50, Voltage: 0, VoltageText: '', ShowSpinner: true, Warning: true };
    },
    created: function() {
      this.GraphOptions = { strokeWidth: 1, labels: ["Variation", "Time"], stepPlot: true, plotter: Graph.multiColumnBarPlotter, colors: [oZm.Series['VOLTAGE'].Color], interactionModel: {}, legend: 'follow', drawHighlightPointCallback: Graph.DrawHighlightBarChartCallback };
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);  
    },
    mounted: function() {
      this.SetAnalyzer();
    },
    methods: {
      SetData(data, voltages) {
        var In  = (data.VoltageIN / data.VoltageSamples) * 100.0;
        var ToShow = [];
        var Analyzer = this.$root.SelectedAnalyzer;  
        for(var i = 0; i < data.Voltage.length; i++) ToShow.push([i, data.Voltage[i]]);
        this.Warning = (In < 95);
        this.Voltage = (In - 90) * 10;
        this.VoltageText = oZm.Round2Plain(In) + '% of time in range';
        var Options = {
          dateWindow: [-0.5, voltages.length - 1.5],
          axes: {
            y: { pixelsPerLabel: 24, axisLabelWidth: 24,
                 valueFormatter: function (x) { return oZm.Round2(x * 100.0 / data.VoltageSamples) + ' %'; },
                 axisLabelFormatter: function (x) { return Math.round(x * 100 / data.VoltageSamples, 0) + '%'; }
               },
            x: { pixelsPerLabel: 30, axisLabelWidth: 30,
                 valueFormatter: function (x) {
                   if((x < 0) || (x >= voltages.length - 1)) return "";
                   if(x === 0) {
                     return "Less than " + oZm.Round2(Analyzer.NominalVoltage * (1 + voltages[x + 1] / 100.0)) + ' V';
                  } else if(x === (voltages.length - 2)) {
                    return "More than " + oZm.Round2(Analyzer.NominalVoltage * (1 + voltages[x] / 100.0)) + ' V';
                  } else {
                    return "From " + oZm.Round2(Analyzer.NominalVoltage * (1 + voltages[x] / 100.0)) + " to " + oZm.Round2(Analyzer.NominalVoltage * (1 + voltages[x + 1] / 100.0)) + ' V';
                  }
                },
                axisLabelFormatter: function (x) {
                  if((x < 0) || (x >= voltages.length - 1)) return "";
                  if(x === 0) {
                    return '≤' + oZm.Round2(Analyzer.NominalVoltage * (1 + voltages[x + 1] / 100.0));
                  } else if(x === (voltages.length - 2)) {
                    return '≥' + oZm.Round2(Analyzer.NominalVoltage * (1 + voltages[x] / 100.0));
                  } else {
                    return oZm.Round2(Analyzer.NominalVoltage * (1 + voltages[x] / 100.0)) + "<br/>" + oZm.Round2(Analyzer.NominalVoltage * (1 + voltages[x + 1] / 100.0));
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
        this.Nominal = this.$root.SelectedAnalyzer.NominalVoltage;
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