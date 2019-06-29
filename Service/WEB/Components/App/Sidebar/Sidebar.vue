<template>
  <nav class="navbar-nav">
    <div class="menu-wrapper">
      <sidebar-analyzer ref="analyzer"></sidebar-analyzer>
      <sidebar-user></sidebar-user>
      <sidebar-menu></sidebar-menu>
    </div>
    <sidebar-news></sidebar-news>
  </nav>
</template>

<script>
import SidebarUser from './SidebarUser.vue';
import SidebarAnalyzer from './SidebarAnalyzer.vue';
import SidebarMenu from './SidebarMenu.vue';
import SidebarNews from './SidebarNews.vue';
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
    $('.link').click(function (e){
      $(e.target).parent().parent().parent().find("[data-sidenav-dropdown]:visible").not($(e.target).parent().parent()).slideUp("fast");
      $('.navbar-nav').toggleClass('show');
      this.hideOverlay();
    }.bind(this));
    $('body').on('click', '.sidenav-overlay', this.hideOverlay);
  },
  methods: {
    showOverlay: function () {
      $('body').toggleClass('sidenav-no-scroll');
      var overlay = $('[data-sidenav-overlay]');
      if(overlay[0] === undefined) {
        overlay = $('<div data-sidenav-overlay class="sidenav-overlay"/>');
        $('body').append(overlay);
      }
      overlay.fadeToggle('fast');
    },
    hideOverlay: function () {
      $('body').removeClass('sidenav-no-scroll');
      $('.navbar-nav').removeClass('show');
      $('[data-sidenav-overlay]').fadeOut('fast');
    }
  },
  components: { SidebarUser, SidebarAnalyzer, SidebarMenu, SidebarNews }
}
</script> 

<style>
  .navbar-nav.show {
    transform: translate(0, 0);
  }
  .menu-wrapper {
    min-height: calc(100vh - 48px);
  }
  .navbar-nav {
    overflow: hidden;
    bottom: 0px;
    background: #212121;
/*    box-shadow: 2px 2px 6px rgba(0,0,0,.3);*/
    display: block;
    font-size: 16px;
    font-weight: 400;
    min-height: 100%;
    left: 0;
    position: fixed;
    overflow: auto;
    transform: translate(-262px,0);
    transition: transform .3s;
    top: 0;
    width: 256px;
    z-index: 100;
    margin: 0px;
  }
  @media(min-width:1280px) {
    .navbar-nav {
      min-height: calc(100vh - 56px);
      transform: none;
      top: 56px;
    }
    .menu-wrapper {
      min-height: calc(100vh - 48px - 56px);
    }
  }
  .sidenav-header {
    padding: 3px 6px;
    color: #DDD;
    font-size: 16px;
    border-bottom: 1px solid #333;
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