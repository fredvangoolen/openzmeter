/* Configuracion de la Wifi */
var NetworksTable   = null;
var NetworksDataset = [];
var EditingNetwork  = null;
//var WifiSpinner     = new Spinner(SpinnerOpts);
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
var TariffsTable = null;
var Tariffs = [];
var TariffKey = null;
var TariffDefinition = null;

var PeriodsTable  = null;
var SeasonsTable  = null;
var HolidaysTable = null;

$('#inputTariffStartDate, inputTariffEndDate').on('dp.change', function(e) {
  if(this.id === 'inputTariffStartDate') TariffDefinition.StartDate = e.date.startOf('day').utc().valueOf();
  if(this.id === 'inputTariffEndDate') TariffDefinition.EndDate = e.date.endOf('day').utc().valueOf();
  CheckTariff();
});

$('#btnTariffPeriods').click(LoadPeriods);
function LoadPeriods() {
  $('#dlgTariff > div > div').addClass('hide');
  $('#divTariffPeriods').removeClass('hide');
  PeriodsTable.clear();
  for(var i = 0; i < TariffDefinition.Details.Periods.length; i++) {
    var RowText = '<tr>' +
                  '<td>P' + (i + 1) + '</td>' +
                  '<td>NONE</td>' +
                  '<td>NONE</td>' +
                  '</tr>';
    PeriodsTable.row.add($(RowText)).draw();
  }
  $('#btnTariffPeriodsDel').toggleClass('disabled', (TariffDefinition.Details.Periods.length <= 1));
  PeriodsTable.columns.adjust();  
}
$('#btnTariffPeriodsDel').click(function() {
  TariffDefinition.Details.Periods.splice(-1, 1);
  LoadPeriods();
});
$('#btnTariffPeriodsAdd').click(function() {
  TariffDefinition.Details.Periods.push({ "ReactiveMode": null, "PowerMode": null });
  LoadPeriods();
});
$('#btnTariffPeriodsBack, #btnTariffSeasonsBack, #btnTariffHolidaysBack').click(function() { 
  $('#dlgTariff > div > div').addClass('hide');
  $('#divTariffMain').removeClass('hide');
  TariffDefinition.WeekendPeriod = PopulatePeriodDropdown($('#btnTariffWeekend'), TariffDefinition.Details.Periods.length, 'P', TariffDefinition.WeekendPeriod, "Don't apply");
  TariffDefinition.HolidayPeriod = PopulatePeriodDropdown($('#btnTariffHoliday'), TariffDefinition.Details.Periods.length, 'P', TariffDefinition.HolidayPeriod, "Don't apply");
  CheckTariff(TariffDefinition);
});
$('#btnTariffSeasons').click(LoadSeasons);
function LoadSeasons() {
  $('#dlgTariff > div > div').addClass('hide');
  $('#divTariffSeasons').removeClass('hide');
  SeasonsTable.clear().draw();
  for(var i = 0; i < TariffDefinition.Details.Seasons.length; i++) {
    var RowText = '<tr>' +
                  '<td>' + TariffDefinition.Details.Seasons[i].Name + '</td>' +
                  '<td>' + moment(TariffDefinition.Details.Seasons[i].StartDate).format('DD/MM/YYYY') + '</td>' +
                  '<td>' + moment(TariffDefinition.Details.Seasons[i].EndDate).format('DD/MM/YYYY') + '</td>' +
                  '<td><div class="btn-group">' +
                  '<button type="button" class="btn btn-default btn-xs">Duplicate</button>' +
                  '<button type="button" class="btn btn-default btn-xs">Edit</button>' +
                  '<button type="button" class="btn btn-danger btn-xs">Delete</button>' +
                  '</div></td>' +
                  '</tr>';
    SeasonsTable.row.add($(RowText)).draw();
  }
  SeasonsTable.columns.adjust();  
}
var SeasonKey = null;
$('#btnTariffSeasonsAdd').click(function() {
  SeasonKey = null;
  LoadSeasonDialog({ StartDate: new Date(new Date().getFullYear(), 0, 1, 0, 0, 0, 0).getTime(), EndDate: new Date(new Date().getFullYear(), 11, 31, 23, 59, 59, 999).getTime(), DayPeriods: new Array(24).fill(1), Name: 'Unnamed season' });
});
$('#SeasonsTable').on('click', 'button:nth-child(1)', function(event) {
  SeasonKey = null;
  LoadSeasonDialog(TariffDefinition.Details.Seasons[SeasonsTable.row($(this).parents('tr')).index()]);
});
$('#SeasonsTable').on('click', 'button:nth-child(2)', function(event) {
  SeasonKey = SeasonsTable.row($(this).parents('tr')).index();
  LoadSeasonDialog(TariffDefinition.Details.Seasons[SeasonKey]);
});
$('#SeasonsTable').on('click', 'button:nth-child(3)', function(event) {
  TariffDefinition.Details.Seasons.splice(SeasonsTable.row($(this).parents('tr')).index(), 1);
  LoadSeasons();
});
function LoadSeasonDialog(season) {
  $('#dlgTariff > div > div').addClass('hide');
  $('#divTariffSeason').removeClass('hide');
  $('#hourlyRates').html('');
  $('#inputSeasonName').val(season.Name);
  $('#inputSeasonFrom').datetimepicker({ format: 'DD/MM/YYYY', viewMode: 'months', defaultDate: new Date(season.StartDate) });
  $('#inputSeasonTo').datetimepicker({ format: 'DD/MM/YYYY', viewMode: 'months', defaultDate: new Date(season.EndDate) });
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
    season.DayPeriods[i] = PopulatePeriodDropdown($('#btnHour' + i), TariffDefinition.Details.Periods.length, 'P', season.DayPeriods[i]);
    $('#btnHour' + i + ' + ul li a').click(DropDownUpdateValue);
  }
}
$('#btnTariffSeasonBack').click(function() {
  $('#dlgTariff > div > div').addClass('hide');
  $('#divTariffSeasons').removeClass('hide');
  var Season = {};
  Season.Name = $('#inputSeasonName').val();
  Season.StartDate = $('#inputSeasonFrom').data("DateTimePicker").viewDate().startOf('day').utc().valueOf();
  Season.EndDate = $('#inputSeasonTo').data("DateTimePicker").viewDate().endOf('day').utc().valueOf();
  Season.DayPeriods = [];
  for(var i = 0; i < 24; i++) Season.DayPeriods.push(DropdownValue($('#btnHour' + i)));
  if(SeasonKey === null) 
    TariffDefinition.Details.Seasons.push(Season);
  else
    TariffDefinition.Details.Seasons[SeasonKey] = Season;
  LoadSeasons();
});
$('#btnTariffHolidays').click(function() { 
  $('#dlgTariff > div > div').addClass('hide');
  $('#divTariffHolidays').removeClass('hide');
});
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

///TODO: Falta guardar tarifa





























// NTP -----------------------------------------------------------------------------------------------------------------
/* Secuencia de inicio para cargar la pagina */
function UpdatePage() {
  AjaxGetTimeZones();
  
  AjaxGetTariffs();
  AjaxGetNetworks();
}