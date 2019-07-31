<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ Time }}</div> <div v-if="ShowWarning" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <div class="header-buttons" ref="Toolbar">
        <toolbar-tools variable="To" ref="tools" :options="['EXPORT', 'PLAY']" v-on:PlayStatus="$parent.PlayStatusChanged" v-on:ExportClick="ExportClick"></toolbar-tools>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <div class="row">
        <div class="col-sm-12 col-md-8 col-lg-6 graphDiv" ref="graphContainer">
          <graph ref="graph" class="graph" :options="GraphOptions"></graph>
        </div>
        <div class="col-sm-12 col-md-4 col-lg-6">
          <h4>Voltage and current phase angle</h4>
          <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped"></vuetable>
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
  import 'CSS/table-responsive.css';
  export default {
    data: function() {
      return { ShowWarning: true,  ShowSpinner: true, Time: 'Loading data...',
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
        highlightCircleSize: 0,  interactionModel: { }, legend: 'never', plotter: this.Plotter, xAxisHeight: 0.1, xLabelHeight: 0.1,
        labels: ['Index', 'Phase', 'Magnitude'], colors: ['#d9534f', '#428bca'], valueRange: [-120, 120], dateWindow: [-120, 120],
        axes: {
          y: { drawAxis: false, drawGrid: false, axisLabelWidth: 0.1, axisLineColor: "#FFF" },
          x: { drawAxis: false, drawGrid: false, axisLabelWidth: 0.1, axisLineColor: "#FFF" }
        },
        underlayCallback: function(canvas, area, g) {
          if((area.h <= 0) || (area.w <= 0)) return;
          //x-axis
          canvas.strokeStyle = '#000';
          canvas.setLineDash([]);
          canvas.lineWidth = 1;
          canvas.beginPath();
          canvas.moveTo(g.toDomXCoord(-115), g.toDomYCoord(0));
          canvas.lineTo(g.toDomXCoord(115), g.toDomYCoord(0));
          canvas.stroke();
          
          canvas.beginPath();
          canvas.translate(g.toDomXCoord(115), g.toDomYCoord(0));
          canvas.moveTo(-10, 4);
          canvas.lineTo(5, 0);
          canvas.lineTo(-10, -4);
          canvas.fill();
          canvas.closePath();
          canvas.restore();
          canvas.save();

          //y-axis
          canvas.strokeStyle = '#000';
          canvas.setLineDash([]);
          canvas.lineWidth = 1;
          canvas.beginPath();
          canvas.moveTo(g.toDomXCoord(0), g.toDomYCoord(-115));
          canvas.lineTo(g.toDomXCoord(0), g.toDomYCoord(115));
          canvas.stroke();
          
          canvas.beginPath();
          canvas.translate(g.toDomXCoord(0), g.toDomYCoord(115));
          canvas.moveTo(4,10);
          canvas.lineTo(0,-5);
          canvas.lineTo(-4,10);
          canvas.fill();
          canvas.closePath();
          canvas.restore();

          canvas.lineWidth = 2;
          canvas.setLineDash(g.user_attrs_.gridLinePattern);
          canvas.strokeStyle = g.user_attrs_.gridLineColor;
          canvas.beginPath();
          for(var n = 0; n < 2 * Math.PI; n = n + Math.PI / 6) {
            canvas.moveTo(g.toDomXCoord(0), g.toDomYCoord(0));
            canvas.lineTo(g.toDomXCoord(120 * Math.cos(n)), g.toDomYCoord(120 * Math.sin(n)));
          }
          canvas.stroke();
          canvas.beginPath();
          for(var n = 1; n < 11; n++) {
            //if(n === 10) continue;      
            canvas.arc(g.toDomXCoord(0), g.toDomYCoord(0), g.toDomXCoord(n * 10) - g.toDomXCoord(0), 0, 2 * Math.PI);
          }    
          canvas.stroke();
          // canvas.strokeStyle = oZm.Series["VOLTAGE"].Color;
          // canvas.beginPath();
          // canvas.arc(g.toDomXCoord(0), g.toDomYCoord(0), g.toDomXCoord(100) - g.toDomXCoord(0), 0, 2 * Math.PI);
          // canvas.stroke();
          // canvas.strokeStyle = oZm.Series["CURRENT"].Color;
          // canvas.beginPath();
          // canvas.arc(g.toDomXCoord(0), g.toDomYCoord(0), g.toDomXCoord(80) - g.toDomXCoord(0), 0, 2 * Math.PI);
          // canvas.stroke();          
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
        var Analyzer = this.$root.SelectedAnalyzer;
        if((Analyzer === null) || (e.seriesIndex !== 0) || (e.plotArea.h < 0) || (e.plotArea.w < 0) || (e.dygraph.file_.length === 0) || (e.dygraph.file_[0].length !== 3)) return;
        var ctx = e.drawingContext;
        var MaxCurrent = 0;
        for(var i = 0; i < e.points.length; i++) {
          if((i % 2) == 1) MaxCurrent = Math.max(MaxCurrent, e.dygraph.file_[i][2]);
        }
        for(var i = 0; i < e.points.length; i++) {
          if(isNaN(e.allSeriesPoints[1][i].yval)) continue;
          var Module = e.allSeriesPoints[1][i].yval / (((i % 2) == 0) ? Analyzer.NominalVoltage : Analyzer.MaxCurrent) * 100.0;
          if(i % 2) {
            var StartAngle = (e.dygraph.file_[i - 1][1] < e.dygraph.file_[i][1]) ? e.dygraph.file_[i - 1][1] * Math.PI / -180.0 : e.dygraph.file_[i][1] * Math.PI / -180.0;
            var EndAngle = (e.dygraph.file_[i - 1][1] < e.dygraph.file_[i][1]) ? e.dygraph.file_[i][1] * Math.PI / -180.0 : e.dygraph.file_[i - 1][1] * Math.PI / -180.0;
            ctx.save();
            var MidAngle = (e.dygraph.file_[i - 1][1] - e.dygraph.file_[i][1]) * Math.PI / -180.0;
            ctx.translate(e.dygraph.toDomXCoord(50 * Math.cos(MidAngle / 2.0)), e.dygraph.toDomYCoord(50 * Math.sin(MidAngle / 2.0)));
            ctx.rotate(MidAngle / -2.0);
            ctx.beginPath();
            ctx.textAlign = "left";
            ctx.font = "10px sans";
            ctx.fillStyle = "#333";
            ctx.fillText(oZm.Round2Plain(Math.abs(e.dygraph.file_[i - 1][1] - e.dygraph.file_[i][1])) + "º", 2, 5);
            ctx.fill();
            ctx.restore();  
            
            ctx.fillStyle = e.dygraph.user_attrs_.gridLineColor;
            ctx.setLineDash([]);
            
            //Angular area
            ctx.beginPath();
            ctx.moveTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0));
            ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(50) - e.dygraph.toDomXCoord(0), StartAngle, EndAngle, true);
            ctx.lineTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0));
            ctx.fill();
            
            //Black arc to delimit angular area.
            ctx.strokeStyle = '#333';
            ctx.beginPath();
            ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(50) - e.dygraph.toDomXCoord(0), StartAngle, EndAngle, true);
            ctx.stroke();
          }

          //Module line
          ctx.setLineDash([]);
          ctx.strokeStyle = e.dygraph.getColors()[i % 2];
          ctx.fillStyle = e.dygraph.getColors()[i % 2];
          ctx.lineWidth = 2;
          
          ctx.beginPath();
          ctx.moveTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0));
          ctx.lineTo(e.dygraph.toDomXCoord(Module * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0)), e.dygraph.toDomYCoord(Module * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0)));
          ctx.stroke();
          
          ctx.lineWidth = 1;
          ctx.beginPath();
          ctx.translate(e.dygraph.toDomXCoord(Module * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0)), e.dygraph.toDomYCoord(Module * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0)));
          ctx.rotate(e.dygraph.file_[i][1] * Math.PI / -180.0);
          ctx.moveTo(-10, 4);
          ctx.lineTo(5, 0);
          ctx.lineTo(-10, -4);
          ctx.fillStyle = e.dygraph.getColors()[i % 2];
          ctx.fill();
          ctx.restore();

          //Dot line for the series
          ctx.lineWidth = 1;
          ctx.setLineDash([2, 2]);
          ctx.strokeStyle = e.dygraph.getColors()[i % 2];
          ctx.beginPath();
          ctx.moveTo(e.dygraph.toDomXCoord(Module * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0)), e.dygraph.toDomYCoord(Module * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0)));
          ctx.lineTo(e.dygraph.toDomXCoord(120 * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0)), e.dygraph.toDomYCoord(120 * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0)));
          ctx.stroke();
          
            
          //Value
          ctx.save();
          ctx.translate(e.dygraph.toDomXCoord(100 * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0)), e.dygraph.toDomYCoord(100 * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0)));
          ctx.rotate(e.dygraph.file_[i][1] * Math.PI / -180.0);
          ctx.beginPath();
          ctx.textAlign = "left";
          ctx.font = "10px sans";
          ctx.fillStyle = e.dygraph.getColors()[i % 2];
          ctx.fillText(oZm.Round2Plain(e.allSeriesPoints[1][i].yval) + ((i % 2) ? "A" : "V"), 2, -5);
          ctx.fill();
          ctx.restore();
          ctx.save();
        }
      },
      Redraw: function() {
        this.$nextTick(function() {
          if((this.$refs === undefined) || (this.$refs.graphContainer === undefined)) return;
          var Width = this.$refs.graphContainer.clientWidth - 30;
          if(Width > 600) Width = 600;
          this.$refs.graph.Redraw(Width, Width);
        });
      },
      ExportClick: function() {
        this.$refs.tools.SaveCVS('Phasors.csv', [['Phase', 'Channel', 'Angle', 'Value']].concat(this.Data));
      },
      SetData(result) {
        var Analyzer = this.$root.SelectedAnalyzer;
        this.ShowWarning = (oZm.ArrayAbsMax(result.Current) < Analyzer.MinCurrent); 
        this.Time = 'Last update at ' + moment(result.Sampletime).format('HH:mm:ss.SSS');
        var Phases = (oZm.Series['VOLTAGE'].Names(Analyzer).length === 1) ? ['L'] : ['R', 'S', 'T'];
        this.Data = [];
        var Table = [];
        var Vectors = [];
        for(var i = 0; i < result['Voltage'].length; i++) {
          Vectors.push([i * 2, 120 * i, result.Voltage[i]]);
          Vectors.push([i * 2 + 1, 120 * i - result.Phi[i], result.Current[i]]);
          this.Data.push([Phases[i], "Voltage", i * 120, result['Voltage'][i]]);
          this.Data.push([Phases[i], "Current", i * 120 - result.Phi[i], result['Current'][i]]);
          Table.push( { Phase: Phases[i], Channel: "Voltage", Angle: oZm.Round2(i * 120) + 'º', Value: oZm.Round2(result['Voltage'][i]) + '<em> V</em>' } );
          Table.push( { Phase: Phases[i], Channel: "Current", Angle: oZm.Round2(i * 120 - result.Phi[i]) + 'º', Value: oZm.Round2(result.Current[i]) + '<em> A</em>'} );
        }
        this.$refs.graph.SetOptions({ file: Vectors });
        this.$refs.table.setData( { data: Table });
        this.ShowSpinner = false;
      }
    },
    components: { Spinner, ToolbarTools, Graph, vuetable }
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
  }
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