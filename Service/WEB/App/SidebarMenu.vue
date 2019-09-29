<template>
  <ul class="sidenav-menu">
    <li>
      <router-link class="link" to="/Dashboard">
        <span class="sidenav-link-icon"><img class="sidenav-link-icon" :src="require('Imgs/dashboard.png')"></span>
        <span class="sidenav-link-title">Dashboard</span>
      </router-link>
    </li>
    <li>
      <a href="javascript:;" data-sidenav-dropdown-toggle>
        <span class="sidenav-link-icon"><img class="sidenav-link-icon" :src="require('Imgs/draw_wave.png')"></span>
        <span class="sidenav-link-title">Analysis</span>
        <span class="sidenav-dropdown-icon show" data-sidenav-dropdown-icon><img :src="require('Imgs/bullet_arrow_down.png')"></span>
        <span class="sidenav-dropdown-icon" data-sidenav-dropdown-icon><img :src="require('Imgs/bullet_arrow_up.png')"></span>          
      </a>
      <ul class="sidenav-dropdown" data-sidenav-dropdown>
        <li><router-link class="link" to="/Analysis/Voltage">Voltage</router-link></li>
        <li><router-link class="link" to="/Analysis/Current">Current</router-link></li>
        <li><router-link class="link" to="/Analysis/Power">Power</router-link></li>
        <li><router-link class="link" to="/Analysis/Energy">Energy</router-link></li>
        <li><router-link class="link" to="/Analysis/Frequency">Frequency</router-link></li>
      </ul>
    </li>
    <li>
      <a href="javascript:;" data-sidenav-dropdown-toggle>
        <span class="sidenav-link-icon"><img class="sidenav-link-icon" :src="require('Imgs/warning.png')"></span>
        <span class="sidenav-link-title">Events</span>
        <span class="sidenav-dropdown-icon show" data-sidenav-dropdown-icon><img :src="require('Imgs/bullet_arrow_down.png')"></span>
        <span class="sidenav-dropdown-icon" data-sidenav-dropdown-icon><img :src="require('Imgs/bullet_arrow_up.png')"></span>
      </a>
      <ul class="sidenav-dropdown" data-sidenav-dropdown>
        <li><router-link class="link" to="/Events/ITIC">ITIC</router-link></li>
        <li><router-link class="link" to="/Events/Disturbances">Disturbances</router-link></li>
        <li><router-link class="link" to="/Events/EN50160">EN50160</router-link></li>
      </ul>
    </li>
    <li>
      <a href="javascript:;" data-sidenav-dropdown-toggle>
        <span class="sidenav-link-icon"><img class="sidenav-link-icon" :src="require('Imgs/table_money.png')"></span>
        <span class="sidenav-link-title">Energy Stats</span>
        <span class="sidenav-dropdown-icon show" data-sidenav-dropdown-icon><img :src="require('Imgs/bullet_arrow_down.png')"></span>
        <span class="sidenav-dropdown-icon" data-sidenav-dropdown-icon><img :src="require('Imgs/bullet_arrow_up.png')"></span>
      </a>
      <ul class="sidenav-dropdown" data-sidenav-dropdown>
        <li><router-link class="link" to="/Stats/Costs">Costs</router-link></li>
        <li><router-link class="link" to="/Stats/Heatmap">Heatmap</router-link></li>
        <li><router-link class="link" to="/Stats/Distribution">Distribution</router-link></li>
      </ul>
    </li>
    <li>
      <router-link class="link" to="/Settings">
        <span class="sidenav-link-icon"><img class="sidenav-link-icon" :src="require('Imgs/cog.png')"></span>
        <span class="sidenav-link-title">Settings</span>
      </router-link>
    </li>
    <li>
      <router-link class="link" to="/Help">
        <span class="sidenav-link-icon"><img class="sidenav-link-icon" :src="require('Imgs/help.png')"></span>
        <span class="sidenav-link-title">Help</span>
      </router-link>
    </li>
  </ul>
</template>

<script>
  export default {
    data() {
      return { }
    }, 
    mounted: function() {
      $('.sidenav-menu').on('click', '[data-sidenav-dropdown-toggle]', function (e) {
        $(this).parent().parent().find("[data-sidenav-dropdown]:visible").not($(this).next('[data-sidenav-dropdown]')).slideUp("fast");
        $(this).next('[data-sidenav-dropdown]').slideToggle('fast');
        $(this).find('[data-sidenav-dropdown-icon]').toggleClass('show');
        e.preventDefault();
      });
    }
  }
</script>

<style scoped>
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
  li ul li a {
    padding: 5px 32px !important;
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