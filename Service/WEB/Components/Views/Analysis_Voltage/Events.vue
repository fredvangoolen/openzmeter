<template>
  <div class="panel panel-other">
    <div class="panel-heading clearfix">
      From {{ StartDate }} to {{ EndDate }}
    </div>
    <div class="panel-body">
      <spinner v-bind:class="{ 'hide': !ShowSpinner }"></spinner>
      <vuetable ref="table" :fields="Columns" :api-mode="false" class="table small table-striped table-responsive">
        <template slot="actions" slot-scope="props">
          <div class="btn-group">
            <button type="button" class="btn btn-xs btn-success" v-on:click="Edit(props.rowData)" :disable="props.rowData.Finished==false">Edit</button>
            <button type="button" class="btn btn-xs btn-default" v-on:click="RMS(props.rowData)" :disable="props.rowData.Finished==false">RMS</button>
            <button type="button" class="btn btn-xs btn-default" v-on:click="RAW(props.rowData)" :disable="props.rowData.Finished==false">Wave</button>
            <button type="button" class="btn btn-danger btn-xs" v-on:click="Delete(props.rowIndex)" :disable="props.rowData.Finished==false">Delete</button>
          </div>
        </template>
      </vuetable>
    </div>
    <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialog" ref="Dialog" data-keyboard="false">
      <div class="modal-backdrop in"></div>
      <div class="modal-dialog full-screen-dialog">
        <div class="modal-content" style="height: 100%">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">Event &raquo; {{ ViewMode }}</h4></div>
            <div class="header-buttons" ref="Toolbar">
              <toolbar-phases variable="DetailsPhaseMulti" ref="phases" :multi="true" v-on:PhasesChanged="PhasesChanged" type="VOLTAGE"></toolbar-phases>
              <toolbar-tools ref="tools" :options="['EXPORT']" v-on:ExportClick="ExportClick"></toolbar-tools>
              <div class="btn-group">
                <button type="button" class="btn btn-default btn-xs" @click="Close"><img :src="require('Imgs/arrow_undo.png')"></button>
              </div>
            </div>
          </div>
          <div class="modal-body">
            <spinner v-if="ShowDialogSpinner"></spinner>
            <graph ref="graph" :options="GraphOptions"></graph>
          </div>
        </div>
      </div>
    </div>
  </div>  
</template>

