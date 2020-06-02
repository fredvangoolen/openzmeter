<template>
  <button type="button" class="btn btn-default btn-xs" @click="$emit('input', !value)" data-toggle="tooltip" :title="value?'Minimize graph' : 'Maximize graph'">
    <img v-if="!value" :src="require('Imgs/arrow_out.png')">
    <img v-if="value" :src="require('Imgs/arrow_in.png')">
  </button>
</template>

<script>
  export default {
    props: { value: Boolean },
    data() {
      return { };
    }
  }
</script>

<style scoped>

</style>
