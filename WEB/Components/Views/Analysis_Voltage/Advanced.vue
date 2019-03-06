<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ Time }}</div>
        <div class="header-buttons" ref="Toolbar">
          <toolbar-phases variable="DetailsPhaseMulti" :multi="true" v-on:PhasesChanged="SetPhases"></toolbar-phases>
          <toolbar-options variable="DetailOptions" :options="['Current', 'Log']" v-on:OptionChanged="SetOption"></toolbar-options>
          <toolbar-tools variable="To" ref="Tools" :options="['EXPORT', 'MAXIMIZE', 'PLAY']"></toolbar-tools>
        </div>
      </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div ref="Graph" class="chart big_chart"></div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import Dygraph from 'JS/dygraph.js';
  import Spinner from 'Components/Spinner.vue';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarOptions from 'Components/ToolbarOptions.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  export default {
    props: ['visible'],
    data() {
      return { ShowSpinner: true, Time: 'Loading data...', Graph: null, Data: null, VoltagePhases: null, ShowCurrent: null, Log: null, VOffset: 0, COffset: 0 };
    },
    methods: {
      Redraw() {
        this.$nextTick(function() {    
          $(this.Graph.graphDiv).parent().removeAttr('style');
          window.dispatchEvent(new Event('resize'));
        });
      },
      SetPhases: function(phases) {
        this.VoltagePhases = phases;
        this.CalcOptions();
      },
      SetOption: function(option, val) {
        if(option === 'Current') this.ShowCurrent = val;
        if(option === 'Log') this.Log = val;
        this.CalcOptions();
      },
      SetData(result) {
        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
        this.Data = [];
        for(var i = 0; i < result.Voltage_FFT[0].length; i++) {
          var Row = [];
          for(var n = 0; n < result.Voltage_FFT.length; n++) Row.push(result.Voltage_FFT[n][i]);
          for(var n = 0; n < result.Current_FFT.length; n++) Row.push(result.Current_FFT[n][i]);
          this.Data.push(Row);
        }
        this.CalcOptions();
        this.ShowSpinner = false;
      },
      CalcOptions() {
        if((this.$root.SelectedAnalyzer === null) || (this.VoltagePhases === null) || (this.ShowCurrent === null) || (this.Log === null) || (this.Data === null)) return;
        var Analyzer = this.$root.SelectedAnalyzer, ToShow = [], Series = {}, CurrentPhases = [];
        for(var i = 0; i < this.Data.length; i++) {
          var Row = [i];
          for(var n = 0; n < this.Data[i].length; n++) Row.push(this.Log ? 20.0 * Math.log10(Math.max(0.001, this.Data[i][n])) : this.Data[i][n]);
          ToShow.push(Row);
        }
        for(var n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length; n++) {
          CurrentPhases.push((this.VoltagePhases[n] === true) && (this.ShowCurrent === true));
          if((this.VoltagePhases[n] === true) && (this.ShowCurrent === true)) Series[oZm.Series['CURRENT'].Names(Analyzer)[n]] = { axis: 'y2' };
        }  
        var Y1Range = oZm.Series['VOLTAGE_SAMPLES'].Range(ToShow, this.VoltagePhases);
        var Y2Range = oZm.Series['CURRENT_SAMPLES'].Range(ToShow, Array(this.VoltagePhases.length).fill(false).concat(CurrentPhases));
        this.VOffset = (Y1Range[0] < 0.0) ? -Y1Range[0] : 0.0;
        this.COffset = (Y2Range[0] < 0.0) ? -Y2Range[0] : 0.0;
        for(var i = 0; i < this.Data.length; i++) {
          var Row = [i];
          for(n = 0; n < oZm.Series['VOLTAGE'].Names(Analyzer).length; n++) Row.push(ToShow[i][n + 1] + this.VOffset);
          for(n = 0; n < oZm.Series['CURRENT'].Names(Analyzer).length; n++) Row.push(ToShow[i][n + 1 + oZm.Series['VOLTAGE'].Names(Analyzer).length] + this.COffset);
          ToShow[i] = Row;
        }
        var Options = {
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], fillGraph: true, interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup  : oZm.mouseUp, dblclick : oZm.dblClick_Restore },
          legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow, labels: ['Bin'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)).concat(oZm.Series['CURRENT'].Names(Analyzer)), series: Series, visibility: this.VoltagePhases.concat(CurrentPhases),
          plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ], showRangeSelector: false,
          colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length).concat(oZm.ColorArray(oZm.Series['CURRENT'].Color, oZm.Series['CURRENT'].Names(Analyzer).length)),
          axes: { 
            y: { pixelsPerLabel: 15, axisLabelWidth: 34, valueRange: [Y1Range[0] + this.VOffset, Y1Range[1] + this.VOffset], 
                 ticker: this.Log ? function(min, max) { return oZm.GraphDBTicks(min, max, this.VOffset); }.bind(this) : Dygraph.numericTicks,
                 valueFormatter: function(x) { return oZm.Round2(x - this.VOffset) + '<em class="x-small">' + (this.Log ? 'dB' : 'V') + '</em>'; }.bind(this),
                 axisLabelFormatter: function (x) { return oZm.Round2(x); }
               },
            y2: { pixelsPerLabel: 15, axisLabelWidth: this.ShowCurrent ? 34 : 0, valueRange: [Y2Range[0] + this.COffset, Y2Range[1] + this.COffset],
                  valueFormatter: function(x) { return oZm.Round2(x - this.COffset) + ' <em class="x-small">' + (this.Log ? 'dB' : 'A') + '</em>'; }.bind(this),
                  ticker: (this.Log || this.ShowCurrent === false) ? function(min, max) { return (this.ShowCurrent === true) ? oZm.GraphDBTicks(min, max, this.COffset) : []; }.bind(this) : Dygraph.numericTicks,
                  axisLabelFormatter: function (x) { return oZm.Round2(x - this.COffset); }.bind(this)
                },
            x: { drawGrid: true,  
                 valueFormatter: function (y) { return Math.round(y / 10 * Analyzer.NominalFreq) + '<em class="small"> Hz</em>'; },
                 axisLabelFormatter: function (y) {
                   y = (y / 10 * Analyzer.NominalFreq);
                   if(y >= 1000) {
                     y = y / 1000;
                     if((y % 1) !== 0) return "";
                     return y + '<em class="small"> kHz</em>';
                   } else {
                     if((y % 1) !== 0) return "";
                     return y + '<em class="small"> Hz</em>';
                   }
                 }
               }
          },
          file: ToShow
        };
        this.Graph.updateOptions({ showRangeSelector: true}); 
        this.Graph.updateOptions(Options);
      }
    },
    watch: {
      visible: function(newVal, oldVal) {
        if(newVal === false) return;      
        this.$nextTick(function() {    
          $(this.Graph.graphDiv).parent().removeAttr('style');
          window.dispatchEvent(new Event('resize'));
        });
      }
    },
    mounted: function() {
      this.Graph = new Dygraph(this.$refs.Graph, [[0, 0]], { });
    },
    components: { Spinner, ToolbarPhases, ToolbarOptions, ToolbarTools }
  }
</script>