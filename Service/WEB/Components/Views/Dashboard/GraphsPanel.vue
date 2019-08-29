<template>
  <div ref="panel" class="panel" :class="SerieProps.CSS_Class">
    <div class="panel-heading">
      <div class="huge header-text">{{ SerieProps.Title }}</div>
      <div class="header-buttons">
        <div style="margin-top: -3px; height: 33px">
          <div style="display: inline-block; font-size: 24px; padding-right: 4px; vertical-align: top"><span v-html="LastValue"></span> <em class="huge-small-unit">{{ UnitsScale }}{{ SerieProps.Units }}</em>
            <div style="font-size: 14px; line-height: 14px">{{ Aggregation }}</div>
          </div>
          <div v-if="PhasesValue.length > 1" style="display: inline-block; border-left: 1px solid #FFF; padding-left: 2px; line-height: 12px; font-size: 12px; vertical-align: top"><div v-for="val in PhasesValue">{{ val }}</div></div>
       </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <graph ref="Chart" :options="GraphOptions"></graph>
    </div>
    <div class="panel-footer clearfix">
      <router-link :to="SerieProps.Link" style="line-height: 22px"><span class="pull-left">View Details</span></router-link>
      <div class="pull-right" ref="Toolbar">
        <toolbar-phases v-on:PhasesChanged="SetPhases" v-if="type != 'FREQUENCY'" :variable="'Dashboard' + type" :type="(type == 'CURRENT') ? 'CURRENT' : 'VOLTAGE'" :multi="true"></toolbar-phases>
        <toolbar-tools :options="['EXPORT', 'MAXIMIZE']" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="MinimizeClick" v-on:ExportClick="ExportClick" ref="tools"></toolbar-tools>
      </div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import Dygraph from 'JS/dygraph.js';
  import moment from 'JS/moment.js';
  import 'JS/dygraph-crosshair.js';
  import Spinner from 'Components/Spinner.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import Graph from 'Components/Graph.vue';
  export default {
    props: ['type'],
    data() {
      return { ShowSpinner: true, PhasesValue:[], LastValue: null, UnitsScale: '', SerieProps: {}, Aggregation: 'Loading data...'        
      };
    },
    methods: {
      CalcOptions() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Range = this.SerieProps.Range(this.Data, this.Phases);
        var Options = {
          visibility: this.Phases, dateWindow: this.DateWindow, file: this.Data, valueRange: Range, selectMode: 'euclidian',
          labels: ['Time'].concat(this.SerieProps.Names(Analyzer)), colors: oZm.ColorArray(this.SerieProps.Color, this.SerieProps.Names(Analyzer).length),
          axes: {
            y: { pixelsPerLabel: 15, axisLabelWidth: 30,
                 valueFormatter: function (x) { var Result = this.SerieProps.FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; }.bind(this),
                 axisLabelFormatter: function (x) { var Units = oZm.GetBestUnits(Math.max(Math.abs(Range[0]), Math.abs(Range[1]))); return oZm.Round2(oZm.ScaleToUnits(x, Units)); }
               },
            x: { valueFormatter: function (x) { return moment(x).format('hh:mm:ss'); }, axisLabelFormatter: function (x) { return moment(x).format('hh:mm'); } }
          }
        };
        this.$refs.Chart.SetOptions(Options);
      },
      UpdateGraph() {
        if(this.$root.SelectedAnalyzer === null) return;
        
          $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Aggreg: '3S', Series: [this.type], To: 0, LastSample: this.LastSample, Analyzer: this.$root.SelectedAnalyzer.Analyzer, Period: 7200000 }), 
            success: (result) => {
              if((result['Sampletime'] === null) || (result['Sampletime'].length === 0)) return;
              this.Data = oZm.GraphDataAppend(result, this.SerieProps.Serie, this.Data);
              var LastSample = result[this.SerieProps.Serie].slice(-1)[0];
              if(this.type == 'FREQUENCY') {
                this.LastValue = oZm.Round3(LastSample);
                this.PhasesValue = [];
              } else if(this.type == 'ACTIVE_POWER') {
                this.LastValue = 0;
                this.PhasesValue = [];
                this.UnitsScale = oZm.GetBestUnits(oZm.ArrayAbsMax(LastSample));
                for(var n = 0; n < LastSample.length; n++) this.LastValue = this.LastValue + LastSample[n];  
                this.LastValue = oZm.Round2(oZm.ScaleToUnits(this.LastValue, this.UnitsScale));
                this.PhasesValue = LastSample.map((n) => {
                  return oZm.Round2Plain(oZm.ScaleToUnits(n, this.UnitsScale));
                });
              } else if(this.type == 'VOLTAGE') {
                this.LastValue = 0;
                this.PhasesValue = [];
                this.UnitsScale = oZm.GetBestUnits(oZm.ArrayAbsMax(LastSample));
                for(var n = 0; n < LastSample.length; n++) this.LastValue = this.LastValue + Math.pow(LastSample[n], 2.0);  
                this.LastValue = oZm.Round2(oZm.ScaleToUnits(Math.sqrt(this.LastValue / LastSample.length), this.UnitsScale));
                this.PhasesValue = LastSample.map((n) => {
                  return oZm.Round2Plain(oZm.ScaleToUnits(n, this.UnitsScale));
                });               
              } else if(this.type == 'CURRENT') {
                this.LastValue = 0;
                this.PhasesValue = [];
                this.UnitsScale = oZm.GetBestUnits(oZm.ArrayAbsMax(LastSample));
                var PhasesCount = oZm.Series['VOLTAGE'].Names(this.$root.SelectedAnalyzer).length;
                for(var n = 0; n < PhasesCount; n++) this.LastValue = this.LastValue + Math.pow(LastSample[n], 2.0);  
                this.LastValue = oZm.Round2(oZm.ScaleToUnits(Math.sqrt(this.LastValue / PhasesCount), this.UnitsScale));
                this.PhasesValue = LastSample.map((n) => {
                  return oZm.Round2Plain(oZm.ScaleToUnits(n, this.UnitsScale));
                });    
              }
              this.Aggregation = 'From ' + oZm.GetAggregLongName(result['Aggregation_Source']) + ' aggregation';
              this.DateWindow = [result['From'], result['To']];
              this.LastSample = oZm.LastSampletime(result, this.LastSample);
              if(this.Data.length > 0) this.CalcOptions();
              this.ShowSpinner = false;
            },
            complete: () => {
              this.UpdateGraphTimer = setTimeout(this.UpdateGraph, 5000);
            }, 
            error: () => {
              this.ShowSpinner = true;
            }
          });

      },
      SetPhases(phases) {
        this.Phases = phases;
        if(this.Data.length > 0) this.CalcOptions();
      },
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
      SetAnalyzer() {
        this.LastSample = 0;
        this.Data = [];
        this.ShowSpinner = true;
        this.UpdateGraph();
      }
    },
    created: function() {
      this.Data = [];
      this.DateWindow = [0, 0];
      this.LastSample = 0;
      this.UpdateGraphTimer = null;
      this.Phases = null;
      this.SerieProps = oZm.Series[this.type];
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
      this.GraphOptions = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, legend: 'follow', 
        plugins: [ new Dygraph.Plugins.Crosshair({ direction: "vertical" }) ], xAxisHeight: 12, legendFormatter: Graph.LegendFormatter_Follow
      };
    },
    beforeDestroy: function() {
      if(this.UpdateGraphTimer !== null) clearTimeout(this.UpdateGraphTimer);
    },
    mounted: function() {
      this.SetAnalyzer();
    },
    components: { ToolbarPhases, ToolbarTools, Spinner, Graph }
  }
</script>
