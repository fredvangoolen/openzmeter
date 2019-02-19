<template>
  <div class="color-bar" v-bind:style="{ width: width }">
    <div class="left-val" v-html="Min"></div>
    <div class="right-val" v-html="Max"></div>
    <div class="units">{{ Units }}</div>
    <div class="bar" v-bind:style="{ width: width, height: height }"></div>
  </div>  
</template>

<script>
  export default {
    props: ['width', 'height'],
    data() {
      return { Units: 'kWh', Min: '1', Max: '100'};
    },
    methods: {
      SetRange(min, max, units) {
        this.Units = units;
        this.Min = min;
        this.Max = max;
        //      var ColorScale = chroma.scale(['#000000', '#0000FF', '#FF0000', '#FFFF00']).domain([0, 1]);
//      ShowColorScale(ColorScale, $('#CalendarScale'));

        
        //function ShowColorScale(scale, outerdiv) {
//  scale.out('hex');
//  var width = outerdiv.width() - 1;
//  for(i = 0; i <= width; i++) outerdiv.append('<div style="width:1px; height:' + $(outerdiv).height() + 'px; display:inline; background:' + scale(i / width) + '"></div>');
//}

        
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
</style>