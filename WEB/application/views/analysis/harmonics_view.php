<!DOCTYPE html>
<html lang="en">
    <head>
        <? $this->load->view('template/commonHead', array('title' => 'Analysis | Harmonics')) ?>
        <link rel="stylesheet" href="lib/datatables/css/dataTables.responsive.css" />
        <link rel="stylesheet" href="lib/datatables/css/dataTables.bootstrap.css" />
    </head>
    <body>
        <? $this->load->view('template/header') ?>
        <div id="wrapper">
            <div id="page-wrapper">
                <div class="row">
                    <div class="col-lg-12">
                        <span class="h2">Analysis</span>
                        <div class="pull-right">
                            <div class="btn-group analysis_sections">
                                <a class="btn btn-default btn-lg" href="<?= site_url('analysis/voltage') ?>">Voltage</a>
                                <a class="btn btn-default btn-lg" href="<?= site_url('analysis/current') ?>">Current</a>
                                <a class="btn btn-default btn-lg" href="<?= site_url('analysis/energy') ?>">Energy/Power</a>
                                <a class="btn btn-default btn-lg" href="<?= site_url('analysis/harmonics') ?>">Harmonics</a>
                                <a class="btn btn-default btn-lg" href="<?= site_url('analysis/frequency') ?>">Frequency</a>
                            </div>
                        </div>
                        <hr>
                    </div>
                </div>
                <div class="row">
                    <div class="col-lg-12">
                        <div class="panel panel-purple">
                            <div class="panel-heading">
                                <i class="fa fa-bar-chart fa-fw"></i> Harmonics
                                <div class="pull-right">
                                    <strong>Time:</strong> <span class="harmonics_time">Retrieving data...</span>
                                    <div class="btn-group" data-toggle="buttons">
                                        <label class="btn btn-default btn-sm active">
                                            <input type="radio" name="harmonics_chart_range" value="3s" checked/> 3s
                                        </label>
                                        <label class="btn btn-default btn-sm ">
                                            <input type="radio" name="harmonics_chart_range" value="10s"  /> 10s
                                        </label>
                                        <label class="btn btn-default btn-sm ">
                                            <input type="radio" name="harmonics_chart_range" value="11m" /> 1m
                                        </label>
                                    </div>
                                    <button type="button" class="btn btn-sm btn-default hideFundamental" data-toggle="button" aria-pressed="false" autocomplete="off">
                                        Hide fundamental
                                    </button>
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
                            </div>
                            <div class="panel-body">
                                <div class="row">
                                    <div class="col col-md-12" style="margin-bottom:15px">
<!--                                        <strong>Legend:</strong> -->
                                        <span><i class="legend_item voltage"></i> Voltage</span> <span><i class="legend_item current"></i> Current</span>
                                        <span class="label label-warning no-current" style="display:none;margin-left: 20px;"><strong>No current or current too low</strong></span>
                                        <!--                                    </div>
                                                                            <div class="col col-md-4">-->
                                        <span class="gray"><strong>Power factor:</strong> <span class="power_factor">?</span></span>
                                        <span class="gray"><strong>PHI:</strong> <span class="phi_angle">?</span></span>
                                        <!--                                    </div>
                                                                            <div class="col col-md-2">-->
                                        <span class="gray"><strong>THD<sub>V</sub>:</strong> <span class="thd_u">?</span> %</span>
                                        <!--                                    </div>
                                                                            <div class="col col-md-2">-->
                                        <span class="gray"><strong>THD<sub>I</sub>:</strong> <span class="thd_i">?</span> %</span>

                                    </div>
                                    <div class="col col-md-12">
                                        <div id="harmonics_chart" class="chart big_chart"></div>
                                    </div>
                                </div>
                                <!-- br -->

                            </div>

                        </div>
                    </div>
                    <!-- /.col-lg-12 -->
                </div>
                <!-- /.row -->
            </div>
            <!-- /#page-wrapper -->

        </div>
        <!-- /#wrapper -->
        <? $this->load->view('template/commonScripts') ?>

        <script src="lib/dygraph/chart-plotter.js"></script>
        <script src="lib/spin.js/spin.min.js"></script>
        <script src="lib/datatables/js/jquery.dataTables.min.js"></script>
        <script src="lib/datatables/js/dataTables.responsive.min.js"></script>
        <script src="lib/datatables/js/dataTables.bootstrap.js"></script>

        <script src="js/analysis/analysis.js"></script>
        <script src="js/analysis/harmonics.js"></script>

    </body>
</html>