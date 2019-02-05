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
module.exports = {
  props: ['color', 'units'],
  data: function() {
          return { value: ['Offline'], prefix: '' };
        },
  methods: {
    setValue: function(vals) {
      if(vals === null) {
        value = ['Offline'];
      } else {
        this.prefix = GetBestUnits(ArrayMax(vals));
        vals = ScaleArrayToUnits(vals, this.prefix);
        this.value = [];
        for(var i = 0; i < vals.length; i++) this.value.push(Round2(vals[i]));
      }
    }
  }
}
</script>
 
<style>
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