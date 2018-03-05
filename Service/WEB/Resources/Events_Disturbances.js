/*
 * Copyright (C) 2015 Pedro Sánchez Alguacil
 *               2017 Eduardo Viciana Gamez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
var Timeline = null;

/* Tabla de eventos */
var LastEventStartTime = 0;
var EventsTable        = null;
var EventChart         = null;
var EventSpinner       = new Spinner(SpinnerOpts);
var DisturbancesTimer  = null;
function AjaxEvents(result) {
  $('#btnShowEventYear').removeClass('active');
  $('#btnShowEventMonth').removeClass('active');
  $('#btnShowEventWeek').removeClass('active');
  $('#btnShowEventDay').removeClass('active');
  if((result['To'] - result['From']) === 86400000) {
    $('#btnShowEventDay').addClass('active');
  } else if((result['To'] - result['From']) === 604800000) {
    $('#btnShowEventWeek').addClass('active');
  } else if((result['To'] - result['From']) === 2592000000) {
    $('#btnShowEventMonth').addClass('active');
  } else if((result['To'] - result['From']) === 31536000000) {
    $('#btnShowEventYear').addClass('active');
  }
  var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
  if(GraphEnd === 0) {
    $('#btnDisturbancesNow').addClass('active');
  } else {
    $('#btnDisturbancesNow').removeClass('active');
  }
  if(EventsTable === null) {
    EventsTable = $('#EventsTable').DataTable({ responsive: false, paging: false, scrollCollapse: true, scrollY: '224px', dom: '<"top float-left"i><"top float-right"f><"clear">t<"clear">', language: { info: "_MAX_ events in selected period", infoEmpty: "No events in selection"},
                                                columnDefs: [ { targets: [5], orderable: false }, { targets: [0, 1, 2, 3, 4], searchable: true } ],
                                                order: [0, 'desc']
    });
  }
  if(LastEventStartTime === 0) {
    EventsTable.clear().draw();
    EventsDatasheet = [];
  }
  if(result['Starttime'] !== null) {
    for(i = 0; i < result['Starttime'].length; i++) {
      var Time = new Date(result['Starttime'][i]);
      EventsTable.row.add( [Time.toLocaleString() + '.' + Time.getMilliseconds(), 
                            result['Type'][i], 
                            Round2(result['Duration'][i] / Nodes['Devices'][Node]['SampleFreq']), 
                            Round2(result['Voltage'][i]), 
                            Round2(result['Percentage'][i]), 
                            '<button class="btn btn-xs btn-primary get_samples"><i class="fa fa-line-chart fa-fw"></i> Get samples</button>'] ).draw();
      LastEventStartTime = result['Starttime'][i];
      EventsDatasheet.push(result['Starttime'][i]);
    }
  }
  if(EventChart === null) {
    EventChart = new Dygraph(document.getElementById("EventChart"), (Nodes['Devices'][Node]['Type'] == '3Phase') ? [[0, 0, 0, 0]] : [[0, 0]], {
      strokeWidth: 0.5,
      gridLineColor: "#999",
      gridLinePattern: [4, 4],
      labels: (Nodes['Devices'][Node]['Type'] == '3Phase') ? ['Time', 'Voltage (R)', 'Voltage (S)', 'Voltage (T)'] :['Time', 'Voltage'],
      colors: ["#d9534f", "#534fd9", "#4fd953"],
      interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
      labelsDiv: "EventChartLegend"
    });
    $('#EventChart').append('<div class="no-data event_instructions"><div class="alert alert-info">Press <strong><em>Get samples</em></strong> to view the waveform of an event.</div></div>');
  }
  
  
  
  
  
  
  var Groups = new vis.DataSet([
    { id: 1, content: 'Dips'},
    { id: 2, content: 'Swells'},
    { id: 3, content: 'Interruptions'},
    { id: 4, content: 'RVC'},
    { id: 5, content: 'Glitchs'}
  ]);
  var Items = new vis.DataSet();
  for(i = 0; i < result['Starttime'].length; i++) {
    switch(result['Type'][i]) {
      case 'INTERRUPTION':
        var className = 'red';
        var group = 3;
        break;
      case 'SWELL':
        var className = 'blue';
        var group = 2;
        break;
      case 'DIP':
        var className = 'blue';
        var group = 1;
        break;
      case 'RVC':
        var className = 'blue';
        var group = 4;
        break;
      }
      var date = new Date(result['Starttime'][i]);
      var percent = result['Percentage'][i];
      var duration = result['Duration'][i] / Nodes['Devices'][Node]['SampleFreq'];
      var diff = 0;
      Items.add({
          id: i,
          group: group,
          start: date,
          className: className,
          content: '<strong>Time:</strong> ' + date.toLocaleString() + '.' + date.getMilliseconds() + '<br><strong>Duration:</strong> ' + duration + '<br><strong>ΔU<sub>max</sub>:</strong> ' + diff + ' V<br><strong>% ΔU<sub>max</sub>:</strong>' + percent + '%'
      });
  }



Timeline = new vis.Timeline(document.getElementById('disturbances_chart'), null, {
  stack: false,
  start: result['From'],
  end: result['To'],
  editable: false,
  margin: {
    item: 10, // minimal margin between items
    axis: 5   // minimal margin between items and the axis
  },
  orientation: 'top'
});
  Timeline.setGroups(Groups);
  Timeline.setItems(Items);
  Timeline.on('rangechanged', function () {
    var range = Timeline.getWindow();
    generateEventTable(range.start.getTime(), range.end.getTime());
  });
  
  

  
  
}
$(document).on("mouseover", '.vis-item', function ($e) {
  $(this).qtip({
    overwrite: false,
      content: {
        text: $('.vis-item-content', this).html()
      },
      position: {
        my: 'top left', // Position my top left...
        at: 'bottom right', // at the bottom right of...
      },
      style: {
        classes: 'qtip-bootstrap'
      }
  });
});

