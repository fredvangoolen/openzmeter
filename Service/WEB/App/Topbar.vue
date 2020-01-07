<template>
  <div>
    <div class="navbar-header">
      <button type="button" class="navbar-toggle"><img :src="require('Imgs/application_menu.png')"></button>
      <div class="navbar-brand">
        <img :src="require('Imgs/Logo.png')">
        <div data-toggle="tooltip" :title="'Uptime: ' + Uptime" v-if="Time != null"><img :src="require('Imgs/system_monitor.png')">{{ Time }}</div>
        <div data-toggle="tooltip" :title="'Disk usage: ' + SDTip" v-if="SDUse != null"><img :src="require('Imgs/sd_memory_adapter.png')">{{ SDUse }}</div>
        <div data-toggle="tooltip" :title="'RAM usage: ' + RAMTip" v-if="RAMUse != null"><img :src="require('Imgs/ddr_memory.png')">{{ RAMUse }}</div>
        <div><img :src="require('Imgs/chip.png')" v-if="CPUUse != null">{{ CPUUse }}</div>
        <!--div data-toggle="tooltip" :title="'WiFi network: ' + WIFINetwork" v-if="WIFIPower != null"><img :src="require('Imgs/network_wireless.png')">{{ WIFIPower }}</div>
        <div data-status="tooltip" :title="'Battery status: ' + BatteryStatus" v-if="BatteryLevel != null"><img :src="require('Imgs/battery.png')">{{ BatteryLevel }}</div-->
        <div v-if="Temp != null"><img :src="require('Imgs/temperature_normal.png')">{{ Temp }}</div>
      </div>

    </div>
    <div class="indicator-bar" v-if="analyzer != null" v-on:click="Grouped = !Grouped">
      <topbar-indicator serie="VOLTAGE" :grouped="Grouped" :value="Values1.Voltage"></topbar-indicator>
      <topbar-indicator serie="CURRENT" :grouped="Grouped" :value="Values1.Current"></topbar-indicator>
      <topbar-indicator serie="ACTIVE_POWER" :grouped="Grouped" :value="Values1.Active_Power"></topbar-indicator>
      <topbar-indicator serie="FREQUENCY" :value="Values1.Frequency" v-if="analyzer.NominalFreq > 0.0"></topbar-indicator>
      <topbar-indicator serie="ACTIVE_ENERGY" :grouped="Grouped" :value="Values2.Active_Energy"></topbar-indicator>
    </div>
  </div>
</template>

