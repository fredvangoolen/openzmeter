/* Configuracion de la Wifi */
var NetworksTable = null;
function AjaxGetNetworks() {
  $.ajax({ url: "getSysInfo",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Type: 'Connectivity' }),
           success: AjaxNetworks
    });
}
function AjaxNetworks(result) {
  if(NetworksTable === null) {
    NetworksTable = $('#NetworksTable').DataTable({ responsive: false, paging: false, dom: '<"top datatable-toolbar"i><"top float-right"f><"clear">t<"clear">', language: { info: '<button class="btn btn-sm btn-default"><i class="fa fa-refresh fa-fw"></i></button> _MAX_ networks found', infoEmpty: "No networks in range"},
                                                    columns: [ { width: '50px', orderable: false }, 
                                                               { orderable: false }, 
                                                               { width: '105px', orderable: false },
                                                               { width: '20px', orderable: false, className: 'dataTables_empty' },
                                                               { width: '120px', orderable: false },
                                                               { width: '20px', orderable: false },
                                                               { orderable: false, searchable: false },
                                                             ],
                                                    order: [5, 'desc']
    });
  }
  NetworksTable.clear().draw();
  NetworksDatasheet = [];
  for(i = 0; i < result['AccessPoints'].length; i++) {
    var Mode = [];
    if(result['AccessPoints'][i]['Flags'] == 0) Mode.push('Open');
    if((result['AccessPoints'][i]['Flags'] & 0x00000033) != 0) Mode.push('WEP');
    if((result['AccessPoints'][i]['Flags'] & 0x00000044) != 0) Mode.push('WPA');
    if((result['AccessPoints'][i]['Flags'] & 0x00000088) != 0) Mode.push('WPA2');
    if((result['AccessPoints'][i]['Flags'] & 0x00000100) != 0) Mode.push('PSK');
    if((result['AccessPoints'][i]['Flags'] & 0x00000200) != 0) Mode.push('802.1X');
    Mode = ((result['AccessPoints'][i]['Mode'] == 1) ? 'ADHOC' : 'AP') + ' (' + Mode.join([separador = ', ']) + ')';
    NetworksTable.row.add( ['', 
                            result['AccessPoints'][i]['SSID'], 
                            result['AccessPoints'][i]['MAC'], 
                            (result['AccessPoints'][i]['Freq'] - 2412) / 5 + 1,
                            Mode, 
                            '<div class="progress" style="margin-bottom:0px; position:relative">' +
                            '  <div class="progress-bar" role="progressbar" style="width:' + result['AccessPoints'][i]['Power'] + '%"></div>' +
                            '  <span style="position: absolute; left:0px; top: 2px; text-align: center; width: 100%">'+ result['AccessPoints'][i]['Power'] + '%</span>' + 
                            '</div>', 
                            '<button class="btn btn-xs btn-primary edit"><span class="glyphicon glyphicon-pencil"></span> Edit</button>'] ).draw();
//      LastEventStartTime = result['Starttime'][i];
//      EventsDatasheet.push(result['Starttime'][i]);
    }
//  if(EventChart === null) {
//    EventChart = new Dygraph(document.getElementById("EventChart"), (Nodes['Devices'][Node]['Type'] == '3Phase') ? [[0, 0, 0, 0]] : [[0, 0]], {
//      strokeWidth: 0.5,
//      gridLineColor: "#999",
//      gridLinePattern: [4, 4],
//      labels: (Nodes['Devices'][Node]['Type'] == '3Phase') ? ['Time', 'Voltage (R)', 'Voltage (S)', 'Voltage (T)'] :['Time', 'Voltage'],
//      colors: ["#d9534f", "#534fd9", "#4fd953"],
//      interactionModel: { mousedown: mouseDownOZM, mousemove: mouseMoveOZM, mouseup  : mouseUpOZM, dblclick : dblClickOZM_Restore },
//      labelsDiv: "EventChartLegend"
//    });
//    $('#EventChart').append('<div class="no-data event_instructions"><div class="alert alert-info">Press <strong><em>Get samples</em></strong> to view the waveform of an event.</div></div>');
//  }
}
$("table#NetworksTable").on('click', 'button.edit', function () {
$('#NetworkModal').modal('show');
});

/* Secuencia de inicio para cargar la pagina */
function UpdatePage() {
  AjaxGetNetworks();
}