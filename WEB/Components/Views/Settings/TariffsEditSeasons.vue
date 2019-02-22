<template>
  
        <div class="modal-content hide" id="divTariffPeriods">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Periods</h4></div>
            <div class="pull-right btn-group">
              <button type="button" class="btn btn-default btn-xs" id="btnTariffPeriodsDel"><img src="Imgs/cross.png" alt="Delete period"></button>
              <button type="button" class="btn btn-default btn-xs" id="btnTariffPeriodsAdd"><img src="Imgs/plus.png" alt="New period"></button>
              <button type="button" class="btn btn-default btn-xs" id="btnTariffPeriodsBack"><img src="Imgs/arrow_undo.png" alt="Go back"></button>
            </div>
          </div>
          <div class="modal-body">
            <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px;">Define the maximum periods of days and their pricing model</div>
            <div class="row">
              <div class="col-xs-12">
                <table class="table small table-striped table-responsive" id="PeriodsTable">
                  <thead><tr><th style="width: 20px">Name</th><th>Power model</th><th>Reactive model</th></thead>
                  <tbody></tbody>
                </table>
              </div>
            </div>
          </div>
        </div> 
        <div class="modal-content hide" id="divTariffSeasons">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Seasons</h4></div>
            <div class="pull-right btn-group">
              <button type="button" class="btn btn-default btn-xs" id="btnTariffSeasonsAdd"><img src="Imgs/plus.png" alt="New season"></button>
              <button type="button" class="btn btn-default btn-xs" id="btnTariffSeasonsBack"><img src="Imgs/arrow_undo.png" alt="Go back"></button>
            </div>
          </div>
          <div class="modal-body">
            <div class="alert alert-info text-center" style="margin-bottom: 5px; padding: 5px;">Define seasons of the year</div>
            <div class="row">
              <div class="col-xs-12">
                <table id="SeasonsTable" class="table small table-striped table-responsive" style="width: 100%">
                  <thead><tr><th>Name</th><th style="width:80px">From</th><th style="width:80px">To</th><th style="width:140px"></th></tr></thead>
                  <tbody></tbody>
                </table>
              </div>
            </div>
          </div>
        </div>
        <div class="modal-content hide" id="divTariffHolidays">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Holidays</h4></div>
            <div class="pull-right btn-group">
              <button type="button" class="btn btn-default btn-xs" id="btnTariffHolidaysAdd"><img src="Imgs/plus.png" alt="New holiday"></button>
              <button type="button" class="btn btn-default btn-xs" id="btnTariffHolidaysBack"><img src="Imgs/arrow_undo.png" alt="Go back"></button>
            </div>
          </div>
          <div class="modal-body">
            <div class="row">
              <div class="col-xs-12">
                <table class="table small table-striped table-responsive">
                  <thead><tr><th style="width: 40px">Driver</th><th>Description</th><th style="width: 250px">Serial</th><th style="width: 20px">Analyzers</th><th style="width: 90px">Status</th><th style="width: 180px"></th></tr></thead>
                  <tbody></tbody>
                </table>
              </div>
            </div>
          </div>
        </div> 
        <div class="modal-content hide" id="divTariffSeason">
          <div class="modal-header">
            <div class="pull-left"><h4 class="modal-title">Tariffs template &raquo; Seasons &raquo; Season</h4></div>
            <div class="pull-right btn-group">
              <button type="button" class="btn btn-default btn-xs" id="btnTariffSeasonBack"><img src="Imgs/arrow_undo.png" alt="Go back"></button>
            </div>
          </div>
          <div class="modal-body">
            <div class="row">
              <div class="col-xs-12">
                <label class="control-label">Name</label>
                <input type="text" class="form-control" id="inputSeasonName">
              </div>
              <div class="col-xs-12 col-sm-6">
                <label class="control-label">From</label>
                <div class='input-group date' id='inputSeasonFrom'>
                  <input type='text' class="form-control" style="pointer-events: none;"/>
                  <span class="input-group-addon">
                    <span class="glyphicon-calendar"></span>
                  </span>
                </div>
              </div>
              <div class="col-xs-12 col-sm-6">
                <label class="control-label">To</label>
                <div class='input-group date' id='inputSeasonTo'>
                  <input type='text' class="form-control" style="pointer-events: none;"/>
                  <span class="input-group-addon">
                    <span class="glyphicon-calendar"></span>
                  </span>
                </div>
              </div>
              <div class="col-xs-12">
                <label class="control-label">Hourly rates</label>
                <div style=" display:flex;flex-flow: wrap; margin: -4px -4px" id="hourlyRates"></div>
              </div>



            </div>
          </div>
        </div> 


</template>