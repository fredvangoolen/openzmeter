/* Opciones por defecto del spinner */
var SpinnerOpts         = { lines: 10,  // The number of lines to draw
                            length: 10, // The length of each line
                            width: 5,   // The line thickness
                            radius: 10  // The radius of the inner circle
                          };
                          
/* Configuracion del plugin de desconexion */
Offline.options = {requests: false};
function DisableUI() {
  $('#DisableDiv').css('display', 'inherit');
}
function EnableUI() {
  $('#DisableDiv').css('display', 'none');
}
Offline.on('confirmed-down', DisableUI);
Offline.on('confirmed-up',   EnableUI);
$(document).ajaxError(function (event, jqxhr, settings, exception) {
  if(jqxhr.status === 401) window.location.href = 'Login';
});

/* Muesto/oculto spinner en un div */
function ShowSpinner(spinner, container) {
  $('#' + container).css('display', 'inherit');
  spinner.spin(document.getElementById(container));  
}
function StopSpinner(spinner, container) {
  $('#' + container).css('display', 'none');
  spinner.stop();  
}

/* Añade los flag de eventos a una grafica */
function AddEvents(result, serieName, graph) {
  var Events = graph.annotations();
  for(i = 0; i < result['Sampletime'].length; i++) {
    if(result['flag'][i] === false) continue;
    var EventID = {
      series: serieName,
      xval: result['Sampletime'][i],
      shortText: 'EVT',
      text: "Este periodo esta marcado por que contiene un evento"
    };
    Events.push(EventID);
  }
  graph.setAnnotations(Events);
}

/* Convierte un array de floats a cadena con dos decimales, separados por el caracter dado */
function Array_Round2(values, separator, units) {
  var Tmp = 0;
  for(i = 0; i < values.length; i++) values[i] = Round2(values[i]) + units;
  return values.join(separator);
}

/* Convierte un numero a cadena con dos decimales y muestra el valor en los huecos que se pasen */
function Round2Labels(value, integer, fraction) {
  if(isNaN(value)) {
    integer.text('?');
    fraction.text('');
  } else {
    var parts = (value + "").split(".");
    integer.text(parts[0]);
    if(parts.length === 1) {
      fraction.text('.00');
    } else {
      parts[1] = parts[1] + "0";
      fraction.text('.' + parts[1].substr(0, 2));
    }
  }
}

/* Retorna un numero como cadena con dos decimales */
function Round2(value) {
  if(isNaN(value)) {
    return '?';
  } else {
    var parts = (value + "").split(".");
    if(parts.length === 1) {
      return parts[0] + '.00';
    } else {
      parts[1] = parts[1] + "0";
      return parts[0] + '.' + parts[1].substr(0, 2);
    }
  }
}

/* Actualiza el texto de la cabecera y lanza una nueva llamada de forma periodica */
function AjaxGetIndicators() {
  $.ajax({ url: "getSeriesNow",
    timeout: 20000,
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data: JSON.stringify({ Series: 'rms_v, rms_i, active_power, freq', Node: Nodes['IDs'][Node]}),
    success: AjaxIndicators,
    error: function() {
      $('#VoltageIndicator').text('Offline');
      $('#CurrentIndicator').text('Offline');
      $('#PowerIndicator').text('Offline');
      $('#FrequencyIndicator').text('Offline');
      $('NodeTime').text('Offline');
    },
    complete: function() {
      setTimeout(AjaxGetIndicators, 3000);
    }
 });
}

