<!DOCTYPE html>
<html lang="en">
  <head>
    <? $this->load->view('template/commonHead', array('title' => 'Analysis | Frequency')) ?>
    <link rel="stylesheet" href="lib/datatables/css/dataTables.responsive.css" />
    <link rel="stylesheet" href="lib/datatables/css/dataTables.bootstrap.css" />
  </head>
  <body>
    <? $this->load->view('template/header') ?>
    <div id="wrapper">
      <div id="page-wrapper">
        <div class="row">
          <div class="col-lg-12">
            <div class="panel panel-green">
              <div class="panel-heading">
                <span class="huge-small">Frequency</span>
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
                  <div class="btn-group">
                    <div class="maximized_panel">
                      <button type="button" class="btn btn-default btn-sm" id="energynow" data-toggle="tooltip" title="Go to most recent">
                        <i class="fa fa-chevron-right"></i>
                      </button>
                      <button type="button" class="btn btn-default btn-sm maximize">
                        <i class="fa fa-expand"></i>
                      </button>
                      <button style="display:none;" type="button" class="btn btn-default btn-sm minimize">
                        <i class="fa fa-compress"></i>
                      </button>
                    </div>
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
              <div class="panel-body">
                <div class="row">
                  <div class="col-md-4 col-lg-4">
                    <div class="row">
                      <div class="col-xs-6 col-md-12 text-center">
                        <h4>Mean RMS:</h4>
                        <span class="huge"><?= (int) $frequency->today ?></span><span class="huge-small">.<?= ltrim(round($frequency->today - (int) $frequency->today, 2), "0.") ?></span> <em class="huge-small-unit">Hz</em>
                        <hr>
                      </div>
                      <div class="col-xs-3 col-md-7 text-center">
                        <span class="huge-small"><?= round($frequency->max_today, 2) ?></span> <em class="huge-small-unit">Hz</em>
                        <h4>max</h4>
                      </div>
                      <div class="col-xs-3 col-md-5 text-center">
                        <span class="huge-small"><?= round($frequency->min_today, 2) ?></span> <em class="huge-small-unit">Hz</em>
                        <h4>min</h4>
                      </div>
                    </div>
                  </div>
                  <div class="col col-md-8 col-lg-8">
                    <div id="frequency_chart" class="chart big_chart"></div>
                  </div>
                </div>
              </div>

            </div>
          </div>
          <!-- /.col-lg-12 -->
        </div>
      </div>
      <!-- /#page-wrapper -->
    </div>
    <!-- /#wrapper -->
    <script>
      frequency =<?= json_encode($frequency) ?>;
      events =<?= json_encode($events) ?>;
    </script>
    <? $this->load->view('template/commonScripts') ?>

    <script src="js/zTicker.js"></script>
    <script src="lib/dygraph/chart-plotter.js"></script>
    <script src="lib/spin.js/spin.min.js"></script>
    <script src="lib/datatables/js/jquery.dataTables.min.js"></script>
    <script src="lib/datatables/js/dataTables.responsive.min.js"></script>
    <script src="lib/datatables/js/dataTables.bootstrap.js"></script>

    <script src="js/analysis/analysis.js"></script>
    <script src="js/analysis/frequency.js"></script>

  </body>
</html>