<template>
  <div class="progress">
    <div class="progress-bar" role="progressbar" v-bind:style="{width:value + '%'}"></div>
  </div>
</template>
<script>
  export default {
      data() {
        return { value: 10 };
    }
  }
</script>
<style>
</style>