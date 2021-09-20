<template>
  <q-page>
    <devices class="q-mb-sm"></devices>
    <tariffs class="q-mb-sm"></tariffs>
    <connectivity class="q-mb-sm"></connectivity>
    <users class="q-mb-sm"></users>
  </q-page>
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
