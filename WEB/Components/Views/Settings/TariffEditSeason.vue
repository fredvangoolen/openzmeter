<template>
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
