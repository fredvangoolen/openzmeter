<template>
  <z-button-group v-if="value!=null">
    <button type="button" class="btn btn-default btn-xs" :class="{ active: value[name] }" v-for="name in options" :key="name" @click="Toogle(name, $event)">{{ name }}</button>
  </z-button-group>
</template>

<script>
  export default {
    props: { value: Object, options: Array, storage: String },
    data() {
      return { };
    },
    watch: {
      value: function(val) {
        if(this.storage != null) localStorage.setItem(this.storage, JSON.stringify(val));
      }
    },
    created: function() {
      var Conf = (this.storage != null) ? JSON.parse(localStorage.getItem(this.storage)) : {};
      if(Conf == undefined) Conf = {};
      for(var n in this.options) {
        if(Conf[this.options[n]] == undefined) Conf[this.options[n]] = false;
      }
      this.$emit('input', Conf);
    },
    methods: {
      Toogle: function(name, evt) {
        $(evt.target).blur();
        var Conf = Object.assign({}, this.value);
        Conf[name] = !Conf[name]
        this.$emit('input', Conf);
      }
    }
  }
</script>

<style scoped>
  .btn-default:active:hover,
  .btn-default.active:hover,
  .btn-default:active:focus,
  .btn-default.active:focus {
    background-color: #ADC5D9;
  }
</style>