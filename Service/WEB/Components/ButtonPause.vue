<template>
  <button type="button" class="btn btn-default btn-xs" v-if="(value == true)" @click="$emit('input', false)" title="Resume live update"><img :src="require('Imgs/play.png')"></button>
  <button type="button" class="btn btn-default btn-xs" v-else @click="$emit('input', true)" title="Pause"><img :src="require('Imgs/pause.png')"></button>
</template>

<script>
  export default {
    props: { value: Boolean },
    data() {
      return { };
    }
  }
</script>