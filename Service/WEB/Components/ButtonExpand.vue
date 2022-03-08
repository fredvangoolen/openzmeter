<template>
  <z-button @click="$emit('input', !value)" :tooltip="value ? 'Minimize graph' : 'Maximize graph'" :image="value ? require('Imgs/arrow_in.png') : require('Imgs/arrow_out.png')" />
</template>

<script>
  export default {
    props: { value: Boolean },
    data() {
      return { };
    }
  }
</script>