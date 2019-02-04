<template>
  <div :class="color">
    <div>
      <div>
        <span class="value" v-for="val in value" v-html="val"></span>
      </div>
      <div>{{ prefix }}{{ units }}</div>
    </div>
  </div>
</template>
 
<script>
module.exports = {
  props: ['color', 'units'],
  data: function() {
          return {
            value: ['Offline'],
            prefix: ''
          }
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

</style>