<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><ntp></ntp></div>
    </div>
    <div class="row">
      <div class="col-lg-12"><devices></devices></div>
    </div>
    <div class="row">
      <div class="col-lg-12"><tariffs></tariffs></div>
    </div>
    <div class="row">
      <div class="col-lg-12"><wifi></wifi></div>
    </div>
  </div>
</template>

<script>
  import Ntp from './NTP.vue';
  import Devices from './Devices.vue';
  import Tariffs from './Tariffs.vue';
  import Wifi from './Wifi.vue';
  export default {
    name: 'Dashboard',
    data () {
      return {  };
    },
    components: { Ntp, Devices, Tariffs, Wifi }
  }
</script>

<style>
</style>