$("table#EventsTable").on('click', 'button.get_samples', function () {
  var Row = EventsTable.row($(this).parents('tr'));
  $('#EventChart div.no-data').addClass('hidden');
  EventsTable.rows().nodes().to$().removeClass('info');
  $(Row.node()).addClass('info');
  ShowSpinner(EventSpinner, 'EventSpinner');
  $.ajax({
    url: "getEventSamples",
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data: JSON.stringify({ Node: Node, Starttime: EventsDatasheet[Row.index()] }),
    success: AjaxSamples
  });
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
      for(n = 0; n < 100; n++) ToShow.push([i + n + 1].concat(Nodes['Devices'][Node]['Type'] == '3Phase') ? ['NaN', 'NaN', 'NaN'] : ['NaN']);
    }
  }
  var Options = {
    panEdgeFraction: 0.000000001,
    file: ToShow,
    dateWindow: [0, result['Samples'].length + ((result['Duration'] > result['Envelope'] * 4) ? 100 : 0)],
    valueRange: CalcRanges(ToShow, (Nodes['Devices'][Node]['Type'] == '3Phase') ? [1, 2, 3] : [1]),
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
          if((result['Duration'] > result['Envelope'] * 4) && (y <= result['Envelope'] * 2)) {
            return Round2((y - result['Envelope']) / Nodes['Devices'][Node]['SampleFreq'] * 1000) + 'ms';
          } else if((result['Duration'] > result['Envelope'] * 4) && (y > result['Envelope'] * 2 + 100)) {
            return Round2((y - result['Envelope'] * 3  - 100) / Nodes['Devices'][Node]['SampleFreq'] * 1000) + 'ms';
          } else if(result['Duration'] > result['Envelope'] * 4) {
            return "";
          } else {
            return Round2(y / Nodes['Devices'][Node]['SampleFreq'] * 1000) + 'ms';
          }
        },
        axisLabelFormatter: function (y) {
          if((result['Duration'] > result['Envelope'] * 4) && (y <= result['Envelope'] * 2)) {
            return Round2((y - result['Envelope']) / Nodes['Devices'][Node]['SampleFreq'] * 1000) + 'ms';
          } else if((result['Duration'] > result['Envelope'] * 4) && (y > result['Envelope'] * 2 + 100)) {
            return Round2((y - result['Envelope'] * 3  - 100) / Nodes['Devices'][Node]['SampleFreq'] * 1000) + 'ms';
          } else if(result['Duration'] > result['Envelope'] * 4) {
            return "";
          } else {
            return Round2(y / Nodes['Devices'][Node]['SampleFreq'] * 1000) + 'ms';
          }
        }
      }
    }
  };
  EventChart.updateOptions(Options);
  EventLoaded = true;
}
function AjaxGetEvents() {
  if(DisturbancesTimer !== null) clearInterval(DisturbancesTimer);
  $('#btnDisturbancesNow').addClass('disabled');
  if(LastEventStartTime === 0) ShowSpinner(EventSpinner, 'EventSpinner');
  var Period = parseInt(localStorage.getItem('GraphPeriod'));
  if(!Period) {
    Period = 86400000;
    localStorage.setItem('GraphPeriod', Period);
  }
  var GraphEnd = parseInt(localStorage.getItem('GraphEnd'));
  if(!GraphEnd) {
    GraphEnd = 0;
    localStorage.setItem('GraphEnd', 0);
  } 
  $.ajax({ url: "getEvents",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: Period, To: GraphEnd, LastEvent: LastEventStartTime, Node: Node}),
           success: AjaxEvents,
           complete: function() { 
             $('#btnDisturbancesNow').removeClass('disabled');
             if(parseInt(localStorage.getItem('GraphEnd')) === 0) DisturbancesTimer = setTimeout(AjaxGetEvents, 3000);
           }
  }); 
}
function ButtonClick(event) {
  var End = new Date();
  End = new Date(End.getFullYear(), End.getMonth(), End.getDate(), 23, 59, 59, 999).getTime();
  var Start;
  if(this.id == 'btnShowEventYear') {
    localStorage.setItem('GraphPeriod', 31536000000);
  } else if(this.id == 'btnShowEventMonth') {
    localStorage.setItem('GraphPeriod', 2592000000);
  } else if(this.id == 'btnShowEventWeek') {
    localStorage.setItem('GraphPeriod', 604800000);
  } else if(this.id == 'btnShowEventDay') {
    localStorage.setItem('GraphPeriod', 86400000);
  }
  LastEventStarttime = 0;
  AjaxGetEvents();
}
$('#btnShowEventYear').click(ButtonClick);
$('#btnShowEventMonth').click(ButtonClick);
$('#btnShowEventWeek').click(ButtonClick);
$('#btnShowEventDay').click(ButtonClick);
$('#btnDisturbancesNow').click(function() {
  var End = parseInt(localStorage.getItem('GraphEnd'));
  if(End === 0) {
    $('#btnDisturbancesNow').removeClass('active');
    localStorage.setItem('GraphEnd', Timeline.getWindow().end.getTime());
    if(DisturbancesTimer !== null) clearInterval(DisturbancesTimer);
  } else {
    $('#btnDisturbancesNow').addClass('active');
    localStorage.setItem('GraphEnd', 0);
    LastEventStarttime = 0;
    AjaxGetEvents();
  }
});

function UpdatePage() {
  AjaxGetEvents();
}