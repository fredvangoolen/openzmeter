<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ Time }}</div>
      <div class="header-buttons" ref="Toolbar">
        <toolbar-tools variable="To" ref="tools" :options="['EXPORT', 'PLAY']" v-on:PlayStatus="$parent.PlayStatusChanged" v-on:ExportClick="ExportClick"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <div class="col-sm-3 col-md-6 col-lg-4" ref="graphContainer">
          <graph ref="graph" class="graph" :options="GraphOptions"></graph>
        </div>
        <div class="col-sm-9 col-md-6 col-lg-8">
          <h4>Voltage and current phase angle</h4>
          <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive"></vuetable>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import * as oZm from 'JS/openZmeter.js';
  import moment from 'JS/moment.js';
  import Dygraph from 'JS/dygraph.js';
  import Spinner from 'Components/Spinner.vue';
  import Graph from 'Components/Graph.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import vuetable from 'vuetable-2';
  export default {
    data() {
      return { ShowSpinner: true, Time: 'Loading data...',
      Columns: [
          {name: 'Phase', title: 'Phase'},
          {name: 'Channel', title: 'Channel'},
          {name: 'Angle', title: 'Angle'}, 
          {name: 'Value', title: 'Value'}
        ]
      };
    },
    created: function() {
      this.Data = [];
      this.GraphOptions = {
        highlightCircleSize: 0,  interactionModel: { }, legend: 'never', valueRange: [-130, 130], dateWindow: [-130, 130], plotter: this.Plotter, xAxisHeight: 0.1, xLabelHeight: 0.1
      };
      $(window).on('resize', this.Redraw.bind(this));
    },
    methods: {
      Plotter: function(e) {
        if((e.seriesIndex !== 0) || (e.plotArea.h < 0) || (e.plotArea.w < 0)) return;
        var ctx = e.drawingContext;
        ctx.lineWidth = 0.5;
        ctx.strokeStyle = e.dygraph.attrs_.axisLineColor;
        ctx.beginPath();
        ctx.moveTo(e.dygraph.toDomXCoord(-100), e.dygraph.toDomYCoord(0));
        ctx.lineTo(e.dygraph.toDomXCoord(100), e.dygraph.toDomYCoord(0));
        ctx.moveTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(-100));
        ctx.lineTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(100));
        ctx.stroke();
        ctx.beginPath();
        ctx.setLineDash(e.dygraph.user_attrs_.gridLinePattern);
        ctx.strokeStyle = e.dygraph.user_attrs_.gridLineColor;
        ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(100) - e.dygraph.toDomXCoord(0), 0, 2 * Math.PI);
        ctx.stroke();
        ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(75) - e.dygraph.toDomXCoord(0), 0, 2 * Math.PI);
        ctx.stroke();
        ctx.beginPath();
        ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(50) - e.dygraph.toDomXCoord(0), 0, 2 * Math.PI);
        ctx.stroke();
        ctx.beginPath();
        ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(25) - e.dygraph.toDomXCoord(0), 0, 2 * Math.PI);
        ctx.stroke();
        ctx.beginPath();
        ctx.fillStyle = '#000';
        ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), 5, 0, 2 * Math.PI, false);
        ctx.fill();
        ctx.setLineDash([]);
        if((e.dygraph.file_.length === 0) || (e.dygraph.file_[0].length !== 3)) return;
        for(var i = 0; i < e.points.length; i++) {
          if(isNaN(e.allSeriesPoints[1][i].yval)) continue;
          var X = e.dygraph.toDomXCoord((100.0 * e.allSeriesPoints[1][i].yval - 5) * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0));
          var Y = e.dygraph.toDomYCoord((100.0 * e.allSeriesPoints[1][i].yval - 5) * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0));
          if(i % 2) {
            ctx.fillStyle = 'rgba(0, 0, 0, 0.1)';
            ctx.beginPath();
            ctx.moveTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0));
            if(e.dygraph.file_[i - 1][1] < e.dygraph.file_[i][1]) {
              ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(100) - e.dygraph.toDomXCoord(0), e.dygraph.file_[i - 1][1] * Math.PI / -180.0, e.dygraph.file_[i][1] * Math.PI / -180.0, true);
            } else {
              ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(100) - e.dygraph.toDomXCoord(0), e.dygraph.file_[i][1] * Math.PI / -180.0, e.dygraph.file_[i - 1][1] * Math.PI / -180.0, true);
            }
            ctx.lineTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0));
            ctx.fill();
          }
          e.points[i].canvasx = X;
          e.points[i].canvasy = Y;
          ctx.strokeStyle = e.dygraph.getColors()[i % 2];
          ctx.fillStyle = e.dygraph.getColors()[i % 2];
          ctx.lineWidth = 1;
          ctx.beginPath();
          ctx.moveTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0));
          ctx.lineTo(X, Y);
          ctx.stroke();
          ctx.save();
          ctx.beginPath();
          ctx.translate(X, Y);
          ctx.rotate(e.dygraph.file_[i][1] * Math.PI / -180.0);
          ctx.moveTo(-5, 4);
          ctx.lineTo(10, 0);
          ctx.lineTo(-5, -4);
          ctx.fill();
          ctx.beginPath();
          ctx.arc(21, 0, 10, 0, 2 * Math.PI);
          ctx.fill();
          ctx.beginPath();
          ctx.font = "bold 12px sans";
          ctx.fillStyle = "#FFF";
          ctx.fillText((i % 2) ? "A" : "V", 17, 4);
          ctx.fill();
          ctx.restore();
        }
      },
      Redraw: function() {
        this.$nextTick(function() {
          var Width = this.$refs.graphContainer.clientWidth;
          this.$refs.graph.Redraw(Width, Width);
        });
      },
      beforeDestroy: function() {
        $(window).off('resize', this.Redraw);
      },
      ExportClick: function() {
        this.$refs.tools.SaveCVS('Phasors.csv', [['Phase', 'Channel', 'Angle', 'Value']].concat(this.Data));
      },
      SetData(result) {
        var Analyzer = this.$root.SelectedAnalyzer;
        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
        var Phases = (oZm.Series['VOLTAGE'].Names(Analyzer).length === 1) ? ['L'] : ['R', 'S', 'T'];
        this.Data = [];
        var Table = [];
        var Vectors = [];
        var MaxCurrent = 0;
        var MaxVoltage = 0;
        for(var i = 0; i < result['Voltage'].length; i++) {
          Vectors.push([i * 2, 120 * i, result.Voltage[i]]);
          Vectors.push([i * 2 + 1, 120 * i - result.Phi[i], result.Current[i]]);
          this.Data.push([Phases[i], "Voltage", i * 120, result['Voltage'][i]]);
          this.Data.push([Phases[i], "Current", i * 120 - result.Phi[i], result['Current'][i]]);
          Table.push( { Phase: Phases[i], Channel: "Voltage", Angle: oZm.Round2(i * 120) + 'º', Value: oZm.Round2(result['Voltage'][i]) + '<em> V</em>' } );
          Table.push( { Phase: Phases[i], Channel: "Current", Angle: oZm.Round2(i * 120 - result.Phi[i]) + 'º', Value: oZm.Round2(result.Current[i]) + '<em> A</em>'} );
          MaxVoltage = Math.max(MaxVoltage, result.Voltage[i]);
          MaxCurrent = Math.max(MaxCurrent, result.Current[i]);
        }
        for(var i = 0; i < Vectors.length; i += 2) {
          Vectors[i][2] = Vectors[i][2] / MaxVoltage;
          Vectors[i + 1][2] = Vectors[i + 1][2] / MaxCurrent;
        }
        var Options = {
          labels: ['Index', 'Phase', 'Magnitude'], colors: ['#d9534f', '#428bca'], 
          axes: {
            y: { drawAxis: false, drawGrid: false, axisLabelWidth: 0.0, axisLineColor: "#FFF" },
            x: { drawAxis: false, drawGrid: false, axisLabelWidth: 0, axisLineColor: "#FFF" }
          },
          file: Vectors
        };
        this.$refs.graph.SetOptions(Options);
        this.$refs.table.setData( { data: Table });
        this.ShowSpinner = false;
      }
    },
    components: { Spinner, ToolbarTools, Graph, vuetable }
  }
</script>

<style scoped>
  .table >>> th:nth-child(1)  {
    width: 20px;
    text-align: center;
  }  
  .table >>> th:nth-child(2)  {
    width: 50px;
    text-align: center;
  }
  .table >>> th:nth-child(3), .table >>> th:nth-child(4)  {
    text-align: right;
  }
  .table >>> td {
    font-size: 16px
  }
  .table >>> td:nth-child(1), .table >>> td:nth-child(1) {
    text-align: center;
  }
  .table >>> td:nth-child(3), .table >>> td:nth-child(4) {
    text-align: right;
  }
</style>