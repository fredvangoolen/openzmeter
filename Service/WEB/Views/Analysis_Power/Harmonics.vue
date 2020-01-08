<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ MouseOver ? 'Paused while over graph' : Time }}</div> <div v-if="ShowWarning" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
        <div class="header-buttons" ref="Toolbar">
          <toolbar-phases variable="DetailsPhaseMulti" ref="phases" :multi="true" v-on:PhasesChanged="$parent.PhasesChangedMulti" type="VOLTAGE"></toolbar-phases>
          <toolbar-options variable="DetailOptions" :options="['1st Harmonic']" v-on:OptionChanged="SetOption"></toolbar-options>
          <toolbar-tools variable="To" ref="tools" :options="['EXPORT', 'MAXIMIZE', 'PLAY']" v-on:PlayStatus="$parent.PlayStatusChanged" v-on:ExportClick="ExportClick" v-on:MaximizeClick="MaximizeClick" v-on:MinimizeClick="Redraw"></toolbar-tools>
        </div>
      </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <graph ref="graph" :options="{}" v-on:mouseOver="MouseOver=$event"></graph>
    </div>
  </div>
</template>
<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import Dygraph from 'JS/dygraph.js';
  import Graph from 'Components/Graph.vue';  
  export default {
    data() {
      return { ShowWarning: true, ShowSpinner: true, MouseOver: false, Time: 'Loading data...' };
    },
    methods: {
      Redraw: function() {
        this.$nextTick(function() {
          this.$refs.graph.Redraw('100%', '300');
        });
      },
      SetPhases: function(phases) {
        this.$refs.phases.SetPhases(phases);
        this.Phases = phases;
        this.CalcOptions();
      },
      SetOption: function(option, val) {
        if(option === '1st Harmonic') this.First = val;
        this.CalcOptions();
      },
      SetData(result) {
        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
        this.Data = [];
        for(var i = 0; i < 50; i++) {
          var Row = [];
          for(var n = 0; n < result.Power_Harmonics.length; n++) Row.push(result.Power_Harmonics[n][i]);
          this.Data.push(Row);
        }
        this.MaxCurrent = oZm.ArrayAbsMax(result.Current);
        this.CalcOptions();
        this.ShowSpinner = false;
      },
      CalcOptions() {
        if((this.$root.SelectedAnalyzer === null) || (this.Phases === null) || (this.First === null) || (this.Data === null)) return;
        var Analyzer = this.$root.SelectedAnalyzer, Series = {}, ToShow = [];
        this.ShowWarning = (this.MaxCurrent < Analyzer.MinCurrent);
        for(var i = this.First ? 0 : 1; i < this.Data.length; i++) {
          var Row = [i];
          for(var n = 0; n < this.Data[i].length; n++) Row.push(this.Data[i][n]);
          ToShow.push(Row);
        }
        var Y1Range = oZm.Series['ACTIVE_POWER'].Range(ToShow, this.Phases);
        var Options = {
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], interactionModel: { }, plotter: Graph.multiColumnBarPlotter, visibility: this.Phases,
          dateWindow: [ToShow[0][0] - 0.5, ToShow[ToShow.length - 1][0] + 0.5], legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow, showRangeSelector: false,
          drawHighlightPointCallback: Graph.DrawHighlightBarChartCallback,
          labels: ['Harmonic'].concat(oZm.Series['ACTIVE_POWER'].Names(Analyzer)), series: Series,
          colors: oZm.SerieStyle(oZm.Series['VOLTAGE'], Analyzer, 0).Colors.concat(oZm.SerieStyle(oZm.Series['CURRENT'], Analyzer, 0).Colors), 
          axes: { 
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: [Y1Range[0] + this.VOffset, Y1Range[1] + this.VOffset],
                 valueFormatter: function(x) { return oZm.Round2(x) + '<em class="x-small">W</em>'; }.bind(this),
                 axisLabelFormatter: function (x) { return oZm.Round2(x); }.bind(this)
               },
            x: { drawGrid: false, ticker: Graph.OddHarmonicsTicks, axisLabelFormatter: function (x) { return oZm.Series['HARMONICS'].Names[x]; },
                 valueFormatter: function(y) { return oZm.Series['HARMONICS'].Names[y] + ' Harmonic (' + (Analyzer.NominalFreq * (y + 1)) + '<em class="small"> Hz</em>)'; }           
               }
          },
          file: ToShow
        };
        this.$refs.graph.SetOptions({ showRangeSelector: true}); 
        this.$refs.graph.SetOptions(Options);
      },
      ExportClick: function() {
        if(this.$root.SelectedAnalyzer === null) return;
        var Analyzer = this.$root.SelectedAnalyzer;
        var Data = [];
        var Header = ['Harmonic'];
        for(var i = 0; i < oZm.Series['VOLTAGE'].Names(Analyzer).length; i++) { 
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i]);
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i] + ' (dB)');
          Header.push(oZm.Series['VOLTAGE'].Names(Analyzer)[i] + ' (Complex)');
        }
        for(var i = 0; i < oZm.Series['CURRENT'].Names(Analyzer).length; i++) {
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i]);
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i] + ' (dB)');
          Header.push(oZm.Series['CURRENT'].Names(Analyzer)[i] + ' (Complex)');
        }
        Data.push(Header);
        for(i = 0; i < oZm.Series['HARMONICS'].Names.length; i++) {
          var Row = [oZm.Series['HARMONICS'].Names[i]];
          for(var n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length + oZm.Series['CURRENT'].Names(Analyzer).length; n++) {
            Row.push(this.BinPower(this.Data[i][n]));
            Row.push(20.0 * Math.log10(Math.max(0.000000001, this.BinPower(this.Data[i][n]))));
            Row.push(String(this.Data[i][n][0]) + ((this.Data[i][n][1] > 0) ? '+' + this.Data[i][n][1] : this.Data[i][n][1]) + 'i');
          }
          Data.push(Row);
        }
        this.$refs.tools.SaveCVS('Harmonics.csv', Data);
      },
      MaximizeClick: function() {
        this.$refs.tools.Maximize('Harmonics', this.$refs.graph.Graph, this.$refs.Toolbar);
      }
    },
    created: function() {
      this.MaxCurrent = 0;
      this.Data = null;
      this.Phases = null;
      this.First = null;
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