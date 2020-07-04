<template>
  <div>
    <router-link to="/Dashboard" class="item" v-if="analyzer!=null">
      <img class="icon" :src="require('Imgs/dashboard.png')">
      <div class="name">Dashboard</div>
    </router-link>
    <div class="item" @click="Submenu = (Submenu == 'Analysis') ? null : 'Analysis'" v-if="analyzer!=null">
      <img class="icon" :src="require('Imgs/draw_wave.png')">
      <div class="name">Analysis</div>
      <span class="dropdown-icon" :class="{ show: (Submenu != 'Analysis') }"><img :src="require('Imgs/bullet_arrow_down.png')"></span>
      <span class="dropdown-icon" :class="{ show: (Submenu == 'Analysis') }"><img :src="require('Imgs/bullet_arrow_up.png')"></span>
    </div>
    <div class="submenu" :class="{ show: (Submenu == 'Analysis') }" v-if="analyzer!=null">
      <router-link to="/Analysis/Voltage">Voltage</router-link>
      <router-link to="/Analysis/Current">Current</router-link>
      <router-link to="/Analysis/Power">Power</router-link>
      <router-link to="/Analysis/Energy">Energy</router-link>
      <router-link to="/Analysis/Frequency" v-if="analyzer.NominalFrequency!=0">Frequency</router-link>
      <router-link to="/Analysis/200MS" v-if="analyzer.Save200MS==true">200 MS</router-link>
    </div>
    <div class="item" @click="Submenu = (Submenu == 'Events') ? null : 'Events'" v-if="analyzer!=null">
      <img class="icon" :src="require('Imgs/warning.png')">
      <span class="badge badge-menu" v-if="(UnreadEvents+UnreadAlarms)>0">{{ UnreadEvents+UnreadAlarms }}</span>
      <div class="name">Events</div>
      <span class="dropdown-icon" :class="{ show: (Submenu != 'Events') }"><img :src="require('Imgs/bullet_arrow_down.png')"></span>
      <span class="dropdown-icon" :class="{ show: (Submenu == 'Events') }"><img :src="require('Imgs/bullet_arrow_up.png')"></span>
    </div>
    <div class="submenu" :class="{ show: (Submenu == 'Events') }" v-if="analyzer!=null">
      <router-link to="/Events/ITIC">ITIC</router-link>
      <a @click="ShowDisturbances">Disturbances<span class="badge badge-submenu" v-if="UnreadEvents>0">{{ UnreadEvents }}</span></a>
      <router-link to="/Events/EN50160">EN50160</router-link>
      <a @click="ShowAlarms">Alarms<span class="badge badge-submenu" v-if="UnreadAlarms>0">{{ UnreadAlarms }}</span></a>
    </div>
    <div class="item" @click="Submenu = (Submenu == 'Costs') ? null : 'Costs'" v-if="analyzer!=null">
      <img class="icon" :src="require('Imgs/table_money.png')">
      <div class="name">Energy Stats</div>
      <span class="dropdown-icon" :class="{ show: (Submenu != 'Costs') }"><img :src="require('Imgs/bullet_arrow_down.png')"></span>
      <span class="dropdown-icon" :class="{ show: (Submenu == 'Costs') }"><img :src="require('Imgs/bullet_arrow_up.png')"></span>
    </div>
    <div class="submenu" :class="{ show: (Submenu == 'Costs') }" v-if="analyzer!=null">
      <router-link class="link" to="/Stats/Costs">Costs</router-link>
      <router-link class="link" to="/Stats/Heatmap">Heatmap</router-link>
      <router-link class="link" to="/Stats/Distribution">Distribution</router-link>
    </div>
    <router-link to="/Settings" class="item">
      <img class="icon" :src="require('Imgs/cog.png')">
      <div class="name">Settings</div>
    </router-link>
    <router-link to="/About" class="item">
      <img class="icon" :src="require('Imgs/help.png')">
      <div class="name">About</div>
    </router-link>
  </div>
</template>

<script>
  import moment from 'moment';
  export default {
    props: { analyzer:Object, expanded: Boolean },
    data() {
      return { Submenu: null, AnalyzerSelected: false, TransferNow: this.CreateSeriesTransfer(this, ['UNREAD']), TitleFrame: 0 }
    },
    created: function() {
      this.TransferNow.UpdateNow(this.analyzer);
      setInterval(() => { this.SetTitle(); }, 2000)
    },
    computed: {
      UnreadEvents: function() {
        if(this.TransferNow.Data == null) return 0;
        return this.TransferNow.Data.Unread.Events;
      },
      UnreadAlarms: function() {
        if(this.TransferNow.Data == null) return 0;
        return this.TransferNow.Data.Unread.Alarms;
      }
    },
    watch: {
      expanded: function(val) {
        if(val == false) this.Submenu = null;
      },
      analyzer: function() {
        this.TransferNow.UpdateNow(this.analyzer);
      }
    },
    methods: {
      SetTitle: function() {
        if(this.TitleFrame == 0) {
          document.title = "openZmeter";
          this.TitleFrame = 1;
        } else if(this.TitleFrame == 1) {
          if((this.UnreadEvents != 0) && (this.UnreadAlarms != 0)) {
            document.title = '[' + this.UnreadEvents + ' new events and ' + this.UnreadAlarms + ' new alarms]';
          } else if(this.UnreadEvents != 0) {
            document.title = '[' + this.UnreadEvents + ' new events]';
          } else if(this.UnreadAlarms != 0) {
            document.title = '[' + this.UnreadAlarms + ' new alarms]';
          } else {
            document.title = "openZmeter";
          }
          this.TitleFrame = 0;
        }
      },
      ShowDisturbances: function() {
        if(this.UnreadEvents != 0) {
          this.$router.push({ path: '/Events/Disturbances', query: { From: this.TransferNow.Data.Unread.EventsFrom, To: this.TransferNow.Data.Sampletime } });
        } else {
          this.$router.push('/Events/Disturbances');
        }
      },
      ShowAlarms: function() {
        if(this.UnreadAlarms != 0) {
          this.$router.push({ path: '/Events/Alarms', query: { From: this.TransferNow.Data.Unread.AlarmsFrom, To: this.TransferNow.Data.Sampletime } });
        } else {
          this.$router.push('/Events/Alarms');
        }
      }
    },
    mounted: function() {
      $('.sidenav-menu').on('click', '[data-sidenav-dropdown-toggle]', function (e) {
        $(this).parent().parent().find("[data-sidenav-dropdown]:visible").not($(this).next('[data-sidenav-dropdown]')).slideUp("fast");
        $(this).next('[data-sidenav-dropdown]').slideToggle('fast');
        $(this).find('[data-sidenav-dropdown-icon]').toggleClass('show');
        e.preventDefault();
      });
    },
    beforeDestroy: function() {
      this.TransferNow.Destroy();
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
    position: relative;
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
    position: relative;
  }
  .badge-menu {
    position: absolute;
    top: 24px;
    right: 260px;
    font-size: 9px;
    padding: 1px 2px;
    background: #c9302c;
    border-radius: 3px;
  }
  .badge-submenu {
    position: absolute;
    top: 8px;
    right: 10px;
    border-radius: 3px;
    background: #c9302c;
  }
</style>