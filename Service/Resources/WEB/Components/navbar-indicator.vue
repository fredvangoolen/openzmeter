<template>
  <div :class="color">
    <div>
      <div v-html="value"></div>
      <div>{{ prefix }}{{ units }}</div>
    </div>
  </div>
</template>
 
<script>
module.exports = {
  props: ['color', 'units'],
  data: function() {
          return {
            value: '<span class="value">Offline</span>',
            prefix: ''
          }
        },
  methods: {
    setValue: function(vals) {
      if(vals === null) {
        value = '<span class="value">Offline</span>';
      } else {
        this.prefix = GetBestUnits(ArrayMax(vals));
        vals = ScaleArrayToUnits(vals, this.prefix);
        var Contents = '';
        for(var i = 0; i < vals.length; i++) Contents += '<span class="value">' + Round2(vals[i]) + '</span>';
        this.value = Contents;
      }
    }
  }
}
</script>
 
<style>

</style>