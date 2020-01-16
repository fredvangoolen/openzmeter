<template>
  <div>
    <router-link to="/Dashboard" class="item">
      <img class="icon" :src="require('Imgs/dashboard.png')">
      <div class="name">Dashboard</div>
    </router-link>
    <div class="item" v-on:click="Submenu = (Submenu == 'Analysis') ? null : 'Analysis'">
      <img class="icon" :src="require('Imgs/draw_wave.png')">
      <div class="name">Analysis</div>
      <span class="dropdown-icon" v-bind:class="{ show: (Submenu != 'Analysis') }"><img :src="require('Imgs/bullet_arrow_down.png')"></span>
      <span class="dropdown-icon" v-bind:class="{ show: (Submenu == 'Analysis') }"><img :src="require('Imgs/bullet_arrow_up.png')"></span>
    </div>
    <div class="submenu" v-bind:class="{ show: (Submenu == 'Analysis') }">
      <router-link to="/Analysis/Voltage">Voltage</router-link>
      <router-link to="/Analysis/Current">Current</router-link>
      <router-link to="/Analysis/Power">Power</router-link>
      <router-link to="/Analysis/Energy">Energy</router-link>
      <router-link to="/Analysis/Frequency">Frequency</router-link>
    </div>
    <div class="item" v-on:click="Submenu = (Submenu == 'Events') ? null : 'Events'">
      <img class="icon" :src="require('Imgs/warning.png')">
      <div class="name">Events</div>
      <span class="dropdown-icon" v-bind:class="{ show: (Submenu != 'Events') }"><img :src="require('Imgs/bullet_arrow_down.png')"></span>
      <span class="dropdown-icon" v-bind:class="{ show: (Submenu == 'Events') }"><img :src="require('Imgs/bullet_arrow_up.png')"></span>
    </div>
    <div class="submenu" v-bind:class="{ show: (Submenu == 'Events') }">
      <router-link to="/Events/ITIC">ITIC</router-link>
      <router-link to="/Events/Disturbances">Disturbances</router-link>
      <router-link to="/Events/EN50160">EN50160</router-link>
      <router-link to="/Events/Alarms">Alarms</router-link>
    </div>
    <div class="item" v-on:click="Submenu = (Submenu == 'Costs') ? null : 'Costs'">
      <img class="icon" :src="require('Imgs/table_money.png')">
      <div class="name">Energy Stats</div>
      <span class="dropdown-icon" v-bind:class="{ show: (Submenu != 'Costs') }"><img :src="require('Imgs/bullet_arrow_down.png')"></span>
      <span class="dropdown-icon" v-bind:class="{ show: (Submenu == 'Costs') }"><img :src="require('Imgs/bullet_arrow_up.png')"></span>
    </div>
    <div class="submenu" v-bind:class="{ show: (Submenu == 'Costs') }">
      <router-link class="link" to="/Stats/Costs">Costs</router-link>
      <router-link class="link" to="/Stats/Heatmap">Heatmap</router-link>
      <router-link class="link" to="/Stats/Distribution">Distribution</router-link>
    </div>
    <router-link to="/Settings" class="item">
      <img class="icon" :src="require('Imgs/cog.png')">
      <div class="name">Settings</div>
    </router-link>
    <router-link to="/Help" class="item">
      <img class="icon" :src="require('Imgs/help.png')">
      <div class="name">Help</div>
    </router-link>


  </div>
</template>

<script>
  export default {
    props: { expanded: Boolean },
    data() {
      return { Submenu: null, AnalyzerSelected: false }
    },
    watch: {
      expanded: function(val) {
        if(val == false) this.Submenu = null;
      }
    },
    mounted: function() {
      $('.sidenav-menu').on('click', '[data-sidenav-dropdown-toggle]', function (e) {
        $(this).parent().parent().find("[data-sidenav-dropdown]:visible").not($(this).next('[data-sidenav-dropdown]')).slideUp("fast");
        $(this).next('[data-sidenav-dropdown]').slideToggle('fast');
        $(this).find('[data-sidenav-dropdown-icon]').toggleClass('show');
        e.preventDefault();
      });
      this.$root.$on('AnalyzerChanged', this.SetAnalyzer);
    },
    methods: {
      SetAnalyzer: function() {
        this.AnalyzerSelected = (this.$root.SelectedAnalyzer !== null);
      }
    }
  }
</script>

<style scoped>
  .item {
    background-color: #474544;
    color: #FAFAFA;
    width: 304px;
    height: 44px;
    display: flex;
    margin-left: 0;
    transition: margin-left 0.2s;
    text-decoration: none;
  }
  .icon {
    height: 24px;
    width: 24px;
    margin: 10px 12px;
  }
  .dropdown-icon {
    height: 16px;
    width: 16px;
    margin-top: 10px;
    display: none;
  }
  .dropdown-icon.show {
    display: inherit;
  }
  .name {
    width: 180px;
    padding-left: 5px;
    line-height: 24px;
    padding-top: 10px;
  }
  .item:hover, .submenu a:hover, .submenu a.active {
    background: #616161;
    color: #BDBDBD
  }
  /*.item.active, .submenu a.active {
    color: #FAFAFA;
    background: #474544;
  }*/
  .submenu {
    display: block;
    padding: 0px;
    overflow: hidden;
    max-height: 0px;
    transition: max-height 0.2s ease-in-out;
  }
  .submenu.show {
    max-height: 200px;
    transition: max-height 0.2s ease-in-out;
  }
  .submenu a {
    background: #191919;
    display: block;
    color: #BDBDBD;
    padding: 6px 42px;
    text-decoration: none;
  }
</style>