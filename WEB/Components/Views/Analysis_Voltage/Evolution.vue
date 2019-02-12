<template>
  <div id="EvolutionPanel">
    <cubism v-for="(serie, index) in Series" :serie="serie" :key="serie" :first="index == 0"></cubism>
  </div>
</template>

<script>
  import Cubism from 'Components/Cubism.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data() {
      return {Series: ['1']};
      
    },
    components: { Cubism }
  }
</script>
