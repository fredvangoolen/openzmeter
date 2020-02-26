<template>
  <div>
    <div class="navbar-header">
      <button type="button" class="navbar-toggle" @mouseenter="$emit('update:expanded', true)" @click="$emit('update:expanded', true)"><img :src="require('Imgs/application_menu.png')"></button>
      <div class="navbar-brand">
        <img :src="require('Imgs/Logo.png')">
        <div data-toggle="tooltip" :title="'Uptime: ' + Uptime" v-if="Time != null"><img :src="require('Imgs/system_monitor.png')">{{ Time }}</div>
        <div data-toggle="tooltip" :title="'Disk usage: ' + SDTip" v-if="SDUse != null"><img :src="require('Imgs/sd_memory_adapter.png')">{{ SDUse }}</div>
        <div data-toggle="tooltip" :title="'RAM usage: ' + RAMTip" v-if="RAMUse != null"><img :src="require('Imgs/ddr_memory.png')">{{ RAMUse }}</div>
        <div><img :src="require('Imgs/chip.png')" v-if="CPUUse != null">{{ CPUUse }}</div>
        <div data-toggle="tooltip" :title="'WiFi network: ' + WIFINetwork" v-if="WIFINetwork != null"><img :src="require('Imgs/network_wireless.png')">{{ WIFIPower }}</div>
        <div data-status="tooltip" :title="'Battery status: ' + BatteryStatus" v-if="BatteryLevel != null"><img :src="require('Imgs/battery.png')">{{ BatteryLevel }}</div>
        <div v-if="Temp != null"><img :src="require('Imgs/temperature_normal.png')">{{ Temp }}</div>
      </div>
    </div>
    <div class="indicator-bar" v-if="analyzer != null" @click="Grouped = !Grouped">
      <topbar-indicator :analyzer="analyzer" serie="Voltage" :grouped="Grouped" :value="Values1"></topbar-indicator>
      <topbar-indicator :analyzer="analyzer" serie="Current" :grouped="Grouped" :value="Values1"></topbar-indicator>
      <topbar-indicator :analyzer="analyzer" serie="Active_Power" :grouped="Grouped" :value="Values1"></topbar-indicator>
      <topbar-indicator :analyzer="analyzer" serie="Frequency" :value="Values1" v-if="analyzer.NominalFrequency > 0.0"></topbar-indicator>
      <topbar-indicator :analyzer="analyzer" serie="Active_Energy" :grouped="Grouped" :value="EnergyUsage"></topbar-indicator>
    </div>
  </div>
</template>

