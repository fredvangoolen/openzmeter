<template>
  <div>
    <dlg-fs :title="'Event » ' + mode" :height.sync="DialogHeight"></dlg-fs>
    <portal to="fs-toolbar" class="header-buttons">
      <toolbar-phases v-model="Phases" storage="Events" :current="false" :multi="true"></toolbar-phases>
      <div class="btn-group">
        <button-export :rows="Export" :file="'Event_' + mode"></button-export>
        <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
      </div>
    </portal>
    <portal to="fs-body" class="panel-body">
      <graph :options="GraphOptions" :height="DialogHeight"></graph>
    </portal>
  </div>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import Dygraph from 'JS/dygraph.js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { mode: String, event: Object },
    data: function() {
      return { Phases: null, DialogHeight: 300 };
    },
    computed: {
      GraphOptions: function() {
        var Opt = { file: [], axes: { x:{}, y: {}} };
        var Gap = 0;
        if(this.Phases == null) return Opt;
        Opt.visibility = this.Phases;
        Opt.plugins = [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ];
        Opt.selectMode = 'euclidian';
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.legend = 'follow';
        Opt.gridLineColor = "#DDD";
        Opt.gridLinePattern = [4, 4];
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
        Opt.interactionModel = Graph.InteractionModels.Fixed(this);
        Opt.xAxisHeight = 24;
        Opt.includeZero = true;
        Opt.showRangeSelector = false;
        Opt.labels = ['Sample'].concat(this.analyzer.SerieNames('Voltage'));
        Opt.colors = this.analyzer.SerieColors('Voltage');
        Opt.axes.y.pixelsPerLabel = 15;
        Opt.axes.y.axisLabelWidth = 34;
        Opt.axes.y.valueFormatter = (x) => { return this.Round(x, 2, true) + ' V'; };
        Opt.axes.y.axisLabelFormatter = (x) => { return this.Round(x, 2, true); };
        Opt.axes.x.pixelsPerLabel = 25;
        for(var i = 0; i < this.event.Samples.length; i++) {
          Opt.file.push([Opt.file.length].concat(this.event.Samples[i]));
          if((this.event.Samples.length == (4 * this.event.Length)) && (i == (2 * this.event.Length - 1))) {
            Gap = Math.round(this.event.Length * 0.1);
            for(var n = 0; n < Gap; n++) Opt.file.push([Opt.file.length].concat(new Array(this.event.Samples[0].length).fill(null)));
          }
        }
        Opt.axes.x.axisLabelFormatter = (x) => {
          var EventDuration = this.event.Duration / this.event.SampleRate;
          if(x < (this.event.Length * 2)) {
            return this.Round((x - this.event.Length) / ((this.mode == 'RMS') ? (this.analyzer.NominalFrequency * 2.0) : (this.event.SampleRate)), 2, true);
          } else if(x > ((this.event.Length * 2) + Gap)) {
            return this.Round(EventDuration + (x - (Opt.file.length - this.event.Length)) / ((this.mode == 'RMS') ? (this.analyzer.NominalFrequency * 2.0) : (this.event.SampleRate)), 2, true);
          } else {
            return '';
          }
        };
        Opt.axes.x.valueFormatter = (x) => { return Opt.axes.x.axisLabelFormatter(x) + ' ms'; };
        Opt.underlayCallback = (canvas, area, g) => {
          //Start and end lines
          canvas.save();
          canvas.setLineDash([3, 2]);
          canvas.strokeStyle = '#ffbf00';
          canvas.lineWidth = 1.5;
          canvas.textAlign = 'left';
          canvas.textBaseline = 'middle';
          canvas.font = "12px sans-serif";
          canvas.fillStyle = "#555";
          canvas.beginPath();
          canvas.moveTo(g.toDomXCoord(this.event.Length), 0);
          canvas.lineTo(g.toDomXCoord(this.event.Length), g.height_);
          canvas.moveTo(g.toDomXCoord(Opt.file.length - this.event.Length), 0);
          canvas.lineTo(g.toDomXCoord(Opt.file.length - this.event.Length), g.height_);
          canvas.stroke();
          //Start and end labels
          canvas.save();
          canvas.translate(g.toDomXCoord(this.event.Length) - 8, g.height_ - 26);
          canvas.rotate(-Math.PI / 2);
          canvas.fillText("Event start", 0 , 0, g.height_);
          canvas.restore();
          canvas.save();
          canvas.translate(g.toDomXCoord(Opt.file.length - this.event.Length) + 8, g.height_ - 26);
          canvas.rotate(-Math.PI / 2);
          canvas.fillText("Event end", 0 , 0, g.height_);
          canvas.restore();
          if((this.event.Type == 'DIP') || (this.event.Type == 'INTERRUPTION') || (this.event.Type == 'SWELL')) {
            var HighStart = (this.event.Type == 'DIP') ? this.event.Reference[0] * 0.9 : (this.event.Type == 'SWELL') ? this.event.Reference[0] * 1.1 : this.event.Reference[0] * 0.1;
            for(var i = 0; i < this.event.Reference.length; i++) {
              if((this.event.Type == 'SWELL') && (this.event.Reference[i] > HighStart)) HighStart = this.event.Reference[i];
              if((this.event.Type != 'SWELL') && (this.event.Reference[i] < HighStart)) HighStart = this.event.Reference[i];
            }

//            var HighStart = (this.event.Type == 'DIP') ? this.event.Reference[0] * 0.9 : (this.event.Type == 'SWELL') ? this.event.Reference[0] * 1.1 : this.event.Reference[0] * 0.1;
//            var HighEnd = (this.event.Type == 'DIP') ? this.event.Reference[0] * 0.92 : (this.event.Type == 'SWELL') ? this.event.Reference[0] * 1.08 : this.event.Reference[0] * 0.12;
            //draw top and bottom lines
            if(this.mode == 'RMS') {
              canvas.beginPath();
//              canvas.moveTo(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(HighStart));
//              canvas.lineTo(g.toDomXCoord(StartCount + EventWidth / 2), g.toDomYCoord(HighStart));
//              canvas.moveTo(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(-HighStart));
//              canvas.lineTo(g.toDomXCoord(StartCount + EventWidth / 2), g.toDomYCoord(-HighStart));
//              canvas.moveTo(g.toDomXCoord(EndCount - EventWidth / 2), g.toDomYCoord(HighEnd));
//              canvas.lineTo(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(HighEnd));
//              canvas.moveTo(g.toDomXCoord(EndCount - EventWidth / 2), g.toDomYCoord(-HighEnd));
//              canvas.lineTo(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(-HighEnd));
//              canvas.stroke();
              canvas.save();
//              canvas.translate(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(HighStart) - 8);
//              canvas.fillText(this.Event.Type + " enter", 0 , 0, g.height_);
              canvas.restore();
              canvas.save();
//              canvas.translate(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(-HighStart) + 8);
//              canvas.fillText(this.Event.Type + " enter", 0 , 0, g.height_);
              canvas.restore();
              canvas.save();
//              canvas.translate(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(HighEnd) - 8);
              canvas.textAlign = 'right';
//              canvas.fillText(this.Event.Type + " exit", 0 , 0, g.height_);
              canvas.restore();
              canvas.save();
//              canvas.translate(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(-HighEnd) + 8);
              canvas.textAlign = 'right';
//              canvas.fillText(this.Event.Type + " exit", 0 , 0, g.height_);
              canvas.restore();
            }
          } else if(this.event.Type == 'RVC') {
//            var Colors = oZm.SerieStyle(oZm.Series['Voltage'], this.analyzer, 0).Colors;
//            canvas.save();
//            var HighStart = 0;
//            var HighEnd = 0;
//            var HighPeak = 0
//            for(var p = 0; p < Colors.length; p++) {
//              if(this.Phases[p] == false) continue;
//              if(this.event.Reference[p] > HighStart) HighStart = this.event.Reference[p];
//              if(this.event.ExitVoltage[p] > HighEnd) HighEnd = this.event.ExitVoltage[p];
//              if(this.event.PeakVoltage[p] > HighPeak) HighPeak = this.event.PeakVoltage[p];
//              //draw top and bottom lines
              if(this.mode==='RMS') {
                canvas.beginPath();
//                canvas.strokeStyle = Colors[p];
//                canvas.moveTo(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(this.event.Reference[p]));
//                canvas.lineTo(g.toDomXCoord(StartCount), g.toDomYCoord(this.event.Reference[p]));
//                canvas.moveTo(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(-this.event.Reference[p]));
//                canvas.lineTo(g.toDomXCoord(StartCount), g.toDomYCoord(-this.event.Reference[p]));
//                canvas.moveTo(g.toDomXCoord(EndCount), g.toDomYCoord(this.event.ExitVoltage[p]));
//                canvas.lineTo(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(this.event.ExitVoltage[p]));
//                canvas.moveTo(g.toDomXCoord(EndCount), g.toDomYCoord(-this.event.ExitVoltage[p]));
//                canvas.lineTo(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(-this.event.ExitVoltage[p]));
//                canvas.moveTo(g.toDomXCoord(StartCount), g.toDomYCoord(this.event.PeakVoltage[p]));
//                canvas.lineTo(g.toDomXCoord(EndCount), g.toDomYCoord(this.event.PeakVoltage[p]));
//                canvas.moveTo(g.toDomXCoord(StartCount), g.toDomYCoord(-this.event.PeakVoltage[p]));
//                canvas.lineTo(g.toDomXCoord(EndCount), g.toDomYCoord(-this.event.PeakVoltage[p]));
                canvas.stroke();
              }
//            }
            if(this.mode == 'RMS') {
              canvas.save();
//              canvas.translate(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(HighStart) - 8);
//              canvas.fillText("RVC enter", 0 , 0, g.height_);
              canvas.restore();
              canvas.save();
//              canvas.translate(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(-HighStart) + 8);
//              canvas.fillText("RVC enter", 0 , 0, g.height_);
              canvas.restore();
              canvas.save();
//              canvas.translate(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(HighEnd) - 8);
              canvas.textAlign = 'right';
//              canvas.fillText("RVC exit", 0 , 0, g.height_);
              canvas.restore();
              canvas.save();
//              canvas.translate(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(-HighEnd) + 8);
              canvas.textAlign = 'right';
//              canvas.fillText("RVC exit", 0 , 0, g.height_);
              canvas.restore();
              canvas.save();
//              canvas.translate(g.toDomXCoord(StartCount + EventWidth / 2), g.toDomYCoord(HighPeak) - 8);
              canvas.textAlign = 'center';
//              canvas.fillText("RVC peak", 0 , 0, g.height_);
              canvas.restore();
              canvas.save();
//              canvas.translate(g.toDomXCoord(StartCount + EventWidth / 2), g.toDomYCoord(-HighPeak) + 8);
              canvas.textAlign = 'center';
//              canvas.fillText("RVC peak", 0 , 0, g.height_);
              canvas.restore();
            }
            canvas.restore();
          }
          canvas.restore();
        };
        return Object.seal(Opt);
      }
    },
    methods: {
       Export: function() {
         var Rows = [['Sample'].concat(this.Analyzer.SerieNames('Voltage'))];
         for(var i = 0; i < this.event.Samples.length; i++) {
          var Row = [i];
          for(var ch = 0; ch < this.Analyzer.VoltageChannels; ch++) Row.push(this.Round(this.event.Samples[i][ch], 2));
          Rows.push(Row);
          if((this.event.Samples.length == (4 * this.event.Length)) && (i == (2 * this.event.Length))) Rows.push(new Array(this.event.Samples[0].length + 1).fill('- GAP -'));
        }
        return Rows;
      },
      ResizeHandler: function() {
        $(this.$refs.graph.Graph.graphDiv).parent().removeAttr('style');
        this.$refs.graph.Graph.resize($(this.$el).width() - 50, $(this.$el).height() - 80);
      },
      WindowRestore: function() {
        this.$refs.graph.SetOptions( {dateWindow: [0, this.Data.length]} );
      }
    }
  };
</script>