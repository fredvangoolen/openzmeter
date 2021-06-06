<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      <div class="header-text">{{ (data==null) ? 'Loading data...' : 'Last update at ' + analyzer.FormatTimeMS(data.Sampletime) }} {{ paused ? '(Paused)' : MouseOver ? '(Graph paused)' : '' }}</div>
      <div v-if="RmsCurrent<analyzer.MinCurrent" class="title-warning"><img :src="require('Imgs/exclamation.png')">No current or current too low</div>
      <div class="header-buttons">
        <div class="btn-group">
          <button-export file="Phasors" :rows="Export"></button-export>
          <button-pause :value="paused" @input="$emit('update:paused', $event)"></button-pause>
        </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="data==null"></spinner>
      <div class="row" v-if="analyzer.VoltageChannels>1">
        <div class="col-xs-12 col-md-4 panel-body" ref="SequenceContainer">
          <table class="main-table">
            <tr><td class="border-bottom">Positive sequence</td></tr>
            <tr><td class="graphDiv" @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
              <graph class="graph" :options="GraphOptionsSequence.Positive" :height="SequenceHeight" :width="SequenceHeight"></graph>
            </td></tr>
          </table>
        </div>
        <div class="col-xs-12 col-md-4 panel-body">
          <table class="main-table">
            <tr><td class="border-bottom">Negative sequence</td></tr>
            <tr><td class="graphDiv" @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
              <graph class="graph" :options="GraphOptionsSequence.Negative" :height="SequenceHeight" :width="SequenceHeight"></graph>
            </td></tr>
          </table>
        </div>
        <div class="col-xs-12 col-md-4 panel-body">
          <table class="main-table">
            <tr><td class="border-bottom">Zero sequence</td></tr>
            <tr><td class="graphDiv" @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
              <graph class="graph" :options="GraphOptionsSequence.Zero" :height="SequenceHeight" :width="SequenceHeight"></graph>
            </td></tr>
          </table>
        </div>
      </div>
      <div class="row">
        <div class="col-sm-12 col-md-8 col-lg-6 panel-body" ref="PhasorContainer">
          <table class="main-table">
            <tr><td class="border-bottom">Phasor view</td></tr>
            <tr><td class="graphDiv" @mouseenter="MouseOver=true" @mouseleave="MouseOver=false">
              <graph class="graph" :options="GraphOptionsPhasors" :height="PhasorHeight" :width="PhasorHeight"></graph>
            </td></tr>
          </table>
        </div>
        <div class="col-sm-12 col-md-4 col-lg-6 panel-body">
          <table class="main-table">
            <tr><td class="border-bottom">Voltage and current phase angle</td></tr>
            <tr>
              <td>
                <vuetable ref="table" :fields="Columns" :api-mode="false" :data="Phasors" class="table small table-striped"></vuetable>
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
  import Graph from 'Components/Graph.vue';
  import vuetable from 'vuetable-2';
  import Complex from 'complex';
  import 'CSS/table-responsive.css';
  export default {
    props: { analyzer: Object, data: Object, paused: Boolean },
    data: function() {
      return { MouseOver: false, PhasorHeight: 200, SequenceHeight: 200,
        Columns: [
          { name: 'Phase', title: 'Phase' },
          { name: 'Channel', title: 'Channel' },
          { name: 'Angle', title: 'Angle' },
          { name: 'SecAngle', title: '' },
          { name: 'Value', title: 'Value' }
        ]
      };
    },
    computed: {
      RmsCurrent: function() { return (this.data == null) ? 0.0 : oZm.ArrayAbsMax(this.data.Current); },
      CommonOptions: function() {
        var Opt = { axes: { x: {}, y: {} } };
        Opt.highlightCircleSize = 0;
        Opt.interactionModel = { };
        Opt.legend = 'never';
        Opt.plotter = this.Plotter;
        Opt.xAxisHeight = 0.1;
        Opt.xLabelHeight = 0.1,
        Opt.gridLinePattern = [4, 4];
        Opt.strokeWidth = 0.5;
        Opt.gridLineColor = "rgba(0, 0, 0, 0.1)";
        Opt.labels = ['Index', 'Phase', 'Magnitude'];
        Opt.colors = ['#d9534f', '#428bca'];
        Opt.valueRange = [-120, 120];
        Opt.dateWindow = [-120, 120];
        Opt.axes.y.drawAxis = false;
        Opt.axes.y.drawGrid = false;
        Opt.axes.y.axisLabelWidth = 0.1;
        Opt.axes.y.axisLineColor = "#FFF";
        Opt.axes.x.drawAxis = false;
        Opt.axes.x.drawGrid = false;
        Opt.axes.x.axisLabelWidth = 0.1;
        Opt.axes.x.axisLineColor = "#FFF";
        Opt.underlayCallback = this.Underlay;
        return Opt;
      },
      GraphOptionsSequence: function() {
        var Opt = {Positive: Object.assign({ file: [] }, this.CommonOptions), Negative: Object.assign({ file: [] }, this.CommonOptions), Zero: Object.assign({ file: [] }, this.CommonOptions) };
        if(this.data == null) return Opt;
        if(this.data.Voltage.length == 3) {
          var A = new Complex(Math.cos(120.0 * Math.PI / 180.0), Math.sin(120.0 * Math.PI / 180.0));
          var A2 = A.clone().mult(A);
          var V1 = new Complex(this.data.Voltage_Harmonics_Complex[0][0][0], this.data.Voltage_Harmonics_Complex[0][0][1]);
          var V2 = new Complex(this.data.Voltage_Harmonics_Complex[1][0][0], this.data.Voltage_Harmonics_Complex[1][0][1]);
          var V3 = new Complex(this.data.Voltage_Harmonics_Complex[2][0][0], this.data.Voltage_Harmonics_Complex[2][0][1]);
          var Positive = V1.clone().add(A.clone().mult(V2)).add(A2.clone().mult(V3)).mult(new Complex(1.0 / 3.0, 0.0));
          var Negative = V1.clone().add(A2.clone().mult(V2)).add(A.clone().mult(V3)).mult(new Complex(1.0 / 3.0, 0.0));
          var Zero = V1.clone().add(V2).add(V3).mult(new Complex(1.0 / 3.0, 0.0));
          var OffsetAngle = Positive.arg() * 180.0 / Math.PI;
          for(var n = 0; n < 3; n++) {
            Opt.Positive.file.push([n * 2, Positive.arg() * 180.0 / Math.PI + (n * 120.0) - OffsetAngle, Positive.abs()]);
            Opt.Negative.file.push([n * 2, Negative.arg() * 180.0 / Math.PI + (n * -120.0) - OffsetAngle, Negative.abs()]);
          }
          Opt.Zero.file.push([n * 2, Zero.arg() * 180.0 / Math.PI - OffsetAngle, Zero.abs()]);
        }
        return Opt;
      },
      GraphOptionsPhasors: function() {
        var Opt = {};
        if(this.data != null) {
          Opt.file = [];
          for(var i in this.data.Voltage) {
            Opt.file.push([i * 2, this.data.Voltage_Phase[i], this.data.Voltage[i]]);
            Opt.file.push([i * 2 + 1, this.data.Voltage_Phase[i] + this.data.Phi[i], this.data.Current[i]]);
          }
        }
        Object.assign(Opt, this.CommonOptions);
        return Opt;
      },
      Phasors: function() {
        if(this.data == null) return [];
        var Phasors = [];
        for(var i in this.data.Voltage) {
          Phasors.push( { Phase: (this.analyzer.PhaseNames == null) ? '-' : this.analyzer.PhaseNames[i], Channel: "Voltage", Angle: this.Round(this.data.Voltage_Phase[i], 2, true) + '°', Value: this.Round(this.data.Voltage[i], 2, true) + ' V' } );
          Phasors.push( { Phase: (this.analyzer.PhaseNames == null) ? '-' : this.analyzer.PhaseNames[i], Channel: "Current", Angle: this.Round(((this.data.Phi[i] == null) || (this.data.Voltage_Phase[i] == null)) ? null : this.data.Voltage_Phase[i] + this.data.Phi[i], 2, true) + '°', SecAngle: '(' + this.Round(this.data.Phi[i], 2, true) + '°)', Value: this.Round(this.data.Current[i], 2, true) + ' A'} );
        }
        return Phasors;
      }
    },
    mounted: function() {
      window.addEventListener('resize', this.Resize);
      this.$nextTick(() => { window.dispatchEvent(new Event('resize')); });
    },
    beforeDestroy: function() {
      window.removeEventListener('resize', this.Resize);
    },
    methods: {
      Export: function() {
        if(this.data == null) return [];
        var Data = [['Phase', 'Channel', 'Angle', 'Value']];
        for(var i in this.data.Voltage) {
          Data.push([(this.analyzer.PhaseNames == null) ? '-' : this.analyzer.PhaseNames[i], "Voltage", this.Round(this.data.Voltage_Phase[i], 3), this.Round(this.data.Voltage[i], 3)]);
          Data.push([(this.analyzer.PhaseNames == null) ? '-' : this.analyzer.PhaseNames[i], "Current", this.Round(this.data.Voltage_Phase[i] + this.data.Phi[i], 3), this.Round(this.data.Current[i], 3)]);
        }
        return Object.seal(Data);
      },
      Underlay: function(canvas, area, g) {
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
      },
      Plotter: function(e) {
        if((this.analyzer == null) || (e.seriesIndex != 0) || (e.plotArea.h < 0) || (e.plotArea.w < 0) || (e.dygraph.file_.length == 0) || (e.dygraph.file_[0].length != 3)) return;
        var ctx = e.drawingContext;
        var MaxCurrent = 0;
        for(var i = 0; i < e.points.length; i++) {
          if((e.dygraph.file_[i][0] % 2) == 1) MaxCurrent = Math.max(MaxCurrent, e.dygraph.file_[i][2]);
        }
        for(var i = 0; i < e.points.length; i++) {
          if(isNaN(e.allSeriesPoints[1][i].yval)) continue;
          var Module = Math.log(e.allSeriesPoints[1][i].yval) / (((e.dygraph.file_[i][0] % 2) == 0) ? Math.log(this.analyzer.NominalVoltage) : Math.log(this.analyzer.MaxCurrent)) * 100.0;
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
            ctx.fillText(this.Round(e.dygraph.file_[i][1] - e.dygraph.file_[i - 1][1], 2, false) + "°", 3, 0);
            ctx.font = "bold 12px sans";
            ctx.textAlign = "right";
            ctx.fillText(this.analyzer.PhaseNames[(i - 1) / 2], -3, 0);
            ctx.fill();
            ctx.restore();
          }
          if(((e.dygraph.file_[i][0] % 2) == 0) && ((e.dygraph.file_.length == (i + 1)) || ((e.dygraph.file_[i + 1][0] % 2) == 0))) {
            //Value legend
            ctx.save();
            ctx.translate(e.dygraph.toDomXCoord(100 * Math.cos(e.dygraph.file_[i][1] * Math.PI / 180.0)), e.dygraph.toDomYCoord(100 * Math.sin(e.dygraph.file_[i][1] * Math.PI / 180.0)));
            ctx.rotate(e.dygraph.file_[i][1] * Math.PI / -180.0);
            ctx.beginPath();
            ctx.textAlign = "right";
            ctx.font = "bold 12px sans";
            ctx.fillStyle = "#333";
            ctx.fillText((e.dygraph.file_.length == 1) ? 'ALL' : this.analyzer.PhaseNames[(e.dygraph.file_[i][0]) / 2], -15, -5);
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
          ctx.fillText(this.Round(e.allSeriesPoints[1][i].yval, 2, false) + ((e.dygraph.file_[i][0] % 2) ? "A" : "V"), 2, -5);
          ctx.fill();
          ctx.restore();
        }
      },
      Resize: function() {
        this.PhasorHeight = Math.min(600, this.$refs.PhasorContainer.clientWidth - 30);
        this.SequenceHeight = Math.min(600, this.$refs.SequenceContainer.clientWidth - 30);
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
  .main-table {
    width: 100%;
    border: 1px solid #ccc;
  }
  .main-table:not(:first-child) {
    margin-top: 5px;
  }
  .main-table .border-bottom {
    border-bottom: 1px solid #ccc;
    background: #eee;
    font-weight: bold;
    padding: 3px;
  }
</style>
