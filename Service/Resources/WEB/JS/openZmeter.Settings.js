/* Configuracion de la Wifi */
var NetworksTable   = null;
var NetworksDataset = [];
var EditingNetwork  = null;
var WifiSpinner     = new Spinner(SpinnerOpts);
function AjaxGetNetworks() {
  $.ajax({ url: "getNetworks", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({}), 
           success: AjaxNetworks,
           complete: function() {
             $('#WifiSpinner').hide();
           }
  });
}
function AjaxNetworks(result) {
  NetworksDataset = [];
  for(i = 0; i < result['AccessPoints'].length; i++) NetworksDataset.push( { MAC: result['AccessPoints'][i]['MAC'], Mode: result['AccessPoints'][i]['Mode'], SSID: result['AccessPoints'][i]['SSID'], Channel: (result['AccessPoints'][i]['Freq'] - 2412) / 5 + 1, Power: result['AccessPoints'][i]['Power'], Status: '' } );
  for(i = 0; i < result['Settings'].length; i++) {
    var Found = 0;
    for(n = 0; n < NetworksDataset.length; n++) {
      if((NetworksDataset[n]['SSID'] === result['Settings'][i]['SSID']) && ((NetworksDataset[n]['MAC'] === result['Settings'][i]['MAC']) || (result['Settings'][i]['MAC'] === undefined))) {
        NetworksDataset[n] = Object.assign(NetworksDataset[n], { Status: 'Configured' }, result['Settings'][i]);
        Found++;
      }
    }
    if(Found === 0) NetworksDataset.push( Object.assign({ MAC: (result['Settings'][i]['MAC'] === undefined) ? 'ANY' : result['Settings'][i]['MAC'], Mode: result['Settings'][i]['Mode'], SSID: result['Settings'][i]['SSID'], Channel: '', Power: 0, Status: 'Out of range' }, result['Settings'][i]));
  }
  for(i = 0; i < result['ActiveConnections'].length; i++) {
    for(n = 0; n < NetworksDataset.length; n++) {
      if((NetworksDataset[n]['SSID'] === result['ActiveConnections'][i]['SSID']) && (NetworksDataset[n]['MAC'] === result['ActiveConnections'][i]['MAC'])) NetworksDataset[n] = Object.assign(NetworksDataset[n], { Status: 'Connected' }, result['ActiveConnections'][i]);
    }
  }
  if(NetworksTable !== null) NetworksTable.destroy(); 
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
    $('#NetworkPassword').prop('placeholder', (EditingNetwork.ConfigPath === undefined) ? 'Password requieres' : 'Blank to keep current');
    $('#NetworkPassword').val('');
    $('#NetworkPassword').removeProp('disabled');  
  } else {
    $('#NetworkPassword').prop('placeholder', '');
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
  var SaveParams = {};
  if(EditingNetwork['ConfigPath'] !== undefined) SaveParams['ConfigPath'] = EditingNetwork['ConfigPath'];
  if(EditingNetwork['UUID'] !== undefined) SaveParams['UUID'] = EditingNetwork['UUID'];
  $('#NetworkDialogBody :focus').blur();
  if($('#NetworkSSID').val().length === 0) {
    $('#NetworkError').text('SSID can not be empty.');
    $('#NetworkErrorBox').slideDown(500);
    return;
  }
  SaveParams['SSID'] = $('#NetworkSSID').val();
  if($('#NetworkBSSID').val().length !== 0) {
    var Regex = new RegExp("^([0-9A-F]{2}[:-]){5}([0-9A-F]{2})$");
    if(Regex.test($('#NetworkBSSID').val()) === false) {
      $('#NetworkError').text('BSSID must be a valid MAC address or left blank.');
      $('#NetworkErrorBox').slideDown(500);
      return;
    }
  }
  if($('#NetworkBSSID').val() !== '') SaveParams['MAC'] = $('#NetworkBSSID').val();
  if(($('#NetworkKeyType').val() === 'WEP') || ($('#NetworkKeyType').val() === 'WPA')) {
    if(($('#NetworkPassword').val().length === 0) && (EditingNetwork.ConfigPath === undefined)) {
      $('#NetworkError').text('Password can not be empty.');
      $('#NetworkErrorBox').slideDown(500);
      return;
    }
    if($('#NetworkPassword').val().length !== 0) SaveParams['Password'] = $('#NetworkPassword').val();
  }
  SaveParams['Mode'] = $('#NetworkKeyType').val();
  if($('#NetworkDHCP').val() === 'Static') {
    var Regex = new RegExp("^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    if(Regex.test($('#NetworkIP').val()) === false) {
      $('#NetworkError').text('IP address must be a valid IPv4 address.');
      $('#NetworkErrorBox').slideDown(500);
      return;
    }
    SaveParams['IP'] = ($('#NetworkIP').val());
    if(Regex.test($('#NetworkGateway').val()) === false) {
      $('#NetworkError').text('Gateway address must be a valid IPv4 address.');
      $('#NetworkErrorBox').slideDown(500);
      return;
    }
    SaveParams['Gateway'] = ($('#NetworkGateway').val());
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
    SaveParams['DNS0'] = ($('#NetworkDNS0').val());
    if($('#NetworkDNS1').val().length !== 0) {
      if(Regex.test($('#NetworkDNS1').val()) === false) {
        $('#NetworkError').text('Secondary DNS address must be a valid IPv4 address.');
        $('#NetworkErrorBox').slideDown(500);
        return;
      }
    }
    SaveParams['DNS1'] = ($('#NetworkDNS1').val());
  }
  SaveParams['DHCP'] = ($('#NetworkDHCP').val() === 'DHCP');
  $.ajax({ url: "setSysInfo",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Type: 'Connectivity', Params: SaveParams }),
           success: function(result) {
             if(result['result'] === false) {
               $('#NetworkError').text('Error saving settings, check password format.');
               $('#NetworkErrorBox').slideDown(500);
             } else {
               $('#NetworkModal').modal('hide');
               EditingNetwork = null;
               AjaxGetNetworks();
             }
           }
  });
});
function NetworDialogShow() {
  $('#NetworkErrorBox').hide();
  if(EditingNetwork['SSID'] === undefined) {
    $('#NetworkSSID').removeProp('disabled');
    $('#NetworkSSID').val('');
  } else {
    $('#NetworkSSID').prop('disabled', 'true');
    $('#NetworkSSID').val(EditingNetwork['SSID']);
  }
  $('#NetworkBSSID').val((EditingNetwork['MAC'] === undefined) ? '' : EditingNetwork['MAC']);
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
function NetworkConfigure(index) {
  EditingNetwork = NetworksDataset[index];
  EditingNetwork['DHCP'] = true;
  EditingNetwork['Mask'] = 24;
  NetworDialogShow();
}
function NetworkAdd() {
  EditingNetwork = [];
  EditingNetwork['DHCP'] = true;
  EditingNetwork['Mask'] = 24;
  NetworDialogShow();
}
function NetworkForget(index) {
  $.ajax({ url: "setSysInfo",
           timeout: 20000,
           type: "POST",
           dataType: "json",
           contentType: 'application/json',
           data: JSON.stringify({ Type: 'Connectivity', Params: { ConfigPath: NetworksDataset[index]['ConfigPath']} }),
           success: function(result) {
             if(result['result'] === false) {
               alert('Unable to delete setting!');
             } else {
               AjaxGetNetworks();
             }
           }
  });
}
// Tariffs -------------------------------------------------------------------------------------------------------------
function PopulatePeriodDropdown(dropdown, count, prefix, selection, nullText) {
  var List = dropdown.parent().children('ul');
  List.html('');
  if(selection < 1) selection = 1;
  if(selection > count) selection = count;
  for(var i = 1; i <= count; i++) List.append('<li class="' + ((selection === i) ? 'active' : '') + '"><a href="' + i + '">' + prefix + i + '</a></li>');
  if(nullText !== undefined) {
    List.append('<li role="separator" class="divider"></li>');
    List.append('<li class="' + ((selection === null) ? 'active' : '') + '"><a href="null">' + nullText + '</a></li>');
    dropdown.children('span:first-child').html(nullText);
  } else {
    dropdown.children('span:first-child').html(prefix + selection);
  }
  return selection;
}
function DropdownValue(dropdown) {
  var Val = dropdown.parent().find('ul li.active a').attr('href');
  return (Val === "null") ? null : parseInt(Val);
}
function DropDownUpdateValue(event) {
  event.preventDefault();
  $(this).parent().parent().find('li').removeClass('active');
  $(this).parent().addClass('active');
  $(this).parent().parent().parent().find('button > span:first-child').html($(this).text())
}
var TariffsTable = null;
var Tariffs = [];
function AjaxGetTariffs() {
  $.ajax({ url: "getTariffs", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), success: 
    function(result) {
      Tariffs = result;
      if(TariffsTable === null) {
        TariffsTable = $('#TariffsTable').DataTable({ responsive: false, paging: false, scrollCollapse: true, scrollY: '224px', dom: '<"clear">t<"clear">', columnDefs: [ { targets: [0, 1, 2, 3], orderable: true }, { targets: [0, 2, 3], className: 'text-center' }, { targets: [4], orderable: false, searchable: false } ], order: [0, 'desc']});
      } else {
        TariffsTable.clear();    
      }        
      for(var Key in Tariffs) {
        var RowText = '<tr>' +
          '<td>' + Tariffs[Key].Country + '</td>' +
          '<td>' + Tariffs[Key].Name + '</td>' +
          '<td>' + Tariffs[Key].StartDate + '</td>' +
          '<td>' + Tariffs[Key].EndDate + '</td>' +
          '<td data-tariff="' + Key + '"><div class="btn-group"><button class="btn btn-xs btn-default">Edit</button><button class="btn btn-xs btn-danger">Delete</button></div></td>' +
          '</tr>';
        TariffsTable.row.add($(RowText)).draw();
      }
    }
  });
}
var TariffKey;
$('#btnAddTariff').click(function() {
  TariffKey = null;
  LoadTariffDialog({ Country: null, StartDate: new Date(new Date().getFullYear(), 0, 1, 0, 0, 0, 0).getTime(), EndDate: new Date(new Date().getFullYear(), 11, 31, 23, 59, 59, 999).getTime(), Name: 'No nane', Description: 'Add tariff description here', DayPeriods: 1, Seasons: [], Periods: [], Holidays: [] });
});
$('#TariffsTable').on('click', 'button:nth-child(1)', function() {
  TariffKey = $(this).parents('td').attr('data-tariff');
  LoadTariffDialog(Tariffs[TariffKey]);
});
///TODO: Falta borrar tarifa
///TODO: Falta guardar tarifa
function LoadTariffDialog(definition) {
  TariffDefinition = definition;
  $('#dlgTariff > div > div').addClass('hide');
  $('#divTariffMain').removeClass('hide');
  $('#dlgTariff').modal('show');
  $('#inputTariffName').val(TariffDefinition.Name);
  $('#inputTariffPeriods').val(TariffDefinition.DayPeriods);
  $('#inputTariffComments').val(TariffDefinition.Description);
  TariffDefinition.WeekendPeriod = PopulatePeriodDropdown($('#btnTariffWeekend'), TariffDefinition.DayPeriods, 'P', TariffDefinition.WeekendPeriod, "Don't apply");
  TariffDefinition.HolidayPeriod = PopulatePeriodDropdown($('#btnTariffHoliday'), TariffDefinition.DayPeriods, 'P', TariffDefinition.HolidayPeriod, "Don't apply");
}
var SeasonsTable = null;
$('#btnTariffSeasons').click(LoadSeasons);
function LoadSeasons() {
  $('#dlgTariff > div > div').addClass('hide');
  $('#divTariffSeasons').removeClass('hide');
  if(SeasonsTable === null) {
    SeasonsTable = $('#SeasonsTable').DataTable({ responsive: false, paging: false, scrollCollapse: true, scrollY: '224px', dom: '<"clear">t<"clear">', columnDefs: [ { targets: [0, 1, 2], orderable: true }, { targets: [1, 2], className: 'text-center' }, { targets: [3], orderable: false } ], order: [1, 'asc']});
  } else {
    SeasonsTable.clear();
  }
  for(var key in TariffDefinition.Seasons) {
    var RowText = '<tr>' +
                  '<td>' + key + '</td>' +
                  '<td>' + moment(TariffDefinition.Seasons[key].StartDate).format('DD/MM HH:mm') + '</td>' +
                  '<td>' + moment(TariffDefinition.Seasons[key].EndDate).format('DD/MM HH:mm') + '</td>' +
                  '<td data-season="' + key + '"><div class="btn-group">' +
                  '<button type="button" class="btn btn-default btn-xs">Duplicate</button>' +
                  '<button type="button" class="btn btn-default btn-xs">Edit</button>' +
                  '<button type="button" class="btn btn-danger btn-xs">Delete</button>' +
                  '</div></td>' +
                  '</tr>';
    SeasonsTable.row.add($(RowText)).draw();
  }
  SeasonsTable.columns.adjust();  
}
var SeasonKey;
$('#btnTariffSeasonsAdd').click(function() {
  SeasonKey = null;
  LoadSeasonDialog({ StartDate:  new Date(new Date().getFullYear(), 0, 1, 0, 0, 0, 0).getTime(), EndDate:  new Date(new Date().getFullYear(), 11, 31, 23, 59, 59, 999).getTime(), DayPeriods: new Array(24).fill(1) });
});
$('#SeasonsTable').on('click', 'button:nth-child(2)', function(event) {
  SeasonKey = $(this).parents('td').attr('data-season');
  LoadSeasonDialog(Tariffs[TariffKey].Seasons[SeasonKey]);
});
$('#SeasonsTable').on('click', 'button:nth-child(3)', function(event) {
  delete Tariffs[TariffKey].Seasons[$(this).parents('td').attr('data-season')];
  LoadSeasons();
});
$('#SeasonsTable').on('click', 'button:nth-child(1)', function(event) {
  SeasonKey = null;
  LoadSeasonDialog(Tariffs[TariffKey].Seasons[$(this).parents('td').attr('data-season')]);
});
function LoadSeasonDialog(season) {
  $('#dlgTariff > div > div').addClass('hide');
  $('#divTariffSeason').removeClass('hide');
  $('#hourlyRates').html('');
  $('#inputSeasonName').val((SeasonKey === null) ? 'No name' : SeasonKey);
  $('#inputSeasonFrom').datetimepicker({ format: 'DD/MM/YYYY HH:mm:00', defaultDate: new Date(season.StartDate), sideBySide: true });
  $('#inputSeasonTo').datetimepicker({ format: 'DD/MM/YYYY HH:mm:59', defaultDate: new Date(season.EndDate), sideBySide: true });
  for(var i = 0; i < 24; i++) {
    $('#hourlyRates').append('<div class="btn-group" style="margin: 4px 4px">' +
                             '  <div class="input-group input-group-xs">' +
                             '    <span class="input-group-addon">' + ("0" + i).slice(-2) + '</span>' +
                             '    <div class="input-group-btn">' +
                             '      <button class="btn btn-default dropdown-toggle" type="button"data-toggle="dropdown" id="btnHour' + i + '"><span style="display: inline-block; width: 30px;">P1</span><span class="caret"></span></button>' +
                             '      <ul class="dropdown-menu pull-right">' +
                             '      </ul>' +
                             '    </div>' +
                             '  </div>' +
                             '</div>');
    season.DayPeriods[i] = PopulatePeriodDropdown($('#btnHour' + i), Tariffs[TariffKey].DayPeriods, 'P', season.DayPeriods[i]);
    $('#btnHour' + i + ' + ul li a').click(DropDownUpdateValue);
  }
}
$('#btnTariffSeasonBack').click(function() {
  $('#dlgTariff > div > div').addClass('hide');
  $('#divTariffSeasons').removeClass('hide');
  var Season = {};
  Season.StartDate = $('#inputSeasonFrom').data("DateTimePicker").viewDate().utc().valueOf();
  Season.EndDate = $('#inputSeasonTo').data("DateTimePicker").viewDate().utc().valueOf();
  Season.DayPeriods = [];
  for(var i = 0; i < 24; i++) Season.DayPeriods.push(DropdownValue($('#btnHour' + i)));
  Tariffs[TariffKey].Seasons[$('#inputSeasonName').val()] = Season;
  LoadSeasons();
});


