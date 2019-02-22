<template>
  <div class="modal-content">
    <div class="modal-header">
      <div class="pull-left"><h4 class="modal-title">Tariffs template</h4></div>
      <div class="pull-right btn-group">
        <button type="button" class="btn btn-default btn-xs disabled" @click="Save"><img src="Imgs/diskette.png" alt="New tariff"></button>
        <button type="button" class="btn btn-default btn-xs" @click="Periods">Periods</button>
        <button type="button" class="btn btn-default btn-xs" @click="Seasons">Seasons</button>
        <button type="button" class="btn btn-default btn-xs" @click="Holidays">Holidays</button>
        <button type="button" class="btn btn-default btn-xs" @click="Close"><img src="Imgs/arrow_undo.png" alt="Discard"></button>
      </div>
    </div>
    <div class="modal-body">
      <div class="alert alert-danger text-center hide" id="divTariffError" style="margin-bottom: 5px; padding: 5px;"></div>
      <div class="alert alert-success text-center" id="divTariffSuccess" style="margin-bottom: 5px; padding: 5px;"></div>
            <div class="row">
              <div class="col-xs-12 col-sm-6">
                <label class="control-label">Name</label>
                <input type="text" class="form-control" id="inputTariffName">
              </div>
              <div class="col-xs-12 col-sm-6">
                <label class="control-label">Country/Zone</label>
                <div class="input-group">
                  <input type="text" class="form-control" value="Spain" style="pointer-events: none" id="inputTariffCountry">
                  <div class="input-group-btn-vertical">
                    <button class="btn btn-default" type="button"><i class="fa fa-caret-up"></i></button>
                    <button class="btn btn-default" type="button"><i class="fa fa-caret-down"></i></button>
                  </div>
                </div>
              </div>
              <div class="col-xs-12 col-sm-6">
                <label class="control-label">Start date</label>
                <div class='input-group date' id='inputTariffStartDate'>
                  <input type='text' class="form-control" />
                  <span class="input-group-addon">
                    <span class="glyphicon-calendar"></span>
                  </span>
                </div>
              </div>
              <div class="col-xs-12 col-sm-6">
                <label class="control-label">End date</label>
                <div class='input-group date' id='inputTariffEndDate'>
                  <input type='text' class="form-control" />
                  <span class="input-group-addon">
                    <span class="glyphicon-calendar"></span>
                  </span>
                </div>
              </div>
              <div class="col-xs-12">
                <label class="control-label">Description</label>
                <textarea class="form-control" rows="5" id="inputTariffComments" style="resize: none"></textarea>
              </div>
              <div class="col-xs-12 col-sm-6">
                <div class="btn-group" style="margin-top:8px;width:100%;">
                  <div class="input-group">
                    <span class="input-group-addon" style="width: 100%"><strong>Weekends as</strong></span>
                    <div class="input-group-btn">
                      <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" aria-expanded="false" id="btnTariffWeekend"><span style="display: inline-block;width: 80px;"></span><span class="caret"></span></button>
                      <ul class="dropdown-menu pull-right">
                      </ul>
                    </div>
                  </div>
                </div>
              </div>
              <div class="col-xs-12 col-sm-6">
                <div class="btn-group" style="margin-top: 8px;width: 100%;">
                  <div class="input-group">
                    <span class="input-group-addon" style="width: 100%"><strong>Holidays as</strong></span>
                    <div class="input-group-btn">
                      <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown" aria-expanded="false" id="btnTariffHoliday"><span style="display: inline-block;width: 80px;"></span><span class="caret"></span></button>
                      <ul class="dropdown-menu pull-right">
                      </ul>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
  </div>
</template>