function AjaxIndicators(result) {
  var Time = new  Date(result['Sampletime']);
  $('#NodeTime').text(Time.toLocaleTimeString() + ' - ' + Time.toLocaleDateString());
  $('#VoltageIndicator').text(Array_Round2(result['rms_v'], ' | ', ''));
  $('#CurrentIndicator').text(Array_Round2(result['rms_i'], ' | ', ''));
  var Units = 'W';
  for(i = 0; i < result['active_power'].length; i++) {
    if(result['active_power'][i] > 1000) Units = 'kW';
  }
  $('#PowerIndicatorUnits').text(Units);
  if(Units == 'kW') {
    for(i = 0; i < result['active_power'].length; i++) result['active_power'][i] = result['active_power'][i] / 1000;
  }
  $('#PowerIndicator').text(Array_Round2(result['active_power'], ' | ', ''));
  $('#FrequencyIndicator').text(Round2(result['freq']));
  //  
//  if(result.sysinfo) {
//    $('#ProgressCPU').LineProgressbar( { percentage: result.sysinfo.cpu, radius: '2px', height: '10px', ShowProgressCount: false, duration: 0 });
//    $('#CPUText').text(Math.round(result.sysinfo.cpu) + '%');
//    var RAM = 100 - (result.sysinfo.ram[0] / result.sysinfo.ram[1] * 100);
//    $('#ProgressRAM').LineProgressbar( { percentage: RAM, radius: '2px', height: '10px', ShowProgressCount: false, duration: 0 });
//    $('#RAMText').text(Math.round(RAM) + '% (' + Math.round(result.sysinfo.ram[0] / 1024.0) + 'Mb free)');
//    var SSD = 100 - (result.sysinfo.ssd[0] / result.sysinfo.ssd[1] * 100);
//    $('#ProgressSDD').LineProgressbar( { percentage: SSD, radius: '2px', height: '10px', ShowProgressCount: false, duration: 0 });
//    $('#SSDText').text(Math.round(SSD) + '% (' + Math.round(result.sysinfo.ssd[0] / 1024.0 / 1024.0) + 'Mb free)');
//    $('#ProgressBattery').LineProgressbar( { percentage: 0, radius: '2px', height: '10px', ShowProgressCount: false, duration: 0 });
//    $('#BatteryText').text('Not avaliable');
//  }
//
//  if(result.lastCurrent) {
//    $('#current_indicator').text(Round2(result.lastCurrent));
//  }
//  if(result.lastPower) {
//    if(result.lastPower > 1000) {
//      $('#power_indicator').text(Round2(result.lastPower / 1000));
//      $('#power_indicator_units').text('kW');
//    } else {
//      $('#power_indicator').text(Round2(result.lastPower));
//      $('#power_indicator_units').text('W');
//    }
//  }  
//  if(result.lastFrequency) {
//    $('#frequency_indicator').text(Round2(result.lastFrequency));
//  }
//  if(result.today_energy) {
//    if(result['today_energy'] > 1000) {
//      $('#energy_indicator').text(Round2(result['today_energy'] / 1000));
//      $('#energy_indicator_units').text('kWh');
//    } else {
//      $('#energy_indicator').text(Round2(result['today_energy']));
//      $('#energy_indicator_units').text('Wh');
//    }
//  }
}


/* Actualiza el desplegable de nodos disponibles y selecciona el que este activo para la sesion */
var Nodes = [];
var Node = parseInt(Cookies.get("SelNode"));
function GetNodesCompletes(result) {
  Nodes = result;
  if(!Node) {
    Cookies.set("SelNode", 0);
    Node = 0;
  }
  for(i = 0; i < result['IDs'].length; i++) {
    $('#SourceList').append(' <li><a href="">' + result['Names'][i] + ' (' + result['Type'][i] + ')</a></li>');
    if(i === Node) $('#SourceSel').text(result['Names'][i] + ' (' + result['Type'][i] + ')');
  }
  AjaxGetIndicators();
  UpdatePage();
}

/* Selecciona el apartado en el menu segun la pagina cargada */
function MenuSelect() {
  $.ajax({ url: "getNodes",
    timeout: 20000,
    type: "POST",
    dataType: "json",
    contentType: 'application/json',
    data: JSON.stringify({ }),
    success: GetNodesCompletes
  });
  $('#side-menu').metisMenu();
  var url = window.location;
  var element = $('ul.nav a').filter(function () {
    return this.href === url || url.href.indexOf(this.href) === 0;
  }).addClass('active').parent().parent().addClass('in').parent();
  if(element.is('div')) {
    element.addClass('active');
  } else {
    $('#side-menu li > a').first().addClass('active');
  }
}

