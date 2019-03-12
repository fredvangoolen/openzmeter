<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><active-power></active-power></div>
      <div class="col-lg-12"><reactive-power></reactive-power></div>
      <div class="col-lg-12"><details-power></details-power></div>
    </div>
  </div>
</template>

<script>
  import ActivePower from './ActivePower.vue';
  import ReactivePower from './ReactivePower.vue';
  import DetailsPower from './DetailsPower.vue';
  export default {
    data () {
      return { };
    },
    components: { ActivePower, ReactivePower, DetailsPower }
  }
</script>