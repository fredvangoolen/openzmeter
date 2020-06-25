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
      <topbar-indicator :analyzer="analyzer" serie="Voltage" :grouped="Grouped" :value="Transfer1.Data"></topbar-indicator>
      <topbar-indicator :analyzer="analyzer" serie="Current" :grouped="Grouped" :value="Transfer1.Data"></topbar-indicator>
      <topbar-indicator :analyzer="analyzer" serie="Active_Power" :grouped="Grouped" :value="Transfer1.Data"></topbar-indicator>
      <topbar-indicator :analyzer="analyzer" serie="Frequency" :value="Transfer1.Data" v-if="analyzer.NominalFrequency > 0.0"></topbar-indicator>
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
      return { Grouped: false,
               Transfer1: this.CreateSeriesTransfer(this, ['VOLTAGE', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY', 'SYS_STATS']),
               Transfer2: this.CreateSeriesTransfer(this, ['ACTIVE_POWER']) };
    },
    created: function() {
      this.Transfer1.UpdateNow(this.analyzer);
      this.Transfer2.Update(this.analyzer, '15M', 'Day');
    },
    beforeDestroy: function() {
      this.Transfer1.Destroy();
      this.Transfer2.Destroy();
    },
    watch: {
      analyzer: function(analyzer) {
        this.$nextTick(() => {
          this.Transfer1.UpdateNow(this.analyzer);
          this.Transfer2.Update(this.analyzer, '15M', 'Day');
        });
      }
    },
    computed: {
      Uptime: function() {
        return ((this.Transfer1.Data != null) && (this.Transfer1.Data.Sys_Stats.UpTime != null)) ? moment.duration(this.Transfer1.Data.Sys_Stats.UpTime, "seconds").humanize() : null;
      },
      Time: function() {
        return (this.Transfer1.Data != null) ? moment(this.Transfer1.Data.Sampletime).format('HH:mm:ss') : null;
      },
      SDTip: function() {
        return ((this.Transfer1.Data != null) && (this.Transfer1.Data.Sys_Stats.Disk_Total != null) && (this.Transfer1.Data.Sys_Stats.Disk_Usage != null)) ? this.BestUnitsBin(this.Transfer1.Data.Sys_Stats.Disk_Total * this.Transfer1.Data.Sys_Stats.Disk_Usage / 100.0) + ' of ' + this.BestUnitsBin(this.Transfer1.Data.Sys_Stats.Disk_Total) : null;
      },
      SDUse: function() {
        return ((this.Transfer1.Data != null) && (this.Transfer1.Data.Sys_Stats.Disk_Usage != null)) ? Math.round(this.Transfer1.Data.Sys_Stats.Disk_Usage) + '%' : null;
      },
      RAMTip: function() {
        return ((this.Transfer1.Data != null) && (this.Transfer1.Data.Sys_Stats.RAM_Total != null) && (this.Transfer1.Data.Sys_Stats.RAM_Usage != null)) ? this.BestUnitsBin(this.Transfer1.Data.Sys_Stats.RAM_Total * this.Transfer1.Data.Sys_Stats.RAM_Usage / 100.0) + ' of ' + this.BestUnitsBin(this.Transfer1.Data.Sys_Stats.RAM_Total) : null;
      },
      RAMUse: function() {
        return ((this.Transfer1.Data != null) && (this.Transfer1.Data.Sys_Stats.RAM_Usage != null)) ? Math.round(this.Transfer1.Data.Sys_Stats.RAM_Usage) + '%' : null;
      },
      CPUUse: function() {
        return ((this.Transfer1.Data != null) && (this.Transfer1.Data.Sys_Stats.CPU_Usage != null)) ? Math.round(this.Transfer1.Data.Sys_Stats.CPU_Usage) + '%' : null;
      },
      Temp: function() {
        return ((this.Transfer1.Data != null) && (this.Transfer1.Data.Sys_Stats.Temperature != null)) ? Math.round(this.Transfer1.Data.Sys_Stats.Temperature) + '°' : null;
      },
      WIFINetwork: function() {
        return ((this.Transfer1.Data != null) && (this.Transfer1.Data.Sys_Stats.WIFI_Network != null)) ? this.Transfer1.Data.Sys_Stats.WIFI_Network : null;
      },
      WIFIPower: function() {
        return ((this.Transfer1.Data != null) && (this.Transfer1.Data.Sys_Stats.WIFI_Power != null)) ? Math.round(this.Transfer1.Data.Sys_Stats.WIFI_Power) + '%' : null;
      },
      BatteryLevel: function() {
        return null;
      },
      BatteryStatus: function() {
        return null;
      },
      EnergyUsage: function() {
        var Energy = new Array(this.analyzer.VoltageChannels).fill(0);
        if(this.Transfer2.Data == null) return { Active_Energy: Energy };
        for(var n = 0; n < this.Transfer2.Data.Active_Power.length; n++) {
          for(var p = 0; p < this.Transfer2.Data['Active_Power'][n].length; p++) {
            if(this.Transfer2.Data['Active_Power'][n][p] === null) continue;
            Energy[p] = Energy[p] + this.Transfer2.Data['Active_Power'][n][p] / 4.0;
          }
        }
        return { Active_Energy: Energy };
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
    margin-left: 48px;
  }
  @media(min-width: 1280px) {
    .navbar-toggle {
      display: none;
    }
    .navbar-brand {
      margin-left: 0px;
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