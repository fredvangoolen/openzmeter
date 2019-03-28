
function PhasorResizeEvent() {
  var Width = $('#PhasorChart').parent().width();
  if(PhasorChart !== null) PhasorChart.resize(Width, Width);
}

/* Grafico de fasores */
var PhasorChart = null;
var PhasorTable = null;
function UpdatePhasorChart(result) {
  var Vectors = [];
  var MaxCurrent = 0;
  var MaxVoltage = 0;
  for(i = 0; i < result['Voltage'].length; i++) {
    Vectors.push([2 * i, 120 * i, result.Voltage[i]]);
    Vectors.push([2 * i + 1, 120 * i - result.Phi[i], result.Current[i]]);
    MaxVoltage = Math.max(MaxVoltage, result.Voltage[i]);
    MaxCurrent = Math.max(MaxCurrent, result.Current[i]);
  }
  for(i = 0; i < Vectors.length; i += 2) {
    Vectors[i][2] = Vectors[i][2] / MaxVoltage;
    Vectors[i + 1][2] = Vectors[i + 1][2] / MaxCurrent;
  }
  var Options = { 
    strokeWidth: 0.5, gridLineColor: "#BBB", gridLinePattern: [4, 4], highlightCircleSize: 0,  interactionModel: { }, legend: 'never', valueRange: [-115, 115], dateWindow: [-115, 115], plotter: polarLinePlotter,
    labels: ['Index', 'Phase', 'Magnitude'], colors: ['#d9534f', '#428bca'], xAxisHeight: 0.1, xLabelHeight: 0.1, plugins: [ new Dygraph.Plugins.PolarCrosshair( { } ) ],
    axes: {
      y: { drawAxis: true, drawGrid: false, axisLabelWidth: 0.0, axisLineColor: "#FFF",
           ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             var Width = document.getElementById("PhasorChart").clientWidth / 2 - 1;
             for(var i = 1; i <= 4; i++) {
               ticks.push({ v: i * 25, label: '<div style="font-size: 11px; background: rgba(255, 255, 255, 0.8); margin-left: ' + Width + 'px; text-align: left; width: 44px; line-height: 12px">' + Round2(i / 4 * MaxVoltage) + '<em>V</em></div>' });
             }
             return ticks;
           },            
         },
      x: { drawAxis: true, drawGrid: false, axisLabelWidth: 0, axisLineColor: "#FFF",
           ticker: function(min, max, pixels, opts, dygraph, vals) {
             var ticks = [];
             var Height = document.getElementById("PhasorChart").clientHeight / 2 + 2;
             for(var i = 1; i <= 4; i++) {
               ticks.push({ v: -i * 25, label: '<div style="font-size: 11px; background: rgba(255, 255, 255, 0.8); margin-left: -20px; margin-top: ' + -Height + 'px; text-align: center; width: 40px; line-height: 12px">' + Round2(i / 4 * MaxCurrent) + '<em>A</em></div>' });
             }
             return ticks;
           }
         }
      }
    };
  if(PhasorChart === null) {
    PhasorChart = new Dygraph(document.getElementById("PhasorChart"), Vectors, Options);
    $(window).bind('resize', PhasorResizeEvent);
    PhasorChart.updateOptions({ file: Vectors });
    PhasorChart.updateOptions(Options);
  }
  if(PhasorTable === null) {
    PhasorTable = $('#PhasorsTable').DataTable({ responsive: true, paging: false, info: false, searching: false,
                                                 columnDefs: [ { targets: [0, 1, 2, 3], searchable: false } ],                            
    });
  }
  PhasorTable.clear();
  for(i = 0; i < result.Voltage.length; i++) {
    var PhaseName = "L";
    if(result.Voltage.length > 1) {
      if(i === 0) PhaseName = "R";
      if(i === 1) PhaseName = "S";
      if(i === 2) PhaseName = "T";
    }
    PhasorTable.row.add( [PhaseName, "Voltage", Round2(i * 120) + 'º', Round2(result.Voltage[i]) + '<em> V</em>'] ).draw(false);
    PhasorTable.row.add( [PhaseName, "Current", Round2(i * 120 + -result.Phi[i]) + 'º', Round2(result.Current[i]) + '<em> A</em>'] ).draw();
  }
  PhasorResizeEvent();
}
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
function ExportDetail() {
  var Data = [];
  var Filename;
  } else if(DetailTabSelector.GetSaved() === 5) {
    var Header = ['Date/Time'].concat(SeriesNames.PowerFactor()).concat(SeriesNames.THDv()).concat(SeriesNames.PHI());
    Data.push(Header);
    for(var i = 0; i < PowerFactorData.length; i++) {
      var Row = [new Date(PowerFactorData[i][0]).toLocaleTimeString()];
      for(var k = 0; k < SeriesNames.PowerFactor().length; k++) Row.push(PowerFactorData[i][k + 1]);
      for(var k = 0; k < SeriesNames.THDv().length; k++) Row.push(THDData[i][k + 1]);
      for(var k = 0; k < SeriesNames.PHI().length; k++) Row.push(PHIData[i][k + 1]);
      Data.push(Row);
    }
    exportToCsv('PF_THDv_PHI.csv', Data);
  }
}