/* Genera la secuencia de pasos una vez cargado todo el javascript para mostrar la pagina */
function StartPageLoad() {
  $('#Header').load("CommonHeader.html", null, MenuSelect);
  $(window).bind("load resize", function () {
    width = (this.window.innerWidth > 0) ? this.window.innerWidth : this.screen.width;
    if(width < 768) {
      $('div.navbar-collapse').addClass('collapse');
    } else {
      $('div.navbar-collapse').removeClass('collapse');
    }
    $("#page-wrapper").css("margin-top", ($("#Header").height() + 15) + "px");
  });  
}

/* Devuelve el texto completo a partir del texto abreviado de agregacion */
function GetAggregText(short) {
  if(short === '1h') {
    return '1 hour';
  } else if(short === '10m') {
    return '10 minutes';
  } else if(short === '1m') {
    return '1 minute';
  } else if(short === '3s') {
    return '3 seconds';
  } else {
    return false;
  }  
}

/* Obtiene el timestamp del inicio del dia actual */
function GetDayTimestamp() {
  var now = new Date();
  return new Date(now.getFullYear(), now.getMonth(), now.getDate(), 0, 0, 0, 0).getTime();
}

/* Obtiene el timestamp del inicio de la semana actual */
function GetWeekTimestamp() {
  var WeekDay = new Date().getDay();
  var Today = GetDayTimestamp();
  return Today - 86400 * WeekDay;
}

/* Obtiene el timestamp del inicio del mes actual */
function GetMonthTimestamp() {
  var now = new Date();
  return new Date(now.getFullYear(), now.getMonth(), 1, 0, 0, 0, 0).getTime();
}

/* Obtiene el timestamp del inicio del año actual */
function GetYearTimestamp() {
  var now = new Date();
  return new Date(now.getFullYear(), 0, 1, 0, 0, 0, 0).getTime();
}


var energyUnit = 'kWh'
var today = new Date();
$('#side-menu').metisMenu();
//Loads the correct sidebar on window load,
//collapses the sidebar on window resize.
// Sets the min-height of #page-wrapper to window size
$(window).bind("load resize", function () {
    topOffset = 50;
    width = (this.window.innerWidth > 0) ? this.window.innerWidth : this.screen.width;
    if (width < 768) {
        $('div.navbar-collapse').addClass('collapse');
        topOffset = 100; // 2-row-menu
    } else {
        $('div.navbar-collapse').removeClass('collapse');
    }

    height = ((this.window.innerHeight > 0) ? this.window.innerHeight : this.screen.height) - 1;
    height = height - topOffset;
    if (height < 1)
        height = 1;
    if (height > topOffset) {
        $("#page-wrapper").css("min-height", (height) + "px");
    }
});
var url = window.location;
var element = $('ul.nav a').filter(function () {
    return this.href == url || url.href.indexOf(this.href) == 0;
}).addClass('active').parent().parent().addClass('in').parent();
if (element.is('div')) {
    element.addClass('active');
} else {
    $('#side-menu li > a').first().addClass('active');
}

$('.maximize').click(function () {
    var panel = $(this).parents('.panel');
    panel.addClass('maximized');
    $(this).hide();
    $('.minimize', panel).show();
    window.dispatchEvent(new Event('resize'));
});
$('.minimize').click(function () {
    var panel = $(this).parents('.panel');
    panel.removeClass('maximized');
    $(this).hide();
    $('.maximize', panel).show();
    window.dispatchEvent(new Event('resize'));
});
$('#side-menu a').click(function () {
//    $(this).addClass('cursor-wait');
//    $('body').addClass('cursor-wait');
});
function getStringEventType(type) {
    switch (type) {
        case 'INTERRUPTION':
            return "Interruption";
        case 'DIP':
            return "Dip";
        case 'SWELL':
            return "Swell";
        case 'RVC':
            return "RVC";
        case 'UNKNOWN':
            return "Unknown";
    }
}

