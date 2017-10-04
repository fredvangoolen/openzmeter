<!DOCTYPE html>
<html lang="en">
    <head>
        <? $this->load->view('template/commonHead', array('title' => 'Settings')) ?>
        <link rel="stylesheet" href="lib/bootstrap-validator/bootstrapValidator.min.css" />
    </head>
    <body>
        <? $this->load->view('template/header') ?>
        <!-- SAVE MODAL -->
        <div class="modal fade" id="save_modal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel" aria-hidden="true" data-backdrop='static'>
            <div class="modal-dialog">
                <div class="modal-content">
                    <div class="modal-header">
                        <h3 class="modal-title" id="myModalLabel"><i class="glyphicon glyphicon-floppy-save"></i> Saving...</h3>
                    </div>
                    <div class="modal-body">
                        <div class="alert alert-info"><img class="inline" src="img/loading.gif"/> Updating settings...</div>
                        <div class="alert alert-success"><span class="glyphicon glyphicon-ok"></span> Settings was successful updated.</div>
                        <div class="alert alert-danger"><span class="glyphicon glyphicon-remove"></span> An error occurred during the update. Please check file permissions and try again.</div>
                    </div>
                    <div class="modal-footer">
                        <button type="button" class="btn btn-success refresh"><span class="glyphicon glyphicon-ok"></span>  Continue</button>
                    </div>
                </div>
            </div>
        </div>
        <div id="wrapper">
            <div id="page-wrapper">
                <div class="row">
                    <div class="col-lg-12">
                        <span class="h2">Settings</span>
                        <div class="pull-right">
                            <div class="btn btn-lg btn-success save"><i class="fa fa-save fa-fw"></i>  Save settings</div>
                        </div>
                        <hr>
                    </div>
                </div>
                <div class="row">
                    <div class="col-lg-4">
                        <div class="panel panel-info">
                            <div class="panel-heading">
                                <i class="fa fa-database fa-fw"></i> Database
                            </div>
                            <div class="panel-body">
                                <form id="database">
                                    <div class="row">
                                        <div class="col-lg-12">
                                            <fieldset>
                                                <legend>PostgreSQL</legend>
                                                <div class="row">
                                                    <div class="col-md-4">
                                                        <div class="form-group">
                                                            <label class="control-label">Host</label>
                                                            <input class="form-control" name="host" data-bv-regexp="true" data-bv-regexp-regexp="^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$" data-bv-regexp-message="Invalid ip" required/>
                                                        </div>
                                                    </div>
                                                    <div class="col-md-3">
                                                        <div class="form-group">
                                                            <label class="control-label">Port</label>
                                                            <input class="form-control" name="port" data-bv-digits required/>
                                                        </div>
                                                    </div>
                                                    <div class="col-md-5">
                                                        <div class="form-group">
                                                            <label class="control-label">Database</label>
                                                            <input class="form-control" name="database" required />
                                                        </div>
                                                    </div>
                                                </div>
                                                <div class="row">
                                                    <div class="col-md-6">
                                                        <div class="form-group">
                                                            <label class="control-label">Username</label>
                                                            <input class="form-control" name="user" required/>
                                                        </div>
                                                    </div>
                                                    <div class="col-md-6">
                                                        <div class="form-group">
                                                            <label class="control-label">Password</label>
                                                            <input class="form-control" name="pass" required/>
                                                        </div>
                                                    </div>
                                                </div>
                                            </fieldset>
                                        </div>
                                    </div>
                                </form>
                            </div>
                        </div>
                    </div>
                    <div class="col-lg-4">
                        <div class="panel panel-warning">
                            <div class="panel-heading">
                                <i class="fa fa-bolt fa-fw"></i> Electrical parameters
                            </div>
                            <div class="panel-body">
                                <form id="electrical">
                                    <div class="row">
                                        <div class="col-lg-12">
                                            <fieldset>
                                                <legend>Frequency</legend>
                                                <div class="col-md-4">
                                                    <div class="form-group">
                                                        <label class="control-label">Sample rate</label>
                                                        <select id="id_certificadora" name="sample_rate" class="form-control" default="40">
                                                            <option value="40">40 kHz</option>
                                                            <option value="20">20 kHz</option>
                                                        </select>
                                                    </div>
                                                </div>
                                                <div class="col-md-4">
                                                    <div class="form-group">
                                                        <label class="control-label">Nominal freq.</label>
                                                        <select id="id_certificadora" name="nominal_frequency" class="form-control" default="50">
                                                            <option value="50">50 Hz</option>
                                                            <option value="60">60 Hz</option>
                                                        </select>
                                                    </div>
                                                </div>
                                                <div class="col-md-4">
                                                    <div class="form-group">
                                                        <label class="control-label"># Harmonics</label>
                                                        <input class="form-control" name="num_stored_harmonics"  data-bv-digits required />
                                                    </div>
                                                </div>
                                            </fieldset>
                                        </div>
                                        <div class="col-lg-12">
                                            <fieldset>
                                                <legend>Voltage</legend>
                                                <div class="col-md-4">
                                                    <div class="form-group">
                                                        <label class="control-label">Nominal value</label>
                                                        <div class="input-group">
                                                            <input type="text" class="form-control" name="nominal_voltage" data-bv-numeric required>
                                                            <span class="input-group-addon"><em><small>V</small></em></span>
                                                        </div>
                                                    </div>
                                                </div>
                                                <div class="col-md-4">
                                                    <div class="form-group">
                                                        <label class="control-label">Scale</label>

                                                        <div class="input-group">
                                                            <input type="text" class="form-control" name="voltage_scale" data-bv-numeric required>
                                                            <span class="input-group-addon"><em><small>V</small></em></span>
                                                        </div>
                                                    </div>
                                                </div>
                                                <div class="col-md-4">
                                                    <div class="form-group">
                                                        <label class="control-label">Offset</label>
                                                        <input class="form-control" name="voltage_offset" data-bv-numeric required/>
                                                    </div>
                                                </div>
                                            </fieldset>
                                        </div>
                                        <div class="col-lg-12">
                                            <fieldset>
                                                <legend>Current</legend>
                                                <div class="col-md-6">
                                                    <div class="form-group">
                                                        <label class="control-label">Scale</label>
                                                        <div class="input-group">
                                                            <input type="text" class="form-control" name="current_scale" data-bv-numeric required>
                                                            <span class="input-group-addon"><em>A</em></span>
                                                        </div>
                                                    </div>
                                                </div>
                                                <div class="col-md-6">
                                                    <div class="form-group">
                                                        <label class="control-label">Offset</label>
                                                        <input class="form-control" name="current_offset" data-bv-numeric required/>
                                                    </div>
                                                </div>
                                            </fieldset>
                                        </div>
                                    </div>
                                </form>
                            </div>
                        </div>
                    </div>
                    <div class="col-lg-4">
                        <div class="panel panel-success">
                            <div class="panel-heading">
                                <i class="fa fa-money fa-fw"></i> Budget
                            </div>
                            <div class="panel-body">
                                <div class="row">
                                    <div class="col-lg-12">
                                        <form id="budget">
                                            <fieldset>
                                                <legend>Calculation</legend>
                                                <div class="col-md-6">
                                                    <div class="form-group">
                                                        <label class="control-label">Fixed price</label>
                                                        <select name="fixed_price" class="form-control" default="true">
                                                            <option value="true">Yes</option>
                                                            <option value="">No (country legislation)</option>
                                                        </select>
                                                    </div>
                                                </div>
                                                <div class="col-md-6">
                                                    <div class="form-group country_container" style="display:none">
                                                        <label class="control-label">Country</label>
                                                        <select id="id_certificadora" name="country" class="form-control" default="spain">
                                                            <option value="spain">Spain</option>
                                                        </select>
                                                    </div>
                                                </div>
                                                <div class="col-md-6">
                                                    <div class="form-group price_container" style="display:none">
                                                        <label class="control-label">Price</label>
                                                        <div class="input-group">
                                                            <input type="text" class="form-control" name="price" data-bv-numeric required>
                                                            <span class="input-group-addon"><em><small>€/kWh</small></em></span>
                                                        </div>
                                                    </div>
                                                </div>
                                            </fieldset>
                                        </form>
                                    </div>
                                    <div class="col-lg-12">
                                        <form id="budget-spain" style="display:none">
                                            <fieldset>
                                                <legend>Spain</legend>
                                                <div class="col-md-6">
                                                    <div class="form-group">
                                                        <label class="control-label">Contracted power</label>
                                                        <div class="input-group">
                                                            <input type="text" class="form-control" name="contracted_power" data-bv-numeric required />
                                                            <span class="input-group-addon"><em><small>kW</small></em></span>
                                                        </div>
                                                    </div>
                                                </div>
                                                <div class="col-md-6">
                                                    <div class="form-group">
                                                        <label class="control-label">Rate</label>
                                                        <select id="id_certificadora" name="rate_type" class="form-control" default="GEN">
                                                            <option value="GEN">General (GEN)</option>
                                                            <option value="NOC">Nocturna (NOC)</option>
                                                            <option value="VHC">Vehículo eléctrico (VHC)</option>
                                                        </select>
                                                    </div>
                                                </div>
                                                <div class="col-md-6">
                                                    <div class="form-group">
                                                        <label class="control-label">CPC</label>
                                                        <div class="input-group">
                                                            <input type="text" class="form-control" name="cpc" data-bv-numeric required />
                                                            <span class="input-group-addon"><em><small>€/year/kW</small></em></span>
                                                        </div>
                                                    </div>
                                                </div>
                                                <div class="col-md-6">
                                                    <div class="form-group">
                                                        <label class="control-label">Discount</label>
                                                        <div class="input-group">
                                                            <input type="text" class="form-control" name="discount_percentage" data-bv-numeric required />
                                                            <span class="input-group-addon"><em>%</em></span>
                                                        </div>
                                                    </div>
                                                </div>
                                                <div class="col-md-6">
                                                    <div class="form-group">
                                                        <label class="control-label">VAT</label>
                                                        <div class="input-group">
                                                            <input type="text" class="form-control" name="vat"data-bv-numeric required />
                                                            <span class="input-group-addon"><em>%</em></span>
                                                        </div>
                                                    </div>
                                                </div>
                                                <div class="col-md-6">
                                                    <div class="form-group">
                                                        <label class="control-label">Tax Percentage</label>
                                                        <div class="input-group">
                                                            <input type="text" class="form-control" name="electricity_tax_percentage" data-bv-numeric required />
                                                            <span class="input-group-addon"><em>%</em></span>
                                                        </div>
                                                    </div>
                                                </div>
                                                <div class="col-md-6">
                                                    <div class="form-group">
                                                        <label class="control-label">Meter rating</label>
                                                        <div class="input-group">
                                                            <input type="text" class="form-control" name="meter_rating" data-bv-numeric required />
                                                            <span class="input-group-addon"><em><small>€/day</small></em></span>
                                                        </div>
                                                    </div>
                                                </div>
                                            </fieldset>
                                        </form>
                                    </div>
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
                <!-- /.row -->
                <div class="text-right hidden-lg">
                    <div class="btn btn-lg btn-success save"><i class="fa fa-save fa-fw"></i>  Save settings</div>
                </div>
                <br>
            </div>
            <!-- /#page-wrapper -->
        </div>
        <script>
            settings =<?= json_encode($settings) ?>;
        </script>
        <!-- /#wrapper -->
        <? $this->load->view('template/commonScripts') ?>
        <script src="lib/bootstrap-validator/bootstrapValidator.min.js"></script>
        <script src="js/utils.js"></script>
        <script src="js/settings.js"></script>
    </body>
</html>