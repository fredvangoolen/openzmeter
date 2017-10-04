
<!-- Navigation -->
<nav class="navbar navbar-default navbar-fixed-top" style="margin-bottom: 0">
  <div class="navbar-header">
    <button type="button" class="navbar-toggle" data-toggle="collapse" data-target=".navbar-collapse">
      <span class="sr-only">Toggle navigation</span>
      <span class="icon-bar"></span>
      <span class="icon-bar"></span>
      <span class="icon-bar"></span>
    </button>
    <a class="navbar-brand" href="<?= site_url('dashboard') ?>"><i class="fa fa-bolt fa-fw"></i> openZmeter</a>
  </div>
  <!-- /.navbar-header -->
  <ul class="nav navbar-top-links navbar-right">
    <!-- /.dropdown -->
    <li class="dropdown">
      <a class="dropdown-toggle" data-toggle="dropdown" href="#">
        <i class="fa fa-tasks fa-fw"></i> <i class="fa fa-caret-down"></i>
      </a>
      <ul class="dropdown-menu dropdown-user" style="font-size: 10px">
        <li>
          <span style="padding-left: 5px">CPU:</span><span style="float: right; padding-right: 5px" id="CPUText"></span><div style="padding: 0px 5px 0px 5px" id="ProgressCPU"></div>
        </li>
        <div class="clearfix"></div>
        <li>
          <span style="padding-left: 5px">RAM:</span><span style="float: right; padding-right: 5px" id="RAMText"></span><div style="padding: 0px 5px 0px 5px" id="ProgressRAM"></div>
        </li>
        <div class="clearfix"></div>
        <li>
          <span style="padding-left: 5px">Storage:</span><span style="float: right; padding-right: 5px" id="SSDText"></span><div style="padding: 0px 5px 0px 5px" id="ProgressSDD"></div>
        </li>
        <div class="clearfix"></div>
        <li>
          <span style="padding-left: 5px">Battery:</span><span style="float: right; padding-right: 5px" id="BatteryText"></span><div class="disabled" style="padding: 0px 5px 0px 5px" id="ProgressBattery"></div>
        </li>
      </ul>
    </li>
    <li  id="eventsDropdown" class="dropdown">
      <a class="dropdown-toggle" data-toggle="dropdown" href="#">
        <span id="unread_events" class="badge alert-danger"><?= $numEvents > 0 ? $numEvents : '' ?></span>
        <i class="fa fa-bell fa-fw"></i>  <i class="fa fa-caret-down"></i>
      </a>
      <ul class="dropdown-menu dropdown-alerts">
        <li>
          <a class="text-center" href="<?= site_url('events/disturbances') ?>">
            <strong>See last events</strong>
            <i class="fa fa-angle-right"></i>
          </a>
        </li>
      </ul>
      <!-- /.dropdown-alerts -->
    </li>
    <!-- /.dropdown -->
    <li class="dropdown">
      <a class="dropdown-toggle" data-toggle="dropdown" href="#">
        <i class="fa fa-user fa-fw"></i>  <i class="fa fa-caret-down"></i>
      </a>
      <ul class="dropdown-menu dropdown-user">
        <li><a href="<?= site_url('account') ?>"><i class="fa fa-gear fa-fw"></i> Account settings</a>
        </li>
        <li class="divider"></li>
        <li><a href="<?= site_url('login/out') ?>"><i class="fa fa-sign-out fa-fw"></i> Logout</a>
        </li>
      </ul>
      <!-- /.dropdown-user -->
    </li>
    <!-- /.dropdown -->
  </ul>
  <!-- /.navbar-top-links -->
  <div class="indicators <?= $this->router->class == 'dashboard' ? '' : '' ?>">
    <div style="display:inline-block;" class="indicator panel panel-red small ">
      <div class="panel-heading"><strong class="hidden-xs">V<span class="hidden-sm">oltage:</span></strong> <span id="voltage_indicator">?</span> <em class="small">V</em></div>
    </div>
    <div style="display:inline-block;" class="indicator panel panel-primary small">
      <div class="panel-heading"><strong class="hidden-xs"><span class="visible-sm-inline-block">I</span><span class="hidden-sm">Current:</span></strong> <span id="current_indicator">?</span> <em class="small">A</em></div>
    </div>
    <div style="display:inline-block;" class="indicator panel panel-yellow small">
      <div class="panel-heading"><strong class="hidden-xs">P<span class="hidden-sm">ower:</span></strong> <span id="power_indicator">?</span> <em class="small" id="power_indicator_units">W</em></div>
    </div>
    <div style="display:inline-block;" class="indicator panel panel-success small">
      <div class="panel-heading"><strong class="hidden-xs">F<span class="hidden-sm">req.:</span></strong> <span id="frequency_indicator">?</span> <em class="small">Hz</em></div>
    </div>
    <div style="display:inline-block;" class="indicator panel panel-info small">
      <div class="panel-heading"><strong class="hidden-xs">E<span class="hidden-sm">nergy today:</span></strong> <span id="energy_indicator">?</span> <em class="small" id="energy_indicator_units">Wh</em></div>
    </div>
  </div>
  <div class="navbar-default sidebar" role="navigation">
    <div class="sidebar-nav navbar-collapse">
      <ul class="nav" id="side-menu">
        <li>
          <a href="<?= site_url('dashboard') ?>"><i class="fa fa-dashboard fa-fw"></i>Dashboard</a>
        </li>
        <li>
          <a href="<?= site_url('analysis') ?>"><i class="fa fa-search-plus fa-fw"></i>Analysis<span class="caret"></span></a>
          <ul class="nav nav-second-level">
            <li>
              <a href="<?= site_url('analysis/voltage') ?>">Voltage</a>
            </li>
            <li>
              <a href="<?= site_url('analysis/current') ?>">Current</a>
            </li>
            <li>
              <a href="<?= site_url('analysis/energy') ?>">Energy</a>
            </li>
            <li>
              <a href="<?= site_url('analysis/frequency') ?>">Frequency</a>
            </li>
          </ul>
        </li>
        <li>
          <a href="<?= site_url('events') ?>"><i class="fa fa-exclamation-triangle fa-fw"></i>Events<span class="caret"></span></a>
          <ul class="nav nav-second-level">
            <li>
              <a href="<?= site_url('events/itic') ?>">ITIC</a>
            </li>
            <li>
              <a href="<?= site_url('events/disturbances') ?>">Disturbances</a>
            </li>
            <li>
              <a href="<?= site_url('events/quality') ?>">Quality</a>
            </li>
          </ul>
        </li>
        <li>
          <a href="<?= site_url('stats') ?>"><i class="fa fa-bar-chart-o fa-fw"></i>Stats</a>
        </li>
        <li>
          <a href="<?= site_url('settings') ?>"><i class="fa fa-gear fa-fw"></i>Settings</a>
        </li>
        <li>
          <a href="<?= site_url('settings') ?>"><i class="fa fa-book fa-fw"></i>Help</a>
        </li>
      </ul>
    </div>
    <!-- /.sidebar-collapse -->
  </div>
  <!-- /.navbar-static-side -->
</nav>
<span class="clearfix"> </span>