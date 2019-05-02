<template>
  <div>
    <div class="navbar-header">
      <div class="navbar-brand">
        <img :src="require('Imgs/Logo.png')">
          <div data-toggle="tooltip" :title="'Uptime: ' + Uptime"><img :src="require('Imgs/system_monitor.png')">{{ Time }}</div>
          <div data-toggle="tooltip" :title="'Disk usage: ' + SDTip"><img :src="require('Imgs/sd_memory_adapter.png')">{{ SDUse }}</div>
          <div data-toggle="tooltip" :title="'RAM usage: ' + RAMTip"><img :src="require('Imgs/ddr_memory.png')">{{ RAMUse }}</div>
          <div><img :src="require('Imgs/chip.png')">{{ CPUUse }}</div>
          <div><img :src="require('Imgs/network_wireless.png')">45%</div>
          <div><img :src="require('Imgs/battery.png')">100%</div>
          <div><img :src="require('Imgs/temperature_normal.png')">{{ Temp }}</div>
      </div>
      <button type="button" class="navbar-toggle"><img :src="require('Imgs/application_menu.png')"></button>
    </div>
    <div class="indicator-bar">
      <topbar-indicator units="V"  ref="voltage_indicator"   color="indicator-voltage"></topbar-indicator>
      <topbar-indicator units="A"  ref="current_indicator"   color="indicator-current"></topbar-indicator>
      <topbar-indicator units="W"  ref="power_indicator"     color="indicator-active-power"></topbar-indicator>
      <topbar-indicator units="Hz" ref="frequency_indicator" color="indicator-frequency"></topbar-indicator>
      <topbar-indicator units="Wh" ref="energy_indicator"    color="indicator-active-energy"></topbar-indicator>
    </div>
  </div>
</template>
 
<script>
import TopbarIndicator from './TopbarIndicator.vue';
import moment from 'JS/moment.js';
import * as oZm from 'JS/openZmeter.js';
export default {
  methods: {
    setValue: function(vals) {
      if(vals === null) {
        this.$refs.voltage_indicator.setValue(null);
        this.$refs.current_indicator.setValue(null);
        this.$refs.power_indicator.setValue(null);
        this.$refs.frequency_indicator.setValue(null);
        this.$refs.energy_indicator.setValue(null);
        this.Time = 'Offline';
        this.Uptime = '';
        this.SDUse = '-';
        this.SDTip = '';
        this.RAMUse = '-';
        this.RAMTip = '';
        this.CPUUse = '-';
        this.WIFIPower = '-';
        this.WIFINetwork = '';
        this.BatteryLevel = '-';
        this.BatteryStatus = '';
        this.Temp = '-';
      } else {
        if(vals['Voltage'] !== undefined) this.$refs.voltage_indicator.setValue(vals['Voltage']);
        if(vals['Current'] !== undefined) this.$refs.current_indicator.setValue(vals['Current']);
        if(vals['Active_Power'] !== undefined) this.$refs.power_indicator.setValue(vals['Active_Power']);
        if(vals['Frequency'] !== undefined) this.$refs.frequency_indicator.setValue([vals['Frequency']]);
        if(vals['Active_Energy'] !== undefined) this.$refs.energy_indicator.setValue(vals['Active_Energy']);
        if(vals['Sampletime'] !== undefined)  this.Time = moment(vals['Sampletime']).format('HH:mm:ss');
        if(vals['Sys_Stats'] !== undefined) {
          this.Uptime = moment.duration(vals['Sys_Stats'].UpTime, "seconds").humanize();
          this.SDUse = Math.round(vals['Sys_Stats'].Disk_Usage) + '%';
          this.SDTip = oZm.GetBestUnitsBin(vals['Sys_Stats'].Disk_Total * vals['Sys_Stats'].Disk_Usage / 100.0) + ' of ' + oZm.GetBestUnitsBin(vals['Sys_Stats'].Disk_Total);
          this.RAMUse = Math.round(vals['Sys_Stats'].RAM_Usage) + '%';
          this.RAMTip = oZm.GetBestUnitsBin(vals['Sys_Stats'].RAM_Total * vals['Sys_Stats'].RAM_Usage / 100.0) + ' of ' + oZm.GetBestUnitsBin(vals['Sys_Stats'].RAM_Total);
          this.CPUUse = Math.round(vals['Sys_Stats'].CPU_Usage) + '%';
          this.WIFIPower = '-';
          this.WIFINetwork = '';
          this.BatteryLevel = '-';
          this.BatteryStatus = '';
          this.Temp = Math.round(vals['Sys_Stats'].Temperature*1000) + 'º';;
        }
      }
    }
  },
  data () {
    return {Time: 'Offline', Uptime: '', SDUse: '-', SDTip: '', RAMUse: '-', RAMTip: '', CPUUse: '-', WIFIPower: '-', WIFINetwork: '', BatteryLevel: '-', BatteryStatus: '', Temp: '-' };
  },
  components: { TopbarIndicator }
}
</script>
 
<style scoped>
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
  .navbar-brand {
    text-align: left;
    padding: 0px 6px;
  }
  .navbar-toggle img {
    margin: 2px 0px;
  }
  .navbar-toggle {
    padding: 5px 5px;
    right: 0px;
    position: absolute;
    margin: 3px;
    color: #FFF;
    border: 1px solid #333;
    background-color: transparent;
  }
  .navbar-header {
    position: relative;
    background-color: #474544;
    width: 100%;
    z-index: 90;
    height: 56px;
  }
</style>