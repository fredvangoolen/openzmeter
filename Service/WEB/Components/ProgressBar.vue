<template>
  <div class="progress" :title="text">
    <div class="progress-bar" role="progressbar" :style="{width:value + '%'}"></div>
  </div>
</template>
<script>
  export default {
    props: { value: Number, text: String },
    data: function() {
      return { };
    }
  }
</script>

<style scoped>
  .progress {
    position: relative;
    background-color: #FFF;
  }
</style>