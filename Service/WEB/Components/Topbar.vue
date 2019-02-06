<template>
  <div>
    <b-navbar>
      <b-navbar-brand>
        <embed src="Imgs/Logo2.svg" type="image/svg+xml"/><br/>
        <div>Node time: {{ time }}</div>
      </b-navbar-brand>
      <b-navbar-toggle target="nav_collapse"><img src="Imgs/application_side_expand.png"></b-navbar-toggle>
    </b-navbar>
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
import moment from 'moment';
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
        this.time = moment(vals.Sampletime).format('DD/MM/YYYY HH:mm:ss');  
      }
    }
  },
  data () {
    return {time: 'No analyzer selected' };
  },
  components: { TopbarIndicator }
}
</script>
 
<style>
  .navbar {
    background-color: #474544;
    height: 56px;
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
    padding: 0px;
    height: 50px;     
  }
  .navbar-toggler {
    padding: 9px 5px;
    right: 2px;
    position: absolute;
  }
</style>