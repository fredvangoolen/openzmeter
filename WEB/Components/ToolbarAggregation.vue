<template>
  <div class="btn-group toolbar-aggregation">
    <div class="input-group input-group-xs">
      <span class="input-group-addon"><img src="Imgs/clock_15.png"/></span>
      <div class="input-group-btn">
        <button class="btn btn-default dropdown-toggle" type="button" data-toggle="dropdown"><span>{{ SelectedText }}</span><span class="caret"></span></button>
        <ul class="dropdown-menu pull-right">
          <li><a href="#">3 seconds</a></li>
          <li><a href="#">1 minute</a></li>
          <li><a href="#">10 minutes</a></li>
          <li><a href="#">1 hour</a></li>
        </ul>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    data() {
      return { SelectedText: '1 hour' };
    }
  }
</script>

<style>
  .toolbar-aggregation button span:first-child {
    display: inline-block; 
    width: 56px; 
    text-align: left;
  }
</style>
