<template>
  <div class="panel panel-other" v-if="Bluetooth_Support">
    <div class="panel-heading">
      <div class="huge-small header-text">Bluetooth</div>
      <div class="header-buttons">
        <div class="btn-group">
          <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Save changes" v-on:click="Save"><img :src="require('Imgs/diskette.png')"> Save</button>
        </div>
      </div>
    </div>
    <div class="panel-body">
      <spinner v-if="Loading"></spinner>
      <div class="row">
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">Device name</label>
          <div class="input-group">
            <span class="input-group-addon input-group-addon-toogle">
              <toggle ref="EnableBT" v-on:Changed="EnableChange"></toggle>
            </span>
            <input type="text" class="form-control" disabled :value="Name">
          </div>
        </div>
        <div class="col-xs-12 col-sm-6">
          <label class="control-label">IP range</label>
          <input type="text" class="form-control" disabled value="Server on 10.0.0.1 clients from 10.0.0.2 to 10.0.0.254">
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  import Toggle from 'Components/Toggle.vue';
  import Spinner from 'Components/Spinner.vue';
  export default {
    data: function() {
      return {Name: '', Bluetooth_Support: false, Loading: false };
    },
    created: function() {
      this.Enabled = false;
    },
    mounted: function() {
      this.Load();
    },
    methods: {
      Load: function() {
        $.ajax({ url: "getBluetooth", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ }), 
          success: function(result) { 
            this.Bluetooth_Support = true;
            this.Enabled = result.Enabled;
            this.Name = result.Name;
            this.Loading = false;
            this.$nextTick(function() { 
              this.$refs.EnableBT.SetState(this.Enabled); 
            });
          }.bind(this)
        });        
      },
      EnableChange: function(value) {
        this.Enabled = value;
      },
      Save: function() {
        this.Loading = true;    
        $.ajax({ url: "setBluetooth", timeout: 20000, type: "POST", dataType: "json", contentType: 'application/json', data: JSON.stringify({ Enabled: this.Enabled }), success: this.Load});
      }
    },
    components: {Toggle, Spinner}
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
      NTP_DateTimePicker = $('#inputCurrentTime').datetimepicker({ format: 'DD/MM/YYYY HH:mm:00', defaultDate: moment(result.Time), sideBySide: true });
      $('#inputCurrentTime > input').attr('value', moment(moment(result.Time)).format('DD/MM/YYYY HH:mm:ss'));
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