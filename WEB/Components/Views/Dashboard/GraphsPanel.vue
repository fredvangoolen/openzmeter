<template>
  <div ref="panel" class="panel" :class="SerieProps.CSS_Class">
    <div class="panel-heading">
      <div class="huge header-text">{{ SerieProps.Title }}</div>
      <div class="header-buttons">
        <div><span class="huge-small" v-html="LastValue"></span><em class="huge-small-unit">{{ SerieProps.Units }}</em></div>
        <div>{{ Aggregation }}</div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div ref="Chart" class="chart"></div>
    </div>
    <div class="panel-footer clearfix">
      <router-link :to="SerieProps.Link" style="line-height: 22px"><span class="pull-left">View Details</span></router-link>
      <toolbar-phases v-on:PhasesChanged="SetPhases" class="pull-right" v-if="type != 'FREQUENCY'" :variable="'Dashboard' + type" :mode="(type == 'CURRENT') ? 'CURRENT' : 'VOLTAGE'" :multi="true"></toolbar-phases>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Spinner from 'Components/Spinner.vue';
  import Dygraph from 'dygraphs';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  export default {
    props: ['type'],
    data() {
      return {
        ShowSpinner: true,
        SerieProps: {},
        Aggregation: 'Loading data...',
        LastValue: [],
        Graph: null,
        Data: [],
        DateWindow: [0, 0],
        LastSample: 0,
        UpdateGraphTimer: null,
        Phases: [true]
      };
    },
    methods: {
      CalcOptions() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Options = {
          strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', interactionModel: {},
          plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], visibility: this.Phases, dateWindow: this.DateWindow, file: this.Data,
          legendFormatter: oZm.LegendFormatter_Follow, labels: ['Time'].concat(this.SerieProps.Names(Analyzer)), colors: oZm.ColorArray(this.SerieProps.Color, this.SerieProps.Names(Analyzer).length),
          underlayCallback: function(canvas, area, g) {
            if(this.type === 'VOLTAGE') {
              var NV = Analyzer.NominalVoltage, Yellow = "rgba(255, 255, 0, 0.05)", Red = "rgba(255, 0, 0, 0.1)"; 
//              oZm.GraphHBackgroundRanges([[NV * 1.1, NV * 1.07, Yellow], [NV * 0.93, NV * 0.9, Yellow], [Infinity, NV * 1.1, Red], [NV * 0.9, -Infinity, Red]], g, canvas);
            }
          },
          axes: {
            y: { pixelsPerLabel: 15, axisLabelWidth: 34,
                 valueFormatter: function (x) { return oZm.Round2(x) + ' <em class="x-small">' + this.SerieProps.Units + '</em>'; }.bind(this),
                 axisLabelFormatter: function (x) { return oZm.Round2(x); }
               },
            x: { valueFormatter: function (x) { return new Date(x).toLocaleTimeString(); },
                 axisLabelFormatter: function (x) { return x.toLocaleTimeString(); }
               }
          }
        };
        this.Graph.updateOptions(Options);
          //GraphPhaseButtons(VoltageChart, $('#groupVoltagePhases'), 'DashboardVoltageVisiblePhases', false, function(series) { return CalcRanges(VoltageData, series, false, true); }, (event !== undefined) ? event.target : undefined); 
      },
      UpdateGraph() {
        if(this.$root.SelectedAnalyzer === null) return;
        $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '3S', Series: [this.type], From: 0, LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.ID, Period: 7200000 }), 
          success: function(result) {
            this.ShowSpinner = false;
            if(result['Sampletime'] === null) return;
            if(result['Sampletime'].length === 0) return;
            this.Data = oZm.GraphDataAppend(result, this.SerieProps.Serie, this.Data);
            this.LastValue = oZm.Array_Round2(result[this.SerieProps.Serie].slice(-1)[0], oZm.ColorArray($(this.$refs.panel).css('border-color'), 4));
            this.Aggregation = 'From ' + oZm.GetAggregLongName(result['Aggregation_Source']) + ' aggregation';
            this.DateWindow = [result['From'], result['To']];
            this.LastSample = oZm.LastSampletime(result, this.LastSample);
            this.CalcOptions();
//    new GraphTools('Voltage', VoltageChart, '#toolbarVoltage', undefined, function() { exportToCsv('Voltage.csv', [['Time'].concat(Analyzer.GetSeriesNames('Voltage'))].concat(VoltageData)); });
//  for(var n = 0; n < Analyzer.GetSeriesNames('Voltage').length; n++) AddEvents(result, Analyzer.GetSeriesNames('Voltage')[n], VoltageChart);
          }.bind(this),
          complete: function() {
            if(this.UpdateGraphTimer !== null) clearTimeout(this.UpdateGraphTimer);
            this.UpdateGraphTimer = setTimeout(this.UpdateGraph, 5000);
          }.bind(this), 
          error: function() {
            this.ShowSpinner = true;
          }.bind(this)
        });
      },
      SetPhases(phases) {
        this.Phases = phases;
        this.CalcOptions();
      },
      SetAnalyzer() {
        this.LastSample = 0;
        this.Data = [];
        this.UpdateGraph();
      }
    },
    created: function() {
      this.SerieProps = oZm.Series[this.type];
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    beforeDestroy: function() {
      if(this.UpdateGraphTimer !== null) clearTimeout(this.UpdateGraphTimer);
    },
    mounted: function() {
      this.Graph = new Dygraph(this.$refs.Chart, [[0, 0]], {});
      this.UpdateGraph();
    },
    components: { ToolbarPhases, Spinner }
  }
</script>