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
            <button type="button" class="btn btn-xs btn-success" v-on:click="Edit(props.rowData)">Edit</button>
            <button type="button" class="btn btn-xs btn-default" v-on:click="RMS(props.rowData)">RMS</button>
            <button type="button" class="btn btn-xs btn-default" v-on:click="RAW(props.rowData)">Wave</button>
            <button type="button" class="btn btn-danger btn-xs" v-on:click="Delete(props.rowData)">Delete</button>
          </div>
        </template>
      </vuetable>

      <div class="row">
        
                <div class="col col-lg-12 col-xl-6 hide">
                  <div class="panel panel-noborder">
                    <div id="EventWaveformPanel">
                      <div id="EventWaveformMaximize">
                        <div id="EventChart" class="chart big_chart"></div>
                      </div>
                    </div>
                  </div>
                </div>
      </div>
    </div>
    <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialogRAW" ref="rawDialog" data-keyboard="false">
      <div class="modal-backdrop in"></div>
      <div class="modal-dialog" style="width: 100%; height: 100%; padding: 10px; margin: 0px">
        <div class="modal-content" style="height: 100%">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">Event &raquo; Waveform</h4></div>
            <div class="pull-right">
              <button type="button" class="btn btn-default btn-xs" @click="Close"><img :src="require('Imgs/arrow_undo.png')"></button>
            </div>
          </div>
          <div class="modal-body">
            <spinner v-if="ShowSpinner"></spinner>
            <graph ref="graphRAW" :options="GraphOptionsRAW"></graph>
          </div>
        </div>
      </div>
    </div>
    <div class="modal" tabindex="1" role="dialog" data-backdrop="false" v-if="ShowDialogRMS" ref="rmsDialog" data-keyboard="false">
      <div class="modal-backdrop in"></div>
      <div class="modal-dialog" style="width: 100%; height: 100%; padding: 10px; margin: 0px">
        <div class="modal-content" style="height: 100%">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">Event &raquo; RMS</h4></div>
            <div class="pull-right">
              <button type="button" class="btn btn-default btn-xs" @click="Close"><img :src="require('Imgs/arrow_undo.png')"></button>
            </div>
          </div>
          <div class="modal-body">
            <spinner v-if="ShowSpinner"></spinner>
            <graph ref="graphRMS" :options="GraphOptionsRMS"></graph>
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
  export default {
    data: function() {
      return { ShowSpinner: true, StartDate: '', EndDate: '', ShowDialogRAW: false, ShowDialogRMS: false,
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
      this.GraphOptionsRAW = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow,
//        zoomCallback: this.ZoomEvt.bind(this), interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: this.MouseUpEvt.bind(this), dblclick: this.DoubleClickEvt.bind(this) }, xAxisHeight: 24, 
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Result = oZm.Series['VOLTAGE'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
               axisLabelFormatter: function (x) { var Result = oZm.Series['VOLTAGE'].FormatValue(x); return Result.Value; }
             },
          x: { axisLabelFormatter: function(x) { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); },
               valueFormatter: function (x) { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); }
             }
        }
      };
      this.GraphOptionsRMS = {
        strokeWidth: 0.5, stepPlot: true, drawGapEdgePoints: true, connectSeparatedPoints: false, fillGraph: false, gridLineColor: "#BBB", gridLinePattern: [4, 4], legend: 'follow', legendFormatter: oZm.LegendFormatter_Follow,
//        zoomCallback: this.ZoomEvt.bind(this), interactionModel: { mousedown: oZm.mouseDown, mousemove: oZm.mouseMove, mouseup: this.MouseUpEvt.bind(this), dblclick: this.DoubleClickEvt.bind(this) }, xAxisHeight: 24, 
        axes: {
          y: { pixelsPerLabel: 15, axisLabelWidth: 34,
               valueFormatter: function (x) { var Result = oZm.Series['VOLTAGE'].FormatValue(x); return Result.Value + ' <em class="x-small">' + Result.Units + '</em>'; },
               axisLabelFormatter: function (x) { var Result = oZm.Series['VOLTAGE'].FormatValue(x); return Result.Value; }
             },
          x: { axisLabelFormatter: function(x) { return moment(x).format('DD/MM/YYYY [<br/>] HH:mm:ss'); },
               valueFormatter: function (x) { return moment(x).format('DD/MM/YYYY - HH:mm:ss'); }
             }
        }
      };
    },
    mounted: function() {
      this.$root.$el.append(this.$refs.rawDialog);
    },
    beforedestroy: function() {
      this.$root.$el.removeChild(this.$refs.rmsDialog);
      this.$root.$el.removeChild(this.$refs.rawDialog);
    },
    methods: {
      Redraw: function() {},
      GetEvents: function(to, period, lastsample) {
        var Analyzer = this.$root.SelectedAnalyzer;
        if(lastsample === 0) {
          this.ShowSpinner = true;
          this.Events = [];          
        }
        $.ajax({ url: 'getEvents', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json', data: JSON.stringify({ To: to, LastSample: lastsample, Analyzer: Analyzer.ID, Period: period }),
          success: function(result) {
            this.EndDate = moment(result.To).format('DD/MM/YYYY - HH:mm:ss');
            this.StartDate = moment(result.From).format('DD/MM/YYYY - HH:mm:ss');
            if((result['StartTime'] === null) || (result['StartTime'].length === 0)) return;
            for(var x = 0; x < result['StartTime'].length; x++) {
              var Event = { Date: moment(result.StartTime[x]).format('DD/MM/YYYY - HH:mm:ss'), Type: result.Type[x] };
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
      RMS: function(evt) {
        this.ShowDialogRMS = true;
        this.$nextTick(function() {
      this.$root.$el.append(this.$refs.rmsDialog);

    $(this.$refs.rmsDialog).modal('show');
                
  }); 
        
//    },
//    destroyed: function() {
//      this.$root.$el.removeChild(this.$el);
//    },
        
        
      },
      RAW: function(evt) {},
      Delete: function(evt) {},
      Edit: function(evt) {},
      Close: function() {}
    },
    components: { Spinner, Vuetable, Graph }
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
      for(var i = 0; i < result['StartTime'].length; i++) {
        var Time = new Date(result['StartTime'][i]);
        var RowText = '<tr>' +
          '<td data-search="' + result['StartTime'][i] + '">' + Time.toLocaleString() + '.' + Time.getMilliseconds() + '</td>' +
          '<td>' + result['Type'][i] + '</td>' +
          '<td>' + Round2(result['Duration'][i] / result['SampleRate'][i]) + '</td>' +
          '<td>' + Round2(result['Voltage'][i]) + '</td>' +
          '<td>' + Round2(result['Percentage'][i]) + '</td>' +
          '<td><div class="btn-group"><button class="btn btn-xs btn-primary"><i class="fa fa-line-chart"></i> Get samples</button><button class="btn btn-xs btn-danger delete_event"><i class="fa fa-trash"></i> Delete event</button></div></td>' +
          '</tr>';
        EventsTable.row.add($(RowText)).draw();
        LastEventStartTime = result['StartTime'][i];
        EventsDatasheet.push(result['StartTime'][i]);
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