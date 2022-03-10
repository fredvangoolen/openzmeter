<template>
  <div class="panel" :style="Border">
    <div class="panel-heading main" :style="Color">
      <div class="left title">{{ title }}</div>
      <slot name="toolbar" />
    </div>
    <div class="panel-heading sub" v-if="$slots.subleft||$slots.subright">
      <div class="left"><slot name="subleft" /></div>
      <slot name="subright" />
    </div>
    <div class="panel-body">
      <z-row>
        <slot />
      </z-row>
    </div>
  </div>
</template>

<script>
export default {
  props: {
    title: { type: String, default: ''},
    color: { type: String, default: ''}
  },
  computed: {
    Border: function() {
      return {
        'border-color': this.color,
        'border-width': '1px',
        'border-style': 'solid'
      };
    },
    Color: function() {
      return {
        'background-color': this.color
      };
    }
  },
  data: function() {
    return { };
  }
}
</script>

<style scoped>
  .main {
    color: white;
  }
  .sub {
    background: #eee;
    border-bottom: #ddd solid 1px;
  }
  .panel-heading {
    display: flex;
    height: 32px;
  }
  .left {
    flex-grow: 1;
  }
  .title {
    font-size: 20px;
    line-height: 20px;
  }
</style>