<template>
   <div id="wrapper">
      <div class="row">
        <div class="col-lg-12">
          <div class="panel panel-active-energy">
            <div id="StatsSpinner" class="spinnerdiv"></div>
            <div class="panel-heading">
              <span class="huge header-text">Energy distribution</span>
              <div class="header-buttons">
                <div class="btn-group" data-load-block="Years_Block"></div>
                <div class="btn-group" data-load-block="PhaseV_Block" id="groupEnergyPhases"></div>
              </div>
              <span class="clearfix"> </span>
            </div>
            <div class="panel-body">
              <div class="row">
                <div class="col-md-12">
                  <div class="panel panel-other"> 
                    <div class="panel-heading">
                      <span>By day</span>
                      <div style="float: right; width: 200px">
                        <div id="CalendarScale" style="height: 10px; width: 200px; display: flex; flex-direction:row; border: 1px solid #EEE"></div>
                        <div style="float: left; font-size: 80%" id="CalendarMin">?</div>
                        <div style="float: right; font-size: 80%" id="CalendarMax">?</div>
                        <div style="text-align: center; font-size: 80%"><em id="CalendarUnits"></em></div>
                      </div>
                    </div>
                    <div id="Calendar"></div>
                  </div>
                </div>
              </div>
              <div class="row">
                <div class="col-md-12 col-lg-6">
                  <div class="panel panel-other">
                    <div class="panel-heading">
                      <i class="fa fa-calendar-o" aria-hidden="true"></i> By month
                      <div class="pull-right" id="MonthUnits"></div>
                    </div>
                    <div class="panel-body">
                      <table id="MonthTable" class="table small table-striped table-responsive">
                        <thead>
                          <tr>
                            <th style="width: 30px">Month</th><th style="width: 100%"></th><th>Weekday</th><th>Weekend</th><th>Total</th>
                          </tr>
                        </thead>
                        <tbody>
                        </tbody>
                      </table>
                    </div>
                  </div>
                  <div class="panel panel-other">
                    <div class="panel-heading">
                      <i class="fa fa-ellipsis-h" aria-hidden="true"></i> By week day
                      <div style="float: right" id="WeekDayUnits"></div>
                    </div>
                    <div class="panel-body">
                      <table id="WeekDayTable" class="table small table-striped table-responsive">
                        <thead>
                          <tr>
                            <th style="width: 30px">Day</th><th style="width: 100%"></th><th>Average</th><th>Accumulated</th>
                          </tr>
                        </thead>
                        <tbody>
                        </tbody>
                      </table>
                    </div>
                  </div>
                  <div class="panel panel-other">
                    <div class="panel-heading">
                      <i class="fa fa-clock-o" aria-hidden="true"></i></i> By hour
                      <div style="float: right" id="HourUnits"></div>
                    </div>
                    <div class="panel-body">
                      <table id="HourTable" class="table small table-striped table-responsive">
                        <thead>
                          <tr>
                            <th style="width: 30px">Hour</th><th style="width: 100%"></th><th>Average</th><th>Accumulated</th>
                          </tr>
                        </thead>
                        <tbody>
                        </tbody>
                      </table>
                    </div>
                  </div>
                </div>
                <div class="col-md-12 col-lg-6">
                  <div class="panel panel-other">
                    <div class="panel-heading">
                      <i class="fa fa-bars" aria-hidden="true"></i> By week
                      <div class="pull-right" id="WeekUnits"></div>
                    </div>
                    <div class="panel-body">
                      <table id="WeekTable" class="table small table-responsive">
                        <thead>
                          <tr>
                            <th style="width: 30px">Week</th><th style="width: 100%"></th><th>Weekday</th><th>Weekend</th><th>Total</th>
                          </tr>
                        </thead>
                        <tbody>
                        </tbody>
                      </table>
                    </div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
 </div>
</template>

<script>
  export default {
    data() {
      return {
      }
    }
  }
</script>


