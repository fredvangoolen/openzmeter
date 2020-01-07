<template>
  <div class="color-bar" v-bind:style="{ width: width }">
    <div class="left-val" v-html="Min"></div>
    <div class="right-val" v-html="Max"></div>
    <div class="units">{{ Prefix }}{{ units }}</div>
    <div class="bar" v-bind:style="{ width: width + 'px', height: height + 'px' }">
      <div v-for="(div, idx) in Divs" :key="idx" v-bind:style="{ background: div }"></div>
    </div>
  </div>
</template>

<script>
  import chroma from 'chroma-js';
  import * as oZm from 'JS/openZmeter.js';
  export default {
    props: { width: Number, height: Number, range: { type: Array, required: true }, units: { type: String, required: true } },
    data() {
      return { Divs: [], Prefix: '', Min: '', Max: '' };
    },
    watch: {
      range: function() { this.CalcRange(); },
    },
    methods: {
      CalcRange: function() {
        this.Prefix = oZm.GetBestUnits(Math.max(Math.abs(this.range[0]), Math.abs(this.range[1])));
        this.Min = oZm.RoundHTML(oZm.ScaleToUnits(this.range[0], this.Prefix), 2);
        this.Max = oZm.RoundHTML(oZm.ScaleToUnits(this.range[1], this.Prefix), 2);
      }
    },
    created: function() {
      this.ColorScale = chroma.bezier(['#00429d', '#96ffea', '#ffffe0', '#ff005e', '#93003a']).scale();
      for(var i = 0; i < this.width; i++) this.Divs.push(this.ColorScale(i / this.width));
      this.CalcRange();
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
    display: inline-block;
  }
</style>