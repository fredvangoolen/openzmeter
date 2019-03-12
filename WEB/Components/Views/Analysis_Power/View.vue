<template>
  <div id="wrapper">
    <div class="row">
      <div class="col-lg-12"><active></active></div>
      <div class="col-lg-12"><reactive></reactive></div>
      <div class="col-lg-12"><details></details></div>
    </div>
  </div>
</template>

<script>
  import Active from './ActivePower.vue';
  import Reactive from './ReactivePower.vue';
  import Details from './Details.vue';
  export default {
    data () {
      return { };
    },
    components: { Active, Reactive, Details }
  }
</script>