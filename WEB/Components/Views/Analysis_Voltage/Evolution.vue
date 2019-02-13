<template>
  <div id="EvolutionPanel">
    <cubism v-for="(serie, index) in Series" :serie="'HARMONICS'" :key="serie" :index="index"></cubism>
  </div>
</template>

<script>
  import Cubism from 'Components/Cubism.vue';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    data() {
      return {Series: oZm.Series['HARMONICS'].Names};
      
    },
    components: { Cubism }
  }
</script>
