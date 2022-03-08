<template>
  <z-row>
    <div class="col-lg-12"><devices></devices></div>
    <div class="col-lg-12"><tariffs></tariffs></div>
    <div class="col-lg-12"><connectivity></connectivity></div>
    <div class="col-lg-12"><users></users></div>
  </z-row>
</template>

<script>
  import Users from './Users.vue';
  import Devices from './Devices.vue';
  import Tariffs from './Tariffs.vue';
  import Connectivity from './Connectivity.vue';
  export default {
    name: 'Dashboard',
    data () {
      return {  };
    },
    components: { Users, Devices, Tariffs, Connectivity }
  }
</script>
