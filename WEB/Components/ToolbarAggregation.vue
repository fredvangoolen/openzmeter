<template>
  <div class="btn-group">
    <div class="input-group input-group-xs">
      <span class="input-group-addon"><img src="Imgs/clock_15.png"></span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown">{{ Selected }}<span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li v-if="options.includes('3S')"><a href="#">3 seconds</a></li>
          <li v-if="options.includes('1M')"><a href="#">1 minute</a></li>
          <li v-if="options.includes('10M')"><a href="#">10 minutes</a></li>
          <li v-if="options.includes('15M')"><a href="#">15 minutes</a></li>
          <li v-if="options.includes('1H')"><a href="#">1 hour</a></li>
        </ul>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    props: { options: Array },
    data() {
      return { Selected: '1 hour' };
    }
  }
</script>