<!DOCTYPE html>
<html lang="en">
    <head>
        <? $this->load->view('template/commonHead', array('title' => 'Stats')) ?>
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
                        <span class="h2">Stats</span>
                        <div class="pull-right">
                            <a class="btn btn-primary" href="javascript:resetCharts();"><i class="fa fa-bolt fa-eraser"></i>  Reset All</a>
                        </div>
                        <hr>
                    </div>
                </div>
                <div class="row">
                    <div class="col-md-12">
                        <div class="panel panel-yellow">
                            <div class="panel-heading">
                                <i class="fa fa-bar-chart fa-fw"></i> Energy usage
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
                            </div>
                            <div class="panel-body">
                                <div id='date_chart'></div>
                                <div class="chart-range" id='date_selector_chart'></div>
                            </div>
                        </div>
                    </div>
                    <div class="col-md-12">
                        <h3 class="text-left">Energy usage by groups
                        </h3>
                        <div class="row">
                            <div class="col-md-6">
                                <h4 class="text-center">By weekday</h4>
                                <div id='weekday_chart'></div>
                            </div>
                            <div class="col-md-6">
                                <h4 class="text-center">By day of month</h4>
                                <div id='monthday_chart'></div>
                            </div>
                            <div class="col-md-12">
                                <h4 class="text-center">By week</h4>
                                <div id='week_chart'></div>
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
            energy =<?= json_encode($energy) ?>;
        </script>
        <? $this->load->view('template/commonScripts') ?>

        <script src="lib/d3/d3.v3.min.js"></script>
        <script src="lib/d3/colorbrewer.js"></script>
        <script src="lib/dc.js/crossfilter.js"></script>
        <script src="lib/dc.js/dc.min.js"></script>
        <script src="lib/d3-tip/d3-tip.min.js"></script>
        <script src="js/stats.js"></script>

    </body>
</html>