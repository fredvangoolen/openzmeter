<!DOCTYPE html>
<html lang="en">
    <head>
        <? $this->load->view('template/commonHead', array('title' => 'Account settings')) ?>
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
                        <div class="alert alert-info"><img class="inline" src="img/loading.gif"/> Updating account...</div>
                        <div class="alert alert-success"><span class="glyphicon glyphicon-ok"></span> Account was successful updated.</div>
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
                        <span class="h2">Account settings</span>
                        <div class="pull-right">
                            <div class="btn btn-lg btn-success save"><i class="fa fa-save fa-fw"></i>  Save settings</div>
                        </div>
                        <hr>
                    </div>
                </div>
                <div class="row">
                    <div class="col-lg-6 col-lg-offset-3">
                        <div class="panel panel-info">
                            <div class="panel-heading">
                                <i class="fa fa-user fa-fw"></i> User account
                            </div>
                            <div class="panel-body">
                                <form id="credentials">
                                    <div class="row">
                                        <div class="col-lg-12">
                                            <fieldset>
                                                <legend>Credentials</legend>
                                                <div class="row">
                                                    <div class="col-md-4">
                                                        <div class="form-group">
                                                            <label class="control-label">Username</label>
                                                            <input class="form-control" name="user" required/>
                                                        </div>
                                                    </div>
                                                    <div class="col-md-4">
                                                        <div class="form-group">
                                                            <label class="control-label">Password</label>
                                                            <input class="form-control" type="password" name="pass" required/>
                                                        </div>
                                                    </div>
                                                    <div class="col-md-4">
                                                        <div class="form-group">
                                                            <label class="control-label">Retype password</label>
                                                            <input class="form-control" type="password" name="pass2" required
                                                                   data-bv-identical="true"
                                                                   data-bv-identical-field="pass"/>
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
        <script src="js/account.js"></script>
    </body>
</html>