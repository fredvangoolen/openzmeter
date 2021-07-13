<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ Time }}</div> <div v-if="ShowWarning" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <div class="header-buttons">
        <!--toolbar-tools variable="To" ref="tools" :options="['EXPORT', 'PLAY']" v-on:PlayStatus="$parent.PlayStatusChanged" v-on:ExportClick="ExportClick"></toolbar-tools-->
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="ShowSpinner"></spinner>
      <div class="row">
        <!--div class="col-sm-12 col-md-8 col-lg-6 graphDiv" ref="graphContainer">
          <graph ref="graph" class="graph" :options="GraphOptions"></graph>
        </div-->
        <div class="col-sm-12">
          <h4>Active power harmonics distribution</h4>
          <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped"></vuetable>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'moment';
  import Dygraph from 'JS/dygraph.js';
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import vuetable from 'vuetable-2';
  import 'CSS/table-responsive.css';
  export default {
    data: function() {
      return { ShowWarning: true,  ShowSpinner: true, Time: 'Loading data...',
        Columns: [
//          {name: 'Color', title: 'Color'},
          {name: 'Harmonic', title: 'Harmonic'},
          {name: 'Power', title: 'Active power'},
          {name: 'Ratio', title: 'Ratio'}
        ]
      };
    },
    created: function() {
      this.Data = [];
      this.GraphOptions = {
        highlightCircleSize: 0,  interactionModel: { }, legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow, plotter: this.Plotter,
        labels: ['Type', 'Total', 'Fundamental', 'Pos. Harmonics', 'Neg. Harmonics'], colors: ['#f0ad4e', '#DF842A', '#3d3d8b', '#3d3d8b'], dateWindow: [-0.5, 1.5],
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Result = oZm.Series['Active_Power'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
               axisLabelFormatter: function (x) { var Result = oZm.Series['Active_Power'].FormatValue(x); return Result.Value; }
             },
          x: { axisLabelWidth: 80,
               axisLabelFormatter: function(x) { return (x == 0) ? 'Active power' : (x == 1) ? 'Breakdown' : '' },
               valueFormatter: function (x) { return (x == 0) ? 'Active power' : (x == 1) ? 'Breakdown' : '' }
             }
        }
      };
    },
    mounted: function() {
      $(window).on('resize', this.Redraw.bind(this));
    },
    updated: function(){
      oZm.createCaptionsForResponsiveTable(this.$refs.table);
    },
    beforeDestroy: function() {
      $(window).off('resize', this.Redraw);
    },
    methods: {
      Plotter: function(e) {
        if(e.seriesIndex !== 0) return;
        var g = e.dygraph;
        var ctx = e.drawingContext;
        var sets = e.allSeriesPoints;
        g.SeriesCount = sets.length;
        var min_sep = Infinity;
        for(var j = 0; j < sets.length; j++) {
          for(var i = 1; i < sets[j].length; i++) min_sep = Math.min(sets[j][i].canvasx - sets[j][i - 1].canvasx, min_sep);
        }
        var bar_width = Math.floor(2.8 / 3 * min_sep);
        for(var i = 0; i < sets[0].length; i++) {
          var PosSum = 0;
          var NegSum = 0;
          for(var j = 0; j < sets.length; j++) {
            ctx.strokeStyle = g.getColors()[j];
            ctx.fillStyle = g.getColors()[j];
            var p = sets[j][i];
            var x_left = p.canvasx - (bar_width / 2);
            if(isNaN(p.yval)) {
              continue;
            } else if(p.yval < 0) {
              var From = e.dygraph.toDomYCoord(0 + NegSum);
              var Height = e.dygraph.toDomYCoord(p.yval) - e.dygraph.toDomYCoord(0);
              ctx.fillRect(x_left, From, bar_width, Height);
              NegSum += p.yval;
            } else {
              var From = e.dygraph.toDomYCoord(0 + PosSum);
              var Height = e.dygraph.toDomYCoord(p.yval) - e.dygraph.toDomYCoord(0);
              ctx.fillRect(x_left, From, bar_width, Height);
              PosSum += p.yval;
            }
          }
        }
      },
      Redraw: function() {
        this.$nextTick(function() {
          if((this.$refs === undefined) || (this.$refs.graphContainer === undefined)) return;
          var Width = this.$refs.graphContainer.clientWidth - 30;
          this.$refs.graph.Redraw(Width, 300);
        });
      },
      ExportClick: function() {
        this.$refs.tools.SaveCVS('PowerHarmonicsDistribution.csv', [['Phase', 'Channel', 'Angle', 'Value']].concat(this.Data));
      },
      SetData(result) {
        var Analyzer = this.$root.SelectedAnalyzer;
        this.ShowWarning = (oZm.ArrayAbsMax(result.Current) < Analyzer.MinCurrent);
        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
        var Total = result.Active_Power[0];
        var Fundamental = result['Power_Harmonics'][0][0];
        var Positive = 0;
        var Negative = 0;
        for(var i = 1; i < result['Power_Harmonics'][0].length; i++) {
          if(result['Power_Harmonics'][0][i] > 0) Positive += result['Power_Harmonics'][0][i];
          if(result['Power_Harmonics'][0][i] < 0) Negative += result['Power_Harmonics'][0][i];
        }
        var Harmonics = Positive + Negative;
        var Noise = Total - Fundamental - Harmonics;
//        this.$refs.graph.SetOptions({ file: [
//          [0, Total, null, null, null],
//          [1, null, Fundamental, Positive, Negative]
//        ] });
        this.$refs.table.setData( { data: [
          { Harmonic: 'Fundamental', Power: oZm.RoundHTML(Fundamental, 2), Ratio: oZm.RoundHTML(Fundamental / Total * 100, 2) + ' %', Color: '<div style="height: 10px; width: 100%; background-color: #DF842A"></div>' },
          { Harmonic: 'Harmonics', Power: oZm.RoundHTML(Harmonics, 2), Ratio: oZm.RoundHTML(Harmonics / Total * 100, 2) + ' %', Color: '<div style="height: 10px; width: 100%; background-color: #3d3d8b"></div>' },
          { Harmonic: 'Noise+Interharmonics', Power: oZm.RoundHTML(Noise, 2), Ratio: oZm.RoundHTML(Noise / Total * 100, 2) + ' %'},
          { Harmonic: 'Total active power', Power: oZm.RoundHTML(Total, 2), Ratio: oZm.RoundHTML(100, 2) + ' %', Color: '<div style="height: 10px; width: 100%; background-color: #f0ad4e "></div>' }
        ] });
        this.ShowSpinner = false;
      }
    },
    components: { Graph, vuetable }
  }
</script>

<style scoped>
  .graphDiv {
    text-align: center;
  }
  .graphDiv >>> .graph {
    margin: auto;
  }
  @media screen and (min-width: 600px) {
    .table >>> th:nth-child(1)  {
      width: 40px;
      text-align: center;
    }
    .table >>> th:nth-child(3), .table >>> th:nth-child(4)  {
      text-align: right;
      width: 100px;
    }
    .table >>> td {
      font-size: 16px
    }
    .table >>> td:nth-child(3), .table >>> td:nth-child(4) {
      text-align: right;
    }
  }
  .header-text {
    font-size: 14px;
  }
</style>
