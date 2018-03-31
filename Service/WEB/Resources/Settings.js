/* Configuracion de la Wifi */
var NetworksTable = null;
var NetworksDataset = [];
function WpaFlagsToString(flags) {
  var Mode = 'Unknown';
  if(flags === 0) Mode = 'Open';
  if((flags & 0x00000033) !== 0) Mode = 'WEP';
  if((flags & 0x00000044) !== 0) Mode = 'WPA';
  if((flags & 0x00000088) !== 0) Mode = 'WPA2';
  return Mode;
}
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
  NetworksDataset = [];
  for(i = 0; i < result['AccessPoints'].length; i++) NetworksDataset.push( { MAC: result['AccessPoints'][i]['MAC'], Mode: WpaFlagsToString(result['AccessPoints'][i]['Flags']), SSID: result['AccessPoints'][i]['SSID'], Channel: (result['AccessPoints'][i]['Freq'] - 2412) / 5 + 1, Power: result['AccessPoints'][i]['Power'], Status: '' } );
  for(i = 0; i < result['ActiveConnections'].length; i++) {
    for(n = 0; n < NetworksDataset.length; n++) {
      if((NetworksDataset[n]['SSID'] === result['ActiveConnections'][i]['SSID']) && (NetworksDataset[n]['MAC'] === result['ActiveConnections'][i]['MAC'])) {
        var Item = NetworksDataset[n];
        Item['Status'] = 'Connected';
        if(result['ActiveConnections'][i]['IP'] !== undefined) Item['IP'] = result['ActiveConnections'][i]['IP'];
        if(result['ActiveConnections'][i]['Mask'] !== undefined) Item['IP'] = result['ActiveConnections'][i]['Mask'];
        if(result['ActiveConnections'][i]['Gateway'] !== undefined) Item['IP'] = result['ActiveConnections'][i]['Gateway'];
        if(result['ActiveConnections'][i]['DNS1'] !== undefined) Item['IP'] = result['ActiveConnections'][i]['DNS1'];
        if(result['ActiveConnections'][i]['DNS2'] !== undefined) Item['IP'] = result['ActiveConnections'][i]['DNS2'];
        if(result['ActiveConnections'][i]['DHCP'] !== undefined) Item['IP'] = result['ActiveConnections'][i]['DHCP'];
        NetworksDataset[n] = Item;
      }
    }
  }
  for(i = 0; i < result['Settings'].length; i++) {
    var Found = 0;
    for(n = 0; n < NetworksDataset.length; n++) {
      if((NetworksDataset[n]['SSID'] === result['Settings'][i]['SSID']) && ((NetworksDataset[n]['MAC'] === result['Settings'][i]['MAC']) || (result['Settings'][i]['MAC'] === undefined))) {
        var Item = NetworksDataset[n];
        if(Item['Status'] === '') Item['Status'] = 'Configured';
        Item['ConfigPath'] = result['Settings'][i]['ConfigPath'];
        if(result['Settings'][i]['IP'] !== undefined) Item['IP'] = result['Settings'][i]['IP'];
        if(result['Settings'][i]['Mask'] !== undefined) Item['IP'] = result['Settings'][i]['Mask'];
        if(result['Settings'][i]['Gateway'] !== undefined) Item['IP'] = result['Settings'][i]['Gateway'];
        if(result['Settings'][i]['DNS1'] !== undefined) Item['IP'] = result['Settings'][i]['DNS1'];
        if(result['Settings'][i]['DNS2'] !== undefined) Item['IP'] = result['Settings'][i]['DNS2'];
        if(result['Settings'][i]['DHCP'] !== undefined) Item['IP'] = result['Settings'][i]['DHCP'];
        NetworksDataset[n] = Item;
        Found++;
      }
    }
    if(Found === 0) {
      var Item = [];
      Item['Status'] = 'Out of range';
      Item['ConfigPath'] = result['Settings'][i]['ConfigPath'];
      Item.MAC = (result['Settings'][i]['MAC'] !== undefined) ? result['Settings'][i]['MAC'] : '';
      Item.Mode = '?';
      //      NetworksDataset.push( { Mode: WpaFlagsToString(result['AccessPoints'][i]['Flags']), 
      Item.SSID = result['Settings'][i]['SSID'];
      Item.Channel = '';
      Item.Power = 0;
      if(result['Settings'][i]['IP'] !== undefined) Item['IP'] = result['Settings'][i]['IP'];
      if(result['Settings'][i]['Mask'] !== undefined) Item['IP'] = result['Settings'][i]['Mask'];
      if(result['Settings'][i]['Gateway'] !== undefined) Item['IP'] = result['Settings'][i]['Gateway'];
      if(result['Settings'][i]['DNS1'] !== undefined) Item['IP'] = result['Settings'][i]['DNS1'];
      if(result['Settings'][i]['DNS2'] !== undefined) Item['IP'] = result['Settings'][i]['DNS2'];
      if(result['Settings'][i]['DHCP'] !== undefined) Item['IP'] = result['Settings'][i]['DHCP'];
      NetworksDataset.push(Item);
      
    }
  }
  var Contents = $('#NetworksTable tbody');
  Contents.html('');
  for(n = 0; n < NetworksDataset.length; n++) {
    var Buttons = '';
    if(NetworksDataset[n].ConfigPath !== undefined) {
      Buttons += '<button class="btn btn-xs btn-default edit" style="width:50%"><i class="fa fa-pencil" aria-hidden="true"></i> Edit</button>';
      Buttons += '<button class="btn btn-xs btn-default edit" style="width:50%"><i class="fa fa-times" aria-hidden="true"></i> Forget</button>';
    } else {
      Buttons += '<button class="btn btn-xs btn-default edit" style="width:100%"><i class="fa fa-plus" aria-hidden="true"></i> Configure</button>';
    }
    Contents.append('<tr class="' + ((NetworksDataset[n].Status === 'Connected') ? 'info' : '') + '">' +
      '<td>' + NetworksDataset[n].Status + '</td>' +
      '<td>' + NetworksDataset[n].SSID + '</td>' +
      '<td>' + NetworksDataset[n].MAC + '</td>' +
      '<td>' + NetworksDataset[n].Channel + '</td>' +
      '<td>' + NetworksDataset[n].Mode + '</td>' +
      '<td data-sort="' + NetworksDataset[n].Power + '"><div class="progress" style="margin-bottom:0px">' +
      '  <div class="progress-bar" role="progressbar" style="width:' + NetworksDataset[n].Power + '%"></div>' +
      '</div></td>' +
      '<td><div class="btn-group" style="width: 100%">' + Buttons + '</div></td></tr>');
  }
  if(NetworksTable !== null) NetworksTable.destroy();    
  var Buttons = '<button class="btn btn-sm btn-default" onclick="AjaxGetNetworks()"><i class="fa fa-refresh"></i></button>';
  NetworksTable = $('#NetworksTable').DataTable({ responsive: false, paging: false, dom: '<"top datatable-toolbar"i><"top float-right"f><"clear">t<"clear">', language: { info: Buttons + ' _MAX_ networks found', infoEmpty: Buttons + 'No networks in range'},
                                                  columns: [ { width: '105px', orderable: false }, 
                                                             { orderable: false }, 
                                                             { width: '105px', orderable: false },
                                                             { width: '20px', orderable: false, className: 'dataTables_empty' },
                                                             { width: '20px', orderable: false },
                                                             { width: '40px', orderable: false },
                                                             { width: '200px', orderable: false, searchable: false }
                                                           ],
                                                  order: [5, 'desc']
  });
}
$("table#NetworksTable").on('click', 'button.edit', function () {
  $('#NetworkModal').modal('show');
});

$('button.NetworkRefresh').click(AjaxGetNetworks);

/* Secuencia de inicio para cargar la pagina */
function UpdatePage() {
  AjaxGetNetworks();
}