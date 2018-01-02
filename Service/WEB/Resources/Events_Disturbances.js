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
function AjaxGetEvents() {
  $.ajax({ url: "getEvents",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Period: 31536000000, To: 0, LastEvent: 0, Node: Node}),
           success: AjaxEvents,
           complete: function (x) {
//             $('#btnCurrentNow').removeClass('disabled');
//             if(parseInt(localStorage.getItem('GraphEnd')) === 0) CurrentTimer = setTimeout(AjaxGetCurrent, 3000);
           }
  }); 
  
  
}
function AjaxEvents(result) {
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
  //    var diff = roundTo((Math.abs(events[i].voltage - 230)), 2).toFixed(2);
  //    var percent = roundTo(100 - events[i].percent, 2).toFixed(2);
      Items.add({
          id: i,
          group: group,
          start: date,
          className: className,
          content: '<strong>Time:</strong> ' + date.toLocaleString() + '.' + date.getMilliseconds() + '\
                          <br><strong>Duration:</strong> ' + duration + '\
                          <br><strong>ΔU<sub>max</sub>:</strong> ' + diff + ' V\
                          <br><strong>% ΔU<sub>max</sub>:</strong>' + percent + '%'

      });
  }


var now = new Date();

// specify options
var date = new Date();
date.setHours(0);
date.setMinutes(0);
date.setSeconds(0);
date.setMilliseconds(0);
date.setDate(-100);
  Timeline = new vis.Timeline(document.getElementById('disturbances_chart'), null, {
    stack: false,
    start: date,
    end: now,
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
function UpdatePage() {
  AjaxGetEvents();
}
function fillEventsTable(events_list) {
    var table = $('table#events_table tbody');
    table.html('');
    var newEvents = [];
    for (var i in events_list) {
        //for (var i in events_list[type]) {
        newEvents.push(events_list[i]);
        var startTime = new Date(+events_list[i].timestamp);
        var duration = formatTimeUnit(events_list[i].duration * 1e6);
        var diff = roundTo((Math.abs(events_list[i].voltage - 230)), 2).toFixed(2);
        var percent = roundTo(100 - events_list[i].percent, 2).toFixed(2);
        table.append('<tr id="' + (+events_list[i].timestamp) + '">\
        <td>' + startTime.toLocaleString() + '.' + startTime.getMilliseconds() + '</td>\
        <td>' + events_list[i].type + '</td>\
        <td>' + duration + '</td>\
        <td>' + diff + ' V' + '</td>\
        <td>' + percent + '%' + '</td>\
        <td><button class="btn btn-xs btn-primary get_samples"><i class="fa fa-line-chart fa-fw"></i> Get samples</button></td>\
        </tr>')
        //}
    }
    events = newEvents;
}

function generateEventTable(startDate, endDate) {
  return;
    var request = $.ajax({
        url: "analysis/ajaxGetEvents",
        timeout: 20000,
        type: "POST",
        dataType: "json",
        data: {
            startDate: startDate,
            endDate: endDate,
            getRVC: true
        }
    });
    request.done(function (result) {
        table_events.clear();
        table_events.destroy();
        fillEventsTable(result);
        table_events = $('table#events_table').DataTable(
                {
                    responsive: true,
                    "columnDefs": [
                        {width: '2%', "targets": -1}
                    ]
                });
        if ($('#event_chart div.no-data').length == 0) {
            $('#event_chart').append('<div class="no-data event_instructions"><div class="alert alert-info">Press <strong><em>Get samples</em></strong> to view the waveform of an event.</div></div>');
        }
    });
}


//
//$('.vis-item').qtip({// Grab some elements to apply the tooltip to
//    content: {
//        text: $('.vis-item-content', this).html()
//    }
//})

$(':input[name="disturbances_chart_range"]').change(function () {
    var date = new Date();
    switch ($(this).val()) {
        case 'today':
            var end = date;
            end.setHours(23);
            end.setMinutes(59);
            end.setSeconds(59);
            var start = new Date(end.getFullYear(), end.getMonth(), end.getDate());
            break;
        case 'yesterday':
            var end = date;
            end.setHours(00);
            end.setMinutes(00);
            end.setSeconds(00);
            start = new Date(end.getFullYear(), end.getMonth(), end.getDate() - 1);
            break;
        case 'week':
            var day = date.getDay();
            start = new Date(date.getFullYear(), date.getMonth(), date.getDate() - day + 1);
            var end = date;
            end.setHours(23);
            end.setMinutes(59);
            end.setSeconds(59);
            break;
        case 'month':
            start = new Date(date.getFullYear(), date.getMonth(), 1);
            end = new Date(date.getFullYear(), date.getMonth() + 1, 1);
            break;
    }
    timeline.setWindow(start, end);
    generateEventTable(start.getTime(), end.getTime());
});


/**
 * Events
 */
var spinner = new Spinner({
    lines: 10 // The number of lines to draw
    , length: 10 // The length of each line
    , width: 5 // The line thickness
    , radius: 10 // The radius of the inner circle
    , scale: 1 // Scales overall size of the spinner
    , corners: 1 // Corner roundness (0..1)
    , color: '#000' // #rgb or #rrggbb or array of colors
    , opacity: 0.25 // Opacity of the lines
    , rotate: 0 // The rotation offset
    , direction: 1 // 1: clockwise, -1: counterclockwise
    , speed: 1 // Rounds per second
    , trail: 60 // Afterglow percentage
    , fps: 20 // Frames per second when using setTimeout() as a fallback for CSS
    , zIndex: 2e9 // The z-index (defaults to 2000000000)
    , className: 'spinner' // The CSS class to assign to the spinner
    , top: '50%' // Top position relative to parent
    , left: '50%' // Left position relative to parent
    , shadow: false // Whether to render a shadow
    , hwaccel: false // Whether to use hardware acceleration
    , position: 'absolute' // Element positioning
});

$("table#events_table").on('click', 'button.get_samples', function () {
    var tr = table_events.row($(this).parents('tr'));
    $('#event_chart div.no-data').fadeOut(200, function () {
        $(this).remove();
    });
    table_events.rows().nodes().to$().removeClass('info');
    $(tr.node()).addClass('info');
    var index = tr.index();
    spinner.spin(eventChart.graphDiv);
    var request = $.ajax({
        url: "events/ajaxGetEventSamples",
        type: "POST",
        dataType: "json",
        data: {
            timestamp: +events[index].timestamp
        }
    });
    request.done(function (result) {
        spinner.stop();
        var eventDataset = [];
        var len = result.length;
        for (var i = 0; i < len / 2; i++) {
            eventDataset.push([i/*new Date(+result[i].timestamp)*/, result[i]/*.value*/]);
        }
        for (var j = 0; j < 100; j++) {
            eventDataset.push([i++, null]);
        }
        for (i = i; i < len; i++) {
            eventDataset.push([i/*new Date(+result[i].timestamp)*/, result[i - 100]/*.value*/]);
        }
        eventChart.updateOptions({'file': eventDataset, dateWindow: [0, result.length], underlayCallback: function (canvas, area, g) {
                var bottom_left = g.toDomCoords(len / 2, -20);
                var top_right = g.toDomCoords(len / 2 + 100, +20);

                var left = bottom_left[0];
                var right = top_right[0];

                canvas.fillStyle = "rgba(240, 240, 240, 1.0)";
                canvas.fillRect(left, area.y, right - left, area.h);
            }});
    });
});

var eventChart = new Dygraph(document.getElementById("event_chart"),
        [[0, 0]],
        {
            strokeWidth: 2,
            labels: ["Sample index", "Voltage"],
            colors: ["#d9534f"],
            ylabel: 'V',
            xlabel: 'Sample index',
            animatedZooms: true,
            interactionModel: {
                'mousedown': mouseDownOZM,
                'mousemove': mouseMoveOZM,
                'mouseup': mouseUpOZM,
                'click': clickV3,
                'dblclick': dblClickV4,
                'mousewheel': scrollV3
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
                        return y;
                    },
                    axisLabelFormatter: function (y) {
                        return y;
                    }
                }
            }
        }
);
$('#event_chart').append('<div class="no-data event_instructions"><div class="alert alert-info">Press <strong><em>Get samples</em></strong> to view the waveform of an event.</div></div>');
//fillEventsTable(events);
//var table_events = $('table#events_table').DataTable(
//        {
//            responsive: true,
//            "columnDefs": [
//                {width: '2%', "targets": -1}
//            ],
//            "order": [0, 'desc']
//        }
//);

//if (timestamp != null) {
//    $('#events_table tr#' + timestamp + ' button.get_samples').click();
//}