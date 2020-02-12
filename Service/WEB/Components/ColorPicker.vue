<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="form-control"><div class="colorbox" :style="{backgroundColor: value }"></div></span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown"><span class="caret"></span></button>
        <ul class="dropdown-menu pull-right" :style="{ columns: columns }">
          <li v-for="(color, idx) in colors" :class="{ active: (value == color)}" :key="idx"><a @click="$emit('input', color)"><div class="colorli" :style="{backgroundColor: color}"></div></a></li>
        </ul>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    props: { 'colors': Array, value: String, columns: Number },
    data() {
      return { };
    }
  }
</script>

<style scoped>
  .colorbox {
    width: 30px;
    height: 14px;
    border: 1px solid #999;
    margin-top: 2px;
  }
  ul {
    padding: 3px;
  }
  ul > li > a {
    padding: 3px 3px;
  }
  .colorli {
    width: 100%;
    height: 14px;
    border: 1px solid #999;
  }
</style>