<script>
import TopbarIndicator from './TopbarIndicator.vue';
import moment from 'moment';
import * as oZm from 'JS/openZmeter.js';
export default {
  props: { analyzer: { type: Object } },
  data () {
    return { Values1: {}, Values2: {}, Grouped: false};
  },
  created: function() {
    this.Query1 = null;
    this.Timer1 = null;
    this.Query2 = null;
    this.Timer2 = null;
    if(this.analyzer !== null) this.UpdateIndicators1();
    if(this.analyzer !== null) this.UpdateIndicators2();
  },
  beforeDestroy: function() {
    if(this.Query1 !== null) this.Query1.abort();
    if(this.Query2 !== null) this.Query2.abort();
  },
  watch: {
    analyzer: function(analyzer) {
      if(this.Query1 !== null) this.Query1.abort();
      if(this.Query2 !== null) this.Query2.abort();
      if(analyzer !== null) {
        this.UpdateIndicators1();
        this.UpdateIndicators2();
      } else {
        this.Values = {};
      }
    }
  },
  computed: {
    Uptime: function() {
      if((this.Values1.Sys_Stats != undefined) && (this.Values1.Sys_Stats.UpTime != undefined)) return moment.duration(this.Values1.Sys_Stats.UpTime, "seconds").humanize();
      return null;
    },
    Time: function() {
      if(this.Values1.Sampletime != undefined) return moment(this.Values1.Sampletime).format('HH:mm:ss');
      return null;
    },
    SDTip: function() {
      if((this.Values1.Sys_Stats != undefined) && (this.Values1.Sys_Stats.Disk_Total != undefined) && (this.Values1.Sys_Stats.Disk_Usage != undefined)) return oZm.GetBestUnitsBin(this.Values1.Sys_Stats.Disk_Total * this.Values1.Sys_Stats.Disk_Usage / 100.0) + ' of ' + oZm.GetBestUnitsBin(this.Values1.Sys_Stats.Disk_Total);
      return null;
    },
    SDUse: function() {
      if((this.Values1.Sys_Stats != undefined) && (this.Values1.Sys_Stats.Disk_Usage != undefined)) return Math.round(this.Values1.Sys_Stats.Disk_Usage) + '%';
      return null;
    },
    RAMTip: function() {
      if((this.Values1.Sys_Stats != undefined) && (this.Values1.Sys_Stats.RAM_Total != undefined) && (this.Values1.Sys_Stats.RAM_Usage != undefined)) return oZm.GetBestUnitsBin(this.Values1.Sys_Stats.RAM_Total * this.Values1.Sys_Stats.RAM_Usage / 100.0) + ' of ' + oZm.GetBestUnitsBin(this.Values1.Sys_Stats.RAM_Total);
      return null;
    },
    RAMUse: function() {
      if((this.Values1.Sys_Stats != undefined) && (this.Values1.Sys_Stats.RAM_Usage != undefined)) return Math.round(this.Values1.Sys_Stats.RAM_Usage) + '%';
      return null;
    },
    CPUUse: function() {
      if((this.Values1.Sys_Stats != undefined) && (this.Values1.Sys_Stats.CPU_Usage != undefined)) return Math.round(this.Values1.Sys_Stats.CPU_Usage) + '%';
      return null;
    },
    Temp: function() {
      if((this.Values1.Sys_Stats != undefined) && (this.Values1.Sys_Stats.Temperature != undefined)) return Math.round(this.Values1.Sys_Stats.Temperature) + '°';
      return null;
    }
  },
  /*
  watch: {
    value: function(vals) {
      if(vals['Sys_Stats'] != null) {

          if(vals['Sys_Stats'].WIFI_Power != undefined) {
            this.WIFIPower = Math.round(vals['Sys_Stats'].WIFI_Power) + '%';
            this.WIFINetwork = vals['Sys_Stats'].WIFI_Network;
          } else {
            this.WIFIPower = null;
          }
          if(vals['Sys_Stats'].Battery_Level != undefined) {
            this.BatteryLevel = Math.round(vals['Sys_Stats'].Battery_Level) + '%';
            this.BatteryStatus = oZm.RoundPlain(vals['Sys_Stats'].Battery_Voltage, 2) + 'v';
          } else {
            this.BatteryLevel = null;
          }
        }
    }
  },*/
  methods: {
    UpdateIndicators1() {
      if(this.Timer1) clearInterval(this.Timer1);
      this.Query1 = $.ajax({ url: 'getSeriesNow', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['VOLTAGE', 'CURRENT', 'ACTIVE_POWER', 'FREQUENCY', 'SYS_STATS'], Analyzer: this.analyzer.Analyzer }),
        success: (result) => {
          this.$set(this, 'Values1', result);
        },
        complete: () => {
          this.Timer1 = setTimeout(this.UpdateIndicators1, 2000);
        },
        error: () => {
          this.$set(this, 'Values1', {});
        }
      });
    },
    UpdateIndicators2() {
      if(this.Timer2) clearInterval(this.Timer2);
      this.Query2 = $.ajax({ url: 'getSeries', timeout: 20000, type: 'POST', dataType: 'json', contentType: 'application/json',  data: JSON.stringify({ Series: ['ACTIVE_POWER'], LastSample: 0, Analyzer: this.analyzer.Analyzer, Aggreg: '15M', To: moment().endOf('day').valueOf(), Period: 86400000 }),
        success: (result) => {
          var Energy;
          if(result['Active_Power'].length > 0) Energy = Array(result['Active_Power'][0].length).fill(0);
          for(var n = 0; n < result['Active_Power'].length; n++) {
            for(var p = 0; p < result['Active_Power'][n].length; p++) {
              if(result['Active_Power'][n][p] === null) continue;
              Energy[p] = Energy[p] + result['Active_Power'][n][p] / 4.0;
            }
          }
          this.Values2["Active_Energy"] = Energy;
        },
        complete: () => {
          this.Timer2 = setTimeout(this.UpdateIndicators2, 1000000);
        },
        error: () => {
          this.$set(this, 'Values2', {});
        }
      });
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
    z-index: 90;
    height: 56px;
  }
  .navbar-toggle {
    display: block;
    padding: 6px 5px;
    right: 0px;
    position: absolute;
    margin: 3px;
    color: #FFF;
    border: 1px solid #333;
    background-color: transparent;
  }
  .navbar-toggle img {
    margin: 2px 0px;
  }
  @media(min-width: 1280px) {
    .navbar-toggle {
      display: none;
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
  .navbar-brand {
    text-align: left;
    padding: 0px 6px;
  }
  .indicator-bar {
    cursor: grabbing;
    background-image: linear-gradient(to bottom,#f3f3f3,#e2e2e2);
    text-align: center;
    font-size: 14px;
    color: #FFF;
    font-weight: 700;
    padding: 2px;
    border-bottom: 1px solid #c9c9c9;
    position: relative;
    z-index: 95;
    font-size: 18x;
    top: 0px;
  }
  @media(min-width:640px) {
    .indicator-bar {
      background-image: none;
      position: absolute;
      right: 48px;
      text-align: right;
      border: none;
    }
  }
  @media(min-width:1280px) {
    .indicator-bar {
      right: 2px;
    }
  }
</style>