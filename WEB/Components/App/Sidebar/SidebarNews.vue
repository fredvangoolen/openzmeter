<template>
  <div class="sidenav-version"><span>{{ BuildDate }}</span>
    <button type="button" class="btn btn-default btn-xs pull-right"><img src="Imgs/newspaper.png"></button>
  </div>
</template>

<script>
  export default {
    data() {
      return { BuildDate: 'TestVersion'};
    }
  }
</script>

<style>
  .sidenav-version {
    position: absolute;
    bottom: 0px;
    color: white;
    width: 100%;
    padding: 10px;
    font-size: 12px;
  }
  .sidenav-version > div {
    width: 210px;
    line-height: 26px;
    font-size: 14px;
    display: inline-block;
  }
  .sidenav-version button {
    padding: 4px;
    background-color: transparent;
    border: 1px solid #333;
  }
  @media(min-width:1024px) {
    .sidenav-version  {
      bottom: 56px;
    }
  }
  .sidenav-version > span {
    vertical-align: middle;
  }
</style>
