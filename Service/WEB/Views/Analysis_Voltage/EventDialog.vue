<template>
  <dlg :fullscreen="true">
    <template v-slot:header>
      <div class="pull-left"><h4 class="modal-title">Event &raquo; {{ mode }}</h4></div>
      <div class="header-buttons">
        <toolbar-phases storage="DetailsPhases" :multi="true" v-model="Phases" :analyzer="analyzer" :current="false"></toolbar-phases>
        <div class="btn-group">
          <button-export file="Event" :rows="[]"></button-export>
          <button type="button" class="btn btn-default btn-xs" @click="$emit('close')"><img :src="require('Imgs/arrow_undo.png')"></button>
        </div>
      </div>
    </template>
    <graph ref="graph" :options="GraphOptions"></graph>
  </dlg>
</template>

<script>
  import Graph from 'Components/Graph.vue';
  import Dygraph from 'JS/dygraph.js';
  import * as oZm from 'JS/openZmeter.js';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  export default {
    props: { analyzer: Object, mode: String, event: Object },
    data: function() {
      return { Phases: null };
    },
    computed: {
      GraphOptions: function() {
        var Opt = { file: [] };
        Opt.selectMode = 'euclidian';
        Opt.stepPlot = true;
        Opt.drawGapEdgePoints = true;
        Opt.connectSeparatedPoints = false;
        Opt.fillGraph = false;
        Opt.legend = 'follow';
        Opt.legendFormatter = Graph.Formatters.Legend_Follow;
//        OinteractionModel: Graph.FixedRangeInteractionModel(this);
        Opt.xAxisHeight = 24;
        Opt.includeZero = true;
        Opt.labels = ['Sample'].concat(this.analyzer.GetSeriesNames('Voltage', this.analyzer.VoltageChannels));
//file: this.Data, ,
//colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length) } );

        return Object.seal(Opt);
      }
    },
    methods: {
      XTicks: function(x) {
        var Analyzer = this.$root.SelectedAnalyzer;
        if((Analyzer == null) || (this.Event == null)) return;
        var EventDuration = this.Event.SampleDuration / Analyzer.SampleRate;
        if(x <= (this.Event.Prev_Samples[0].length + this.Event.Samples[0].length / 2.0)) {
          return oZm.Round3((x - this.Event.Prev_Samples[0].length) / ((this.ViewMode == 'RMS') ? (Analyzer.NominalFreq * 2.0) : (Analyzer.SampleRate)));
        } else if(x >= (this.Data.length - (this.Event.Post_Samples[0].length + this.Event.Samples[0].length / 2.0))) {
          return oZm.Round3(EventDuration + (x - (this.Data.length - this.Event.Post_Samples[0].length)) / ((this.ViewMode == 'RMS') ? (Analyzer.NominalFreq * 2.0) : (Analyzer.SampleRate)));
        } else {
          return "";
        }
      },
      Show: function(evt, mode) {
            this.Data = [];
            for(var i = 0; i < result.Prev_Samples[0].length; i++) {
              var Row = [this.Data.length];
              for(var n = 0; n < result.Prev_Samples.length; n++) Row.push(result.Prev_Samples[n][i]);
              this.Data.push(Row);
            }
            for(var i = 0; i < result.Samples[0].length; i++) {
              if((result.Samples[0].length == (2 * result.Prev_Samples[0].length)) && (i === result.Prev_Samples[0].length)) {
                for(var n = 0; n < (result.Prev_Samples[0].length * 0.1); n++) this.Data.push([this.Data.length].concat(new Array(result.Samples.length).fill(null)));
              }
              var Row = [this.Data.length];
              for(var n = 0; n < result.Samples.length; n++) Row.push(result.Samples[n][i]);
              this.Data.push(Row);
            }
            for(var i = 0; i < result.Post_Samples[0].length; i++) {
              var Row = [this.Data.length];
              for(var n = 0; n < result.Post_Samples.length; n++) Row.push(result.Post_Samples[n][i]);
              this.Data.push(Row);
            }
            this.$refs.graph.SetOptions( { plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
              axes: {
                y: { pixelsPerLabel: 15, axisLabelWidth: 34,
                     valueFormatter: function (x) { return oZm.Round2(x) + ' <em class="x-small">V</em>'; },
                     axisLabelFormatter: function (x) { return oZm.Round2(x); }
                },
                x: { pixelsPerLabel: 25, axisLabelFormatter: this.XTicks.bind(this),
                     valueFormatter: function (x) { return this.XTicks(x) + ' <em class="x-small">ms</em>'; }.bind(this)
                }
              },
              underlayCallback: function(canvas, area, g) {
                var Analyzer = this.$root.SelectedAnalyzer;
                if((Analyzer == null) || (this.Event == null)) return;
                var StartCount = this.Event.Prev_Samples[0].length;
                var EndCount = this.Data.length - this.Event.Post_Samples[0].length;
                var EventWidth = this.Event.Samples[0].length;
                //Start and end lines
                canvas.save();
                canvas.setLineDash([3, 2]);
                canvas.strokeStyle = '#5cb85c';
                canvas.lineWidth = 1.5;
                canvas.textAlign = 'left';
                canvas.textBaseline = 'middle';
                canvas.font = "12px sans-serif";
                canvas.fillStyle = "#555";
                canvas.beginPath();
                canvas.moveTo(g.toDomXCoord(StartCount), 0);
                canvas.lineTo(g.toDomXCoord(StartCount), g.height_);
                canvas.moveTo(g.toDomXCoord(EndCount), 0);
                canvas.lineTo(g.toDomXCoord(EndCount), g.height_);
                canvas.stroke();
                //Start and end labels
                canvas.save();
                canvas.translate(g.toDomXCoord(StartCount) - 8, g.height_ - 26);
                canvas.rotate(-Math.PI / 2);
                canvas.fillText("Event start", 0 , 0, g.height_);
                canvas.restore();
                canvas.save();
                canvas.translate(g.toDomXCoord(EndCount) + 8, g.height_ - 26);
                canvas.rotate(-Math.PI / 2);
                canvas.fillText("Event end", 0 , 0, g.height_);
                canvas.restore();
                if((this.Event.Type == 'DIP') || (this.Event.Type == 'INTERRUPTION') || (this.Event.Type == 'SWELL')) {
                  var HighStart = (this.Event.Type == 'DIP') ? this.Event.Reference[0] * 0.9 : (this.Event.Type == 'SWELL') ? this.Event.Reference[0] * 1.1 : this.Event.Reference[0] * 0.1;
                  var HighEnd = (this.Event.Type == 'DIP') ? this.Event.Reference[0] * 0.92 : (this.Event.Type == 'SWELL') ? this.Event.Reference[0] * 1.08 : this.Event.Reference[0] * 0.12;
                  //draw top and bottom lines
                  if(this.ViewMode==='RMS') {
                    canvas.beginPath();
                    canvas.moveTo(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(HighStart));
                    canvas.lineTo(g.toDomXCoord(StartCount + EventWidth / 2), g.toDomYCoord(HighStart));
                    canvas.moveTo(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(-HighStart));
                    canvas.lineTo(g.toDomXCoord(StartCount + EventWidth / 2), g.toDomYCoord(-HighStart));
                    canvas.moveTo(g.toDomXCoord(EndCount - EventWidth / 2), g.toDomYCoord(HighEnd));
                    canvas.lineTo(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(HighEnd));
                    canvas.moveTo(g.toDomXCoord(EndCount - EventWidth / 2), g.toDomYCoord(-HighEnd));
                    canvas.lineTo(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(-HighEnd));
                    canvas.stroke();
                    canvas.save();
                    canvas.translate(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(HighStart) - 8);
                    canvas.fillText(this.Event.Type + " enter", 0 , 0, g.height_);
                    canvas.restore();
                    canvas.save();
                    canvas.translate(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(-HighStart) + 8);
                    canvas.fillText(this.Event.Type + " enter", 0 , 0, g.height_);
                    canvas.restore();
                    canvas.save();
                    canvas.translate(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(HighEnd) - 8);
                    canvas.textAlign = 'right';
                    canvas.fillText(this.Event.Type + " exit", 0 , 0, g.height_);
                    canvas.restore();
                    canvas.save();
                    canvas.translate(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(-HighEnd) + 8);
                    canvas.textAlign = 'right';
                    canvas.fillText(this.Event.Type + " exit", 0 , 0, g.height_);
                    canvas.restore();
                  }
                } else if(this.Event.Type == 'RVC') {
                  var Colors = oZm.SerieStyle(oZm.Series['VOLTAGE'], Analyzer, 0).Colors;
                  canvas.save();
                  var HighStart = 0;
                  var HighEnd = 0;
                  var HighPeak = 0
                  for(var p = 0; p < Colors.length; p++) {
                    if(this.Phases[p] == false) continue;
                    if(this.Event.Reference[p] > HighStart) HighStart = this.Event.Reference[p];
                    if(this.Event.ExitVoltage[p] > HighEnd) HighEnd = this.Event.ExitVoltage[p];
                    if(this.Event.PeakVoltage[p] > HighPeak) HighPeak = this.Event.PeakVoltage[p];
                    //draw top and bottom lines
                    if(this.ViewMode==='RMS'){
                      canvas.beginPath();
                      canvas.strokeStyle = Colors[p];
                      canvas.moveTo(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(this.Event.Reference[p]));
                      canvas.lineTo(g.toDomXCoord(StartCount), g.toDomYCoord(this.Event.Reference[p]));
                      canvas.moveTo(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(-this.Event.Reference[p]));
                      canvas.lineTo(g.toDomXCoord(StartCount), g.toDomYCoord(-this.Event.Reference[p]));
                      canvas.moveTo(g.toDomXCoord(EndCount), g.toDomYCoord(this.Event.ExitVoltage[p]));
                      canvas.lineTo(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(this.Event.ExitVoltage[p]));
                      canvas.moveTo(g.toDomXCoord(EndCount), g.toDomYCoord(-this.Event.ExitVoltage[p]));
                      canvas.lineTo(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(-this.Event.ExitVoltage[p]));
                      canvas.moveTo(g.toDomXCoord(StartCount), g.toDomYCoord(this.Event.PeakVoltage[p]));
                      canvas.lineTo(g.toDomXCoord(EndCount), g.toDomYCoord(this.Event.PeakVoltage[p]));
                      canvas.moveTo(g.toDomXCoord(StartCount), g.toDomYCoord(-this.Event.PeakVoltage[p]));
                      canvas.lineTo(g.toDomXCoord(EndCount), g.toDomYCoord(-this.Event.PeakVoltage[p]));
                      canvas.stroke();
                    }
                  }
                  if(this.ViewMode==='RMS') {
                    canvas.save();
                    canvas.translate(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(HighStart) - 8);
                    canvas.fillText("RVC enter", 0 , 0, g.height_);
                    canvas.restore();
                    canvas.save();
                    canvas.translate(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(-HighStart) + 8);
                    canvas.fillText("RVC enter", 0 , 0, g.height_);
                    canvas.restore();
                    canvas.save();
                    canvas.translate(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(HighEnd) - 8);
                    canvas.textAlign = 'right';
                    canvas.fillText("RVC exit", 0 , 0, g.height_);
                    canvas.restore();
                    canvas.save();
                    canvas.translate(g.toDomXCoord(EndCount + StartCount / 8), g.toDomYCoord(-HighEnd) + 8);
                    canvas.textAlign = 'right';
                    canvas.fillText("RVC exit", 0 , 0, g.height_);
                    canvas.restore();
                    canvas.save();
                    canvas.translate(g.toDomXCoord(StartCount + EventWidth / 2), g.toDomYCoord(HighPeak) - 8);
                    canvas.textAlign = 'center';
                    canvas.fillText("RVC peak", 0 , 0, g.height_);
                    canvas.restore();
                    canvas.save();
                    canvas.translate(g.toDomXCoord(StartCount + EventWidth / 2), g.toDomYCoord(-HighPeak) + 8);
                    canvas.textAlign = 'center';
                    canvas.fillText("RVC peak", 0 , 0, g.height_);
                    canvas.restore();
                    canvas.restore();
                  }
                }
                canvas.restore();
              }.bind(this),
        });
      },
      ExportClick: function() {
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS('Event_' + this.ViewMode + '.csv', [['Sample'].concat(oZm.Series['VOLTAGE'].Names(Analyzer))].concat(this.Data));
      },
      PhasesChanged: function(phases) {
        this.Phases = phases;
        this.Data = [[]];
        this.$refs.graph.SetOptions({visibility: phases});
      },
      ResizeHandler: function() {
        $(this.$refs.graph.Graph.graphDiv).parent().removeAttr('style');
        this.$refs.graph.Graph.resize($(this.$el).width() - 50, $(this.$el).height() - 80);
      },
      WindowChange: function() {},
      WindowRestore: function() {
        this.$refs.graph.SetOptions( {dateWindow: [0, this.Data.length]} );
      }
    },
    components: { Graph, ToolbarPhases }
  };
</script>