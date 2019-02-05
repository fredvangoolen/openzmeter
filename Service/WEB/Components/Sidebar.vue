<template>
  <b-navbar-nav is-nav id="nav_collapse">
    <user-header></user-header>
    <analyzer-header></analyzer-header>
    <ul class="sidenav-menu hide" id="AnalyzerMenu">
      <li>
        <a href="Dashboard">
          <span class="sidenav-link-icon"><img class="sidenav-link-icon" src="Imgs/dashboard.png"></span>
          <span class="sidenav-link-title">Dashboard</span>
        </a>
      </li>
      <li>
        <a href="javascript:;" data-sidenav-dropdown-toggle>
          <span class="sidenav-link-icon"><img class="sidenav-link-icon" src="Imgs/draw_wave.png"></span>
          <span class="sidenav-link-title">Analysis</span>
          <span class="sidenav-dropdown-icon show" data-sidenav-dropdown-icon><img src="Imgs/bullet_arrow_down.png"></span>
          <span class="sidenav-dropdown-icon" data-sidenav-dropdown-icon><img src="Imgs/bullet_arrow_up.png"></span>          
        </a>
        <ul class="sidenav-dropdown" data-sidenav-dropdown>
          <li><a href="Analysis/Voltage">Voltage</a></li>
          <li><a href="Analysis/Current">Current</a></li>
          <li><a href="Analysis/Power">Power</a></li>
          <li><a href="Analysis/Energy">Energy</a></li>
          <li><a href="Analysis/Frequency">Frequency</a></li>
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
          <li><a href="Events/ITIC">ITIC</a></li>
          <li><a href="Events/Disturbances">Disturbances</a></li>
          <li><a href="Events/EN50160">EN50160</a></li>
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
          <li><a href="Stats/Heatmap">Heatmap</a></li>
          <li><a href="Stats/Distribution">Distribution</a></li>
        </ul>
      </li>
    </ul>
    <ul class="sidenav-menu">
      <li class="hide" id="menuSettings">
        <a href="Settings">
          <span class="sidenav-link-icon"><img class="sidenav-link-icon" src="Imgs/cog.png"></span>
          <span class="sidenav-link-title">Settings</span>
        </a>
      </li>
      <li>
        <a href="Help">
          <span class="sidenav-link-icon"><img class="sidenav-link-icon" src="Imgs/help.png"></span>
          <span class="sidenav-link-title">Help</span>
        </a>
      </li>
    </ul>
    <div class="sidenav-version"><span>Build 21-01-2018</span>
      <div class="btn-group pull-right">
        <button type="button" class="btn btn-default btn-xs navbar-btn" id="btnSearchAnalyzer">
          <img src="Imgs/newspaper.png">
        </button>
      </div>
    </div>
  </b-navbar-nav>
</template>

<script>
import UserHeader from './UserHeader.vue';
import AnalyzerHeader from './AnalyzerHeader.vue';
export default {
    data() { return {} },
  components: {
    UserHeader,
    AnalyzerHeader
  }
}
</script> 

<style>
  .navbar-nav {
    bottom: 0px;
    height: unset;
    background: #212121;
    box-shadow: 2px 2px 6px rgba(0,0,0,.3);
    display: block;
    font-size: 16px;
    font-weight: 400;
    height: 100%;
    left: 0;
    position: fixed;
    overflow: auto;
    transform: translate(-262px,0);
    transition: transform .3s;
    top: 0;
    width: 256px;
    z-index: 100;
  }
  @media(min-width:1024px) {
    .navbar-nav {
      transform: none;
      top: 56px;
      box-shadow: none;
    }
  }
  .sidenav-header {
    padding: 5px 10px;
    color: #DDD;
    font-size: 16px;
    border-bottom: 1px solid #424242;
  }
  .sidenav-header > div:first-child {
    float: left;
    width: 154px;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
    line-height: 28px;
  }
  .sidenav-header .btn-group {
    float: right;
  }
  .sidenav-header button {
    padding: 1px 5px;
    background-color: transparent;
    border: 1px solid #333;
  }
</style>