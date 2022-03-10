<template>
  <button :class="Class" :title="tooltip" :disabled="disabled" @click="onClick">
    <img v-if="image!=null" :src="image">
    {{ caption }}
    <slot />
  </button>
</template>

<script>
export default {
  props: {
    tooltip: { type: String, default: '' },
    size: { type: String, default: 'xs' },
    caption: { type: String, default: '' },
    image: { type: String, default: null },
    disabled: { type: Boolean, default: false },
    variant: { type: String, default: 'default' },
    active: { type: Boolean, default: false }
  },
  data: function() {
    return { };
  },
  computed: {
    Class: function() {
      let Classes = ['btn'];
      Classes.push('btn-' + this.size);
      Classes.push('btn-' + this.variant);
      if(this.active) Classes.push('active');
      return Classes;
    }
  },
  methods: {
    onClick: function(evt) {
      if(this.disabled && evt instanceof Event) {
        evt.preventDefault();
        evt.stopPropagation();
      } else {
        this.$emit('click');
      }
    }
  }
}
</script>