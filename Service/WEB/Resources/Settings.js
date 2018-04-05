/* Configuracion de la Wifi */
var NetworksTable = null;
var NetworksDataset = [];
var EditingNetwork = null;
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
  if(Item['Status'] === '') Item['Status'] = status;
  if(attribs['IP']         !== undefined) Item['IP']         = attribs['IP'];
  if(attribs['Mask']       !== undefined) Item['Mask']       = attribs['Mask'];
  if(attribs['Gateway']    !== undefined) Item['Gateway']    = attribs['Gateway'];
  if(attribs['DNS0']       !== undefined) Item['DNS0']       = attribs['DNS0'];
  if(attribs['DNS1']       !== undefined) Item['DNS1']       = attribs['DNS1'];
  if(attribs['DHCP']       !== undefined) Item['DHCP']       = attribs['DHCP'];
  if(attribs['ConfigPath'] !== undefined) Item['ConfigPath'] = attribs['ConfigPath'];
  NetworksDataset[index] = Item;
}
function AjaxNetworks(result) {
  NetworksDataset = [];
  for(i = 0; i < result['AccessPoints'].length; i++) NetworksDataset.push( { MAC: result['AccessPoints'][i]['MAC'], Mode: result['AccessPoints'][i]['Mode'], SSID: result['AccessPoints'][i]['SSID'], Channel: (result['AccessPoints'][i]['Freq'] - 2412) / 5 + 1, Power: result['AccessPoints'][i]['Power'], Status: '' } );
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
      NetworksDataset.push( { MAC: (result['Settings'][i]['MAC'] === undefined) ? 'ANY' : result['Settings'][i]['MAC'], Mode: result['Settings'][i]['Mode'], SSID: result['Settings'][i]['SSID'], Channel: '', Power: 0, Status: '' } );
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
  var Buttons = '<div class="btn-group">' +
                '  <button class="btn btn-sm btn-default" onclick="AjaxGetNetworks()"><i class="fa fa-refresh"></i> Refresh</button>' +
                '  <button class="btn btn-sm btn-default" onclick="NetworkAdd()"><i class="fa fa-plus-square-o" aria-hidden="true"></i> Add</button>' +
                '</div>';
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
$('#NetworkDialogBody').on('focus', '*', function() {
  $('#NetworkErrorBox').hide(500);
});
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
    $('#NetworkDNS0').removeProp('disabled');  
    $('#NetworkDNS1').removeProp('disabled');  
  } else {
    $('#NetworkIP').prop('disabled', 'true');
    $('#NetworkGateway').prop('disabled', 'true');
    $('#NetworkDNS0').prop('disabled', 'true');  
    $('#NetworkDNS1').prop('disabled', 'true');   
  }
});
$('#NetworkConfigClose').click(function() {
  $('#NetworkModal').modal('hide');
});
$('#NetworkConfigSave').click(function() {
  $('#NetworkDialogBody :focus').blur();
  if($('#NetworkSSID').val().length === 0) {
    $('#NetworkError').text('SSID can not be empty.');
    $('#NetworkErrorBox').slideDown(500);
    return;
  }
  EditingNetwork['SSID'] = $('#NetworkSSID').val();
  if($('#NetworkBSSID').val().length !== 0) {
    var Regex = new RegExp("^([0-9A-F]{2}[:-]){5}([0-9A-F]{2})$");
    if(Regex.test($('#NetworkBSSID').val()) === false) {
      $('#NetworkError').text('BSSID must be a valid MAC address or left blank.');
      $('#NetworkErrorBox').slideDown(500);
      return;
    }
  }
  EditingNetwork['BSSID'] = $('#NetworkBSSID').val();
  if(($('#NetworkKeyType').val() === 'WEP') || ($('#NetworkKeyType').val() === 'WPA')) {
    if($('#NetworkPassword').val().length === 0) {
      $('#NetworkError').text('Password can not be empty.');
      $('#NetworkErrorBox').slideDown(500);
      return;
    }
  }
  EditingNetwork['Mode'] = $('#NetworkKeyType').val();
  if($('#NetworkDHCP').val() === 'Static') {
    var Regex = new RegExp("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    if(Regex.test($('#NetworkIP').val()) === false) {
      $('#NetworkError').text('IP address must be a valid IPv4 address.');
      $('#NetworkErrorBox').slideDown(500);
      return;
    }
    EditingNetwork['IP'] = ($('#NetworkIP').val());
    if(Regex.test($('#NetworkGateway').val()) === false) {
      $('#NetworkError').text('Gateway address must be a valid IPv4 address.');
      $('#NetworkErrorBox').slideDown(500);
      return;
    }
    EditingNetwork['Gateway'] = ($('#NetworkGateway').val());
    if($('#NetworkDNS0').val().length === 0) {
      $('#NetworkError').text('At least primary DNS is necesary.');
      $('#NetworkErrorBox').slideDown(500);
      return;
    }
    if(Regex.test($('#NetworkDNS0').val()) === false) {
      $('#NetworkError').text('Primary DNS address must be a valid IPv4 address.');
      $('#NetworkErrorBox').slideDown(500);
      return;
    }
    EditingNetwork['DNS0'] = ($('#NetworkDNS0').val());
    if($('#NetworkDNS1').val().length !== 0) {
      if(Regex.test($('#NetworkDNS1').val()) === false) {
        $('#NetworkError').text('Secondary DNS address must be a valid IPv4 address.');
        $('#NetworkErrorBox').slideDown(500);
        return;
      }
    }
    EditingNetwork['DNS1'] = ($('#NetworkDNS1').val());
  }
  EditingNetwork['DHCP'] = ($('#NetworkDHCP').val === 'DHCP');
  $.ajax({ url: "setSysInfo",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Type: 'Connectivity', Params: EditingNetwork }),
           success: function(result) {
             $('#NetworkModal').modal('hide');
             EditingNetwork = null;        
           }
  });
});
function NetworDialogShow() {
  if(EditingNetwork['SSID'] === undefined) {
    $('#NetworkSSID').removeProp('disabled');
    $('#NetworkSSID').val('');
  } else {
    $('#NetworkSSID').prop('disabled', 'true');
    $('#NetworkSSID').val(EditingNetwork['SSID']);
  }
  $('#NetworkBSSID').val((EditingNetwork['BSSID'] === undefined) ? '' : EditingNetwork['BSSID']);
  $('#NetworkKeyType').val((EditingNetwork['Mode'] === undefined) ? 'Open' : EditingNetwork['Mode']);
  $('#NetworkKeyType').change();
  $('#NetworkDHCP').val((EditingNetwork['DHCP'] === undefined) ? false : EditingNetwork['DHCP'] ? 'DHCP' : 'Static');    
  $('#NetworkDHCP').change();
  $('#NetworkIP').val((EditingNetwork['IP'] === undefined) ? '' : EditingNetwork['IP']);
  $('#NetworkGateway').val((EditingNetwork['Gateway'] === undefined) ? '' : EditingNetwork['Gateway']);
  $('#NetworkDNS0').val((EditingNetwork['DNS0'] === undefined) ? '' : EditingNetwork['DNS0']);
  $('#NetworkDNS1').val((EditingNetwork['DNS1'] === undefined) ? '' : EditingNetwork['DNS1']);
  $('#NetworkNetmask').text((EditingNetwork['Mask'] === undefined) ? '24' : EditingNetwork['Mask']);
  $('#NetworkModal').modal('show');  
}
function NetworkEdit(index) {
  EditingNetwork = NetworksDataset[index];
  NetworDialogShow();
}
function NetworkAdd() {
  EditingNetwork = [];
  EditingNetwork['DHCP'] = true;
  EditingNetwork['Mask'] = 24;
  NetworDialogShow();
}
/* Secuencia de inicio para cargar la pagina */
function UpdatePage() {
  AjaxGetNetworks();
}