<script>
  import TopbarIndicator from './TopbarIndicator.vue';
  import moment from 'moment';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { analyzer: Object, expanded: Boolean },
    data () {
      return { Values1: { }, Data: null, Grouped: false, ControlGroup1: null, ControlGroup2: null };
    },
    created: function() {
      this.UpdateIndicators1(true);
      this.UpdateIndicators2(true);
    },
    beforeDestroy: function() {
      this.CancelTransfer(this.ControlGroup1);
      this.CancelTransfer(this.ControlGroup2);
    },
    watch: {
      analyzer: function(analyzer) { 
        this.$nextTick(() => { this.UpdateIndicators1(true); this.UpdateIndicators2(true); }); 
      }
    },
    computed: {
      Uptime: function() {
        if((this.Values1.Sys_Stats != undefined) && (this.Values1.Sys_Stats.UpTime != undefined)) return moment.duration(this.Values1.Sys_Stats.UpTime, "seconds").humanize();
        return null;
      },
      Time: function() {
        if(this.Values1.Sampletime != null) return moment(this.Values1.Sampletime).format('HH:mm:ss');
        return null;
      },
      SDTip: function() {
        if((this.Values1.Sys_Stats != null) && (this.Values1.Sys_Stats.Disk_Total != null) && (this.Values1.Sys_Stats.Disk_Usage != null)) return oZm.GetBestUnitsBin(this.Values1.Sys_Stats.Disk_Total * this.Values1.Sys_Stats.Disk_Usage / 100.0) + ' of ' + oZm.GetBestUnitsBin(this.Values1.Sys_Stats.Disk_Total);
        return null;
      },
      SDUse: function() {
        if((this.Values1.Sys_Stats != null) && (this.Values1.Sys_Stats.Disk_Usage != null)) return Math.round(this.Values1.Sys_Stats.Disk_Usage) + '%';
        return null;
      },
      RAMTip: function() {
        if((this.Values1.Sys_Stats != null) && (this.Values1.Sys_Stats.RAM_Total != null) && (this.Values1.Sys_Stats.RAM_Usage != null)) return oZm.GetBestUnitsBin(this.Values1.Sys_Stats.RAM_Total * this.Values1.Sys_Stats.RAM_Usage / 100.0) + ' of ' + oZm.GetBestUnitsBin(this.Values1.Sys_Stats.RAM_Total);
        return null;
      },
      RAMUse: function() {
        if((this.Values1.Sys_Stats != null) && (this.Values1.Sys_Stats.RAM_Usage != null)) return Math.round(this.Values1.Sys_Stats.RAM_Usage) + '%';
        return null;
      },
      CPUUse: function() {
        if((this.Values1.Sys_Stats != null) && (this.Values1.Sys_Stats.CPU_Usage != null)) return Math.round(this.Values1.Sys_Stats.CPU_Usage) + '%';
        return null;
      },
      Temp: function() {
        if((this.Values1.Sys_Stats != null) && (this.Values1.Sys_Stats.Temperature != null)) return Math.round(this.Values1.Sys_Stats.Temperature) + '°';
        return null;
      },
      WIFINetwork: function() {
        if((this.Values1.Sys_Stats != null) && (this.Values1.Sys_Stats.WIFI_Network != null)) return this.Values1.Sys_Stats.WIFI_Network;
        return null;
      },
      WIFIPower: function() {
        if((this.Values1.Sys_Stats != null) && (this.Values1.Sys_Stats.WIFI_Power != null)) return Math.round(this.Values1.Sys_Stats.WIFI_Power) + '%';
        return null;
      },
      BatteryLevel: function() {
        return null;
      },
      BatteryStatus: function() {
        return null;
      },
      EnergyUsage: function() {
        var Energy = Array(this.analyzer.VoltageChannels).fill(0);
        if(this.Data == null) return { Active_Energy: Energy };
        for(var n = 0; n < this.Data.Active_Power.length; n++) {
          for(var p = 0; p < this.Data['Active_Power'][n].length; p++) {
            if(this.Data['Active_Power'][n][p] === null) continue;
            Energy[p] = Energy[p] + this.Data['Active_Power'][n][p] / 4.0;
          }
        }
        return { Active_Energy: Energy };
      }
    },
    methods: {
      UpdateIndicators1: function(clear) {
        if(clear == true) this.Values1 = {};
        this.ControlGroup1 = this.CancelTransfer(this.ControlGroup1);
        if(this.analyzer == null) return;
        this.ControlGroup1.CancelFunc = this.analyzer.GetSeriesNow(['VOLTAGE', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY', 'SYS_STATS'],
          (working) => { },
          (result) => { this.Values1 = Object.seal(result) },
          (timeout) => { this.ControlGroup1.Timer = setTimeout( () => { this.UpdateIndicators1(false); }, timeout); }
        );
      },
      UpdateIndicators2: function(clear) {
        if(clear == true) this.Data = null;
        this.ControlGroup2 = this.CancelTransfer(this.ControlGroup2);
        if(this.analyzer == null) return;
        var From = moment().tz(this.analyzer.TimeZone).startOf('day').valueOf();
        var To = moment().tz(this.analyzer.TimeZone).endOf('day').valueOf();
        this.ControlGroup1.CancelFunc = this.analyzer.GetSeriesRange('15M', [From, To], this.Data, ['ACTIVE_POWER'],
          (working) => { },
          (result) => { this.Data = Object.seal(result) },
          (timeout) => { this.ControlGroup1.Timer = setTimeout( () => { this.UpdateIndicators2(false); }, timeout); }
        );
      }
    },
    components: { TopbarIndicator }
  }
</script>

<style scoped>
  .navbar-header {
    position: relative;
    background-color: #474544;
    width: 100%;
    z-index: 95;
    height: 56px;
  }
  .navbar-toggle {
    padding: 6px 5px;
    left: 0px;
    position: absolute;
    margin: 3px;
    color: #FFF;
    border: 1px solid #333;
    background-color: transparent;
    display: block;
    z-index: 100;
  }
  .navbar-toggle img {
    margin: 2px 0px;
  }
  .navbar-brand {
    padding: 0px 6px;
  }
  @media(min-width: 1280px) {
    .navbar-toggle {
      display: none;
    }
  }
  @media(max-width: 1280px) {
    .navbar-brand {
      margin-left: 48px;
    }
  }
  .navbar-brand > img {
    margin-top: 4px;
  }
  .navbar-brand > div {
    font-size: 10px;
    text-align: center;
    color: #FFF;
    line-height: 16px;
    margin-right: 2px;
    display: inline-block;
  }
  .navbar-brand > div > img {
    margin: 0px 2px;
  }
  .sys-stats {
    font-size: 10px;
    text-align: center;
    color: #FFF;
    width: 100%;
    line-height: 16px;
  }
  .sys-stats img {
    margin-right: 1px;
  }
  .indicator-bar {
    cursor: grabbing;
    background-image: linear-gradient(to bottom,#f3f3f3,#e2e2e2);
    text-align: center;
    font-size: 14px;
    color: #FFF;
    font-weight: 700;
    padding-top: 2px;
    border-bottom: 1px solid #c9c9c9;
    position: relative;
    z-index: 90;
    top: 0px;
  }
  @media(min-width:640px) {
    .indicator-bar {
      background-image: none;
      position: absolute;
      right: 2px;
      text-align: right;
      z-index: 95;
      border: none;
    }
  }
  @media(min-width:1280px) {
    .indicator-bar {
      right: 2px;
    }
  }
</style>