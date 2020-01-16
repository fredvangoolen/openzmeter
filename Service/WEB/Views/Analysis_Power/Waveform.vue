<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ MouseOver ? 'Paused while over graph' : Time }}</div> <div v-if="ShowWarning" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <div class="header-buttons" ref="Toolbar">
        <toolbar-phases storage="DetailsPhaseMulti" :multi="true" :analyzer="analyzer" v-model="Phases"></toolbar-phases>
        <toolbar-options storage="DetailOptions" :options="['Voltage/Current']" v-model="Options"></toolbar-options>
        <!--toolbar-tools variable="To" ref="tools" :options="['EXPORT', 'MAXIMIZE', 'PLAY']" v-on:PlayStatus="$parent.PlayStatusChanged" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="Redraw"></toolbar-tools-->
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="data==null"></spinner>
      <graph ref="graph" :options="GraphOptions" v-on:mouseOver="MouseOver=$event"></graph>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';
  export default {
    props: { analyzer: { type: Object }, data: { type: Object } },
    data() {
      return { Phases: null, Options: {}, ShowWarning: true, MouseOver: false, Time: 'Loading data...' };
    },
    computed: {
      GraphOptions: function() {
        var Opt = {};
        Opt.drawAxesAtZero = true;
        Opt.interactionModel = Graph.FixedRangeInteractionModel(this);
        Opt.plugins = [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ];
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.LegendFormatter_Follow;
        Opt.axes = { y: {}, y2: {}, x: {} };

        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        //, valueRange: Y1Range,
        Opt.axes.y.valueFormatter = (x) => { var Result = oZm.Series['ACTIVE_POWER'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; };
        Opt.axes.y.axisLabelFormatter = (x) => { var Result = oZm.Series['ACTIVE_POWER'].FormatValue(x); return Result.Value; };
        Opt.axes.y2.pixelsPerLabel = 15;
        Opt.axes.y2.axisLabelWidth = 72;
        //Opt.axes.y2.valueRange: VoltageRange,
        Opt.axes.y2.valueFormatter = (x, opts, seriesName) => {
                    return oZm.Round2((seriesName == 'Voltage') ? x : (x / this.CurrentMult)) + ((seriesName == 'Voltage') ? ' V' : ' A');
                  };
        Opt.axes.y2.axisLabelFormatter = (x) => { return '<div style="display: inline-block; text-align: right; width: 36px; color: ' + oZm.Series['VOLTAGE'].Color + ' ">' + oZm.Round2(x) + '</div><div style="display: inline-block; text-align: right; width: 36px; color: ' + oZm.Series['CURRENT'].Color + ' ">' + oZm.Round2(x / this.CurrentMult) + '</div>'; };
        Opt.axes.x.ticker = (min, max) => { return Graph.WaveTimeTicks(max, Analyzer); };
        Opt.axes.x.valueFormatter = (x) => { return oZm.Round2((x * 1000) / Analyzer.SampleRate) + ' <em class="x-small">ms</em>'; };

//        , series: Series, labels: ((this.ShowVoltageCurrent === true) && (PhasesCount === 1)) ? ['Sample', 'Voltage', 'Current', 'Power'] : ['Sample', 'Power'],
//          colors: ((this.ShowVoltageCurrent === true) && (PhasesCount === 1)) ? [oZm.Series['VOLTAGE'].Color, oZm.Series['CURRENT'].Color, oZm.Series['ACTIVE_POWER'].Color] : [oZm.Series['ACTIVE_POWER'].Color],
//          showRangeSelector: false,



        if((this.$root.SelectedAnalyzer === null) || (this.Phases === null) || (this.Data === null)) return;
        var Analyzer = this.$root.SelectedAnalyzer, Series = { 'Voltage': {axis: 'y2'}, 'Current': {axis: 'y2'}, 'Power': { fillGraph: true }};
//        this.ShowWarning = (this.Data.Current[this.Phase] < Analyzer.MinCurrent);
        var ToShow = [];
        var PhasesCount = 0;
        for(var j = 0; j < this.Phases.length; j++) {
          if(this.Phases[j] === true) PhasesCount++;
        }
        for(var i = 0; i < this.data.Voltage_Samples[0].length; i++) {
          var Row = [i];
          var Sum = 0;
          for(var j = 0; j < this.Phases.length; j++) {
            if(this.Phases[j] === true) Sum = Sum + this.Data.Voltage_Samples[j][i] * this.Data.Current_Samples[j][i];
            if((this.ShowVoltageCurrent === true) && (PhasesCount === 1) && (this.Phases[j] === true)) Row = Row.concat([this.Data.Voltage_Samples[j][i], this.Data.Current_Samples[j][i]]);
          }
          ToShow.push(Row.concat([Sum]));
        }
        var Y1Range = oZm.Series['VOLTAGE_SAMPLES'].Range(ToShow, ((this.ShowVoltageCurrent === true) && (PhasesCount === 1)) ? [false, false, true] : [true]);
        if(Y1Range[0] > 0) Y1Range[0] = 0;
        if((this.ShowVoltageCurrent === true) && (PhasesCount === 1)) {
          var VoltageRange = oZm.Series['VOLTAGE_SAMPLES'].Range(ToShow, [true, false, false]);
          var CurrentRange = oZm.Series['VOLTAGE_SAMPLES'].Range(ToShow, [false, true, false]);
          var MaxVoltage = Math.max(Math.abs(VoltageRange[0]), Math.abs(VoltageRange[1]));
          var MaxCurrent = Math.max(Math.abs(CurrentRange[0]), Math.abs(CurrentRange[1]));
          var MaxY1      = Math.max(Math.abs(Y1Range[0]), Math.abs(Y1Range[1]));
          Y1Range      = [-MaxY1, MaxY1];
          VoltageRange = [-MaxVoltage, MaxVoltage];
          this.CurrentMult = MaxVoltage / MaxCurrent;
          for(var i = 0; i < ToShow.length; i++) ToShow[i][2] = ToShow[i][2] * this.CurrentMult;
        }
//        var Options = {
//          },
//          file: ToShow
//        };
//        this.$refs.graph.SetOptions({ showRangeSelector: true});
//        this.$refs.graph.SetOptions(Options);

        return Opt;
      }
    },
    methods: {
      Redraw: function() {
        this.$nextTick(function() {
          this.$refs.graph.Redraw('100%', '300');
        });
      },
//      SetPhases: function(phases) {
//        this.$refs.phases.SetPhases(phases);
//        this.Phases = phases;
//        var PhasesCount = 0;
//        for(var j = 0; j < this.Phases.length; j++) {
//          if(this.Phases[j] === true) PhasesCount++;
//        }
//        this.$refs.options.SetEnable(PhasesCount === 1);
//        this.CalcOptions();
//      },
//      SetOption: function(option, val) {
//        if(option === 'Voltage/Current') this.ShowVoltageCurrent = val;
//        this.CalcOptions();
//      },
//      SetData: function(result) {
//        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
//        this.Data = result;
//        this.CalcOptions();
//        this.ShowSpinner = false;
//      },
//      CalcOptions: function() {
//      },
      ExportClick: function() {
        var ToShow = [];
        for(var i = 0; i < this.Data.Voltage_Samples[0].length; i++) ToShow.push([i, this.Data.Voltage_Samples[this.Phase][i], this.Data.Current_Samples[this.Phase][i], this.Data.Voltage_Samples[this.Phase][i] * this.Data.Current_Samples[this.Phase][i]]);
        this.$refs.tools.SaveCVS('PowerWaveform.csv', [['Time', 'VOLTAGE', 'CURRENT', 'INSTANTANEOUS POWER']].concat(ToShow));
      },
      MaximizeClick: function() {
        this.$refs.tools.Maximize('Waveform', this.$refs.graph.Graph, this.$refs.Toolbar);
      },
      WindowChange: function() {},
      WindowRestore: function() {
        this.$refs.graph.SetOptions( {dateWindow: [0, this.Data.length]} );
      }
    },
    created: function() {
//      this.CurrentMult = 1.0;
//      this.Data = null;
//      this.Phases = null;
//      this.ShowVoltageCurrent = false;
    }
  }
</script>

<style scoped>
  .header-text {
    font-size: 14px;
  }
  .title-warning {
    line-height: 20px;
    font-size: 14px;
  }
  .title-warning img {
    margin: -4px 4px 0px 0px;
  }
</style>