var HolidaysTable = null;











$('#btnTariffHolidays').click(function() { 
  $('#dlgTariff > div > div').addClass('hide');
  $('#divTariffHolidays').removeClass('hide');
});

$('#btnTariffHolidaysBack, #btnTariffSeasonsBack').click(function() { 
  $('#dlgTariff > div > div').addClass('hide');
  $('#divTariffMain').removeClass('hide');
});










// Devices -------------------------------------------------------------------------------------------------------------
var DeviceTable = null;
function AjaxGetDevices() {
$.ajax({ url: "getDevices", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), success: 
    function(result) {
      DeviceTable = $('#DeviceTable').DataTable({ responsive: false, paging: false, scrollCollapse: true, scrollY: '224px', dom: '<"clear">t<"clear">', columnDefs: [ { targets: [0, 1, 2, 3, 4, 5], orderable: false }, { targets: [2, 3, 4], className: 'text-center' } ], order: [0, 'desc']});
      for(var i = 0; i < result.length; i++) {
        var RowText = '<tr>' +
          '<td>' + result[i]['Driver'] + '</td>' +
          '<td>' + result[i]['Name'] + '</td>' +
          '<td>' + result[i]['Serial'] + '</td>' +
          '<td>' + result[i]['Analyzers'] + '</td>' +
          '<td>' + result[i]['Status'] + '</td>' +
          '<td><div class="btn-group" style="width:100%"><button class="btn btn-xs btn-success" style="width:40%">Start</button><button class="btn btn-xs btn-default" style="width:60%">Configure</button></div></td>' +
          '</tr>';
        DeviceTable.row.add($(RowText)).draw();
      }
    }
  });
}
function StartDevice() {
  
}
function ConfigureDevice() {
  
}
// NTP -----------------------------------------------------------------------------------------------------------------
var NTP_DateTimePicker = null;
$('#checkNTP').change(function() {
  if($(this).prop('checked')) {
    $('#inputCurrentTime > span').css("pointer-events", "none");
    $('#inputCurrentTime > input').attr("disabled", "true");
    $('#inputNTPServer').removeAttr("disabled");
  } else {
    $('#inputNTPServer').attr("disabled", "true");
    $('#inputCurrentTime > input').removeAttr("disabled");
    $('#inputCurrentTime > span').css("pointer-events", "inherit");
  }
});
function AjaxGetTimeZones() {
  $.ajax({ url: "getTimeSettings", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), success: 
    function(result) {
      NTP_DateTimePicker = $('#inputCurrentTime').datetimepicker({ format: 'DD/MM/YYYY HH:mm:00', defaultDate: new Date(result.Time), sideBySide: true });
      $('#inputCurrentTime > input').attr('value', moment(new Date(result.Time)).format('DD/MM/YYYY HH:mm:ss'));
      $('#dropdownTimeZone span:first-child').text(result.TimeZone);
      $('#inputNTPServer').val(result.NTP_Server);
      if(result.NTP_Enabled === true) {
        $('#checkNTP').bootstrapToggle('on');
        $('#inputNTPServer').removeAttr("disabled");
      } else {
        $('#checkNTP').bootstrapToggle('off');
        $('#inputNTPServer').attr("disabled", "true");
      }
      result.TimeZones.sort();
      for(i = 0; i < result.TimeZones.length; i++) $('#dropdownListTimeZone').append('<li><a href="#">' + result.TimeZones[i] + '</a></li>');
      
      $("#dropdownListTimeZone li a").click(function(event) {
        event.preventDefault();
        $('#dropdownTimeZone span:first-child').text($(this).text());
     });
    }
  });
}
$('#btnNTPSave').click(function() {
  $('#btnNTPSave').attr("disabled");
  $.ajax({ url: "setTimeSettings", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', 
    data: JSON.stringify({ NTP_Enabled: $('#checkNTP').is(':checked'), NTP_Server: $('#inputNTPServer').val(), TimeZone: $('#dropdownTimeZone span:first-child').text(), Time: moment.tz($('#inputCurrentTime > input').val(), 'DD/MM/YYYY HH:mm:ss', $('#dropdownTimeZone span:first-child').text()).utc().valueOf() }), 
    success: function(result) {
      $('#dlgSave .modal-body > p').html("Settings changed correctly!");
      $('#dlgSave').modal('show'); 
    },
    error: function(result) {
      $('#dlgSave .modal-body > p').html("Failed to save settings!");
      $('#dlgSave').modal('show'); 
    },
    complete: function(result) {
      $('#btnNTPSave').removeAttr("disabled");    
    }
  });
});
/* Secuencia de inicio para cargar la pagina */
function UpdatePage() {
  AjaxGetTimeZones();
  AjaxGetDevices();
  AjaxGetTariffs();
  AjaxGetNetworks();
}