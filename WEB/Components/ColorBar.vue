<template>
  <div class="color-bar" v-bind:style="{ width: width }">
    <div class="left-val">{{ Min }}</div>
    <div class="right-val">{{ Max }}</div>
    <div class="units">{{ Units }}</div>
    <div class="bar" v-bind:style="{ width: width, height: height }"></div>
  </div>  
</template>

<script>
  export default {
      props: ['width', 'height'],
      data() {
        return { Units: 'kWh', Min: '1', Max: '100'};
      }
  }
</script>

<style>
  .color-bar {
    display: inline-block;
    vertical-align: text-bottom;
    font-size: 12px;
    line-height: 12px;
  }
  .color-bar .left-val {
    float: left;
  }
  .color-bar .right-val {
    float:right;
  }
  .color-bar .units {
    text-align: center;
    font-style: italic;
  }
  .color-bar .bar {
     display: flex;
     flex-direction: row;
     border: 1px solid #EEE;
  }
</style>