<template>
  <div class="color-bar" v-bind:style="{ width: width }">
    <div class="left-val" v-html="Min"></div>
    <div class="right-val" v-html="Max"></div>
    <div class="units">{{ Units }}</div>
    <div class="bar" v-bind:style="{ width: width + 'px', height: height + 'px' }">
      <div v-for="div in Divs" v-bind:style="{ background: div }"></div>
    </div>
  </div>  
</template>

<script>
  import chroma from 'JS/chroma.js';
  export default {
    props: {width: Number, height: Number},
    data() {
      return { Units: 'kWh', Min: '1', Max: '100', ColorScale: chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]), Divs: [] };
    },
    mounted: function() {
      var Divs = [];
      for(var i = 0; i < this.width; i++) Divs.push(this.ColorScale(i / this.width));
      this.Divs = Divs;      
    },
    methods: {
      SetRange(min, max, units) {
        this.Units = units;
        this.Min = min;
        this.Max = max;
      }
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
  .color-bar .bar div {
    width:1px;
    height: 100%;
    display: inline;
  }
</style>