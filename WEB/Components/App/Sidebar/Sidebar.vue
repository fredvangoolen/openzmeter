<template>
  <nav class="navbar-nav">
    <sidebar-user></sidebar-user>
    <sidebar-analyzer></sidebar-analyzer>
    <sidebar-menu></sidebar-menu>
    <div class="sidenav-version"><span>Build 21-01-2018</span>
      <div class="btn-group pull-right">
        <button type="button" class="btn btn-default btn-xs navbar-btn" id="btnSearchAnalyzer"><img src="Imgs/newspaper.png"></button>
      </div>
    </div>
  </nav>
</template>

<script>
import SidebarUser from './SidebarUser.vue';
import SidebarAnalyzer from './SidebarAnalyzer.vue';
import SidebarMenu from './SidebarMenu.vue';
export default {
  data() { return {} },
  mounted: function() {
    $('.navbar-toggle').click(function (e) {
      $('.navbar-nav').toggleClass('show');
      if($('.navbar-nav').hasClass('show')) {
        this.showOverlay();
      } else {
        this.hideOverlay();
      }
    }.bind(this));
  },
  methods: {
    showOverlay: function () {
      $('body').toggleClass('sidenav-no-scroll');
      var overlay = $('[data-sidenav-overlay]');
      if(overlay[0] === undefined) {
        overlay = $('<div data-sidenav-overlay class="sidenav-overlay"/>');
        $('body').append(overlay);
        $('body').on('click', '[data-sidenav-overlay]', this.hideOverlay);
      }
      overlay.fadeToggle('fast');
    },
    hideOverlay: function () {
      $('body').removeClass('sidenav-no-scroll');
      $('.navbar-nav').removeClass('show');
      $('[data-sidenav-overlay]').fadeOut('fast');
    }
  },
  components: { SidebarUser, SidebarAnalyzer, SidebarMenu }
}
</script> 

<style>
  .navbar-nav.show {
    transform: translate(0, 0);
  }
  .navbar-nav {
    bottom: 0px;
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
    }
  }
  .sidenav-header {
    padding: 3px 6px;
    color: #DDD;
    font-size: 16px;
    border-bottom: 1px solid #424242;
  }
  .sidenav-header > div:first-child {
    float: left;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
    line-height: 28px;
  }
  .sidenav-header button {
    padding: 4px;
    background-color: transparent;
    border: 1px solid #333;
  }
  .sidenav-overlay {
    background: #000;
    opacity: 0.5;
    display:none;
    height:100%;
    left:0;
    position:fixed;
    top:0;
    width:100%;
    z-index:99
  }
  .sidenav-no-scroll { 
    overflow: hidden!important
  }
</style>