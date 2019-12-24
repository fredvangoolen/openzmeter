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
        <div class="col-xs-12 col-md-4 panel-body">
          <table>
            <tr><td class="border-bottom">Positive sequence</td></tr>
            <tr>
              <td ref="positiveContainer" class="graphDiv">
                <graph ref="positive" class="graph" :options="GraphOptions"></graph>
              </td>
            </tr>
          </table>
        </div>
        <div class="col-xs-12 col-md-4 panel-body">
          <table>
            <tr><td class="border-bottom">Negative sequence</td></tr>
            <tr>
              <td ref="negativeContainer" class="graphDiv">
                <graph ref="negative" class="graph" :options="GraphOptions"></graph>
              </td>
            </tr>
          </table>
        </div>
        <div class="col-xs-12 col-md-4 panel-body">
          <table>
            <tr><td class="border-bottom">Zero sequence</td></tr>
            <tr>
              <td ref="zeroContainer" class="graphDiv">
                <graph ref="zero" class="graph" :options="GraphOptions"></graph>
              </td>
            </tr>
          </table>
        </div>
      </div>
      <div class="row">
        <div class="col-sm-12 col-md-8 col-lg-6 panel-body">
          <table>
            <tr><td class="border-bottom">Phasor view</td></tr>
            <tr>
              <td ref="graphContainer" class="graphDiv">
                <graph ref="graph" class="graph" :options="GraphOptions"></graph>
              </td>
            </tr>
          </table>
        </div>
        
        <div class="col-sm-12 col-md-8 col-lg-6 panel-body">
          <table>
            <tr><td class="border-bottom">Voltage and current phase angle</td></tr>
            <tr>
              <td ref="graphContainer">
                <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped"></vuetable>
              </td>
            </tr>
          </table>
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
  import ToolbarTools from 'Components/ToolbarTools.vue';
  import vuetable from 'vuetable-2';
  import Complex from 'complex';
  import 'CSS/table-responsive.css';
  export default {
    data: function() {
      return { ShowWarning: true,  ShowSpinner: true, Time: 'Loading data...',
      Columns: [
          {name: 'Phase', title: 'Phase'},
          {name: 'Channel', title: 'Channel'},
          {name: 'Angle', title: 'Angle'}, 
          {name: 'SecAngle', title: ''},
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
          canvas.lineWidth = 0.5;
          canvas.beginPath();
          canvas.moveTo(g.toDomXCoord(-115), g.toDomYCoord(0));
          canvas.lineTo(g.toDomXCoord(115), g.toDomYCoord(0));
          canvas.stroke();
          //y-axis
          canvas.strokeStyle = '#000';
          canvas.setLineDash([]);
          canvas.lineWidth = 0.5;
          canvas.beginPath();
          canvas.moveTo(g.toDomXCoord(0), g.toDomYCoord(-115));
          canvas.lineTo(g.toDomXCoord(0), g.toDomYCoord(115));
          canvas.stroke();
          canvas.lineWidth = 1;
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
            canvas.arc(g.toDomXCoord(0), g.toDomYCoord(0), g.toDomXCoord(120 * (Math.log(n * 10) / Math.log(120))) - g.toDomXCoord(0), 0, 2 * Math.PI);
          }    
          canvas.stroke();
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
        var Phases = (oZm.Series['VOLTAGE'].Names(Analyzer).length === 1) ? ['L'] : ['R', 'S', 'T'];
        var ctx = e.drawingContext;
        var MaxCurrent = 0;
        for(var i = 0; i < e.points.length; i++) {
          if((e.dygraph.file_[i][0] % 2) == 1) MaxCurrent = Math.max(MaxCurrent, e.dygraph.file_[i][2]);
        }
        for(var i = 0; i < e.points.length; i++) {
          if(isNaN(e.allSeriesPoints[1][i].yval)) continue;
          var Module = Math.log(e.allSeriesPoints[1][i].yval) / (((e.dygraph.file_[i][0] % 2) == 0) ? Math.log(Analyzer.NominalVoltage) : Math.log(Analyzer.MaxCurrent)) * 100.0;
          Module = Math.max(10, Module);
          if(e.dygraph.file_[i][0] % 2) {
            var StartAngle = (e.dygraph.file_[i - 1][1] < e.dygraph.file_[i][1]) ? e.dygraph.file_[i][1] * Math.PI / 180.0 : e.dygraph.file_[i - 1][1] * Math.PI / 180.0;
            var EndAngle   = (e.dygraph.file_[i - 1][1] < e.dygraph.file_[i][1]) ? e.dygraph.file_[i - 1][1] * Math.PI / 180.0 : e.dygraph.file_[i][1] * Math.PI / 180.0;
            //Angular area
            ctx.fillStyle = e.dygraph.user_attrs_.gridLineColor;
            ctx.beginPath();
            ctx.moveTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0));
            ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(75) - e.dygraph.toDomXCoord(0), -StartAngle, -EndAngle);
            ctx.lineTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0));
            ctx.fill();
            //Black arc to delimit angular area
            ctx.setLineDash([]);
            ctx.strokeStyle = '#333';
            ctx.beginPath();
            ctx.arc(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0), e.dygraph.toDomXCoord(75) - e.dygraph.toDomXCoord(0), -StartAngle, -EndAngle);
            ctx.stroke();
            //Angular legend
            ctx.save();
            var MidAngle = EndAngle + (StartAngle - EndAngle) / 2.0;
            ctx.translate(e.dygraph.toDomXCoord(75 * Math.cos(MidAngle)), e.dygraph.toDomYCoord(75 * Math.sin(MidAngle)));
            ctx.rotate(-MidAngle);
            ctx.beginPath();
            ctx.textAlign = "left";
            ctx.textBaseline = "middle";
            ctx.font = "10px sans";
            ctx.fillStyle = "#333";
            ctx.fillText(oZm.Round2Plain(e.dygraph.file_[i][1] - e.dygraph.file_[i - 1][1]) + "°", 3, 0);
            ctx.font = "bold 12px sans";
            ctx.textAlign = "right";
            ctx.fillText(Phases[(i - 1) / 2], -3, 0);
            ctx.fill();
            ctx.restore();  
          }
          if(((e.dygraph.file_[i][0] % 2) === 0) && ((e.dygraph.file_.length === (i + 1)) || ((e.dygraph.file_[i + 1][0] % 2) === 0))) {
            //Value legend
            ctx.save();
            ctx.translate(e.dygraph.toDomXCoord(100 * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0)), e.dygraph.toDomYCoord(100 * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0)));
            ctx.rotate(e.dygraph.file_[i][1] * Math.PI / -180.0);
            ctx.beginPath();
            ctx.textAlign = "right";
            ctx.font = "bold 12px sans";
            ctx.fillStyle = "#333"; 
            ctx.fillText((e.dygraph.file_.length === 1) ? 'ALL' : Phases[(e.dygraph.file_[i][0]) / 2], -15, -5);
            ctx.fill();
            ctx.restore();
          }
          //Module line
          ctx.setLineDash([]);
          ctx.strokeStyle = e.dygraph.getColors()[e.dygraph.file_[i][0] % 2];
          ctx.fillStyle = e.dygraph.getColors()[e.dygraph.file_[i][0] % 2];
          ctx.lineWidth = 2;
          ctx.beginPath();
          ctx.moveTo(e.dygraph.toDomXCoord(0), e.dygraph.toDomYCoord(0));
          ctx.lineTo(e.dygraph.toDomXCoord(Module * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0)), e.dygraph.toDomYCoord(Module * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0)));
          ctx.stroke();
          //Arrow
          ctx.lineWidth = 1;
          ctx.save();
          ctx.beginPath();
          ctx.translate(e.dygraph.toDomXCoord(Module * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0)), e.dygraph.toDomYCoord(Module * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0)));
          ctx.rotate(e.dygraph.file_[i][1] * Math.PI / -180.0);
          ctx.moveTo(-10, 4);
          ctx.lineTo(5, 0);
          ctx.lineTo(-10, -4);
          ctx.fillStyle = e.dygraph.getColors()[e.dygraph.file_[i][0] % 2];
          ctx.fill();
          ctx.restore();
          //Dot line for the series
          ctx.lineWidth = 1;
          ctx.setLineDash([2, 2]);
          ctx.strokeStyle = e.dygraph.getColors()[e.dygraph.file_[i][0] % 2];
          ctx.beginPath();
          ctx.moveTo(e.dygraph.toDomXCoord(Module * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0)), e.dygraph.toDomYCoord(Module * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0)));
          ctx.lineTo(e.dygraph.toDomXCoord(120 * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0)), e.dygraph.toDomYCoord(120 * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0)));
          ctx.stroke();
          //Value legend
          ctx.save();
          ctx.translate(e.dygraph.toDomXCoord(100 * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0)), e.dygraph.toDomYCoord(100 * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0)));
          ctx.rotate(e.dygraph.file_[i][1] * Math.PI / -180.0);
          ctx.beginPath();
          ctx.textAlign = "left";
          ctx.font = "10px sans";
          ctx.fillStyle = e.dygraph.getColors()[e.dygraph.file_[i][0] % 2];
          ctx.fillText(oZm.Round2Plain(e.allSeriesPoints[1][i].yval) + ((e.dygraph.file_[i][0] % 2) ? "A" : "V"), 2, -5);
          ctx.fill();
          ctx.restore();
        }
      },
      Redraw: function() {
        this.$nextTick(function() {
          if((this.$refs === undefined) || (this.$refs.graphContainer === undefined)) return;
          var Width = this.$refs.graphContainer.clientWidth - 30;
          if(Width > 600) Width = 600;
          this.$refs.graph.Redraw(Width, Width);
          Width = this.$refs.positiveContainer.clientWidth - 30;
          if(Width > 600) Width = 600;
          this.$refs.positive.Redraw(Width, Width);
          this.$refs.negative.Redraw(Width, Width);
          this.$refs.zero.Redraw(Width, Width);
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
          Vectors.push([i * 2, result.Voltage_Phase[i], result.Voltage[i]]);
          Vectors.push([i * 2 + 1, result.Voltage_Phase[i] + result.Phi[i], result.Current[i]]);
          this.Data.push([Phases[i], "Voltage", result.Voltage_Phase[i], result['Voltage'][i]]);
          this.Data.push([Phases[i], "Current", result.Voltage_Phase[i] + result.Phi[i], result['Current'][i]]);
          Table.push( { Phase: Phases[i], Channel: "Voltage", Angle: oZm.Round2(result.Voltage_Phase[i]) + '°', Value: oZm.Round2(result['Voltage'][i]) + ' V' } );
          Table.push( { Phase: Phases[i], Channel: "Current", Angle: oZm.Round2(((result.Phi[i] === null) || (result.Voltage_Phase[i] === null)) ? null : result.Voltage_Phase[i] + result.Phi[i]) + '°', SecAngle: '(' + oZm.Round2(result.Phi[i]) + '°)', Value: oZm.Round2(result.Current[i]) + ' A'} );
        }
        if(result['Voltage'].length === 3) {
          var A = new Complex(Math.cos(120.0 * Math.PI / 180.0), Math.sin(120.0 * Math.PI / 180.0));
          var A2 = A.clone().mult(A);
          var V1 = new Complex(result.Voltage_Harmonics[0][0][0], result.Voltage_Harmonics[0][0][1]);
          var V2 = new Complex(result.Voltage_Harmonics[1][0][0], result.Voltage_Harmonics[1][0][1]);
          var V3 = new Complex(result.Voltage_Harmonics[2][0][0], result.Voltage_Harmonics[2][0][1]);
          var Zero = V1.clone().add(V2).add(V3).mult(new Complex(1.0 / 3.0, 0.0));
          var Positive = V1.clone().add(A.clone().mult(V2)).add(A2.clone().mult(V3)).mult(new Complex(1.0 / 3.0, 0.0));
          var Negative = V1.clone().add(A2.clone().mult(V2)).add(A.clone().mult(V3)).mult(new Complex(1.0 / 3.0, 0.0));
          var PositiveVectors = [], NegativeVectors = [], ZeroVectors = [];
          var OffsetAngle = Positive.arg() * 180.0 / Math.PI;
          for(var n = 0; n < 3; n++) {
            PositiveVectors.push([n * 2, Positive.arg() * 180.0 / Math.PI + (n * 120.0) - OffsetAngle, Positive.abs()]);
            NegativeVectors.push([n * 2, Negative.arg() * 180.0 / Math.PI + (n * -120.0) - OffsetAngle, Negative.abs()]);
          }
          ZeroVectors.push([n * 2, Zero.arg() * 180.0 / Math.PI - OffsetAngle, Zero.abs()]);
          this.$refs.positive.SetOptions({ file: PositiveVectors });
          this.$refs.negative.SetOptions({ file: NegativeVectors });
          this.$refs.zero.SetOptions({ file: ZeroVectors });
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
  .table {
    border: none;
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
    .table >>> th:nth-child(3), .table >>> th:nth-child(5)  {
      text-align: right;
    }
    .table >>> td {
      font-size: 16px
    }
    .table >>> td:nth-child(1), .table >>> td:nth-child(1) {
      text-align: center;
    }
    .table >>> td:nth-child(3), .table >>> td:nth-child(5) {
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
    table {
     width: 100%;
     border-radius: 6px;
     border: 1px solid #ccc;
  }
  table:not(:first-child), .output-table:not(:first-child) {
    margin-top: 5px;
  }
  table tr td {
    border-spacing: 0px;
    padding: 3px;
  }
  table .border-bottom {
    border-bottom: 1px solid #ccc;
    background: #eee;
    font-weight: bold;
  }
  table .border-right {
    border-right: 1px solid #ccc;
  }
</style>