<script>
  import Spinner from 'Components/Spinner.vue';
  import Vuetable from 'vuetable-2';
  import moment from 'JS/moment.js';
  import * as oZm from 'JS/openZmeter.js';
  import Graph from 'Components/Graph.vue';
  import Dygraph from 'JS/dygraph.js';
  import ToolbarPhases from 'Components/ToolbarPhases.vue';
  import ToolbarTools from 'Components/ToolbarTools.vue';
  export default {
    data: function() {
      return { ShowSpinner: true, StartDate: '', EndDate: '', ShowDialog: false, ViewMode: 'RMS', ShowDialogSpinner: false,
        Columns: [
          {name: 'Date', title: 'Date'},
          {name: 'Type', title: 'Type'}, 
          {name: 'Duration', title: 'Duration <em>(s)</em>'},
          {name: 'Change', title: 'ΔU <em>(V)</em>'},
          {name: 'Percent', title: 'ΔU <em>(%)</em>'},
          {name: 'Notes', title: 'Notes'},
          {name: '__slot:actions', title: ''}
        ]
      };
    },
    created: function() {
      this.Events = [];
      this.Phases = null;
      this.LastSample = 0;
      this.c = null;
      this.GraphOptions = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#EEE", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: Graph.LegendFormatter_Follow,
        interactionModel: { mousedown: Graph.mouseDown, mousemove: Graph.mouseMove, mouseup : Graph.mouseUp, dblclick : Graph.dblClick_Restore }, xAxisHeight: 24, plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ], showRangeSelector: false,
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,/* valueRange: Y1Range, */
               valueFormatter: function (x) { return oZm.Round2(x) + ' <em class="x-small">V</em>'; },
               axisLabelFormatter: function (x) { return oZm.Round2(x); }
          },
          x: { ticker: function(min, max) { 
                 var Analyzer = this.$root.SelectedAnalyzer;
                 if(Analyzer == null) return [];
                 return Graph.WaveTimeTicks(max, Analyzer); 
               }.bind(this),
               valueFormatter: function (x) {
                 var Analyzer = this.$root.SelectedAnalyzer;
                 if(Analyzer == null) return [];
                 return oZm.Round2((x * 1000) / Analyzer.SampleFreq) + ' <em class="x-small">ms</em>'; 
               }.bind(this)
          }
        },
        underlayCallback: function(canvas, area, g) {
          var Analyzer = this.$root.SelectedAnalyzer;
          if((Analyzer == null) || (this.Event == null)) return;
          var LineDash = canvas.getLineDash();
          var StartCount = this.Event.Prev_Samples[0].length;
          var EndCount = StartCount + this.Event.Samples[0].length + ((this.Event.SampleDuration > (2 * this.Event.Prev_Samples[0].length)) ? this.Event.Prev_Samples[0].length * 0.1 : 0);
          //Start and end lines
          canvas.save();
          canvas.setLineDash([3, 2]);
          canvas.strokeStyle = '#5cb85c';
          canvas.lineWidth = 1.5;
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
          canvas.textAlign = 'left';
          canvas.textBaseline = 'middle';
          canvas.font = "12px sans-serif";
          canvas.fillStyle = "#555";
          canvas.fillText("Event start", 0 , 0, g.height_);
          canvas.restore();
          canvas.save();
          canvas.translate(g.toDomXCoord(EndCount) + 8, g.height_ - 26);
          canvas.rotate(-Math.PI / 2);
          canvas.textAlign = 'left';
          canvas.textBaseline = 'middle';
          canvas.font = "12px sans-serif";
          canvas.fillStyle = "#555";
          canvas.fillText("Event end", 0 , 0, g.height_);
          canvas.restore();
//    return;
//      canvas.fillStyle = "#EEE";
//      canvas.fillRect(g.toDomXCoord(0), 0, g.toDomXCoord(result['Envelope']) - g.toDomXCoord(0), g.height_);
//      canvas.fillRect(g.toDomXCoord(ToShow.length - result['Envelope']), 0, g.toDomXCoord(ToShow.length), g.height_);
//      var NominalVoltage = Nodes['NominalVoltage'] * Math.SQRT2;
//      canvas.beginPath();
//      canvas.moveTo(g.toDomXCoord(0), g.toDomYCoord(NominalVoltage));
//      canvas.lineTo(g.width_, g.toDomYCoord(NominalVoltage));
//      canvas.moveTo(g.toDomXCoord(0), g.toDomYCoord(-NominalVoltage));
//      canvas.lineTo(g.width_, g.toDomYCoord(-NominalVoltage));
//      canvas.strokeStyle = '#f00';
//      canvas.lineWidth = 0.5;
//      canvas.stroke();
//      if(result['Duration'] > result['Envelope'] * 4) {
//        canvas.fillStyle = "#fff";
//        canvas.fillRect(g.toDomXCoord(result['Envelope'] * 2), 0, g.toDomXCoord(result['Envelope'] * 2 + 100) - g.toDomXCoord(result['Envelope'] * 2), g.height_);

        if(this.Event.Type == 'DIP') {
          var HighStart = this.Event.Reference[0] * 0.9;
          var HighEnd = this.Event.Reference[0] * 0.95;
          //draw top and bottom lines
          canvas.beginPath();
          canvas.moveTo(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(HighStart));
          canvas.lineTo(g.toDomXCoord(StartCount + StartCount / 2), g.toDomYCoord(HighStart));
          canvas.moveTo(g.toDomXCoord(StartCount - StartCount / 8), g.toDomYCoord(-HighStart));
          canvas.lineTo(g.toDomXCoord(StartCount + StartCount / 2), g.toDomYCoord(-HighStart));
          canvas.stroke();
       return;
//      canvas.moveTo(g.toDomXCoord(result.Prev_Samples[0].length - result.Prev_Samples[0].length / 8), g.toDomYCoord(-evt.Reference[0] * 0.90));  canvas.lineTo(g.toDomXCoord(result.Prev_Samples[0].length + result.Samples[0].length / 2), g.toDomYCoord(-evt.Reference[0] * 0.9));
//          canvas.moveTo(g.toDomXCoord(result.Prev_Samples[0].length + result.Samples[0].length / 2), g.toDomYCoord(evt.Reference[0] * 0.95));   canvas.lineTo(g.toDomXCoord(result.Prev_Samples[0].length + result.Samples[0].length + result.Post_Samples[0].length / 8), g.toDomYCoord(evt.Reference[0] * 0.95));
//          canvas.moveTo(g.toDomXCoord(result.Prev_Samples[0].length + result.Samples[0].length / 2), g.toDomYCoord(-evt.Reference[0] * 0.95));  canvas.lineTo(g.toDomXCoord(result.Prev_Samples[0].length + result.Samples[0].length + result.Post_Samples[0].length / 8), g.toDomYCoord(-evt.Reference[0] * 0.95));


          
          canvas.translate(g.toDomXCoord(result.Prev_Samples[0].length - result.Prev_Samples[0].length / 8), g.toDomYCoord(evt.Reference[0] * 0.9) - 8);
          canvas.textAlign = 'left';
          canvas.textBaseline = 'middle';
          canvas.font = "12px sans-serif";
          canvas.fillStyle = "#555";
          canvas.fillText("DIP enter", 0 , 0, g.height_);
          canvas.restore();
          canvas.save();
          canvas.translate(g.toDomXCoord(result.Prev_Samples[0].length - result.Prev_Samples[0].length / 8), g.toDomYCoord(-evt.Reference[0] * 0.9) + 12);
          canvas.textAlign = 'left';
          canvas.textBaseline = 'middle';
          canvas.font = "12px sans-serif";
          canvas.fillStyle = "#555";
          canvas.fillText("DIP enter", 0 , 0, g.height_);
          canvas.restore();

          canvas.save();
          canvas.translate(g.toDomXCoord(result.Prev_Samples[0].length + result.Samples[0].length + result.Post_Samples[0].length / 8) - 46, g.toDomYCoord(evt.Reference[0] * 0.95) - 8);
          canvas.textAlign = 'left';
          canvas.textBaseline = 'middle';
          canvas.font = "12px sans-serif";
          canvas.fillStyle = "#555";
          canvas.fillText("DIP exit", 0 , 0, g.height_);
          canvas.restore();
          canvas.save();
          canvas.translate(g.toDomXCoord(result.Prev_Samples[0].length + result.Samples[0].length + result.Post_Samples[0].length / 8) - 46, g.toDomYCoord(-evt.Reference[0] * 0.95) + 12);
          canvas.textAlign = 'left';
          canvas.textBaseline = 'middle';
          canvas.font = "12px sans-serif";
          canvas.fillStyle = "#555";
          canvas.fillText("DIP exit", 0 , 0, g.height_);
          canvas.restore();
          
        }
    
        canvas.setLineDash([3, 2]);
        canvas.strokeStyle = '#5cb85c';
        canvas.lineWidth = 1.5;
        canvas.stroke();



        
        
        //      
//      }
//      canvas.beginPath();
//      canvas.moveTo(g.toDomXCoord(result['Envelope']), 0);
//      canvas.lineTo(g.toDomXCoord(result['Envelope']), g.height_);
//      canvas.moveTo(g.toDomXCoord(ToShow.length - result['Envelope']), 0);
//      canvas.lineTo(g.toDomXCoord(ToShow.length - result['Envelope']), g.height_);
      canvas.strokeStyle = '#000';
      canvas.lineWidth = 0.5;
//      canvas.stroke();

//canvas.restore();
canvas.setLineDash(LineDash);  
    }.bind(this),
      };
    },
    methods: {
      PhasesChanged: function(phases) {
        this.Phases = phases;
        this.Data = [[]];
        this.$refs.graph.SetOptions({visibility: phases});
      },
      Redraw: function() {},
      GetEvents: function(to, period, lastsample) {
        var Analyzer = this.$root.SelectedAnalyzer;
        if(lastsample === 0) {
          this.ShowSpinner = true;
          this.Events = [];
          this.LastSample = 0;
        }
        $.ajax({ url: 'getEvents', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ To: to, LastSample: this.LastSample, Analyzer: Analyzer.ID, Period: period }),
          success: function(result) {
            this.EndDate = moment(result.To).format('DD/MM/YYYY - HH:mm:ss');
            this.StartDate = moment(result.From).format('DD/MM/YYYY - HH:mm:ss');
            if((result['StartTime'] === null) || (result['StartTime'].length === 0)) return;
            for(var x = 0; x < result['StartTime'].length; x++) {
              if(result['StartTime'][x] > this.LastSample) this.LastSample = result['StartTime'][x];
              var Event = { StartTime: result.StartTime[x], Date: moment(result.StartTime[x]).format('DD/MM/YYYY - HH:mm:ss'), Type: result.Type[x], Finished: result.Finished[x], Reference: result.Reference[x], SampleDuration: result.Duration[x] };
              Event.Duration = (result.Duration === 0) ? 'RUNNNING' : oZm.Round2Plain(result.Duration[x] / result.SampleRate[x]);
              var Max = 0;        
              for(var i = 0; i < result.Reference[x].length; i++) {
                if(Math.abs(result.Reference[x][i] - result.Peak[x][i]) > Math.abs(Max)) Max = result.Peak[x][i] - result.Reference[x][i];
              }
             if(Event.Type === 'RVC') {
                var Diff = 0;
                for(var i = 0; i < result.Reference[x].length; i++) {
                  if(Math.abs(result.Reference[x][i] - result.Change[x][i]) > Math.abs(Diff)) Diff = result.Change[x][i] - result.Reference[x][i];
                }
                Event.Change = oZm.Round2Plain(Diff) + '<br>(' + oZm.Round2Plain(Max) + ' Max)';
                Event.Percent = oZm.Round2Plain(Diff / result.Reference[x][0] * 100.0);                
              } else {
                Event.Change = oZm.Round2Plain(Max);
                Event.Percent = oZm.Round2Plain(Max / result.Reference[x][0] * 100.0);
              }       
              this.Events.push(Event);
            }
            this.$refs.table.setData( { data: this.Events });
          }.bind(this),
          complete: function() {
            this.ShowSpinner = false;
          }.bind(this),
          error: function(response) {
            this.ShowSpinner = true;
          }.bind(this)
        });
      },
      ResizeHandler: function() {
        $(this.$refs.graph.Graph.graphDiv).parent().removeAttr('style');
        this.$refs.graph.Graph.resize($(this.$refs.Dialog).width() - 50, $(this.$refs.Dialog).height() - 80);
      },
      Close: function() {
        $(window).off('resize', this.ResizeHandler);
        this.$root.$el.removeChild(this.$refs.Dialog);
        this.ShowDialog = false;
      },
      Show: function() {
        this.ShowDialog = true;
        this.ShowDialogSpinner = true;
        $(window).on('resize', this.ResizeHandler);
        this.$nextTick(function() {
          this.$root.$el.append(this.$refs.Dialog);
          $(this.$refs.Dialog).modal('show');
          this.$nextTick(function() { window.dispatchEvent(new Event('resize')); });
        });        
      },
      ExportClick: function() {
        var Analyzer = this.$root.SelectedAnalyzer;
        this.$refs.tools.SaveCVS('Event_' + this.ViewMode + '.csv', [['Sample'].concat(oZm.Series['VOLTAGE'].Names(Analyzer))].concat(this.Data));
      },
      RMS: function(evt) {
        this.ViewMode = 'RMS';
        this.GetSamples(evt);
      },
      RAW: function(evt) {
        this.ViewMode = 'RAW';
        this.GetSamples(evt);
      },
      GetSamples: function(evt) {
        this.Show();
        var Analyzer = this.$root.SelectedAnalyzer;
        $.ajax({ url: 'getEvent', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ StartTime: evt.StartTime, Analyzer: Analyzer.ID, Type: evt.Type, Samples: this.ViewMode }),
          success: function(result) {
            this.Event = Object.assign(evt, result);
            this.Data = [];
            for(var i = 0; i < result.Prev_Samples[0].length; i++) {
              var Row = [this.Data.length];
              for(var n = 0; n < result.Prev_Samples.length; n++) Row.push(result.Prev_Samples[n][i]);
              this.Data.push(Row);
            }
            for(var i = 0; i < result.Samples[0].length; i++) {
              if((evt.SampleDuration > (2 * result.Prev_Samples[0].length)) && (i === result.Prev_Samples[0].length)) {
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
            this.$refs.graph.SetOptions( { file: this.Data, labels: ['Sample'].concat(oZm.Series['VOLTAGE'].Names(Analyzer)), colors: oZm.ColorArray(oZm.Series['VOLTAGE'].Color, oZm.Series['VOLTAGE'].Names(Analyzer).length) } );
            this.ShowDialogSpinner = false;
          }.bind(this)
        }); 
      },  
      Delete: function(evt) {
        var Analyzer = this.$root.SelectedAnalyzer;
        $.ajax({ url: 'delEvent', timeout: 20000, type: 'POST', dataType: 'text', contentType: 'application/json', data: JSON.stringify({ StartTime: this.Events[evt].StartTime, Analyzer: Analyzer.ID, Type: this.Events[evt].Type }),
          success: function(result) {
            this.Events.splice(evt, 1);
            this.$refs.table.setData( { data: this.Events });
          }.bind(this)
        });
      },
      Edit: function(evt) {}
    },
    components: { Spinner, Vuetable, Graph, ToolbarPhases, ToolbarTools }
  }
