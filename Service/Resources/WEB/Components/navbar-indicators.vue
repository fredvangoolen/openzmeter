<template>
  <div class="indicator-bar">
    <navbar-indicator units="V"  ref="voltage_indicator"   color="indicator-voltage"></navbar-indicator>
    <navbar-indicator units="A"  ref="current_indicator"   color="indicator-current"></navbar-indicator>
    <navbar-indicator units="W"  ref="power_indicator"     color="indicator-active-power"></navbar-indicator>
    <navbar-indicator units="Hz" ref="frequency_indicator" color="indicator-frequency"></navbar-indicator>
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