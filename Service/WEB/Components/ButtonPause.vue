<template>
  <z-button v-if="(value == true)" @click="$emit('input', !value)" :tooltip="value ? 'Resume live update' : 'Pause'" :image="value ? require('Imgs/play.png') : require('Imgs/pause.png')" />
</template>

<script>
  export default {
    props: { value: Boolean },
    data() {
      return { };
    }
  }
</script>