</script>

<style scoped>
  .table >>> th:nth-child(1) {
    width: 160px;
  } 
  .table >>> th:nth-child(2) {
    width: 60px;
    text-align: center;
  }
  .table >>> td:nth-child(2) {
    text-align: center;
  }
  .table >>> th:nth-child(3), .table >>> th:nth-child(4), .table >>> th:nth-child(5) {
    width: 100px;
    text-align: right;
  }
  .table >>> td:nth-child(3), .table >>> td:nth-child(4), .table >>> td:nth-child(5) {
    text-align: right;
  }
  .table >>> th:nth-child(7) {
    width: 170px;
  } 
</style>


/* Tabla de eventos */
var LastEventStartTime = 0;
var EventsDatasheet    = [];
var EventsTable        = null;
var EventChart         = null;
//var EventSpinner       = new Spinner(SpinnerOpts);
function AjaxGetEvents(from, to, clear) {
  if(clear) LastEventStartTime = 0;
  Analyzer.GetEvents(to - from, to, LastEventStartTime,
    function(result) {
      if(EventsTable === null) EventsTable = $('#EventsTable').DataTable({ responsive: false, paging: false, scrollCollapse: true, scrollY: '224px', dom: '<"top float-left"i><"top float-right"f><"clear">t<"clear">', language: { info: "_MAX_ events in selected period", infoEmpty: "No events in selection"}, columnDefs: [ { targets: [5], orderable: false }, { targets: [0, 1, 2, 3, 4], searchable: true } ], order: [0, 'desc']});
      if(LastEventStartTime === 0) {
        EventsTable.clear().draw();
        EventsDatasheet = [];
      }
      if(EventChart === null) {
        EventChart = new Dygraph(document.getElementById("EventChart"), [Array(Analyzer.GetSeriesNames('Voltage').length + 1).fill(0)], {
          strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], labels: ['Time'].concat(Analyzer.GetSeriesNames('Voltage')), legend: 'follow', legendFormatter: LegendFormatter_Follow,
          colors: ColorArray($('#VoltagePanel').css('border-color'), 3), plugins: [ new Dygraph.Plugins.Crosshair( { direction: "vertical" } ) ],
          interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
        });
      }
    }
  );
}
$("#EventsTable").on('click', 'button:nth-child(1)', function () {
  var Row = EventsTable.row($(this).parents('tr'));
  $('#EventChart div.no-data').addClass('hidden');
  EventsTable.rows().nodes().to$().removeClass('info');
  $(Row.node()).addClass('info');
//  ShowSpinner(EventSpinner, 'EventSpinner');
  $.ajax({
    url: "getEventSamples",
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data: JSON.stringify({ Node: Nodes[Node].ID, Starttime: EventsDatasheet[Row.index()] }),
    success: AjaxSamples
  });
});
$("#EventsTable").on('click', 'button:nth-child(2)', function () {
  var Row = EventsTable.row($(this).parents('tr'));
//  $('#EventChart div.no-data').addClass('hidden');
  EventsTable.rows().nodes().to$().removeClass('info');
  $(Row.node()).addClass('info');
  $('#dlgDeleteEventDialog').modal('show');
//  ShowSpinner(EventSpinner, 'EventSpinner');
//  $.ajax({
//    url: "getEventSamples",
//    type: "POST",
//    dataType: "json",
//    contentType: 'application/json',
//    data: JSON.stringify({ Node: Nodes[Node].ID, Starttime: EventsDatasheet[Row.index()] }),
//    success: AjaxSamples
//  });
});
function AjaxSamples(result) {
  StopSpinner(EventSpinner, 'EventSpinner');
  var ToShow = [];
  for(i = 0; i < result['Samples'].length; i++) {
    if((result['Duration'] > result['Envelope'] * 4) && (i > result['Envelope'] * 2)) {
      ToShow.push([i + 100].concat(result['Samples'][i]));
    } else {
      ToShow.push([i].concat(result['Samples'][i]));
    }
    if((result['Duration'] > result['Envelope'] * 4) && (i === result['Envelope'] * 2)) {
      for(n = 0; n < 100; n++) ToShow.push([i + n + 1].concat(Nodes[Node]['Type'] == '3Phase') ? ['NaN', 'NaN', 'NaN'] : ['NaN']);
    }
  }
  var Options = {
    panEdgeFraction: 0.000000001,
    file: ToShow,
    dateWindow: [0, result['Samples'].length + ((result['Duration'] > result['Envelope'] * 4) ? 100 : 0)],
    valueRange: CalcRanges(ToShow, (Nodes[Node]['Type'] == '3Phase') ? [1, 2, 3] : [1]),
    underlayCallback: function(canvas, area, g) {
      canvas.fillStyle = "#EEE";
      canvas.fillRect(g.toDomXCoord(0), 0, g.toDomXCoord(result['Envelope']) - g.toDomXCoord(0), g.height_);
      canvas.fillRect(g.toDomXCoord(ToShow.length - result['Envelope']), 0, g.toDomXCoord(ToShow.length), g.height_);
      var NominalVoltage = Nodes['NominalVoltage'] * Math.SQRT2;
      canvas.beginPath();
      canvas.moveTo(g.toDomXCoord(0), g.toDomYCoord(NominalVoltage));
      canvas.lineTo(g.width_, g.toDomYCoord(NominalVoltage));
      canvas.moveTo(g.toDomXCoord(0), g.toDomYCoord(-NominalVoltage));
      canvas.lineTo(g.width_, g.toDomYCoord(-NominalVoltage));
      canvas.strokeStyle = '#f00';
      canvas.lineWidth = 0.5;
      canvas.stroke();
      if(result['Duration'] > result['Envelope'] * 4) {
        canvas.fillStyle = "#fff";
        canvas.fillRect(g.toDomXCoord(result['Envelope'] * 2), 0, g.toDomXCoord(result['Envelope'] * 2 + 100) - g.toDomXCoord(result['Envelope'] * 2), g.height_);
        canvas.beginPath();
        canvas.moveTo(g.toDomXCoord(result['Envelope'] * 2), 0);
        canvas.lineTo(g.toDomXCoord(result['Envelope'] * 2), g.height_);
        canvas.moveTo(g.toDomXCoord(result['Envelope'] * 2 + 100), 0);
        canvas.lineTo(g.toDomXCoord(result['Envelope'] * 2 + 100), g.height_);
        canvas.strokeStyle = '#000';
        canvas.lineWidth = 0.5;
        canvas.stroke();
        canvas.save();
        canvas.translate(g.toDomXCoord(result['Envelope'] * 2 + 50), g.height_ / 2);
        canvas.rotate(-Math.PI / 2);
        canvas.textAlign = 'center';
        canvas.textBaseline = 'middle';
        canvas.font = "12px sans-serif";
        canvas.fillStyle = "#555";
        canvas.fillText(Round2Plain((result['Duration'] - result['Envelope'] * 2) / Nodes[Node]['SampleFreq']) + " seconds gap", 0 , 0, g.height_);
        canvas.restore();
      }
      canvas.beginPath();
      canvas.moveTo(g.toDomXCoord(result['Envelope']), 0);
      canvas.lineTo(g.toDomXCoord(result['Envelope']), g.height_);
      canvas.moveTo(g.toDomXCoord(ToShow.length - result['Envelope']), 0);
      canvas.lineTo(g.toDomXCoord(ToShow.length - result['Envelope']), g.height_);
      canvas.strokeStyle = '#000';
      canvas.lineWidth = 0.5;
      canvas.stroke();
    },
    axes: {
      y: {
        valueFormatter: function (x) {
          return Round2(x) + ' <em class="small">V</em>';
        },
        axisLabelFormatter: function (x) {
          return '<small>' + Round2(x) + ' <em class="small">V</em></small>';
        }
      },
      x: {
        valueFormatter: function (y) {
          var Label = '';
          if((result['Duration'] > result['Envelope'] * 4) && (y <= result['Envelope'] * 2)) {
            Label = Round2((y - result['Envelope']) / Nodes[Node]['SampleFreq'] * 1000);
          } else if((result['Duration'] > result['Envelope'] * 4) && (y > result['Envelope'] * 3 + 100)) {
            Label = 'd+' + Round2((y - result['Envelope'] * 3  - 100) / Nodes[Node]['SampleFreq'] * 1000);
          } else if((result['Duration'] > result['Envelope'] * 4) && (y > result['Envelope'] * 2 + 100)) {
            Label = 'd-' + Round2((y - result['Envelope'] * 3  - 100) / Nodes[Node]['SampleFreq'] * -1000);
          } else if(result['Duration'] > result['Envelope'] * 4) {
            Label = "";
          } else {
            Label = Round2((y - result['Envelope']) / Nodes[Node]['SampleFreq'] * 1000);
          }
          return Label + ' ms';
        },
        axisLabelFormatter: function (y) {
          var Label = '';
          if((result['Duration'] > result['Envelope'] * 4) && (y <= result['Envelope'] * 2)) {
            Label = Round2((y - result['Envelope']) / Nodes[Node]['SampleFreq'] * 1000);
          } else if((result['Duration'] > result['Envelope'] * 4) && (y > result['Envelope'] * 3 + 100)) {
            Label = 'd+' + Round2((y - result['Envelope'] * 3  - 100) / Nodes[Node]['SampleFreq'] * 1000);
          } else if((result['Duration'] > result['Envelope'] * 4) && (y > result['Envelope'] * 2 + 100)) {
            Label = 'd-' + Round2((y - result['Envelope'] * 3  - 100) / Nodes[Node]['SampleFreq'] * -1000);
          } else if(result['Duration'] > result['Envelope'] * 4) {
            Label = "";
          } else {
            Label = Round2((y - result['Envelope']) / Nodes[Node]['SampleFreq'] * 1000);
          }
          return '<div style="font-size: 10px; margin-top: -4px">' + Label + ' <em class="small">ms</em></div>';
        }
      }
    }
  };
  EventChart.updateOptions(Options);
  EventLoaded = true;
}