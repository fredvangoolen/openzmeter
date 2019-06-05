<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><devices></devices></div>
      <div class="col-lg-12"><tariffs></tariffs></div>
      <div class="col-lg-12"><connectivity></connectivity></div>
    </div>
  </div>
</template>

<script>

  import Devices from './Devices.vue';
  import Tariffs from './Tariffs.vue';
  import Connectivity from './Connectivity.vue';
  export default {
    name: 'Dashboard',
    data () {
      return {  };
    },
    components: { Devices, Tariffs, Connectivity }
  }
</script>
