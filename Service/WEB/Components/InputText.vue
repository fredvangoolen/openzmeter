<template>
  <div class="input-group input-group-xs">
    <span v-if="prepend" class="input-group-addon">{{ prepend }}</span>
    <input type="text" class="form-control" :class="{ rounded: prepend==null }" :disabled="disabled" :value="value" @change="$emit('input', $event.target.value)">
  </div>
</template>

<script>
  export default {
    props: { prepend: String, value: String, disabled: Boolean },
    data: function() {
      return { };
    },
    methods: {
      Change: function(val) {
        var Value = parseFloat(eval(val));
        if(isNaN(Value) === true) Value = this.min;
        if(Value > this.max) Value = this.max;
        if(Value < this.min) Value = this.min;
        this.$emit('change', Value);
        this.$emit('input', Value);
      }
    }
  }
</script>

<style scoped>
  .rounded {
    border-radius: 4px !important;
  }
</style>