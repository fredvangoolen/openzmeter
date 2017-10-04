<!DOCTYPE html>
<html lang="en">
  <head>
    <? $this->load->view('template/commonHead', array('title' => 'Analysis | Energy/Power')) ?>
    <link rel="stylesheet" href="lib/datatables/css/dataTables.responsive.css" />
    <link rel="stylesheet" href="lib/datatables/css/dataTables.bootstrap.css" />
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
                <span class="huge-small" id="EnergyPowerTitle">Energy</span>
                <div class="pull-right">
                  <span class="small">Time scale:</span><br>
                  <div class="btn-group" data-toggle="buttons">
                    <label class="btn btn-default btn-sm" id="energy_year" data-toggle="tooltip" title="Zoom the graph to one year, keeping current aggregation level while possible">
                      <input type="radio" name="chart_range" value="year"/>Year
                    </label>
                    <label class="btn btn-default btn-sm" id="energy_month" data-toggle="tooltip" title="Zoom the graph to one month, keeping current aggregation level while possible">
                      <input type="radio" name="chart_range" value="month"/>Month
                    </label>
                    <label class="btn btn-default btn-sm" id="energy_week" data-toggle="tooltip" title="Zoom the graph to one week, keeping current aggregation level while possible">
                      <input type="radio" name="chart_range" value="week"/>Week
                    </label>
                    <label class="btn btn-default btn-sm" id="energy_day" data-toggle="tooltip" title="Zoom the graph to one day, keeping current aggregation level while possible">
                      <input type="radio" name="chart_range" value="day"/>Day
                    </label>
                  </div>
                  <div class="btn-group" data-toggle="buttons">
                    <label class="btn btn-default btn-sm active" id="energynow" data-toggle="tooltip" title="Move to most recent samples and update in real time.">
                      <input type="checkbox" name="chart_range" value="now"/><i class="fa fa-chevron-right"></i>
                    </label>
                  </div>
                </div>
                <div class="pull-right">&nbsp;</div>
                <div class="pull-right">
                  <span class="small">Aggregation scale:</span><br>
                  <div class="btn-group" data-toggle="buttons">
                    <label class="btn btn-default btn-sm" id="energy_3s" data-toggle="tooltip" title="Fetch graph from 3 seconds aggregation">
                      <input type="radio" name="chart_range" value="3s"/>3 Seg
                    </label>
                    <label class="btn btn-default btn-sm" id="energy_1m" data-toggle="tooltip" title="Fetch graph from 1 minute aggregation">
                      <input type="radio" name="chart_range" value="1m"/>1 Min
                    </label>
                    <label class="btn btn-default btn-sm" id="energy_10m" data-toggle="tooltip" title="Fetch graph from 10 minutes aggregation">
                      <input type="radio" name="chart_range" value="10m"/>10 Min
                    </label>
                    <label class="btn btn-default btn-sm" id="energy_1h" data-toggle="tooltip" title="Fetch graph from 1 hour aggregation">
                      <input type="radio" name="chart_range" value="1h"/>1 Hour
                    </label>
                  </div>
                </div>
                <div class="pull-right">&nbsp;</div>
                <div class="pull-right">
                  <span class="small">&nbsp;</span><br>
                  <div class="btn-group" data-toggle="buttons">
                    <label class="btn btn-default btn-sm active">
                      <input type="radio" name="chart_range" value="energy" checked="checked"/> Energy
                    </label>
                    <label class="btn btn-default btn-sm">
                      <input type="radio" name="chart_range" value="power"/> Power
                    </label>
                  </div>
                </div>
                <span class="clearfix"> </span> 
              </div>
              <div id="EnergySubPanel">
                <div class="panel-body no-padding panel-bigheader">
                  <div class="panel-info panel-heading">
                    <div style="display:inline-block"><i class="fa fa-area-chart fa-fw"></i>Active energy</div>
                    <div class="pull-right btn-group">
                      <button type="button" class="btn btn-default btn-sm" id="maximizeActiveEnergy">
                        <i class="fa fa-expand"></i>
                      </button>
                    </div>
                  </div>
                  <div id="ActiveEnergyCol">
                    <div id="ActiveEnergyMaximizeDiv">
                      <div style="padding:5px">
                        <div style="display:inline-block" id="ActiveEnergyAggregation">Loading data...</div>
                        <div id="ActiveEnergyLegend" style="float: right"></div>
                      </div>
                      <div id="active_energy_chart" class="chart big_chart"></div>
                    </div>
                  </div>
                </div>
                <div class="panel-body no-padding panel-bigheader">
                  <div class="panel-info panel-heading no-padding"></div>
                  <div class="panel-info panel-heading">
                    <div style="display:inline-block"><i class="fa fa-area-chart fa-fw"></i>Reactive energy</div>
                    <div class="pull-right btn-group">
                      <button type="button" class="btn btn-default btn-sm" id="maximizeReactiveEnergy">
                        <i class="fa fa-expand"></i>
                      </button>
                    </div>
                  </div>
                  <div id="ReactiveEnergyCol">
                    <div id="ReactiveEnergyMaximizeDiv">
                      <div style="padding:5px">
                        <div style="display:inline-block" id="ReactiveEnergyAggregation">Loading data...</div>
                        <div id="ReactiveEnergyLegend" style="float: right"></div>
                      </div>
                      <div id="reactive_energy_chart" class="chart big_chart"></div>
                    </div>
                  </div>
                </div>
              </div>
              <div id="PowerSubPanel" class="hidden">
                <div class="panel-body no-padding panel-bigheader">
                  <div class="panel-info panel-heading">
                    <div style="display:inline-block"><i class="fa fa-area-chart fa-fw"></i>Active power</div>
                    <div class="pull-right btn-group">
                      <button type="button" class="btn btn-default btn-sm" id="maximizeActivePower">
                        <i class="fa fa-expand"></i>
                      </button>
                    </div>
                  </div>
                  <div id="ActivePowerCol">
                    <div id="ActivePowerMaximizeDiv">
                      <div style="padding:5px">
                        <div style="display:inline-block" id="ActivePowerAggregation">Loading data...</div>
                        <div id="ActivePowerLegend" style="float: right"></div>
                      </div>
                      <div id="active_power_chart" class="chart big_chart"></div>
                    </div>
                  </div>
                </div>
                <div class="panel-body no-padding panel-bigheader">
                  <div class="panel-info panel-heading no-padding"></div>
                  <div class="panel-info panel-heading">
                    <div style="display:inline-block"><i class="fa fa-area-chart fa-fw"></i>Reactive power</div>
                    <div class="pull-right btn-group">
                      <button type="button" class="btn btn-default btn-sm" id="maximizeReactivePower">
                        <i class="fa fa-expand"></i>
                      </button>
                    </div>
                  </div>
                  <div id="ReactivePowerCol">
                    <div id="ReactivePowerMaximizeDiv">
                      <div style="padding:5px">
                        <div style="display:inline-block" id="ReactivePowerAggregation">Loading data...</div>
                        <div id="ReactivePowerLegend" style="float: right"></div>
                      </div>
                      <div id="reactive_power_chart" class="chart big_chart"></div>
                    </div>
                  </div>
                </div>
              </div>
            </div> 
          </div>
        </div>
      </div>
    </div>
    <!-- /#wrapper -->
    <? $this->load->view('template/commonScripts') ?>
    <script src="lib/dygraph/chart-plotter.js"></script>
    <script src="lib/dygraph/synchronize.js"></script>
    <script src="lib/spin.js/spin.min.js"></script>
    <script src="lib/datatables/js/jquery.dataTables.min.js"></script>
    <script src="lib/datatables/js/dataTables.responsive.min.js"></script>
    <script src="lib/datatables/js/dataTables.bootstrap.js"></script>

    <script src="js/analysis/analysis.js"></script>
    <script src="js/analysis/energy_power.js"></script>

  </body>
</html>