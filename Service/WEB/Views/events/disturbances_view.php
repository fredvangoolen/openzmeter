<!DOCTYPE html>
<html lang="en">
    <head>
        <? $this->load->view('template/commonHead', array('title' => 'Events | Disturbances')) ?>
        <link rel="stylesheet" href="lib/datatables/css/dataTables.responsive.css" />
        <link rel="stylesheet" href="lib/datatables/css/dataTables.bootstrap.css" />
        <link href="lib/visjs/vis.min.css" rel="stylesheet">
        <link href="lib/qtip2/jquery.qtip.min.css" rel="stylesheet">
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
                    <div class="col-lg-12">
                        <div class="text-right">
                            <div class="btn-group" data-toggle="buttons">
                                <label class="btn btn-default btn-sm active">
                                    <input type="radio" name="disturbances_chart_range" value="today"/> Today
                                </label>
                                <label class="btn btn-default btn-sm">
                                    <input type="radio" name="disturbances_chart_range" value="yesterday"/> Yesterday
                                </label>
                                <label class="btn btn-default btn-sm ">
                                    <input type="radio" name="disturbances_chart_range" value="week"  checked="checked"/> This week
                                </label>
                                <label class="btn btn-default btn-sm">
                                    <input type="radio" name="disturbances_chart_range" value="month"/> This month
                                </label>
                            </div>
                        </div>
                        <br>
                        <div id="disturbances_chart">
                        </div>
                        <br>
                        <div class="row">
                            <div class="col-lg-12">
                                <div class="panel panel-default">
                                    <div class="panel-heading">
                                        <i class="fa fa-table fa-fw"></i> Details
                                    </div>
                                    <div class="panel-body">
                                        <div class="events row">
                                            <div class="col col-md-5">
                                                <table id="events_table" class="table small table-striped table-responsive" cellspacing="0" width="100%">
                                                    <thead>
                                                        <tr>
                                                            <th>Date</th><th>Type</th><th>Duration</th><th>ΔU<sub>max</sub></th><th>ΔU<sub>max</sub> (%)</th><th class="all"></th>
                                                        </tr>
                                                    </thead>
                                                    <tbody>
                                                    </tbody>
                                                </table>
                                            </div>
                                            <div class="col col-md-7">
                                                <div id="event_chart" class="chart big_chart"></div>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
            <script>
                events =<?= json_encode($events) ?>;
                timestamp =<?= isset($timestamp) ? json_encode($timestamp) : 'null' ?>;
            </script>
            <? $this->load->view('template/commonScripts') ?>

            <script src="lib/dygraph/chart-plotter.js"></script>
            <script src="lib/spin.js/spin.min.js"></script>
            <script src="lib/datatables/js/jquery.dataTables.min.js"></script>
            <script src="lib/datatables/js/dataTables.responsive.min.js"></script>
            <script src="lib/datatables/js/dataTables.bootstrap.js"></script>
            <script src="lib/visjs/vis.js"></script>
            <script src="lib/qtip2/jquery.qtip.min.js"></script>
            <script src="js/events/events.js"></script>
            <script src="js/events/disturbances.js"></script>

    </body>
</html>