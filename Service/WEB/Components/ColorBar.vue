<template>
  <div class="color-bar" :style="{ width: width }">
    <div class="left-val" v-html="Min"></div>
    <div class="right-val" v-html="Max"></div>
    <div class="units">{{ Prefix }}{{ units }}</div>
    <div class="bar" :style="{ width: width + 'px', height: height + 'px' }">
      <div v-for="(div, idx) in Divs" :key="idx" :style="{ background: div }"></div>
    </div>
  </div>
</template>

<script>
  import chroma from 'chroma-js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { width: Number, height: Number, range: Array, units: String },
    data() {
      return { Divs: [], Prefix: '', Min: '', Max: '' };
    },
    watch: {
      range: function() { this.CalcRange(); },
    },
    methods: {
      CalcRange: function() {
        this.Prefix = this.BestUnits(Math.max(Math.abs(this.range[0]), Math.abs(this.range[1])));
        this.Min = this.Round(oZm.ScaleToUnits(this.range[0], this.Prefix), 2, true);
        this.Max = this.Round(oZm.ScaleToUnits(this.range[1], this.Prefix), 2, true);
      }
    },
    created: function() {
      this.ColorScale = chroma.bezier(['#00429d', '#96ffea', '#ffffe0', '#ff005e', '#93003a']).scale();
      for(var i = 0; i < this.width; i++) this.Divs.push(this.ColorScale(i / this.width));
      this.CalcRange();
    }
  }
</script>

<style scoped>
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
    display: inline-block;
  }
</style>