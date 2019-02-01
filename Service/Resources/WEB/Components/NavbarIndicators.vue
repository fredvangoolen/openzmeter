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
module.exports = {
  props: ['interval'],
  data: function() {
          return {
          }
        },
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
  }
}
</script>
 
<style>

</style>