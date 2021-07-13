<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="input-group-addon"><img :src="Icon"></span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown"><span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li v-for="option in options" :key="option"><a @click="$emit('add', option)">{{ option }}</a></li>
        </ul>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    props: { options: Array, icon: String },
    data() {
      return { Icon: require('Imgs/plus.png') };
    },
    created: function() {
      if(this.icon != null) this.Icon = this.icon;
    }
  }
</script>

<style scoped>
  .btn-group ul {
    max-height: 142px;
    overflow-y: auto;
  }
</style>