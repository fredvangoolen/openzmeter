<!DOCTYPE html>
<html lang="en">
    <head>
        <? $this->load->view('template/commonHead', array('title' => 'Events | ITIC')) ?>
        <link href="lib/dc.js/dc.css" rel="stylesheet">
        <link href="lib/d3-tip/d3-tip.min.css" rel="stylesheet">
        <link href="lib/d3/colorbrewer.css" rel="stylesheet">
    </head>

    <body>
        <? $this->load->view('template/header') ?>
        <div id="wrapper">
            <div id="page-wrapper">
                <div class="row">
                    <div class="col-lg-12">
                        <span class="h2">Events</span>
                        <div class="pull-right">
                            <div class="btn-group events_sections">
                                <a class="btn btn-default btn-lg" href="<?= site_url('events/itic') ?>">ITIC</a>
                                <a class="btn btn-default btn-lg" href="<?= site_url('events/disturbances') ?>">Disturbances</a>
                            </div>
                        </div>
                        <hr>
                    </div>
                </div>
                <div class="row">
                    <div class="col-md-6">
                        <h3 class="text-center">ITI (CBEMA) Curve <small>(Revised 2000)</small>
                        </h3>
                        <div id='itic_chart'>
                        </div>
                    </div>
                    <div class="col-md-6">
                        <h3 class="text-center">Number of events
                            <div class="pull-right">
                                <a class="btn btn-sm btn-primary" href="javascript:resetCharts();"><i class="fa fa-bolt fa-eraser"></i>  Reset All</a>
                            </div>
                        </h3>
                        <div class="row">
                            <div class="col-md-6">
                                <h4 class="text-center">By % RMS <small>(greater than)</small></h4>
                                <div id='rms_chart'></div>
                            </div>
                            <div class="col-md-6">
                                <h4 class="text-center">By > duration <small>(greater than)</small></h4>
                                <div id='duration_chart'></div>
                            </div>
                            <div class="col-md-12">
                                <h4 class="text-center">By date
                                    <div class="pull-right">
                                        <div class="btn-group" data-toggle="buttons">
                                            <label class="btn btn-default btn-sm">
                                                <input type="radio" name="date_chart_range" value="today"/> Today
                                            </label>
                                            <label class="btn btn-default btn-sm">
                                                <input type="radio" name="date_chart_range" value="yesterday"/> Yesterday
                                            </label>
                                            <label class="btn btn-default btn-sm active">
                                                <input type="radio" name="date_chart_range" value="week"  checked="checked"/> This week
                                            </label>
                                            <label class="btn btn-default btn-sm">
                                                <input type="radio" name="date_chart_range" value="month"/> This month
                                            </label>
                                        </div>
                                    </div>
                                </h4>
                                <div id='date_chart'></div>
                                <div class="chart-range" id='date_selector_chart'></div>
                            </div>
                            <div class="col-md-4">
                                <h4 class="text-center">By type</h4>
                                <div id='type_chart'></div>
                            </div>
                            <!-- div class="col-md-4">
                                <h4 class="text-center">By day of month</h4>
                                <div id='month_chart'></div>
                            </div -->
                            <div class="col-md-4">
                                <h4 class="text-center">By day of week</h4>
                                <div id='weekday_chart'></div>
                            </div>
                        </div>
                    </div>
                </div>
                <!-- /.row -->
            </div>
            <!-- /#page-wrapper -->

        </div>
        <!-- /#wrapper -->
        <script>
            events =<?= json_encode($events) ?>;
        </script>
        <? $this->load->view('template/commonScripts') ?>

        <script src="lib/d3/d3.v3.min.js"></script>
        <script src="lib/d3/colorbrewer.js"></script>
        <script src="lib/dc.js/crossfilter.js"></script>
        <script src="lib/dc.js/dc.min.js"></script>
        <script src="lib/d3-tip/d3-tip.min.js"></script>
        <script src="js/events/events.js"></script>
        <script src="js/events/itic.js"></script>

    </body>
</html>