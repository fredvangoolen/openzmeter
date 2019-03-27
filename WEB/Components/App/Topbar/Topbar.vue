<template>
  <div>
    <div class="navbar-header">
      <div class="navbar-brand">
        <img :src="require('Imgs/Logo.png')">
        <div>Node time: {{ time }}</div>
      </div>
      <button type="button" class="navbar-toggle"><img :src="require('Imgs/application_menu.png')"></button>
    </div>
    <div class="indicator-bar">
      <topbar-indicator units="V"  ref="voltage_indicator"   color="indicator-voltage"></topbar-indicator>
      <topbar-indicator units="A"  ref="current_indicator"   color="indicator-current"></topbar-indicator>
      <topbar-indicator units="W"  ref="power_indicator"     color="indicator-active-power"></topbar-indicator>
      <topbar-indicator units="Hz" ref="frequency_indicator" color="indicator-frequency"></topbar-indicator>
      <div class="indicator-active-energy">
        <div>
          <div id="IndicatorTodayEnergyValues">
            <span class="value">Offline</span>
          </div>
          <div id="IndicatorActiveEnergyUnits">Wh</div>
        </div>
      </div>
    </div>
  </div>
</template>
 
<script>
import TopbarIndicator from './TopbarIndicator.vue';
import moment from 'JS/moment.js';
export default {
  methods: {
    setValue: function(vals) {
      if(vals === null) {
        this.$refs.voltage_indicator.setValue(null);
        this.$refs.current_indicator.setValue(null);
        this.$refs.power_indicator.setValue(null);
        this.$refs.frequency_indicator.setValue(null);
        this.time = 'No analyzer selected';  
      } else {
        this.$refs.voltage_indicator.setValue(vals['Voltage']);
        this.$refs.current_indicator.setValue(vals['Current']);
        this.$refs.power_indicator.setValue(vals['Active_Power']);
        this.$refs.frequency_indicator.setValue([vals['Frequency']]);
        this.time = moment(vals.Sampletime).format('DD/MM/YYYY - HH:mm:ss');  
      }
    }
  },
  data () {
    return {time: 'No analyzer selected' };
  },
  components: { TopbarIndicator }
}
</script>
 
<style scoped>
  img {
    margin: 4px 0px;
  }
  .navbar-brand > embed {
    width: 100%;
  }
  .navbar-brand > div {
    font-size: 12px; 
    color: #FFF; 
    margin-top: -5px;
  }
  .navbar-brand {
    text-align: left;
    width: 260px;
    padding: 6px;
    height: 50px;     
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