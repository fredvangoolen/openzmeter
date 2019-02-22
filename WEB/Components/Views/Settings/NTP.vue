<template>
  <div class="panel panel-other">
    <div class="panel-heading">
      <div class="huge-small header-text">System time</div>
      <div class="header-buttons">
        <div class="btn-group">
          <button type="button" class="btn btn-default btn-xs disabled" data-toggle="tooltip" id="btnNTPSave" title="Save changes"><img src="Imgs/diskette.png"> Save</button>
        </div>
      </div>
    </div>
    <div class="panel-body">
      <div class="row">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Current time/zone</label>
          <div class='input-group date' id='inputCurrentTime'>
            <div class="input-group-btn">
              <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false" id="dropdownTimeZone"><span>UTC</span><span> &#9662;</span></button>
              <ul class="dropdown-menu scrollable-menu" id="dropdownListTimeZone">
              </ul>
            </div>
            <input type='text' class="form-control" />
            <span class="input-group-addon">
              <span class="glyphicon-calendar"></span>
            </span>
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">NTP Server</label>
          <div class="input-group">
            <span class="input-group-addon input-group-addon-toogle">
              <input type="checkbox" data-toggle="toggle" id="checkNTP" data-size="small" data-onstyle="success">
            </span>
            <input type="text" class="form-control" id="inputNTPServer">
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    data: function() {
        return {};
    },
    mounted: function() {
      AjaxGetTimeZones();
    }
  }
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
  $('#checkNTP').bootstrapToggle();
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

</script>