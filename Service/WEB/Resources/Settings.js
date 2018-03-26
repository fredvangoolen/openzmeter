/* Configuracion de la Wifi */
var NetworksTable   = null;
var NetworksDataset = [];
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
function AddNetworkToList(item) {
  if(NetworksDataset[item['MAC']] !== undefined) return;
  var Mode = [];
  if(item['Flags'] == 0) Mode.push('Open');
  if((item['Flags'] & 0x00000033) != 0) Mode.push('WEP');
  if((item['Flags'] & 0x00000044) != 0) Mode.push('WPA');
  if((item['Flags'] & 0x00000088) != 0) Mode.push('WPA2');
  if((item['Flags'] & 0x00000100) != 0) Mode.push('PSK');
  if((item['Flags'] & 0x00000200) != 0) Mode.push('802.1X');
  NetworksDataset[item['MAC']] = { 
    Mode: ((item['Mode'] == 1) ? 'ADHOC' : 'AP') + ' (' + Mode.join([separador = ', ']) + ')',
    SSID: item['SSID'],
    Channel: (item['Freq'] - 2412) / 5 + 1,
    Power: item['Power'],
    Status: ''
  };
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
                                                               { width: '200px', orderable: false, searchable: false }
                                                             ],
                                                    order: [5, 'desc']
    });
  }
  NetworksTable.clear().draw();
  NetworksDataset = [];
  for(i = 0; i < result['AccessPoints'].length; i++) {
    AddNetworkToList(result['AccessPoints'][i]);
  }
  for(i = 0; i < result['ActiveConnections'].length; i++) {
    AddNetworkToList(result['ActiveConnections'][i]);
    var Item = NetworksDataset[result['ActiveConnections'][i]['MAC']];
    Item['Status'] = 'Connected';
    NetworksDataset[result['ActiveConnections'][i]['MAC']] = Item;   
  }
  for(var MAC in NetworksDataset) {
    var Buttons = '';
    if(NetworksDataset[MAC].Status === 'Connected') {
      Buttons += '<button class="btn btn-xs btn-default edit" style="width:50%"><i class="fa fa-pencil" aria-hidden="true"></i> Edit</button>';
      Buttons += '<button class="btn btn-xs btn-default edit" style="width:50%"><i class="fa fa-times" aria-hidden="true"></i> Forget</button>';
    } else {
      Buttons += '<button class="btn btn-xs btn-default edit" style="width:100%"><i class="fa fa-plus" aria-hidden="true"></i> Configure</button>';
    }
    NetworksTable.row.add( [NetworksDataset[MAC].Status, 
                            NetworksDataset[MAC].SSID, 
                            MAC, 
                            NetworksDataset[MAC].Channel,
                            NetworksDataset[MAC].Mode, 
                            '<div class="progress" style="margin-bottom:0px; position:relative">' +
                            '  <div class="progress-bar" role="progressbar" style="width:' + NetworksDataset[MAC].Power + '%"></div>' +
                            '  <span style="position: absolute; left:0px; top: 2px; text-align: center; width: 100%">'+ NetworksDataset[MAC].Power + '%</span>' + 
                            '</div>', '<div class="btn-group" style="width: 100%">' + Buttons + '</div>'] ).draw();
    if(NetworksDataset[MAC].Status === 'Connected') $(NetworksTable.row(NetworksTable.data().length - 1).node()).addClass('info');
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