/* Configuracion de la Wifi */
var NetworksTable = null;
var NetworksDataset = [];
var EditingPos = -1;
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
function setConnProperties(index, status, attribs) {
  var Item = NetworksDataset[index];
  if(Item['ConfigPath'] === undefined) Item['Status'] = status;
  if(attribs['IP']         !== undefined) Item['IP']         = attribs['IP'];
  if(attribs['Mask']       !== undefined) Item['Mask']       = attribs['Mask'];
  if(attribs['Gateway']    !== undefined) Item['Gateway']    = attribs['Gateway'];
  if(attribs['DNS1']       !== undefined) Item['DNS1']       = attribs['DNS1'];
  if(attribs['DNS2']       !== undefined) Item['DNS2']       = attribs['DNS2'];
  if(attribs['DHCP']       !== undefined) Item['DHCP']       = attribs['DHCP'];
  if(attribs['ConfigPath'] !== undefined) Item['ConfigPath'] = attribs['ConfigPath'];
  NetworksDataset[index] = Item;
}
function AjaxNetworks(result) {
  NetworksDataset = [];
  for(i = 0; i < result['AccessPoints'].length; i++) NetworksDataset.push( { MAC: result['AccessPoints'][i]['MAC'], Mode: WpaFlagsToString(result['AccessPoints'][i]['Flags']), SSID: result['AccessPoints'][i]['SSID'], Channel: (result['AccessPoints'][i]['Freq'] - 2412) / 5 + 1, Power: result['AccessPoints'][i]['Power'], Status: '' } );
  for(i = 0; i < result['ActiveConnections'].length; i++) {
    for(n = 0; n < NetworksDataset.length; n++) {
      if((NetworksDataset[n]['SSID'] === result['ActiveConnections'][i]['SSID']) && (NetworksDataset[n]['MAC'] === result['ActiveConnections'][i]['MAC'])) {
        setConnProperties(n, 'Connected', result['ActiveConnections'][i]);
      }
    }
  }
  for(i = 0; i < result['Settings'].length; i++) {
    var Found = 0;
    for(n = 0; n < NetworksDataset.length; n++) {
      if((NetworksDataset[n]['SSID'] === result['Settings'][i]['SSID']) && ((NetworksDataset[n]['MAC'] === result['Settings'][i]['MAC']) || (result['Settings'][i]['MAC'] === undefined))) {
        setConnProperties(n, 'Configured', result['Settings'][i]);
        Found++;
      }
    }
    if(Found === 0) {
      NetworksDataset.push( { MAC: result['Settings'][i]['MAC'], Mode: '?', SSID: result['Settings'][i]['SSID'], Channel: '', Power: 0, Status: '' } );
      setConnProperties(NetworksDataset.length - 1, 'Out of range', result['Settings'][i]);
    }
  }
  var Contents = $('#NetworksTable tbody');
  Contents.html('');
  for(n = 0; n < NetworksDataset.length; n++) {
    var Buttons = '';
    if(NetworksDataset[n].ConfigPath !== undefined) {
      Buttons += '<button class="btn btn-xs btn-default" onclick="NetworkEdit(' + n + ')" style="width:50%"><i class="fa fa-pencil" aria-hidden="true"></i> Edit</button>';
      Buttons += '<button class="btn btn-xs btn-default" onclick="NetworkForget(' + n + ')" style="width:50%"><i class="fa fa-times" aria-hidden="true"></i> Forget</button>';
    } else {
      Buttons += '<button class="btn btn-xs btn-default" onclick="NetworkConfigure(' + n + ')" style="width:100%"><i class="fa fa-plus" aria-hidden="true"></i> Configure</button>';
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
$('#NetworkKeyType').change(function() {
  if((this.value === "WEP") || (this.value === "WPA")) {
    $('#NetworkPassword').val('00000000');
    $('#NetworkPassword').removeProp('disabled');  
  } else {
    $('#NetworkPassword').val('');
    $('#NetworkPassword').prop('disabled', 'true');    
  }
});
$('#NetworkDHCP').change(function() {
  if(this.value === "Static") {
    $('#NetworkIP').removeProp('disabled');
    $('#NetworkGateway').removeProp('disabled');
    $('#NetworkDNS1').removeProp('disabled');  
    $('#NetworkDNS2').removeProp('disabled');  
  } else {
    $('#NetworkIP').prop('disabled', 'true');
    $('#NetworkGateway').prop('disabled', 'true');
    $('#NetworkDNS1').prop('disabled', 'true');  
    $('#NetworkDNS2').prop('disabled', 'true');   
  }
});
$('#NetworkConfigClose').click(function() {
  $('#NetworkModal').modal('hide');
});
$('#NetworkConfigSave').click(function() {
  $.ajax({ url: "setSysInfo",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Type: 'Connectivity' }),
           success: function(result) {
             
           }
  });
  $('#NetworkModal').modal('hide');
});
function NetworkEdit(index) {
  EditingPos = index;
  $('#NetworkSSID').val(NetworksDataset[index].SSID);
  $('#NetworkSSID').prop('disabled', 'true');  
  $('#NetworkBSSID').val(NetworksDataset[index].BSSID);
  $('#NetworkKeyType').val((NetworksDataset[index].KeyType === undefined) ? 'Open' : NetworksDataset[index].KeyType);
  $('#NetworkKeyType').change();
  $('#NetworkDHCP').val((NetworksDataset[index].DHCP === true) ? 'DHCP' : 'Static');
  $('#NetworkDHCP').change();
  $('#NetworkIP').val(NetworksDataset[index].IP);
  $('#NetworkGateway').val(NetworksDataset[index].Gateway);
  $('#NetworkDNS1').val(NetworksDataset[index].DNS1);
  $('#NetworkDNS2').val(NetworksDataset[index].DNS2);
  $('#NetworkNetmask').text('/' + NetworksDataset[index].Mask);
  $('#NetworkModal').modal('show');
}
/* Secuencia de inicio para cargar la pagina */
function UpdatePage() {
  AjaxGetNetworks();
}