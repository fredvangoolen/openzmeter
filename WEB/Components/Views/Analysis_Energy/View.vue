<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><active></active></div>
      <div class="col-lg-12"><reactive></reactive></div>
    </div>
  </div>
</template>

<script>
  import Active from './ActiveEnergy.vue';
  import Reactive from './ReactiveEnergy.vue';
  export default {
    data () {
      return { };
    },
    components: { Active, Reactive }
  }
</script>