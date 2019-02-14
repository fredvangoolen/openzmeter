<template>
  <div>
    <ul class="sidenav-menu" id="AnalyzerMenu">
      <li>
        <router-link to="/Dashboard">
          <span class="sidenav-link-icon"><img class="sidenav-link-icon" src="Imgs/dashboard.png"></span>
          <span class="sidenav-link-title">Dashboard</span>
        </router-link>
      </li>
      <li>
        <a href="javascript:;" data-sidenav-dropdown-toggle>
          <span class="sidenav-link-icon"><img class="sidenav-link-icon" src="Imgs/draw_wave.png"></span>
          <span class="sidenav-link-title">Analysis</span>
          <span class="sidenav-dropdown-icon show" data-sidenav-dropdown-icon><img src="Imgs/bullet_arrow_down.png"></span>
          <span class="sidenav-dropdown-icon" data-sidenav-dropdown-icon><img src="Imgs/bullet_arrow_up.png"></span>          
        </a>
        <ul class="sidenav-dropdown" data-sidenav-dropdown>
          <li><router-link to="/Analysis/Voltage">Voltage</router-link></li>
          <li><router-link to="/Analysis/Current">Current</router-link></li>
          <li><router-link to="/Analysis/Power">Power</router-link></li>
          <li><router-link to="/Analysis/Energy">Energy</router-link></li>
          <li><router-link to="/Analysis/Frequency">Frequency</router-link></li>
        </ul>
      </li>
      <li>
        <a href="javascript:;" data-sidenav-dropdown-toggle>
          <span class="sidenav-link-icon"><img class="sidenav-link-icon" src="Imgs/warning.png"></span>
          <span class="sidenav-link-title">Events</span>
          <span class="sidenav-dropdown-icon show" data-sidenav-dropdown-icon><img src="Imgs/bullet_arrow_down.png"></span>
          <span class="sidenav-dropdown-icon" data-sidenav-dropdown-icon><img src="Imgs/bullet_arrow_up.png"></span>
        </a>
        <ul class="sidenav-dropdown" data-sidenav-dropdown>
          <li><router-link to="/Events/ITIC">ITIC</router-link></li>
          <li><router-link to="/Events/Disturbances">Disturbances</router-link></li>
          <li><router-link to="/Events/EN50160">EN50160</router-link></li>
        </ul>
      </li>
      <li>
        <a href="javascript:;" data-sidenav-dropdown-toggle>
          <span class="sidenav-link-icon"><img class="sidenav-link-icon" src="Imgs/table_money.png"></span>
          <span class="sidenav-link-title">Energy Stats</span>
          <span class="sidenav-dropdown-icon show" data-sidenav-dropdown-icon><img src="Imgs/bullet_arrow_down.png"></span>
          <span class="sidenav-dropdown-icon" data-sidenav-dropdown-icon><img src="Imgs/bullet_arrow_up.png"></span>
        </a>
        <ul class="sidenav-dropdown" data-sidenav-dropdown>
          <li><a href="Stats/Costs">Costs</a></li>
          <li><router-link to="/Stats/Heatmap">Heatmap</router-link></li>
          <li><router-link to="/Stats/Distribution">Distribution</router-link></li>
        </ul>
      </li>
    </ul>
    <ul class="sidenav-menu">
      <li class="hide" id="menuSettings">
        <router-link to="/Settings">
          <span class="sidenav-link-icon"><img class="sidenav-link-icon" src="Imgs/cog.png"></span>
          <span class="sidenav-link-title">Settings</span>
        </router-link>
      </li>
      <li>
        <a href="Help">
          <span class="sidenav-link-icon"><img class="sidenav-link-icon" src="Imgs/help.png"></span>
          <span class="sidenav-link-title">Help</span>
        </a>
      </li>
    </ul>
  </div>
</template>

<script>
  export default {
    data() {
      return { }
    }, 
    mounted: function() {
      $('.sidenav-menu').on('click', '[data-sidenav-dropdown-toggle]', function (e) {
      $(this).next('[data-sidenav-dropdown]').slideToggle('fast');
      $(this).find('[data-sidenav-dropdown-icon]').toggleClass('show');
      e.preventDefault();
    });
    }
  }
</script>

<style>
  .sidenav-menu {
    padding: 0px;
  }
  .sidenav-menu li {
    display: block;
    position: relative
  }
  .sidenav-dropdown-icon, 
  .sidenav-link-icon {
    position: absolute;
    text-align: center;
    top: 16px
  }
  .sidenav-menu a {
    background: 0 0;
    color: #BDBDBD;
    display: block;
    line-height: 24px;
    padding: 10px 32px;
    text-decoration: none
  }
  .sidenav-menu a:hover {
    background: #616161;
    color: #BDBDBD
  }
  .sidenav-menu a.active,
  .sidenav-menu a.active .sidenav-dropdown-icon,
  .sidenav-menu a.active .sidenav-link-icon {
    color: #FAFAFA;
    background: #474544;
  }
  .sidenav-link-icon {
    color: #757575;
    display: block;
    height: 24px;
    left: 6px;
    width: 24px;
    top: 7px;
  }
  .sidenav-link-title {
    display: block;
    padding: 0px 16px 0px 14px;
  }
  .sidenav-dropdown {
    display: none;
    list-style: none;
    padding: 0
  }
  .sidenav-dropdown a {
    background: #191919;
    color: #BDBDBD;
    padding-left: 42px
  }
  .sidenav-dropdown a:hover {
    background: #616161;
    color: #BDBDBD
  }
  .sidenav-dropdown li:first-child a {
    box-shadow: inset 0 1px 1px rgba(0, 0, 0, .1)
  }
  .sidenav-dropdown-icon {
    color: #757575;
    display: none;
    height: 16px;
    right: 16px;
    width: 16px;
    top: 10px;
  }
  .sidenav-dropdown-icon.show {
    display: block
  } 
</style>