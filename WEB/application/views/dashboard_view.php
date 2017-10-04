<!DOCTYPE html>
<html lang="en">
  <head>
    <? $this->load->view('template/commonHead', array('title' => 'Dashboard')) ?>
  </head>
  <body>
    <? $this->load->view('template/header') ?>
    <div class="modal fade" id="aggreg_modal" tabindex="-1" role="dialog" data-backdrop='static'>
      <div class="modal-dialog">
        <div class="modal-content">
          <div class="modal-header">
            <h3 class="modal-title" id="myModalLabel"><i class="fa fa-exclamation-triangle"></i> Aggregation change...</h3>
          </div>
          <div class="modal-body">
            <p>Requested aggregation returns to much points.<br/>Zoom in and try again.</p><p>Aggregation period has been changed to the lowest allowed.</p>
          </div>
          <div class="modal-footer">
            <button type="button" class="btn btn-success" id="aggreg_modal_close"><span class="glyphicon glyphicon-ok"></span>  Continue</button>
          </div>
        </div>
      </div>
    </div>
    <div class="modal" id="GraphDialog" tabindex="1" role="dialog" data-backdrop='static'>
      <div class="modal-dialog" style="width: 100%; height: 100%; padding: 10px; margin: 0px">
        <div class="modal-content" style="height: 100%" id="GraphDialogContents">
          <div class="modal-header">
            <div class="pull-left">
            <h3 class="modal-title" id="GraphDialogTitle"></h3>
            </div>
            <div class="pull-right">
              <button type="button" class="btn btn-default btn-sm" id="GraphDialogClose">
                <i class="fa fa-compress"></i>
              </button>
            </div>
            <div class="clearfix"></div>
          </div>
          <div class="modal-body" id="GraphDialogBody" style="padding-top: 0px"></div>
        </div>
      </div>
    </div>
    <div id="wrapper">
      <div id="page-wrapper">
        <div class="row">
          <div class="col-lg-12">
            <div class="panel panel-info" id="EnergyPanel">
              <div class="panel-heading">
                <span class="huge-small">Energy consumption</span>
                <div class="pull-right">
                  <span class="small">Time scale:</span><br>
                  <div class="btn-group" data-toggle="buttons" id="energybtnA">
                    <label class="btn btn-default btn-sm" id="energy_year" data-toggle="tooltip" title="Zoom the graph to one year, keeping current aggregation level while possible">
                      <input type="radio" name="energy_chart_range" value="year"/>Year
                    </label>
                    <label class="btn btn-default btn-sm" id="energy_month" data-toggle="tooltip" title="Zoom the graph to one month, keeping current aggregation level while possible">
                      <input type="radio" name="energy_chart_range" value="month"/>Month
                    </label>
                    <label class="btn btn-default btn-sm" id="energy_week" data-toggle="tooltip" title="Zoom the graph to one week, keeping current aggregation level while possible">
                      <input type="radio" name="energy_chart_range" value="week"/>Week
                    </label>
                    <label class="btn btn-default btn-sm" id="energy_day" data-toggle="tooltip" title="Zoom the graph to one day, keeping current aggregation level while possible">
                      <input type="radio" name="energy_chart_range" value="day"/>Day
                    </label>
                  </div>
                  <div class="btn-group" data-toggle="buttons" id="energybtnC">
                    <label class="btn btn-default btn-sm active" id="energynow" data-toggle="tooltip" title="Move to most recent samples and update in real time.">
                      <input type="checkbox" checked="checked"/><i class="fa fa-chevron-right"></i>
                    </label>
                  </div>
                  <div class="btn-group">
                    <button type="button" class="btn btn-default btn-sm disabled" id="maximizeEnergy">
                      <i class="fa fa-expand"></i>
                    </button>
                  </div>
                </div>
                <div class="pull-right">&nbsp;</div>
                <div class="pull-right">
                  <span class="small">Aggregation scale:</span><br>
                  <div class="btn-group" data-toggle="buttons" id="energybtnB">
                    <label class="btn btn-default btn-sm" id="energy_3s" data-toggle="tooltip" title="Fetch graph from 3 seconds aggregation">
                      <input type="radio" name="energy_chart_range" value="3s"/>3 Seg
                    </label>
                    <label class="btn btn-default btn-sm" id="energy_1m" data-toggle="tooltip" title="Fetch graph from 1 minute aggregation">
                      <input type="radio" name="energy_chart_range" value="1m"/>1 Min
                    </label>
                    <label class="btn btn-default btn-sm" id="energy_10m" data-toggle="tooltip" title="Fetch graph from 10 minutes aggregation">
                      <input type="radio" name="energy_chart_range" value="10m"/>10 Min
                    </label>
                    <label class="btn btn-default btn-sm" id="energy_1h" data-toggle="tooltip" title="Fetch graph from 1 hour aggregation">
                      <input type="radio" name="energy_chart_range" value="1h"/>1 Hour
                    </label>
                  </div>
                </div>
                <span class="clearfix"> </span>
              </div>
              <div class="row">
                <div class="col-md-6 col-lg-3">
                  <div class="row">
                    <div class="col-xs-6 col-md-12">
                      <div class="panel panel-lightblue-info" id="EnergyBox">
                        <h4 id="EnergyPeriod">Today</h4>
                        <span id="energy_integer" class="huge">?</span>
                        <span id="energy_decimal" class="huge-small"></span>&nbsp;<em class="huge-small-unit" id="EnergyUnits">kWh</em>
                        <p><span id="PrevEnergyPeriod">Yesterday</span>&nbsp;<span id="PrevEnergy">?</span>&nbsp;<em class="small" id="PrevEnergyUnits">kWh</em></p>
                      </div>
                    </div>
                    <div class="col-xs-6 col-md-12">
                      <div class="panel panel-lightblue-info" id="BudgetBox">
                        <h4><span id="BudgetPeriod">Today</span> budget</h4>
                        <span id="budget_integer" class="huge">?</span>
                        <span id="budget_decimal" class="huge-small"></span>&nbsp;<em class="huge-small-unit">€</em>
                        <p>Up to <span id="budget_lastcalc">?</span></p>
                      </div>
                    </div>
                  </div>
                </div>
                <div class="col-md-6 col-lg-9" id="EnergyCol">
                  <div id="EnergyMaximizeDiv">
                    <div style="padding:5px">
                      <div style="display:inline-block" id="EnergyAggregation">Loading data...</div>
                      <div id="EnergyLegend" style="float: right"></div>
                    </div>
                    <div id="energy_chart" class="chart big_chart"></div>
                  </div>
                </div>
              </div>
            </div>
          </div>
          <!-- /.col-lg-12 -->
        </div>
        <!-- /.row -->
        <div class="row">
          <div class="col-md-6">
            <div class="panel panel-red" id="VoltagePanel">
              <div class="panel-heading big-heading">
                <div class="row">
                  <div class="col-xs-6 col-md-2 col-lg-4">
                    <span class="huge">V</span><span class="huge-small hidden-md">oltage</span>
                  </div>
                  <div class="col-xs-6  col-md-10 col-lg-8 text-right">
                    <span id="voltage_now_integer" class="huge">?</span>
                    <span id="voltage_now_decimal" class="huge-small"></span>
                    <em class="huge-small-unit">V</em>
                  </div>
                </div>
              </div>
              <div class="panel-body no-padding" id="VoltageCol">
                <div id="VoltageMaximizeDiv">
                  <div style="padding:5px">
                    <div style="display:inline-block" id="VoltageAggregation">Loading data...</div>
                    <div id="VoltageLegend" style="float: right"></div>
                  </div>
                  <div id="voltage_chart" class="chart"></div>
                </div>
              </div>
              <div class="panel-footer">
                <a href="<?= site_url('analysis/voltage') ?>">
                  <span class="pull-left">View Details  <i class="fa fa-arrow-circle-right"></i></span>
                </a>
                  <button type="button" class="btn btn-default btn-xs pull-right" id="maximizeVoltage">
                    <i class="fa fa-expand"></i>
                  </button>
                <div class="clearfix"></div>
              </div>
            </div>
          </div>
          <div class="col-md-6">
            <div class="panel panel-primary" id="CurrentPanel">
              <div class="panel-heading big-heading">
                <div class="row">
                  <div class="col-xs-6 col-md-2 col-lg-4">
                    <span class="huge">C</span><span class="huge-small hidden-md">urrent</span>
                  </div>
                  <div class="col-xs-6  col-md-10 col-lg-8 text-right">
                    <span id="current_now_integer"  class="huge">?</span>
                    <span id="current_now_decimal" class="huge-small"></span>
                    <em class="huge-small-unit">A</em>
                  </div>
                </div>
              </div>
              <div class="panel-body no-padding" id="CurrentCol">
                <div id="CurrentMaximizeDiv">
                  <div style="padding:5px">
                    <div style="display:inline-block" id="CurrentAggregation">Loading data...</div>
                    <div id="CurrentLegend" style="float: right"></div>
                  </div>
                  <div id="current_chart" class="chart"></div>
                </div>
              </div>
              <div class="panel-footer">
                <a href="<?= site_url('analysis/current') ?>">
                  <span class="pull-left">View Details  <i class="fa fa-arrow-circle-right"></i></span>
                </a>
                <button type="button" class="btn btn-default btn-xs pull-right" id="maximizeCurrent">
                  <i class="fa fa-expand"></i>
                </button>
                <div class="clearfix"></div>
              </div>
            </div>
          </div>
        </div>
        <div class="row">
          <div class="col-md-6">
            <div class="panel panel-yellow" id="PowerPanel">
              <div class="panel-heading big-heading">
                <div class="row">
                  <div class="col-xs-6 col-md-2 col-lg-6">
                    <span class="visible-sm-inline-block huge">P</span><span class="hidden-sm huge">A</span><span class="hidden-sm huge-small">ctive&nbsp;power</span>
                  </div>
                  <div class="col-xs-6  col-md-10 col-lg-6 text-right">
                    <span id="power_now_integer" class="huge">?</span>
                    <span id="power_now_decimal" class="huge-small"></span>
                    <em class="huge-small-unit" id="power_units">W</em>
                  </div>
                </div>
              </div>
              <div class="panel-body no-padding" id="PowerCol">
                <div id="PowerMaximizeDiv">
                  <div style="padding:5px">
                    <div style="display:inline-block" id="PowerAggregation">Loading data...</div>
                    <div id="PowerLegend" style="float: right"></div>
                  </div>
                  <div id="power_chart" class="chart"></div>
                </div>
              </div>
              <div class="panel-footer">
                <a href="<?= site_url('analysis/energy') ?>">
                  <span class="pull-left">View Details  <i class="fa fa-arrow-circle-right"></i></span>
                </a>
                <button type="button" class="btn btn-default btn-xs pull-right" id="maximizePower">
                  <i class="fa fa-expand"></i>
                </button>
                <div class="clearfix"></div>
              </div>
            </div>
          </div>
          <div class="col-md-6">
            <div class="panel panel-green" id="FrequencyPanel">
              <div class="panel-heading big-heading">
                <div class="row">
                  <div class="col-xs-6 col-md-2 col-lg-4">
                    <span class="huge">F</span><span class="huge-small hidden-md">requency</span>
                  </div>
                  <div class="col-xs-6  col-md-10 col-lg-8 text-right">
                    <span id="frequency_now_integer" class="huge">?</span>
                    <span id="frequency_now_decimal" class="huge-small"></span>
                    <em class="huge-small-unit">Hz</em>
                  </div>
                </div>
              </div>
              <div class="panel-body no-padding" id="FrequencyCol">
                <div id="FrequencyMaximizeDiv">
                  <div style="padding:5px">
                    <div style="display:inline-block" id="FrequencyAggregation">Loading data...</div>
                    <div id="FrequencyLegend" style="float: right"></div>
                  </div>
                  <div id="frequency_chart" class="chart"></div>
                </div>
              </div>
              <div class="panel-footer">
                <a href="<?= site_url('analysis/frequency') ?>">
                  <span class="pull-left">View Details  <i class="fa fa-arrow-circle-right"></i></span>
                </a>
                <button type="button" class="btn btn-default btn-xs pull-right" id="maximizeFrequency">
                  <i class="fa fa-expand"></i>
                </button>
                <div class="clearfix"></div>
              </div>
            </div>
          </div>
        </div>
        <!-- /.row -->
        <div class="row">
          <div class="col-lg-12">
            <div class="panel panel-default" id="MainParamPanel">
              <div class="panel-heading">
                <i class="fa fa-area-chart fa-fw"></i> Main parameters
              </div>
              <div id="EvolutionPanel" style="padding: 5px; min-height: 340px"></div>
            </div>
          </div>
        </div>
        <!-- /.row -->
      </div>
      <!-- /#page-wrapper -->
    </div>
    <!-- /#wrapper -->
    <? $this->load->view('template/commonScripts') ?>
    <script src="lib/spin.js/spin.min.js"></script>
    <script src="lib/dygraph/chart-plotter.js"></script>
    <script src="lib/dygraph/crosshair.js"></script>
    <script src="lib/dygraph/synchronize.js"></script>
    <script src="js/dashboard.js"></script>
  </body>
</html>