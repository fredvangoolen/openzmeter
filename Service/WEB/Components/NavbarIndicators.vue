<template>
  <div class="indicator-bar">
    <navbar-indicators-panel units="V"  ref="voltage_indicator"   color="indicator-voltage"></navbar-indicators-panel>
    <navbar-indicators-panel units="A"  ref="current_indicator"   color="indicator-current"></navbar-indicators-panel>
    <navbar-indicators-panel units="W"  ref="power_indicator"     color="indicator-active-power"></navbar-indicators-panel>
    <navbar-indicators-panel units="Hz" ref="frequency_indicator" color="indicator-frequency"></navbar-indicators-panel>
    <div class="indicator-active-energy">
      <div>
        <div id="IndicatorTodayEnergyValues">
          <span class="value">Offline</span>
        </div>
        <div id="IndicatorActiveEnergyUnits">Wh</div>
      </div>
    </div>
  </div>
</template>
 
<script>
import NavbarIndicatorsPanel from './NavbarIndicatorsPanel.vue';
export default {
  methods: {
    setValue: function(vals) {
      if(vals === null) {
        this.$refs.voltage_indicator.setValue(null);
        this.$refs.current_indicator.setValue(null);
        this.$refs.power_indicator.setValue(null);
        this.$refs.frequency_indicator.setValue(null);
      } else {
        this.$refs.voltage_indicator.setValue(vals['Voltage']);
        this.$refs.current_indicator.setValue(vals['Current']);
        this.$refs.power_indicator.setValue(vals['Active_Power']);
        this.$refs.frequency_indicator.setValue([vals['Frequency']]);
      }
    }
  },
  data () {
    return { };
  },
  components: { NavbarIndicatorsPanel }
}
</script>
 
<style>
.indicator-bar {
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
@media(min-width:768px) {
  .indicator-bar {
    background-image: none;
    position: absolute;
    right: 48px;
    text-align: right;
    border: none;
  }
}
@media(min-width:1024px) {
  .indicator-bar {
    right: 2px;
  }
}  
</style>