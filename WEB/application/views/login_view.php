
<!DOCTYPE html>
<html lang="en">
    <head>
        <? $this->load->view('template/commonHead', array('title' => 'Login')) ?>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <link href="css/login.css" rel="stylesheet">
    </head>
    <body>
        <!--
    you can substitue the span of reauth email for a input with the email and
    include the remember me checkbox
        -->
        <div class="container">
            <div class="card card-container">
                <img id="profile-img" class="profile-img-card" src="img/logo.png" />
                <p id="profile-name" class="profile-name-card"></p>
                <form class="form-signin" method="post" action="<?= site_url('login/in') ?>">
                    <span id="reauth-email" class="reauth-email"></span>
                    <input name="username" type="text" id="user" class="form-control" placeholder="Username" required autofocus>
                    <input name="password" type="password" id="password" class="form-control" placeholder="Password" required>
                    <!-- div id="remember" class="checkbox">
                        <label>
                            <input type="checkbox" value="remember-me"> Remember me
                        </label>
                    </div -->
                    <button class="btn btn-lg btn-primary btn-block btn-signin" type="submit">Sign in</button>
                    <div class="alert alert-success text-center" style="display:none">
                        <i class="fa fa-check"></i> Welcome again, redirecting...
                    </div>
                    <div class="alert alert-danger text-center" style="display:none">
                        <i class="fa fa-times"></i> Wrong user/password combination
                    </div>
                </form><!-- /form -->
                <!-- a href="#" class="forgot-password">
                    Forgot the password?
                </a -->
            </div><!-- /card-container -->
        </div><!-- /container -->
        <? $this->load->view('template/commonScripts') ?>
        <script src="js/login.js"></script>
    </body>
</html>
