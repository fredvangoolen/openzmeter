<template>
  <div class="panel" :class="PanelClass">
    <div class="panel-heading">
      <div class="huge header-text">{{ PanelTitle }}</div>
      <div class="header-buttons">
        <div style="margin-top: -3px">
          <div style="display: inline-block; font-size: 24px; padding-right: 4px; vertical-align: top"><span v-html="LastValue.Value"></span> <span class="huge-small-unit">{{ LastValue.Units }}</span>
            <div style="font-size: 14px; line-height: 14px" v-if="data!=null">From {{ analyzer.FormatAggregation(data.Aggregation_Source) }} aggregation {{ paused ? '(Paused)' : '' }}</div>
          </div>
          <div v-if="LastValue.Phases.length > 1" class="phases">
            <div v-for="(val, idx) in LastValue.Phases" :key="idx" v-html="val"></div>
          </div>
       </div>
      </div>
    </div>
    <div class="panel-body" @mouseenter="$emit('mouseOver', +1)" @mouseleave="$emit('mouseOver', -1)">
      <spinner v-if="data==null"></spinner>
      <graph ref="Chart" :options="GraphOptions" :height="150" :paused="paused"></graph>
    </div>
    <div class="panel-footer clearfix">
      <router-link :to="SerieProps.Link" style="line-height: 22px"><span class="pull-left">View Details</span></router-link>
      <div class="pull-right" ref="Toolbar">
        <toolbar-phases :analyzer="analyzer" v-model="Phases" v-if="serie != 'FREQUENCY'" :storage="'Dashboard_' + serie" :current="serie=='CURRENT'" :multi="true"></toolbar-phases>
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
    props: { serie: String, analyzer: Object, data: Object, paused: Boolean },
    data() {
      return { ShowSpinner: true, Phases: null };
    },
    computed: {
      PanelClass: function() {
        if(this.serie == 'VOLTAGE') return 'panel-voltage';
        if(this.serie == 'CURRENT') return 'panel-current';
        if(this.serie == 'ACTIVE_POWER') return 'panel-active-power';
        if(this.serie == 'FREQUENCY') return 'panel-frequency';
      },
      PanelTitle: function() {
        if(this.serie == 'VOLTAGE') return 'Voltage';
        if(this.serie == 'CURRENT') return 'Current';
        if(this.serie == 'ACTIVE_POWER') return 'Active Power';
        if(this.serie == 'FREQUENCY') return 'Frequency';
      },
      LastValue: function() {
        var Val = { Value: '-', Units: '', Phases: [] };
        if(this.GraphOptions.file.length == 0) return Val;
        var LastSample = this.GraphOptions.file.slice(-1)[0];
        if(this.serie == 'FREQUENCY') {
          Val.Value = this.Round(LastSample[1], 3, true);
          Val.Units = 'Hz';
        } else if(this.serie == 'ACTIVE_POWER') {
          Val.Value = 0.0;
          for(var n = 1; n < LastSample.length; n++) Val.Value += LastSample[n];
          Val.Units = this.BestUnits(Val.Value);
          Val.Value = this.Round(this.ScaleUnits(Val.Value, Val.Units), 2, true);
          for(var n = 1; n < LastSample.length; n++) Val.Phases.push(this.Round(this.ScaleUnits(LastSample[n], Val.Units), 2, true) + ' ' + Val.Units + 'W');
          Val.Units += 'W';
        } else if((this.serie == 'VOLTAGE') || (this.serie == 'CURRENT')) {
          Val.Value = 0.0;
          var PhasesCount = oZm.Series['VOLTAGE'].Names(this.analyzer).length;
          for(var n = 0; n < PhasesCount; n++) Val.Value += Math.pow(LastSample[n + 1], 2.0);
          Val.Value = Math.sqrt(Val.Value / PhasesCount);
          Val.Units = this.BestUnits(Val.Value);
          Val.Value = this.Round(this.ScaleUnits(Val.Value, Val.Units), 2, true);
          for(var n = 1; n < LastSample.length; n++) Val.Phases.push(this.Round(this.ScaleUnits(LastSample[n], Val.Units), 2, true) + ' ' + Val.Units + this.SerieProps.Units);
          Val.Units += this.SerieProps.Units;
        }
        return Val;
      },
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
        Opt.axes.x.valueFormatter = (x) => { return this.analyzer.FormatTime(x); };
        Opt.axes.x.axisLabelFormatter = (x) => { return this.analyzer.FormatTime(x); };
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 30;
        if(this.data != null) {
          Opt.visibility =  this.Phases;
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
          if((this.serie == 'VOLTAGE') || (this.serie == 'CURRENT')) {
            Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.Phases), { defMin: 0, defMax: 1, percent: 0.05, minWidth: 10 });
          } else if(this.serie == 'ACTIVE_POWER') {
            Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false].concat(this.Phases), { defMin: 0, defMax: 1, percent: 0.05, minWidth: 10 });
          } else if(this.serie == 'FREQUENCY') {
            Opt.axes.y.valueRange = this.MatrixColsRange(Opt.file, [false, true], { defMin: 0, defMax: 1, percent: 0.005, minWidth: 0.5 });
          }
          var Units = this.BestUnits(Math.max(Math.abs(Opt.axes.y.valueRange[0]), Math.abs(Opt.axes.y.valueRange[1])));
          Opt.axes.y.valueFormatter = (x) => {
            var Result = oZm.FormatSerieValue(x, this.SerieProps);
            return Result.ValuePlain + ' ' + Result.Units;
          };
          Opt.axes.y.axisLabelFormatter = (x) => {
            return oZm.FormatSerieValue(x, this.SerieProps, Units).ValuePlain;
          };
        }
        return Opt;
      }
    },
    methods: {
      bliu: function(val) {
        console.log(val);
      },
      ExportClick() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS(this.SerieProps.Title + '.csv', [['Time'].concat(this.SerieProps.Names(Analyzer))].concat(this.Data));
      },
    },
    created: function() {
      this.SerieProps = oZm.Series[this.serie];
    },
  }
</script>

<style scoped>
  .phases {
    display: inline-block;
    border-left: 1px solid #FFF;
    padding-left: 2px;
    line-height: 11px;
    font-size: 12px;
    vertical-align: top;
  }
</style>