<template>
  <div id="wrapper">
    <b-row>
      <!-- NTP Time configuration -->
      <b-col>
          <div class="panel panel-other">
            <div class="panel-heading">
              <div class="huge-small header-text">System time</div>
              <div class="header-buttons">
                <div class="btn-group">
                  <b-button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" id="btnNTPSave" title="Save changes"><img src="Imgs/diskette.png"> Save</b-button>
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
        </b-col>
    </b-row>
    <b-row>
      <!-- Device configuration -->
      <b-col>
        <div class="panel panel-other">
          <div class="panel-heading">
            <div class="huge-small header-text">Input devices</div>
          </div>
          <div class="panel-body">
            <table id="DeviceTable" class="table small table-striped table-responsive">
              <thead><tr><th style="width: 40px">Driver</th><th>Description</th><th style="width: 250px">Serial</th><th style="width: 20px">Analyzers</th><th style="width: 90px">Status</th><th style="width: 180px"></th></tr></thead>
              <tbody></tbody>
            </table>
          </div>
        </div>
      </b-col>
    </b-row>
    <b-row>
        <!-- Tariffs configuration -->
        <b-col>
          <div class="panel panel-other">
            <div class="panel-heading">
              <div class="huge-small header-text">Tariffs</div>
              <div class="header-buttons">
                <div class="btn-group">
                  <button type="button" class="btn btn-default btn-xs" data-toggle="tooltip" title="Save changes" id="btnAddTariff"><img src="Imgs/plus.png"> Add</button>
                </div>
              </div>
            </div>
            <div class="panel-body">
              <table id="TariffsTable" class="table small table-striped table-responsive">
                <thead><tr><th style="width: 20px">Country</th><th>Name</th><th style="width: 120px">Start date</th><th style="width: 120px">End date</th><th style="width: 80px"></th></tr></thead>
                <tbody></tbody>
              </table>
            </div>
          </div>
        </b-col>
    </b-row>
    <b-row>
        <!-- Wifi configuration -->
        <b-col>
          <div class="panel panel-other">
            <!--div id="WifiSpinner" class="spinnerdiv"></div-->
            <div class="panel-heading">
              <i class="fa fa-wifi fa-fw"></i> Wifi connection
            </div>
            <div class="panel-body">
              <div class="row">
                <div class="col-sm-12">
                  <table id="NetworksTable" class="table small table-striped table-responsive" cellspacing="0" width="100%">
                    <thead><tr><th>Status</th><th>SSID</th><th>BSSID</th><th>Channel</th><th>Mode</th><th>Power</th><th></th></tr></thead>
                    <tbody>
                    </tbody>
                  </table>
                </div>
              </div>
            </div>
          </div>
        </b-col>
    </b-row>        
  </div>
</template>

<script>
export default {
  name: 'Dashboard',
  data () {
    return {
      msg: 'Welcome to Your Vue.js App'
    }
  }
}
</script>

<style>
</style>