function getStringEventNum(type) {
    switch (type) {
        case 'INTERRUPTION':
            return 0;
        case 'DIP':
            return 1;
        case 'SWELL':
            return 2;
        case 'RVC':
            return 3;
        case 'UNKNOWN':
            return 4;
    }
}

function getIconEventType(type) {
    switch (type) {
        case 'SWELL':
            return 'fa-level-up';
        case 'DIP':
            return 'fa-level-down';
        case 'INTERRUPTION':
            return "fa-window-close-o";
        case 'RVC':
            return 'fa-line-chart';
        case 'UNKNOWN':
            return 'fa-question';
    }
}

function getStringEventTypeShort(type) {
    switch (type) {
        case 0:
            return "I";
        case 1:
            return "D";
        case 2:
            return "S";
        case 3:
            return "R";
        case 4:
            return "U";
    }
}
function getStringEventTypeByNum(type) {
    switch (type) {
        case 0:
            return "Interruption";
        case 1:
            return "Dip";
        case 2:
            return "Swell";
        case 3:
            return "RVC";
        case 4:
            return "UNKNOWN";
    }
}

function ajaxGetLastEvents() {
    var request = $.ajax({
        url: "events/ajaxGetLastEvents",
        timeout: 20000,
        type: "POST",
        dataType: "json"
    });
    return request;
}

function generateEventAlertBox(event, timestamp) {
    var newEvent = "";
    if (event.timestamp > timestamp) {
        newEvent = "new_event";
    }
    var startTime = new Date(+event.timestamp);
    if (event.end_time != -1) {
        var duration = formatTimeUnit(event.duration * 1e6);
    } else {
        duration = 'In progress...'
    }
    var diff = (Math.abs(event.voltage - 230)).toFixed(2);
    var percent = (100 - event.percent).toFixed(2);
    return '<li class="event_alert ' + newEvent + '">\
                <div>\
                    <p><i class="fa ' + getIconEventType(event.type) + ' fa-fw"></i> ' + getStringEventType(event.type) + '\
                    <span class="pull-right text-muted small">' + (startTime.toLocaleString() + '.' + startTime.getMilliseconds()) + '</span></p>\
                    <div class="row small">\
                        <div class="col col-md-4">\
                            <em>Duration:</em><br> ' + duration + '\
                        </div>\
                        <div class="col col-md-5">\
                            <em>ΔU<sub>max</sub>:</em> ' + diff + ' V<br>(' + percent + '%)\
                        </div>\
                        <div class="col col-md-3">\
                            <a class="btn btn-primary btn-sm" href="events/disturbances/' + (+event.timestamp) + '" alt="See details..." title="See details..."><i class="fa fa-search-plus fa-fw"></i></a>\
                        </div>\
                    </div>\
                </div>\
            </li>\
            <li class="divider"></li>';
}

$('#eventsDropdown').on('show.bs.dropdown', function () {
    var dropdown = $('.dropdown-alerts', this).hide();
    var request = ajaxGetLastEvents();
    request.done(function (data) {
        var html = '';
        var timestamp = data.timestamp;
        for (var i in data.events) {
            //for (var j in data.events[i])
//                var generate = true;
//            if (i > 0) {
//                if (data.events[i][j - 1].time == data.events[i][j].time && data.events[i][j].end_time == -1) {
//                    generate = false;
//                }
//            }
//            if (generate) {
            html += generateEventAlertBox(data.events[i], timestamp);
//            }
        }
        if (html != '') {
            $('#eventsDropdown .dropdown-alerts').prepend(html);
        }
        $('#eventsDropdown .dropdown-alerts').slideDown(200);
    })
})

$('#eventsDropdown').on('hidden.bs.dropdown', function () {
    $('#eventsDropdown .event_alert').remove();
    $('#eventsDropdown .divider').remove();
})