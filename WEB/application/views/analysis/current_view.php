<!DOCTYPE html>
<html lang="en">
  <head>
    <? $this->load->view('template/commonHead', array('title' => 'Analysis | Current')) ?>
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
            <div class="panel panel-primary" id="CurrentPanel">
              <div class="panel-heading">
                <span class="huge-small">Current</span>
                <div class="pull-right">
                  <span class="small">Time scale:</span><br>
                  <div class="btn-group" data-toggle="buttons">
                    <label class="btn btn-default btn-sm" id="current_year" data-toggle="tooltip" title="Zoom the graph to one year, keeping current aggregation level while possible">
                      <input type="radio" name="current_chart_range" value="year"/>Year
                    </label>
                    <label class="btn btn-default btn-sm" id="current_month" data-toggle="tooltip" title="Zoom the graph to one month, keeping current aggregation level while possible">
                      <input type="radio" name="current_chart_range" value="month"/>Month
                    </label>
                    <label class="btn btn-default btn-sm" id="current_week" data-toggle="tooltip" title="Zoom the graph to one week, keeping current aggregation level while possible">
                      <input type="radio" name="current_chart_range" value="week"/>Week
                    </label>
                    <label class="btn btn-default btn-sm" id="current_day" data-toggle="tooltip" title="Zoom the graph to one day, keeping current aggregation level while possible">
                      <input type="radio" name="current_chart_range" value="day"/>Day
                    </label>
                  </div>
                  <div class="btn-group" data-toggle="buttons">
                    <label class="btn btn-default btn-sm active" id="currentnow" data-toggle="tooltip" title="Move to most recent samples and update in real time.">
                      <input type="checkbox" name="current_chart_range" value="now"/><i class="fa fa-chevron-right"></i>
                    </label>
                  </div>
                  <div class="btn-group">
                    <button type="button" class="btn btn-default btn-sm disabled" id="maximizeCurrent">
                      <i class="fa fa-expand"></i>
                    </button>
                  </div>
                </div>
                <div class="pull-right">&nbsp;</div>
                <div class="pull-right">
                  <span class="small">Aggregation scale:</span><br>
                  <div class="btn-group" data-toggle="buttons">
                    <label class="btn btn-default btn-sm" id="current_3s" data-toggle="tooltip" title="Fetch graph from 3 seconds aggregation">
                      <input type="radio" name="current_chart_range" value="3s"/>3 Seg
                    </label>
                    <label class="btn btn-default btn-sm" id="current_1m" data-toggle="tooltip" title="Fetch graph from 1 minute aggregation">
                      <input type="radio" name="current_chart_range" value="1m"/>1 Min
                    </label>
                    <label class="btn btn-default btn-sm" id="current_10m" data-toggle="tooltip" title="Fetch graph from 10 minutes aggregation">
                      <input type="radio" name="current_chart_range" value="10m"/>10 Min
                    </label>
                    <label class="btn btn-default btn-sm" id="current_1h" data-toggle="tooltip" title="Fetch graph from 1 hour aggregation">
                      <input type="radio" name="current_chart_range" value="1h"/>1 Hour
                    </label>
                  </div>
                </div>
                <span class="clearfix"> </span>
              </div>
              <div class="row">
                <div class="col-md-6 col-lg-3">
                  <div class="row">
                    <div class="col-xs-4 col-md-12">
                      <div class="panel panel-lightblue2-info" id="PanelAvg">
                        <h4>Last hour RMS</h4>
                        <span class="huge-small-unit" id="current_hour">?</span><em class="huge-small-unit">&nbsp;A</em>
                        <p>24 hours RMS&nbsp;<span id="current_last_day">?</span><em class="small">&nbsp;A</em></p>
                      </div>
                    </div>
                    <div class="col-xs-4 col-md-12">
                      <div class="panel panel-lightblue2-info" id="PanelMin">
                        <h4>Last hour min</h4>
                        <span class="huge-small-unit" id="current_hour_min">?</span><em class="huge-small-unit">&nbsp;A</em><br/>
                        <p>24 hours min&nbsp;<span id="current_min_day">?</span><em class="small">&nbsp;A</em></p>
                      </div>
                    </div>
                    <div class="col-xs-4 col-md-12">
                      <div class="panel panel-lightblue2-info" id="PanelMax">
                        <h4>Last hour max</h4>  
                        <span class="huge-small-unit" id="current_hour_max">?</span><em class="huge-small-unit">&nbsp;A</em><br/>
                        <p>24 hours max&nbsp;<span id="current_max_day">?</span><em class="small">&nbsp;A</em></p>
                      </div>
                    </div>
                  </div>
                </div>
                <div class="col-md-6 col-lg-9" id="CurrentCol">
                  <div id="CurrentMaximizeDiv">
                    <div style="padding:5px">
                      <div style="display:inline-block" id="CurrentAggregation">Loading data...</div>
                      <div id="CurrentLegend" style="float: right"></div>
                    </div>
                    <div id="current_chart" class="chart big_chart"></div>
                  </div>
                </div>
              </div>
            </div>
          </div>
          <!-- /.col-lg-12 -->
        </div>
        <!-- /.row -->
        <div class="row">
          <div class="col-lg-12">
            <div class="panel panel-default" id="DetailPanel">
              <div class="panel-heading">
                <i class="fa fa-area-chart fa-fw"></i>Details
                <div id="LowWarning" class="title-warning hidden"><i class="fa fa-exclamation-triangle"></i> No current or current too low</div>
                <div class="pull-right">
                  <div class="btn-group" data-toggle="buttons">
                    <label class="btn btn-default btn-sm active">
                      <input type="radio" name="detail_change" value="waveform" checked="checked"/>Waveform
                    </label>
                    <label class="btn btn-default btn-sm">
                      <input type="radio" name="detail_change" value="harmonics"/>Harmonic analysis
                    </label>
                    <label class="btn btn-default btn-sm disabled" id="HarmonicEvolutionBtn">
                      <input type="radio" name="detail_change" value="evolution"/><span id="HarmonicEvolutionLoad"><img src="img/load.gif" alt="Loading..."/> </span>Harmonic evolution
                    </label>
                    <label class="btn btn-default btn-sm">
                      <input type="radio" name="detail_change" value="fft"/>Advanced
                    </label>
                  </div>
                  <div class="btn-group" data-toggle="buttons">
                    <label class="btn btn-default btn-sm" id="btnPause">
                      <input class="live" type="checkbox"><span class="live-pause"><i class="fa fa-pause fa-fw"></i> Pause</span> <span class="live-resume" style="display:none"><i class="fa fa-play fa-fw"></i> Resume</span>
                    </label>
                    <label class="btn btn-default btn-sm disabled" id="btnFundamental">
                      <input class="live" type="checkbox">Hide Fundamental
                    </label>
                    <label class="btn btn-default btn-sm" id="btnVoltage">
                      <input class="live" type="checkbox">Show voltage
                    </label>
                  </div>
                  <div class="btn-group">
                    <button type="button" class="btn btn-default btn-sm" id="btnExport">
                      <i class="fa fa-table fa-fw"></i> Export data
                    </button>
                  </div>
                  <div class="btn-group">
                    <div class="maximized_panel">
                      <button type="button" class="btn btn-default btn-sm maximize">
                        <i class="fa fa-expand"></i>
                      </button>
                      <button style="display:none;" type="button" class="btn btn-default btn-sm minimize">
                        <i class="fa fa-compress"></i>
                      </button>
                    </div>
                  </div>
                </div>
                <span class="clearfix"> </span>
              </div>
              <div class="panel-body" id="Tabs" style="padding:5px">
                <div class="no-padding" id="WaveformPanel">
                  <div style="padding:5px">
                    <div style="display:inline-block" id="WaveformTime">Loading data...</div>
                    <div id="WaveformLegend" style="float: right"></div>
                  </div>
                  <div id="waveform_chart" class="chart big_chart"></div>
                </div>
                <div class="hidden" id="FFTPanel">
                  <div style="padding:5px">
                    <div style="display:inline-block" id="FFTTime">Loading data...</div>
                    <div id="FFTLegend" style="float: right"></div>
                  </div>
                  <div id="fft_chart" class="chart big_chart"></div>
                </div>
                <div class="hidden" id="HarmonicsPanel">
                  <div style="padding:5px">
                    <div style="display:inline-block" id="HarmonicsTime">Loading data...</div>
                    <div id="HarmonicsLegend" style="float: right"></div>
                  </div> 
                  <div id="harmonics_chart" class="chart big_chart"></div>
                </div>
                <div class="hidden" id="EvolutionPanel">
                </div>
              </div>
            </div>
          </div>
        </div>
        <!-- /.row -->
      </div>
      <!-- /#page-wrapper -->
    </div>
    <!-- /#wrapper -->
    <? $this->load->view('template/commonScripts') ?>
    <script>events =<?= json_encode($events) ?>;</script>
    <script src="lib/dygraph/crosshair.js"></script>
    <script src="lib/dygraph/synchronize.js"></script>
    <script src="lib/dygraph/chart-plotter.js"></script>
    <script src="lib/spin.js/spin.min.js"></script>
    <script src="js/analysis/analysis.js"></script>
    <script src="js/analysis/current.js"></script>
  </body>
</html>