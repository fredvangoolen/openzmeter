<template>
  <div :class="color">
    <div>
      <div>
        <span v-for="val in value" v-html="val"></span>
      </div>
      <div>{{ prefix }}{{ units }}</div>
    </div>
  </div>
</template>
 
<script>
  import * as oZm from '../JS/openZmeter.js'
  export default {
    props: ['color', 'units'],
    data: function() {
            return { value: ['Offline'], prefix: '' };
          },
    methods: {
      setValue: function(vals) {
        if(vals === null) {
          this.value = ['Offline'];
        } else {
          this.prefix = oZm.GetBestUnits(oZm.ArrayAbsMax(vals));
          vals = oZm.ScaleArrayToUnits(vals, this.prefix);
          this.value = [];
          for(var i = 0; i < vals.length; i++) this.value.push(oZm.Round2(vals[i]));
        }
      }
    }
  }
</script>
 
<style>
    .indicator-bar {
    background-image: linear-gradient(to bottom,#f3f3f3,#e2e2e2);
    text-align: center;
    font-size: 14px;
    color: #FFF;
    font-weight: 700;
    padding: 2px;
    border-bottom: 1px solid #c9c9c9;
    position: relative;
    z-index: 95;
    font-size: 18x;
    top: 0px;
  }
  @media(min-width:768px) {
    .indicator-bar {
      background-image: none;
      position: absolute;
      right: 48px;
      text-align: right;
      border: none;
    }
  }
  @media(min-width:1024px) {
    .indicator-bar {
      right: 2px;
    }
  } 
@media(min-width:768px) {
  .indicator-bar > div > div > div:first-child {
    font-size: 12px;
  }
}
.indicator-bar > div {
  border-radius: 4px;
  display: inline-block;
  height: 50px;
}
.indicator-bar > div > div {
  display: table;
  height: 48px;
}
.indicator-bar > div > div > div {
  display: table-cell;
  vertical-align: middle;
  padding: 0 4px;
  text-align: center;
}
.indicator-bar > div > div > div:first-child {
  text-align: right;
  background-color: #fff;
  color: #333;
  border-radius: 4px;
  font-size: 11px;
  font-weight: normal;
  line-height: 10px;
  padding: 2px;
  min-width: 32px;
}
.indicator-bar > div > div > div > span {
  display: block;
}
</style>